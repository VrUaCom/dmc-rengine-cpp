#!/usr/bin/env python3
"""Conservative static CFG traversal, seeded by PE runtime entries and entrypoint.

Requires Capstone 5 and GNU objdump. Does not infer indirect jump targets or
claim runtime reachability. Emits address metadata, never executable bytes.
"""
import argparse
import bisect
import collections
import csv
import gzip
import hashlib
import io
import json
import pathlib
import re
import struct
import subprocess

import capstone

CANONICAL = 'e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'

# Canonical-only, manually reviewed bounds/load/add/jump consumers. Entries are
# (dispatch RVA, table RVA, entry count, unsigned bound check RVA, load RVA).
# The first table uses a separate 241-byte selector remap at RVA 0x2c85c.
SWITCHES = [
    (0x2c48d, 0x2c818, 17, 0x2c466, 0x2c482),
    (0xb3fbd, 0xb4fe8, 8, 0xb3fa9, 0xb3fb2),
    (0x2417d1, 0x24190c, 9, 0x2417af, 0x2417bf),
    (0x27cad1, 0x27d3c0, 11, 0x27cab5, 0x27cac7),
    (0x295615, 0x295a28, 13, 0x2955fd, 0x29560b),
    (0x2b0e19, 0x2b1104, 8, 0x2b0dff, 0x2b0e0f),
]


