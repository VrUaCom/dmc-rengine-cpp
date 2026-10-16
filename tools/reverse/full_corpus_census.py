#!/usr/bin/env python3
"""Inventory every file in the extracted DMC corpus.

The census is deliberately filename-independent: it records relative path,
root family, extension, size buckets, first 16 bytes and a printable prefix.
It is the coverage ledger for subsequent Native Reader and EXE checks.
"""
from __future__ import annotations
import argparse, json
from collections import Counter, defaultdict
from pathlib import Path

def label(data: bytes) -> str:
    if not data: return '<empty>'
    if all(32 <= b < 127 for b in data): return data.decode('ascii', 'replace')
    return data.hex(' ').upper()

def bucket(n: int) -> str:
    if n < 16: return '<16'
    if n < 256: return '16-255'
    if n < 4096: return '256-4095'
    if n < 65536: return '4K-64K'
    if n < 1048576: return '64K-1M'
    if n < 16777216: return '1M-16M'
    if n < 134217728: return '16M-128M'
    return '>=128M'

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument('--source', required=True, type=Path)
    ap.add_argument('--output', required=True, type=Path)
    ap.add_argument('--rows', action='store_true', help='include one compact row per file')
    a=ap.parse_args()
    src=a.source.resolve()
    ext=Counter(); top=Counter(); top_ext=defaultdict(Counter); sizes=Counter(); prefixes=defaultdict(Counter); rows=[]; dirs=0; short=0
    for p in src.rglob('*'):
        if p.is_dir(): dirs += 1; continue
        try:
            st=p.stat(); size=st.st_size
            with p.open('rb') as f: head=f.read(16)
        except OSError as e:
            continue
        rel=p.relative_to(src)
        parts=rel.parts
        family=parts[0] if parts else '<root>'
        suffix=p.suffix.lower() or '<none>'
        ext[suffix]+=1; top[family]+=1; top_ext[family][suffix]+=1; sizes[bucket(size)]+=1; prefixes[suffix][label(head[:4])]+=1
        if size < 16: short += 1
        if a.rows: rows.append({'path':rel.as_posix(),'family':family,'extension':suffix,'size':size,'head16':head.hex(' '),'prefix4':label(head[:4])})
    report={'schema_version':1,'source':str(src),'directories':dirs,'files':sum(ext.values()),'short_lt16':short,'extensions':dict(sorted(ext.items(), key=lambda x:(-x[1],x[0]))),'size_buckets':dict(sorted(sizes.items())),'top_level_families':dict(sorted(top.items(), key=lambda x:(-x[1],x[0]))),'extension_by_family':{k:dict(sorted(v.items(), key=lambda x:(-x[1],x[0]))) for k,v in sorted(top_ext.items())},'prefixes_by_extension':{k:dict(v.most_common(32)) for k,v in sorted(prefixes.items())}}
    if a.rows: report['rows']=sorted(rows,key=lambda r:r['path'])
    a.output.parent.mkdir(parents=True,exist_ok=True); a.output.write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print(json.dumps({'files':report['files'],'directories':report['directories'],'extensions':len(ext),'families':len(top),'short_lt16':short},ensure_ascii=False))
    return 0
if __name__=='__main__': raise SystemExit(main())
