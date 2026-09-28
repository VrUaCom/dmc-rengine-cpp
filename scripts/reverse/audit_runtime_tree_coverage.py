#!/usr/bin/env python3
"""Audit known structural roots against a bounded CFG, without claiming closure."""
import argparse
import bisect
import collections
import csv
import gzip
import hashlib
import io
import json
from pathlib import Path
import struct

HASH='e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'


def audit(exe,runtime,cfg,structure,out):
    b=exe.read_bytes()
    if hashlib.sha256(b).hexdigest()!=HASH:raise ValueError('noncanonical EXE')
    def read(root,name):
        p=root/name
        with (gzip.open(p,'rt') if name.endswith('.gz') else p.open()) as f:
            return list(csv.DictReader(f,delimiter='\t'))
    pe=struct.unpack_from('<I',b,0x3c)[0];opt=pe+24
    base=struct.unpack_from('<Q',b,opt+24)[0]
    ns=struct.unpack_from('<H',b,pe+6)[0];sz=struct.unpack_from('<H',b,pe+20)[0]
    sections=[]
    for i in range(ns):
        p=opt+sz+40*i
        vs,rva,rs,raw=struct.unpack_from('<IIII',b,p+8)
        sections.append((base+rva,min(vs,rs),raw,struct.unpack_from('<I',b,p+36)[0]))
    def off(va,n):
        for a,size,raw,flags in sections:
            if a<=va and va+n<=a+size:return raw+va-a
        raise ValueError(hex(va))
    def code(va):return any(flags&0x20000000 and a<=va<a+size for a,size,raw,flags in sections)
    visited={int(r['instruction_va'],16):int(r['size']) for r in read(cfg,'visited.tsv.gz')}
    starts=sorted(visited)
    def coverage(va):
        if va in visited:return 'visited_start'
        i=bisect.bisect_right(starts,va)-1
        return 'inside_visited_instruction' if i>=0 and va<starts[i]+visited[starts[i]] else 'not_visited'
    types=read(runtime,'types.tsv');inherit=read(runtime,'inheritance.tsv')
    anchors=read(runtime,'vtable_anchors.tsv');ranges=read(runtime,'runtime_ranges.tsv')
    derived={r['derived_type_va'] for r in inherit}
    roots=[r for r in types if r['type_va'] not in derived]
    crt=read(structure,'crt-initializers.tsv.gz')
    crtrecords=[]
    for r in crt:
        va=int(r['target_va'],16)
        if va:
            assert code(va)
            crtrecords.append((r['table'],r['slot_va'],hex(va),coverage(va)))
    first=[]
    for r in anchors:
        va=int(r['first_entry_va'],16)
        assert struct.unpack_from('<Q',b,off(int(r['vftable_va'],16),8))[0]==va
        first.append((r['vftable_va'],r['class'],hex(va),coverage(va)))
    # Non-first slots are deliberately candidates. A run of executable pointers
    # is not proof of a vtable's end or of virtual-slot signatures.
    anchor_addresses=sorted(int(r['vftable_va'],16) for r in anchors)
    slots=[];runstops=collections.Counter()
    for r in anchors:
        a=int(r['vftable_va'],16)
        idx=bisect.bisect_right(anchor_addresses,a)
        limit=anchor_addresses[idx]-8 if idx<len(anchor_addresses) else a+256*8
        for i in range(256):
            slot=a+i*8
            if slot>=limit:runstops['next_locator_boundary']+=1;break
            try:v=struct.unpack_from('<Q',b,off(slot,8))[0]
            except ValueError:runstops['unbacked']+=1;break
            if not code(v):runstops['noncode_pointer']+=1;break
            slots.append((hex(a),i,hex(slot),hex(v),coverage(v),'STRUCTURAL_CONFIRMED' if i==0 else 'SEMANTIC_CANDIDATE'))
        else:runstops['cap_256']+=1
    handlers=[]
    for r in ranges:
        va=int(r['unwind_va'],16);p=off(va,4)
        version=b[p]&7;flags=b[p]>>3;count=b[p+2]
        if flags&3 and not flags&4:
            assert version==1
            hp=off(va+4+((count+1)&~1)*2,4)
            target=base+struct.unpack_from('<I',b,hp)[0]
            assert code(target)
            handlers.append((r['begin_va'],hex(va),hex(target),coverage(target)))
    out.mkdir(parents=True,exist_ok=True)
    def write(name,header,rows):
        s=io.StringIO();w=csv.writer(s,delimiter='\t',lineterminator='\n');w.writerow(header);w.writerows(rows)
        payload=s.getvalue().encode()
        (out/name).write_bytes(gzip.compress(payload,mtime=0) if name.endswith('.gz') else payload)
    write('crt-root-coverage.tsv.gz',['table','slot_va','target_va','coverage'],crtrecords)
    write('vtable-first-entry-coverage.tsv.gz',['vtable_va','class','target_va','coverage'],first)
    write('vtable-slot-candidates.tsv.gz',['anchor_va','slot_index','slot_va','target_va','coverage','status'],slots)
    write('exception-handler-coverage.tsv.gz',['range_begin_va','unwind_va','handler_va','coverage'],handlers)
    write('rtti-roots.tsv',['type_va','name'],[(r['type_va'],r['name']) for r in roots])
    def stats(rows,target_index,coverage_index):
        return dict(rows=len(rows),unique_targets=len({r[target_index] for r in rows}),
            coverage=dict(collections.Counter(r[coverage_index] for r in rows)),
            unique_not_visited=len({r[target_index] for r in rows if r[coverage_index]=='not_visited'}))
    result=dict(schema='dmc3-tree-completeness-audit-v1',sha256=HASH,tree_complete=False,
        hierarchy_types=len(types),inheritance_edges=len(inherit),rtti_roots_without_observed_base=len(roots),
        crt={name:stats([r for r in crtrecords if r[0]==name],2,3) for name in sorted({r[0] for r in crtrecords})},
        vtable_first_entries=stats(first,2,3),vtable_pointer_run_candidates=stats(slots,3,4),
        candidate_run_stop_reasons=dict(runstops),exception_handlers=stats(handlers,2,3),
        input_file_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [runtime/'types.tsv',runtime/'inheritance.tsv',runtime/'vtable_anchors.tsv',runtime/'runtime_ranges.tsv',cfg/'visited.tsv.gz',structure/'crt-initializers.tsv.gz']},
        limitations=['Not visited means absent from this CFG, not unknown to all prior research.',
          'A first vtable entry may be a thunk or shared method; it is not necessarily a unique function.',
          'Executable pointer runs after first entries are candidates, not proven vtable extents.',
          'RTTI roots can be legitimate interfaces or external bases, not automatically missing parent classes.',
          'No exhaustive universe of classes, callbacks, globals, ownership, update/draw or shutdown edges has been established.'])
    (out/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
    return result


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('exe','runtime','cfg','structure','out'):p.add_argument(n,type=Path)
    a=p.parse_args();print(json.dumps(audit(a.exe,a.runtime,a.cfg,a.structure,a.out),indent=2))
