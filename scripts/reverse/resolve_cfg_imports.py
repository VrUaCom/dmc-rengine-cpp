#!/usr/bin/env python3
"""Resolve canonical CFG FF RIP-relative import operands and one-hop thunks.

Python stdlib only. Register/memory dispatch remains unresolved. No execution.
"""
import argparse
import collections
import csv
import gzip
import hashlib
import io
import json
from pathlib import Path
import struct

HASH = 'e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'


def classify_ff(code, va):
    i = 0
    address_override = False
    while i < len(code) and (code[i] in (0x66,0x67,0xf2,0xf3,0x2e,0x3e,0x26,0x36,0x64,0x65) or 0x40 <= code[i] <= 0x4f):
        address_override |= code[i] in (0x67,0x64,0x65)
        i += 1
    if i+2 > len(code) or code[i] != 0xff:
        return 'unsupported', None
    modrm = code[i+1]
    if (modrm >> 3) & 7 not in (2,4):
        return 'unsupported', None
    if modrm >> 6 == 3:
        return 'register', None
    if modrm & 0xc7 == 5 and not address_override and len(code) == i+6:
        return 'rip_memory', va+len(code)+struct.unpack_from('<i',code,i+2)[0]
    return 'other_memory', None


def run(exe, cfg, out):
    data = exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != HASH:
        raise ValueError('canonical SHA-256 mismatch')
    pe = struct.unpack_from('<I',data,0x3c)[0]
    opt = pe+24
    assert data[pe:pe+4] == b'PE\0\0'
    assert struct.unpack_from('<H',data,opt)[0] == 0x20b
    base = struct.unpack_from('<Q',data,opt+24)[0]
    count = struct.unpack_from('<H',data,pe+6)[0]
    osize = struct.unpack_from('<H',data,pe+20)[0]
    sections = [struct.unpack_from('<IIII',data,opt+osize+40*i+8) for i in range(count)]
    def offset(rva,n=1):
        for vs,a,size,raw in sections:
            if a <= rva and rva+n <= a+min(vs,size):
                return raw+rva-a
        raise ValueError(f'unbacked RVA {rva:x}')
    def text(rva):
        p = offset(rva)
        return data[p:data.index(0,p)].decode('ascii')
    irva,isize = struct.unpack_from('<II',data,opt+120)
    imports = {}
    for i in range(0,isize,20):
        oft,t,c,name,ft = struct.unpack_from('<IIIII',data,offset(irva+i,20))
        if not any((oft,t,c,name,ft)):
            break
        dll = text(name)
        j = 0
        while True:
            v = struct.unpack_from('<Q',data,offset((oft or ft)+8*j,8))[0]
            if v == 0:
                break
            symbol = f'ordinal:{v & 0xffff}' if v >> 63 else text(v+2)
            imports[base+ft+8*j] = dll+'!'+symbol
            j += 1
    def read(name):
        with gzip.open(cfg/name,'rt') as f:
            return list(csv.DictReader(f,delimiter='\t'))
    summary = json.loads((cfg/'summary.json').read_text())
    assert summary['sha256'] == HASH
    visited = {int(r['instruction_va'],16):int(r['size']) for r in read('visited.tsv.gz')}
    edges = read('edges.tsv.gz')
    indirect = []
    resolved_jump_thunks = {}
    for r in edges:
        if r['target_va']:
            continue
        a = int(r['site_va'],16)
        n = visited[a]
        p = offset(a-base,n)
        kind,slot = classify_ff(data[p:p+n],a)
        symbol = imports.get(slot,'')
        indirect.append((a,r['kind'], 'import_slot' if symbol else kind,slot,symbol))
        if symbol and r['kind']=='jump':
            resolved_jump_thunks[a] = (slot,symbol)
    wrappers = []
    for r in edges:
        if not r['target_va'] or r['kind'] not in ('call','jump'):
            continue
        target = int(r['target_va'],16)
        if target in resolved_jump_thunks:
            slot,symbol = resolved_jump_thunks[target]
            wrappers.append((int(r['site_va'],16),r['kind'],target,slot,symbol))
    out.mkdir(parents=True,exist_ok=True)
    def hx(v):
        return '' if v is None else hex(v)
    def write(name,header,rows):
        stream = io.StringIO()
        w = csv.writer(stream,delimiter='\t',lineterminator='\n')
        w.writerow(header)
        w.writerows(rows)
        (out/name).write_bytes(gzip.compress(stream.getvalue().encode(),mtime=0))
    write('indirect-sites.tsv.gz',['site_va','kind','classification','slot_va','symbol'],
          [(hx(a),b,c,hx(d),e) for a,b,c,d,e in indirect])
    write('one-hop-import-thunks.tsv.gz',['site_va','kind','thunk_va','iat_slot_va','symbol'],
          [(hx(a),b,hx(c),hx(d),e) for a,b,c,d,e in wrappers])
    result = dict(schema='dmc3-cfg-import-resolution-v1',sha256=HASH,status='EXE_CONFIRMED',
        input_cfg_sha256={n:hashlib.sha256((cfg/n).read_bytes()).hexdigest() for n in ('summary.json','visited.tsv.gz','edges.tsv.gz')},
        import_slots=len(imports),indirect_sites=len(indirect),
        classifications=dict(sorted(collections.Counter(b+':'+c for a,b,c,d,e in indirect).items())),
        one_hop_thunk_sites=len(wrappers),unique_referenced_imports=len({e for a,b,c,d,e in indirect if e}),
        import_site_frequency=dict(collections.Counter(e for a,b,c,d,e in indirect if e).most_common()),
        limitations=['Import resolution identifies static IAT operands, not the eventual loaded function address or runtime behavior.',
          'One-hop thunks require the direct target itself to be a visited unconditional RIP-relative IAT jump.',
          'No vtable target, register value, arbitrary wrapper or delay import is inferred.',
          'Only the previously visited CFG is analyzed; full-game completeness is not claimed.'])
    (out/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
    return result


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('exe',type=Path)
    p.add_argument('cfg',type=Path)
    p.add_argument('out',type=Path)
    a = p.parse_args()
    print(json.dumps(run(a.exe,a.cfg,a.out),indent=2))