def run(exe, out):
    data = exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != CANONICAL:
        raise ValueError('canonical SHA-256 mismatch')
    def u(fmt, p):
        return struct.unpack_from('<'+fmt, data, p)
    pe, = u('I',0x3c)
    assert data[pe:pe+4] == b'PE\0\0'
    opt = pe+24
    assert u('H',pe+4)[0] == 0x8664 and u('H',opt)[0] == 0x20b
    base, = u('Q',opt+24)
    count, = u('H',pe+6)
    optsize, = u('H',pe+20)
    sections = []
    for i in range(count):
        p = opt+optsize+i*40
        vs, rva, size, raw = u('IIII',p+8)
        sections.append((rva, min(vs,size), raw, u('I',p+36)[0]))
    def offset(rva, n=1):
        for a,size,raw,flags in sections:
            if a <= rva and rva+n <= a+size:
                return raw+rva-a
        raise ValueError(f'RVA not backed: {rva:x}')
    def executable(va):
        return any(flags & 0x20000000 and base+a <= va < base+a+size
                   for a,size,raw,flags in sections)
    prva, psz = u('II',opt+112+3*8)
    assert psz % 12 == 0
    ranges = [u('III',offset(prva+i,12)) for i in range(0,psz,12)]
    assert ranges == sorted(ranges)
    starts = [base+a for a,b,w in ranges]
    def owner(va):
        i = bisect.bisect_right(starts,va)-1
        return starts[i] if i >= 0 and va < base+ranges[i][1] else None
    switch_targets = {}
    switch_evidence = []
    table_intervals = []
    for dispatch,table,n,bound,load in SWITCHES:
        targets = [base+u('I',offset(table+i*4,4))[0] for i in range(n)]
        assert all(executable(a) for a in targets)
        switch_targets[base+dispatch] = sorted(set(targets))
        table_intervals.append((base+table,base+table+4*n))
        switch_evidence.append(dict(status='EXE_CONFIRMED',dispatch_va=hex(base+dispatch),
            table_va=hex(base+table),entry_count=n,bound_check_va=hex(base+bound),
            load_va=hex(base+load),target_by_slot=[hex(a) for a in targets]))
    remap = list(data[offset(0x2c85c,241):offset(0x2c85c,241)+241])
    assert min(remap) == 0 and max(remap) == 16
    switch_evidence[0].update(selector_remap_va=hex(base+0x2c85c),
        selector_max=240,slot_by_selector=remap)
    table_intervals.append((base+0x2c85c,base+0x2c94d))
    original_seeds = set(starts) | {base+u('I',opt+16)[0]}
    excluded_seeds = {a for a in original_seeds if any(lo <= a < hi for lo,hi in table_intervals)}
    seeds = original_seeds-excluded_seeds
    pending = list(sorted(seeds,reverse=True))
    decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64)
    visited = {}
    edges = []
    failures = set()
    outside = set()
    def enqueue(va):
        if executable(va):
            if va not in visited:
                pending.append(va)
        else:
            outside.add(va)
    while pending:
        va = pending.pop()
        if va in visited or va in failures:
            continue
        if not executable(va):
            outside.add(va)
            continue
        section = next(s for s in sections if s[3]&0x20000000 and base+s[0] <= va < base+s[0]+s[1])
        end = base+section[0]+section[1]
        while va < end and va not in visited and va not in failures:
            # Decode a bounded block, then restart at the next exact address.
            p = offset(va-base)
            block_end = min(va+4096,end)
            made_progress = False
            terminal = False
            for addr,size,mnemonic,operands in decoder.disasm_lite(data[p:p+block_end-va],va):
                if addr in visited:
                    terminal = True
                    break
                made_progress = True
                visited[addr] = size
                op = mnemonic.split()[-1]  # Handles bnd/notrack/rep prefixes.
                next_va = addr+size
                is_jump = op.startswith('j') or op.startswith('loop')
                is_call = op in ('call','lcall')
                target = int(operands,16) if re.fullmatch(r'0x[0-9a-f]+',operands) else None
                if is_call or is_jump:
                    kind = 'call' if is_call else ('jump' if op in ('jmp','ljmp') else 'conditional')
                    if kind == 'jump' and addr in switch_targets:
                        assert target is None
                        for switch_target in switch_targets[addr]:
                            edges.append((addr,'switch',switch_target,None))
                            enqueue(switch_target)
                    else:
                        edges.append((addr,kind,target,next_va if is_call or kind=='conditional' else None))
                    if target is not None:
                        enqueue(target)
                    if kind == 'jump':
                        terminal = True
                        break
                if op.startswith(('ret','iret')) or op in ('int3','ud2','hlt','lret'):
                    terminal = True
                    break
                va = next_va
            if terminal:
                break
            if not made_progress:
                # Retry a full instruction at a window boundary before failing.
                failures.add(va)
                break
    linear = subprocess.check_output(['objdump','-d','-z','-w','-M','intel',str(exe)],text=True)
    old_linear = subprocess.check_output(['objdump','-d','-M','intel','--no-show-raw-insn',str(exe)],text=True)
    bad = []
    old_bad = []
    old_first_token_bad = []
    for line in old_linear.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\s+(.*)$',line)
        if m and '(bad)' in m[2]:
            old_bad.append(int(m[1],16))
            if m[2].split()[0] == '(bad)':
                old_first_token_bad.append(int(m[1],16))
    linear_map = {}
    for line in linear.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s+(.+)$',line)
        if not m:
            continue
        addr = int(m[1],16)
        linear_map[addr] = (len(m[2].split()),m[3])
        if '(bad)' in m[3]:
            bad.append(addr)
    addresses = sorted(visited)
    def coverage(va):
        i = bisect.bisect_right(addresses,va)-1
        if va in visited:
            return 'visited_start'
        if i >= 0 and va < addresses[i]+visited[addresses[i]]:
            return 'inside_visited_instruction'
        return 'not_visited'
    disagreements = []
    agrees = 0
    for va,size in sorted(visited.items()):
        other = linear_map.get(va)
        if other is not None and other[0] == size and '(bad)' not in other[1]:
            agrees += 1
        else:
            disagreements.append((va,size,other[0] if other else None,'missing_linear_start' if other is None else 'decode_mismatch'))
    overlaps = [(a,a+visited[a],b) for a,b in zip(addresses,addresses[1:]) if a+visited[a] > b]
    reanchored = []
    for va,size,other,reason in disagreements:
        listing = subprocess.check_output(['objdump','-d','-z','-w','-M','intel',
            '--start-address='+hex(va),'--stop-address='+hex(va+16),str(exe)],text=True)
        m = re.search(r'^\s*'+f'{va:x}'+r':\s+((?:[0-9a-f]{2} )+)\s+(.+)$',listing,re.M)
        length = len(m[1].split()) if m else None
        reanchored.append((va,size,length,bool(m and '(bad)' not in m[2] and length==size)))
    out.mkdir(parents=True,exist_ok=True)
    def hx(v):
        return '' if v is None else f'0x{v:X}'
    def table(name,header,rows,compressed=False):
        buf = io.StringIO()
        writer = csv.writer(buf,delimiter='\t',lineterminator='\n')
        writer.writerow(header)
        writer.writerows(rows)
        payload = buf.getvalue().encode()
        (out/name).write_bytes(gzip.compress(payload,mtime=0) if compressed else payload)
    table('edges.tsv.gz',['site_va','kind','target_va','assumed_fallthrough_va'],
          [(hx(a),b,hx(c),hx(d)) for a,b,c,d in sorted(edges)],True)
    table('visited.tsv.gz',['instruction_va','size','range_begin_va'],
          [(hx(a),visited[a],hx(owner(a))) for a in addresses],True)
    table('bad-decode-disposition.tsv',['objdump_bad_va','range_begin_va','traversal_disposition'],
          [(hx(a),hx(owner(a)),coverage(a)) for a in bad])
    table('decoder-disagreements.tsv',['instruction_va','capstone_size','linear_size','reason'],
          [(hx(a),b,c,d) for a,b,c,d in disagreements])
    table('unresolved.tsv',['kind','site_or_target_va'],
          [('decode_failure',hx(a)) for a in sorted(failures)]+
          [('outside_executable_target',hx(a)) for a in sorted(outside)])
    table('overlaps.tsv',['instruction_va','end_va','overlapping_start_va'],
          [(hx(a),hx(b),hx(c)) for a,b,c in overlaps])
    table('reanchored-comparison.tsv',['instruction_va','capstone_size','objdump_size','length_agrees'],
          [(hx(a),b,c,d) for a,b,c,d in reanchored])
    table('excluded-data-seeds.tsv',['seed_va','reason'],
          [(hx(a),'EXE_CONFIRMED switch table data') for a in sorted(excluded_seeds)])
    (out/'switches.json').write_text(json.dumps(switch_evidence,indent=2)+'\n')
    summary = dict(schema='dmc3-static-cfg-v1',sha256=CANONICAL,
        capstone_version=capstone.__version__,
        status='STRUCTURAL_CONFIRMED',semantic_completion_claim=False,
        seed_count=len(seeds),runtime_range_count=len(ranges),visited_instruction_starts=len(visited),
        excluded_confirmed_data_seeds=len(excluded_seeds),resolved_switch_sites=len(switch_targets),
        visited_instruction_bytes_sum=sum(visited.values()),decode_failures=len(failures),
        overlapping_instructions=len(overlaps),outside_executable_targets=len(outside),
        transfer_counts=dict(collections.Counter(kind+(':direct' if target is not None else ':indirect_unresolved') for a,kind,target,d in edges)),
        linear_first_token_bad_count=len(old_first_token_bad),
        linear_all_bad_count=len(old_bad),linear_all_bad_inside_ranges=sum(owner(a) is not None for a in old_bad),
        zero_preserving_linear_all_bad_count=len(bad),
        bad_dispositions=dict(collections.Counter(coverage(a) for a in bad)),
        common_start_length_agreements=agrees,decoder_disagreements=len(disagreements),
        reanchored_length_agreements=sum(d for a,b,c,d in reanchored),
        limitations=['Seeds are static metadata, not proof of execution.',
          'Only six reviewed switch sites are resolved; other indirect jumps terminate traversal. Callbacks, TLS callbacks, vtables and exception handlers are not comprehensively seeded.',
          'Calls conservatively assume fallthrough, including potentially noreturn functions.',
          'Unvisited bytes are not proven data or unreachable.',
          'Second-decoder comparison verifies lengths at common starts, not full instruction semantics.'])
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    return summary


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe',type=pathlib.Path)
    parser.add_argument('out',type=pathlib.Path)
    args = parser.parse_args()
    print(json.dumps(run(args.exe,args.out),indent=2))
