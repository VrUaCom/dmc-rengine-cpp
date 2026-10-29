#!/usr/bin/env python3
"""Independent stdlib verification of CFG address arithmetic and exclusions."""
import argparse
import csv
import gzip
import hashlib
import json
import pathlib
import struct


def verify(exe, folder):
    data = exe.read_bytes()
    summary = json.loads((folder/'summary.json').read_text())
    if hashlib.sha256(data).hexdigest() != summary['sha256']:
        raise ValueError('source hash mismatch')
    def rows(name):
        opener = gzip.open if name.endswith('.gz') else open
        with opener(folder/name,'rt') as f:
            return list(csv.DictReader(f,delimiter='\t'))
    # Independent PE section mapping; do not assume VA == file offset.
    pe = struct.unpack_from('<I',data,0x3c)[0]
    opt = pe+24
    base = struct.unpack_from('<Q',data,opt+24)[0]
    n = struct.unpack_from('<H',data,pe+6)[0]
    size = struct.unpack_from('<H',data,pe+20)[0]
    sections = [struct.unpack_from('<IIII',data,opt+size+40*i+8) for i in range(n)]
    def raw(va,n):
        for vs,rva,rs,off in sections:
            d = va-base-rva
            if 0 <= d and d+n <= min(vs,rs):
                return data[off+d:off+d+n]
        raise ValueError('unbacked VA')
    visited = {int(r['instruction_va'],16):int(r['size']) for r in rows('visited.tsv.gz')}
    addresses = sorted(visited)
    assert all(a+visited[a] <= b for a,b in zip(addresses,addresses[1:]))
    assert len(visited) == summary['visited_instruction_starts']
    direct = switch = 0
    for r in rows('edges.tsv.gz'):
        a = int(r['site_va'],16)
        assert a in visited
        if r['assumed_fallthrough_va']:
            assert int(r['assumed_fallthrough_va'],16) == a+visited[a]
            assert a+visited[a] in visited
        if not r['target_va']:
            continue
        target = int(r['target_va'],16)
        assert target in visited
        if r['kind'] == 'switch':
            switch += 1
            continue
        b = raw(a,visited[a])
        i = 0
        while b[i] in (0x66,0x67,0xf2,0xf3,0x2e,0x3e):
            i += 1
        op = b[i]
        if op in (0xe8,0xe9):
            displacement = struct.unpack_from('<i',b,i+1)[0]
        elif op == 0xeb or 0x70 <= op <= 0x7f or 0xe0 <= op <= 0xe3:
            displacement = struct.unpack_from('<b',b,i+1)[0]
        elif op == 0x0f and 0x80 <= b[i+1] <= 0x8f:
            displacement = struct.unpack_from('<i',b,i+2)[0]
        else:
            raise ValueError(f'unsupported direct encoding at {a:x}')
        assert a+len(b)+displacement == target
        direct += 1
    switches = json.loads((folder/'switches.json').read_text())
    table_slots = 0
    for s in switches:
        a = int(s['table_va'],16)
        n = s['entry_count']
        actual = [base+x for x in struct.unpack('<'+'I'*n,raw(a,n*4))]
        assert actual == [int(x,16) for x in s['target_by_slot']]
        assert all(target in visited for target in actual)
        assert not any(a <= v < a+n*4 for v in visited)
        if 'slot_by_selector' in s:
            assert list(raw(int(s['selector_remap_va'],16),s['selector_max']+1)) == s['slot_by_selector']
        table_slots += n
    assert all(int(r['seed_va'],16) not in visited for r in rows('excluded-data-seeds.tsv'))
    assert all(r['length_agrees']=='True' for r in rows('reanchored-comparison.tsv'))
    assert summary['common_start_length_agreements']+summary['reanchored_length_agreements']==len(visited)
    assert summary['decode_failures']==0 and summary['overlapping_instructions']==0
    return dict(status='PASS',sha256=summary['sha256'],direct_edges_verified_from_bytes=direct,
                switch_edges=switch,switch_table_slots_verified=table_slots,
                instruction_boundaries_checked=len(visited),
                checks=['direct displacements','edge endpoint closure','call/conditional fallthrough',
                        'no overlapping instructions','confirmed tables excluded from instruction starts',
                        'switch slots and remap match EXE','second decoder length agreement'])


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('exe',type=pathlib.Path)
    p.add_argument('folder',type=pathlib.Path)
    a = p.parse_args()
    print(json.dumps(verify(a.exe,a.folder),indent=2))
