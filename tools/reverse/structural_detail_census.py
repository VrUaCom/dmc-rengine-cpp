#!/usr/bin/env python3
"""Collect detailed Native Reader structure for every SCM or PAC file."""
from __future__ import annotations
import argparse,json,re,subprocess
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
EXE='C:/Users/ADA/AppData/Local/DMC-Rengine/Native-Reader-CLI/dmc-rengine.exe'

def inspect_scm(p:Path,src:Path,timeout:float)->dict:
 rel=p.relative_to(src).as_posix()
 try:
  q=subprocess.run([EXE,'inspect-scm',str(p),'--json'],capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=timeout)
 except subprocess.TimeoutExpired:
  return {'path':rel,'ok':False,'error':'timeout'}
 first=q.stdout.find('{'); last=q.stdout.rfind('}')
 if first<0 or last<=first:return {'path':rel,'ok':False,'error':'invalid-json','exit_code':q.returncode}
 try:o=json.loads(q.stdout[first:last+1])
 except Exception:return {'path':rel,'ok':False,'error':'decode','exit_code':q.returncode}
 r=o.get('resource') or {}
 return {'path':rel,'ok':q.returncode==0,'size':o.get('byte_size') or r.get('source_size') or r.get('size'),'format':r.get('format'),'coverage_bytes':o.get('coverage_bytes'),'region_count':len(o.get('regions') or []),'field_count':len(o.get('fields') or []),'ownership_count':len(o.get('ownership') or []),'annotation_count':len(o.get('annotations') or []),'unknown_range_count':len(o.get('unknown_ranges') or []),'region_conflict_count':len(o.get('region_conflicts') or []),'ownership_conflict_count':len(o.get('ownership_conflicts') or [])}

def inspect_container(p:Path,src:Path,timeout:float)->dict:
 rel=p.relative_to(src).as_posix()
 try:q=subprocess.run([EXE,'list-container',str(p)],capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=timeout)
 except subprocess.TimeoutExpired:return {'path':rel,'ok':False,'error':'timeout'}
 out=q.stdout
 fmt=re.search(r'^  format\s+:\s+(\S+)',out,re.M); hdr=re.search(r'^(PAC|PNST)\s+\S+\s+\((\d+) slots\)',out,re.M); slots=re.findall(r'^  slot\s+(\d+)\s+off=\s*(\d+)\s+size=\s*(\d+)\s+\[\s*(\w+)\]',out,re.M); empties=len(re.findall(r'^  slot\s+\d+\s+\(empty\)',out,re.M)); children={}
 for _,_,_,kind in slots: children[kind]=children.get(kind,0)+1
 pm=re.search(r'^  parsed bytes\s+:\s+(\d+)',out,re.M); errors=sorted(set(re.findall(r'^  \[error\] ([^:]+):',out,re.M)))
 return {'path':rel,'ok':q.returncode==0,'format':fmt.group(1) if fmt else '<none>','slot_count':int(hdr.group(2)) if hdr else None,'nonempty_slots':len(slots),'empty_slots':empties,'child_formats':children,'parsed_bytes':int(pm.group(1)) if pm else None,'diagnostic_codes':errors}

def main()->int:
 ap=argparse.ArgumentParser(); ap.add_argument('--source',required=True,type=Path); ap.add_argument('--output',required=True,type=Path); ap.add_argument('--mode',choices=['scm','container'],required=True); ap.add_argument('--workers',type=int,default=8); ap.add_argument('--timeout',type=float,default=30); a=ap.parse_args(); src=a.source; want='.scm' if a.mode=='scm' else '.pac'; files=sorted(p for p in src.rglob('*') if p.is_file() and p.suffix.lower()==want); fn=inspect_scm if a.mode=='scm' else inspect_container; rows=[]
 with ThreadPoolExecutor(max_workers=max(1,a.workers)) as pool:
  futures=[pool.submit(fn,p,src,a.timeout) for p in files]
  for f in as_completed(futures): rows.append(f.result())
 rows.sort(key=lambda x:x['path']); out={'schema_version':1,'mode':a.mode,'source':str(src),'files':len(rows),'ok':sum(bool(x.get('ok')) for x in rows),'failed':sum(not x.get('ok') for x in rows),'rows':rows}; a.output.parent.mkdir(parents=True,exist_ok=True); a.output.write_text(json.dumps(out,indent=2,ensure_ascii=False)+'\n',encoding='utf-8'); print(json.dumps({k:out[k] for k in ('files','ok','failed')},ensure_ascii=False)); return 0 if out['failed']==0 else 2
if __name__=='__main__': raise SystemExit(main())

