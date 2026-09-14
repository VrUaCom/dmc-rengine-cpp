#!/usr/bin/env python3
"""Address-only PE/unwind/call census; stdlib + GNU objdump, no game execution."""
import argparse
import bisect
import collections
import hashlib
import json
import pathlib
import re
import struct
import subprocess

CANONICAL = 'e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'


def census(exe, out):
    data = exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != CANONICAL:
        raise ValueError('canonical SHA-256 mismatch')
    def unpack(fmt, offset):
        return struct.unpack_from('<' + fmt, data, offset)
    pe, = unpack('I', 0x3c)
    assert data[pe:pe+4] == b'PE\0\0'
    machine, nsections = unpack('HH', pe+4)
    optional_size, = unpack('H', pe+20)
    opt = pe+24
    assert machine == 0x8664 and unpack('H', opt)[0] == 0x20b
    base, = unpack('Q', opt+24)
    sections = []
    for i in range(nsections):
        p = opt+optional_size+40*i
        vs, va, size, raw = unpack('IIII', p+8)
        flags, = unpack('I', p+36)
        sections.append(dict(name=data[p:p+8].rstrip(b'\0').decode(), rva=va,
                             virtual_size=vs, raw_size=size, raw_offset=raw, flags=flags))
    def offset(rva, size=1):
        for s in sections:
            rel = rva-s['rva']
            if 0 <= rel and rel+size <= s['raw_size']:
                pos = s['raw_offset']+rel
                if pos+size <= len(data):
                    return pos
        raise ValueError(f'unbacked RVA {rva:x}, size {size}')
    def string(rva):
        p = offset(rva)
        end = data.index(b'\0', p)
        return data[p:end].decode('ascii')
    def directory(i):
        return unpack('II', opt+112+8*i)
    prva, psz = directory(3)
    assert psz % 12 == 0
    ranges = [unpack('III', offset(prva+i, 12)) for i in range(0, psz, 12)]
    assert all(a < b for a,b,u in ranges)
    assert ranges == sorted(ranges)
    assert all(ranges[i][1] <= ranges[i+1][0] for i in range(len(ranges)-1))
    starts = [a for a,b,u in ranges]
    def owner(va):
        rva = va-base
        i = bisect.bisect_right(starts, rva)-1
        return base+ranges[i][0] if i >= 0 and rva < ranges[i][1] else None
    imports = {}
    irva, isz = directory(1)
    for rel in range(0, isz, 20):
        oft, stamp, chain, name, ft = unpack('IIIII', offset(irva+rel, 20))
        if not any((oft,stamp,chain,name,ft)):
            break
        dll = string(name)
        j = 0
        while True:
            thunk, = unpack('Q', offset((oft or ft)+j*8, 8))
            if not thunk:
                break
            symbol = f'ordinal:{thunk & 0xffff}' if thunk >> 63 else string(thunk+2)
            imports[base+ft+j*8] = dll+'!'+symbol
            j += 1
    dis = subprocess.run(['objdump','-d','-M','intel','--no-show-raw-insn',str(exe)],
                         check=True, capture_output=True, text=True).stdout
    rows = []
    per_range = collections.Counter()
    instructions = 0
    unowned = 0
    bad = 0
    bad_in_ranges = 0
    for line in dis.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\s+(\S+)\s*(.*)$', line)
        if not m:
            continue
        va, op, operands = int(m[1],16), m[2], m[3]
        instructions += 1
        fn = owner(va)
        per_range[fn] += 1
        unowned += fn is None
        bad += op == '(bad)'
        bad_in_ranges += op == '(bad)' and fn is not None
        if op not in ('call','jmp'):
            continue
        direct = re.match(r'^0x([0-9a-f]+)(?:\s|$)', operands)
        rip = re.search(r'#\s*0x([0-9a-f]+)', operands) if '[rip' in operands else None
        target = int(direct[1],16) if direct else None
        slot = int(rip[1],16) if rip else None
        kind = 'direct' if direct else ('import_slot' if slot in imports else 'indirect_unresolved')
        rows.append((va, fn, op, kind, target, owner(target) if target else None,
                     slot, imports.get(slot,'')))
    out.mkdir(parents=True, exist_ok=True)
    def hx(n):
        return '' if n is None else f'0x{n:X}'
    def tsv(name, header, values):
        (out/name).write_text('\t'.join(header)+'\n'+''.join('\t'.join(map(str,r))+'\n' for r in values))
    tsv('transfers.tsv', ['site_va','range_begin_va','opcode','classification','target_va',
                          'target_range_begin_va','rip_slot_va','import'],
        [(hx(a),hx(b),c,d,hx(e),hx(f),hx(g),h) for a,b,c,d,e,f,g,h in rows])
    tsv('ranges.tsv', ['begin_va','end_va_exclusive','unwind_rva','instruction_starts','status'],
        [(hx(base+a),hx(base+b),hx(u),per_range[base+a],'SEMANTIC_CANDIDATE') for a,b,u in ranges])
    tsv('imports.tsv', ['iat_slot_va','symbol'], [(hx(a),b) for a,b in sorted(imports.items())])
    counts = collections.Counter((r[2],r[3]) for r in rows)
    summary = dict(schema='dmc3-exe-census-v1', sha256=digest, file_size=len(data),
                   status='STRUCTURAL_CONFIRMED', semantic_completion_claim=False,
                   image_base=hx(base), entry_va=hx(base+unpack('I',opt+16)[0]),
                   objdump_version=subprocess.check_output(['objdump','--version'],text=True).splitlines()[0],
                   sections=sections, runtime_function_ranges=len(ranges),
                   import_slots=len(imports), disassembled_instruction_starts=instructions,
                   instruction_starts_outside_runtime_ranges=unowned, bad_decodes=bad,
                   bad_decodes_inside_runtime_ranges=bad_in_ranges,
                   transfer_counts={f'{a}:{b}':n for (a,b),n in sorted(counts.items())},
                   ranges_without_instruction_starts=sum(per_range[base+a]==0 for a,b,u in ranges),
                   limitations=['Unwind ranges are not a complete function list: leaf functions may have no entry; fragments may be separate entries.',
                                'Linear disassembly is not proof of reachability or code/data separation.',
                                'Indirect register/vtable transfers, delay imports, callback and exception edges are not resolved.',
                                'No semantic coverage percentage is inferred from instruction or range counts.'])
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    return summary


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe', type=pathlib.Path)
    parser.add_argument('out', type=pathlib.Path)
    args = parser.parse_args()
    print(json.dumps(census(args.exe,args.out),indent=2))
