#!/usr/bin/env python3
"""Remove an arbitrary pointer-run cap; retain semantic vtable uncertainty."""
import argparse
import bisect
import collections
import csv
import gzip
import hashlib
import io
import json
import struct
from pathlib import Path

import capstone
from canonical_image import CANONICAL, CanonicalImage, read_tsv

LEA_SITES = {
    0x1404c6a18: [0x140062a2d,0x140062a66,0x140062aba],
    0x1404c75e0: [0x14006e58d,0x14006e5c6,0x14006e61a],
    0x1404c7ea0: [0x14007991d,0x140079956,0x1400799aa],
}


def run(exe,repo,out):
    image=CanonicalImage(exe)
    anchors=read_tsv(repo/'data/reverse/runtime-tree-20260913/vtable_anchors.tsv')
    anchors.sort(key=lambda r:int(r['vftable_va'],16))
    visited={int(r['instruction_va'],16):int(r['size']) for r in read_tsv(repo/'data/reverse/root-expansion-20260915/visited.tsv.gz')}
    starts=sorted(visited)
    def coverage(va):
        if va in visited:return 'visited_start'
        i=bisect.bisect_right(starts,va)-1
        return 'inside_visited_instruction' if i>=0 and va<starts[i]+visited[starts[i]] else 'not_visited'
    slots=[];runs=[];vptr=[];thunks=[]
    for index,r in enumerate(anchors):
        anchor=int(r['vftable_va'],16);col=int(r['locator_va'],16)
        assert image.u64(anchor-8)==col
        signature,offset,cd,typ,chd,self_rva=struct.unpack('<IIIIII',image.raw(col,24))
        assert signature==1 and image.base+self_rva==col and image.base+typ==int(r['type_va'],16)
        assert offset==int(r['subobject_offset'])
        limit=int(anchors[index+1]['vftable_va'],16)-8 if index+1<len(anchors) else anchor+32768
        count=0;stop=None
        while count<4096:
            slot=anchor+count*8
            if slot>=limit:
                stop='next_validated_locator_slot';break
            try:target=image.u64(slot)
            except ValueError:
                stop='unbacked';break
            if not image.executable(target):
                stop='noncode_pointer';break
            slots.append((hex(anchor),count,hex(slot),hex(target),coverage(target),
                          'STRUCTURAL_CONFIRMED' if count==0 else 'SEMANTIC_CANDIDATE'))
            count+=1
        if stop is None:raise ValueError('new safety cap reached; no closure claim')
        runs.append(dict(anchor=hex(anchor),class_name=r['class'],count=count,stop=stop,
                         end_exclusive=hex(anchor+count*8),semantic_extent_confirmed=False))
        if anchor not in LEA_SITES:continue
        assert stop=='next_validated_locator_slot'
        nxt=anchors[index+1]
        assert nxt['class']==r['class'] and int(nxt['subobject_offset'])==208
        for site in LEA_SITES[anchor]:
            i=image.instruction(site)
            assert i.mnemonic=='lea' and i.operands[1].mem.base==capstone.x86.X86_REG_RIP
            assert site+i.size+i.operands[1].mem.disp==anchor
            reg=i.operands[0].reg;cursor=site+i.size
            for _ in range(3):
                s=image.instruction(cursor);ops=s.operands
                if s.mnemonic=='mov' and ops[0].type==capstone.x86.X86_OP_MEM and ops[1].type==capstone.x86.X86_OP_REG and ops[1].reg==reg:
                    assert ops[0].size==8 and ops[0].mem.disp==0 and not ops[0].mem.index
                    vptr.append(dict(anchor=hex(anchor),load_site=hex(site),store_site=hex(cursor),
                        receiver_register=s.reg_name(ops[0].mem.base),receiver_displacement=0,
                        range_begin=hex(image.owner(site)[0]),status='EXE_CONFIRMED'))
                    break
                assert reg not in s.regs_access()[1] and not any(s.group(g) for g in (capstone.CS_GRP_CALL,capstone.CS_GRP_JUMP,capstone.CS_GRP_RET))
                cursor+=s.size
            else:raise ValueError('vptr store not established')
        thunk=int(nxt['first_entry_va'],16);i=image.instruction(thunk);j=image.instruction(thunk+i.size)
        assert i.mnemonic=='sub' and i.operands[0].reg==capstone.x86.X86_REG_RCX and i.operands[1].imm==208
        assert j.mnemonic=='jmp' and j.operands[0].imm==int(r['first_entry_va'],16)
        thunks.append(dict(class_name=r['class'],secondary_anchor=nxt['vftable_va'],subobject_offset=208,
            thunk=hex(thunk),adjustment=-208,target=r['first_entry_va'],status='EXE_CONFIRMED'))
    old=read_tsv(repo/'data/reverse/root-expansion-audit-20260915/vtable-slot-candidates.tsv.gz')
    oldslots={int(r['slot_va'],16):int(r['target_va'],16) for r in old}
    actual={int(s[2],16):int(s[3],16) for s in slots}
    assert all(actual[k]==v for k,v in oldslots.items())
    added=[s for s in slots if int(s[2],16) not in oldslots]
    summary=dict(schema='dmc3-vtable-pointer-boundaries-v1',sha256=CANONICAL,
        anchors=len(anchors),pointer_slot_candidates=len(slots),additional_slots=len(added),
        unique_targets=len({s[3] for s in slots}),
        unique_unvisited_targets=len({s[3] for s in slots if s[4]=='not_visited'}),
        stops=dict(collections.Counter(r['stop'] for r in runs)),
        formerly_capped_runs=[r for r in runs if int(r['anchor'],16) in LEA_SITES],
        vptr_stores_verified=len(vptr),secondary_this_adjustors_verified=len(thunks),
        input_sha256={str(p.relative_to(repo)):hashlib.sha256(p.read_bytes()).hexdigest() for p in
            (repo/'data/reverse/runtime-tree-20260913/vtable_anchors.tsv',
             repo/'data/reverse/root-expansion-20260915/visited.tsv.gz',
             repo/'data/reverse/root-expansion-audit-20260915/vtable-slot-candidates.tsv.gz')},
        limitations=['A contiguous code-pointer run bounded by RTTI is not proof that every slot is a virtual method.',
          'Typed receiver/slot consumers, exact method signatures and inherited slot mappings remain open.',
          'Unvisited candidates remain excluded from confirmed metadata-root seeding.'])
    out.mkdir(parents=True,exist_ok=True)
    s=io.StringIO();w=csv.writer(s,delimiter='\t',lineterminator='\n')
    w.writerow(['anchor_va','slot_index','slot_va','target_va','coverage','status']);w.writerows(slots)
    (out/'pointer-slots.tsv.gz').write_bytes(gzip.compress(s.getvalue().encode(),mtime=0))
    for name,data in [('summary.json',summary),('pointer-runs.json',runs),('additional-slots.json',added),
                      ('vptr-stores.json',vptr),('secondary-adjustors.json',thunks)]:
        (out/name).write_text(json.dumps(data,indent=2)+'\n')
    return summary


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo,a.out),indent=2))
