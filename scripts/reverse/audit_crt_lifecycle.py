#!/usr/bin/env python3
"""Inspect transfer-bearing CRT bodies and exit callbacks, with conservative arguments.

Calls are not recursively followed. Constants are propagated only within a basic
block, with Win64 volatile registers invalidated at calls; blank means unknown.
No object ownership or complete constructor/destructor semantics are inferred.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path

import capstone
from canonical_image import CANONICAL, CanonicalImage, read_tsv

X = capstone.x86
REGISTER_EXIT = 0x1403458d0
VECTOR_CONSTRUCT = 0x140345b24


def family(name):
    for root, aliases in [('rax', ('rax','eax','ax','al','ah')), ('rbx', ('rbx','ebx','bx','bl','bh')),
                          ('rcx', ('rcx','ecx','cx','cl','ch')), ('rdx', ('rdx','edx','dx','dl','dh')),
                          ('rsi', ('rsi','esi','si','sil')), ('rdi', ('rdi','edi','di','dil')),
                          ('rbp', ('rbp','ebp','bp','bpl')), ('rsp', ('rsp','esp','sp','spl'))]:
        if name in aliases:
            return root
    for i in range(8,16):
        if name in (f'r{i}', f'r{i}d', f'r{i}w', f'r{i}b'):
            return f'r{i}'
    return name


def body(image, root, crt_roots):
    owner = image.owner(root)
    next_root = min((v for v in crt_roots if v > root), default=root + 4096)
    end = owner[1] if owner else min(next_root, root + 4096)
    todo = [root]
    instructions = {}
    successors = {}
    terminals = []
    while todo:
        va = todo.pop()
        if va in instructions:
            continue
        if len(instructions) >= 2048 or not root <= va < end:
            raise ValueError(f'body boundary/budget at {root:#x}: {va:#x}')
        ins = image.instruction(va)
        instructions[va] = ins
        nxt = va + ins.size
        target = ins.operands[0].imm if ins.operands and ins.operands[0].type == X.X86_OP_IMM else None
        if ins.group(capstone.CS_GRP_RET) or ins.mnemonic in ('int3','ud2','hlt'):
            dest = []
            terminals.append((va, ins.mnemonic, None))
        elif ins.group(capstone.CS_GRP_JUMP):
            dest = [target] if target is not None and root <= target < end else []
            if not dest:
                terminals.append((va, 'external_jump', target))
            if ins.mnemonic != 'jmp':
                dest.append(nxt)
        else:
            dest = [nxt]
        successors[va] = dest
        todo.extend(dest)
    return instructions, successors, terminals, end


def inspect(image, root, crt_roots):
    insns, succ, terminals, end = body(image, root, crt_roots)
    leaders = {root}
    for va, ins in insns.items():
        if ins.group(capstone.CS_GRP_JUMP):
            leaders.update(succ[va])
            leaders.add(va + ins.size)
    regs = {}
    stack5 = None
    transfers = []
    writes = []
    previous = None
    for va, ins in sorted(insns.items()):
        if va in leaders or previous != va:
            regs = {}
            stack5 = None
        previous = va + ins.size
        old = dict(regs)
        def regval(reg):
            name = ins.reg_name(reg)
            v = old.get(family(name))
            if v is None:
                return None
            if name in ('ah','bh','ch','dh'):
                return (v >> 8) & 255
            return v
        def address(mem):
            if mem.segment:
                return None
            base = va + ins.size if mem.base == X.X86_REG_RIP else (regval(mem.base) if mem.base else 0)
            index = regval(mem.index) if mem.index else 0
            return None if base is None or index is None else base + index * mem.scale + mem.disp
        def value(op):
            if op.type == X.X86_OP_IMM:
                return op.imm
            if op.type == X.X86_OP_REG:
                v = regval(op.reg)
                return None if v is None else v & ((1 << (8 * op.size)) - 1)
            return None
        ops = ins.operands
        if ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_JUMP):
            target = ops[0].imm if ops and ops[0].type == X.X86_OP_IMM else None
            symbol = None
            if target is not None and image.executable(target):
                imp = image.import_thunk(target)
                symbol = imp[1] if imp else None
            elif ops and ops[0].type == X.X86_OP_MEM:
                symbol = image.imports.get(address(ops[0].mem))
            transfers.append(dict(site=hex(va), kind=ins.mnemonic,
                target=hex(target) if target is not None else None, import_symbol=symbol,
                local_arguments={r:hex(old[r]) if r in old else None for r in ('rcx','rdx','r8','r9')},
                local_stack_argument5=hex(stack5) if stack5 is not None and target==VECTOR_CONSTRUCT else None,
                conditional_on_site_reached=True))
        result = None
        if len(ops) == 2:
            if ins.mnemonic in ('mov','movabs','movzx'):
                result = value(ops[1])
            elif ins.mnemonic == 'lea' and ops[1].type == X.X86_OP_MEM:
                result = address(ops[1].mem)
            elif ins.mnemonic in ('xor','sub') and ops[0].type == ops[1].type == X.X86_OP_REG and ops[0].reg == ops[1].reg:
                result = 0
            elif ins.mnemonic in ('add','sub','or','and','xor'):
                a, b = value(ops[0]), value(ops[1])
                if a is not None and b is not None:
                    result = {'add':lambda:a+b,'sub':lambda:a-b,'or':lambda:a|b,'and':lambda:a&b,'xor':lambda:a^b}[ins.mnemonic]()
        if ops and ops[0].type == X.X86_OP_MEM and ops[0].access & capstone.CS_AC_WRITE:
            dest = address(ops[0].mem)
            if dest is not None:
                writes.append(dict(site=hex(va), destination=hex(dest), width=ops[0].size,
                    constant=hex(result & ((1 << (8*ops[0].size))-1)) if result is not None else None,
                    operation=ins.mnemonic))
            if ops[0].mem.base == X.X86_REG_RSP and not ops[0].mem.index and ops[0].mem.disp < 0x28 and ops[0].mem.disp + ops[0].size > 0x20:
                stack5 = (result & 0xffffffffffffffff) if ops[0].mem.disp == 0x20 and ops[0].size == 8 and result is not None else None
        _, changed = ins.regs_access()
        for reg in changed:
            key = family(ins.reg_name(reg))
            regs.pop(key, None)
            if key == 'rsp':
                stack5 = None
        if ops and ops[0].type == X.X86_OP_REG and ops[0].size in (4,8) and result is not None:
            regs[family(ins.reg_name(ops[0].reg))] = result & ((1 << (8 * ops[0].size)) - 1)
        if ins.group(capstone.CS_GRP_CALL):
            for name in ('rax','rcx','rdx','r8','r9','r10','r11'):
                regs.pop(name, None)
            stack5 = None
    return dict(root=hex(root), instruction_count=len(insns), boundary_end=hex(end),
        boundary_kind='unwind_range_end' if image.owner(root) else 'bounded_search_envelope_not_function_extent',
        body_sha256=hashlib.sha256(b''.join(bytes(i.bytes) for _,i in sorted(insns.items()))).hexdigest(),
        instruction_addresses=[hex(a) for a in sorted(insns)], transfers=transfers,
        explicit_known_address_writes=writes,
        terminals=[dict(site=hex(a),kind=k,target=hex(t) if t is not None else None) for a,k,t in terminals])


def run(exe, repo, out):
    image = CanonicalImage(exe)
    folder = repo/'data/reverse/root-expansion-20260915'
    roots = read_tsv(folder/'metadata-roots.tsv.gz')
    crts = {int(r['target_va'],16) for r in roots if r['kind'].startswith('crt')}
    for r in roots:
        if r['kind'].startswith('crt'):
            assert image.u64(int(r['source_slot_va'],16))==int(r['target_va'],16)
    actual_crts={image.u64(slot) for lo,hi in ((0x14034f808,0x14035d250),(0x14035d258,0x14035d278)) for slot in range(lo,hi,8)}-{0}
    assert crts==actual_crts
    prefixes = read_tsv(folder/'crt-prefixes.tsv.gz')
    selected = sorted(int(r['target_va'],16) for r in prefixes if r['prefix_stop'] != 'ret')
    assert len(selected) == 83 and len(crts) == 6987
    # Prove registration wrapper's two import paths from the canonical bytes.
    for site, target in ((0x1403458d4,0x140345880),(0x1403458a8,0x140346c86),(0x1403458b9,0x140346c80)):
        i = image.instruction(site)
        assert i.mnemonic == 'call' and i.operands[0].imm == target
    assert image.import_thunk(0x140346c86)[1].endswith('!_crt_atexit')
    assert image.import_thunk(0x140346c80)[1].endswith('!_register_onexit_function')
    bodies = [inspect(image, a, crts) for a in selected]
    callbacks = []
    for b in bodies:
        for t in b['transfers']:
            if t['target'] == hex(REGISTER_EXIT):
                target = t['local_arguments']['rcx']
                callbacks.append(dict(initializer=b['root'], registration_site=t['site'], callback=target,
                                      status='EXE_CONFIRMED' if target else 'UNRESOLVED'))
    known = sorted({int(r['callback'],16) for r in callbacks if r['callback']})
    cleanup = [inspect(image, a, crts) for a in known]
    vector = [dict(initializer=b['root'], **t) for b in bodies for t in b['transfers'] if t['target']==hex(VECTOR_CONSTRUCT)]
    by_root = {b['root']:b for b in bodies + cleanup}
    anchors = {r['vftable_va']:r for r in read_tsv(repo/'data/reverse/runtime-tree-20260913/vtable_anchors.tsv')}
    def require_edge(root,site,target,rcx):
        t=next(t for t in by_root[hex(root)]['transfers'] if t['site']==hex(site))
        assert t['target']==hex(target) and t['local_arguments']['rcx']==hex(rcx)
    require_edge(0x140025050,0x14002505b,0x140314c50,0x140d5b860)
    require_edge(0x14034eca0,0x14034eca7,0x140314d00,0x140d5b860)
    for site,anchor in ((0x140314c66,0x140507b60),(0x140314d12,0x140507b60),(0x140314d38,0x140507b30)):
        i=image.instruction(site);s=image.instruction(site+i.size)
        assert i.mnemonic=='lea' and i.operands[1].mem.base==X.X86_REG_RIP
        assert i.address+i.size+i.operands[1].mem.disp==anchor
        assert s.mnemonic=='mov' and s.operands[0].type==X.X86_OP_MEM and s.operands[0].size==8
        assert s.operands[1].type==X.X86_OP_REG and s.operands[1].reg==i.operands[0].reg
    assert anchors['0x140507b60']['class']=='CPtxManager'
    assert anchors['0x140507b30']['class']=='IPtxManager'
    for site,reg,value in ((0x140314c87,X.X86_REG_EDX,0x218),(0x140314c8c,X.X86_REG_R8D,32),
                           (0x140314d27,X.X86_REG_EDX,0x218),(0x140314d2c,X.X86_REG_R8D,32)):
        i=image.instruction(site)
        assert i.mnemonic=='mov' and i.operands[0].reg==reg and i.operands[1].imm==value
    for site,target in ((0x140314c92,0x140345b24),(0x140314d32,0x140345b94)):
        i=image.instruction(site)
        assert i.mnemonic=='call' and i.operands[0].imm==target
    require_edge(0x140023e10,0x140023e50,0x140329200,0x140cf3598)
    require_edge(0x140023e10,0x140023e6e,0x140329200,0x140cf35c0)
    require_edge(0x14034eb80,0x14034eb99,0x140337440,0x140cf35c0)
    require_edge(0x14034eb80,0x14034eba9,0x140337440,0x140cf3598)
    assert anchors['0x140508728']['class']=='CMcAppli'
    links=[dict(class_name='CMcAppli',global_va='0x140cf3310',initializer='0x140023e10',
                registered_cleanup='0x14034eb80',vtable='0x140508728',
                initialized_subobjects=['0x140cf3598','0x140cf35c0'],
                cleanup_subobjects_in_order=['0x140cf35c0','0x140cf3598'],
                status='EXE_CONFIRMED',scope='call arguments and order, not full member semantics'),
           dict(class_name='CPtxManager',global_va='0x140d5b860',initializer='0x140025050',
                constructor='0x140314c50',registered_cleanup='0x14034eca0',destructor_target='0x140314d00',
                vtable='0x140507b60',base_vtable_after_cleanup='0x140507b30',
                element_stride=0x218,element_count=32,status='EXE_CONFIRMED',
                scope='global binding, vptr writes and array-helper arguments; full slot semantics open')]
    summary = dict(schema='dmc3-crt-lifecycle-v1', sha256=CANONICAL, status='STRUCTURAL_CONFIRMED',
        complex_crt_roots=len(bodies), exit_registration_sites=len(callbacks),
        resolved_callback_targets=len(known), unresolved_callback_sites=sum(r['callback'] is None for r in callbacks),
        callback_terminal_kinds=dict(collections.Counter(t['kind'] for b in cleanup for t in b['terminals'])),
        no_operation_callbacks=sum(b['instruction_count']==1 and b['terminals'][0]['kind'].startswith('ret') for b in cleanup),
        vector_construction_sites=len(vector),
        input_sha256={str(p.relative_to(repo)):hashlib.sha256(p.read_bytes()).hexdigest() for p in
            (folder/'metadata-roots.tsv.gz',folder/'crt-prefixes.tsv.gz',repo/'data/reverse/runtime-tree-20260913/vtable_anchors.tsv')},
        limitations=['Local constants stop at branch boundaries and calls invalidate Win64 volatile registers.',
          'Reported arguments are conditional on reaching the site; no path feasibility or initialization success is claimed.',
          'Callees, exception cleanup and object users are not recursively reconstructed.',
          'Registration does not imply a nontrivial destructor; no-op callbacks remain explicit.',
          'No full-game or full-lifecycle completion claim.'])
    out.mkdir(parents=True,exist_ok=True)
    for name, data in [('summary.json',summary),('crt-bodies.json',bodies),('exit-registrations.json',callbacks),
                       ('cleanup-bodies.json',cleanup),('vector-construction-sites.json',vector),
                       ('global-lifecycle-links.json',links)]:
        (out/name).write_text(json.dumps(data,indent=2)+'\n')
    return summary


if __name__ == '__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):
        p.add_argument(name,type=Path)
    a=p.parse_args()
    print(json.dumps(run(a.exe,a.repo,a.out),indent=2))
