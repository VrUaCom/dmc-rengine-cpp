#!/usr/bin/env python3
"""Per-file DMC3 resource nature census.

Visits every file once, fingerprints the payload rather than trusting the
extension, and emits JSONL rows plus compact aggregate indexes. It never
modifies the source corpus.
"""
from __future__ import annotations
import argparse,hashlib,json,math,re
from collections import Counter,defaultdict
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path

MAGICS={
 b'SCM ': 'scm', b'MOD ': 'mod', b'EFM ': 'efm', b'SHW ': 'shw', b'PAC\x00':'pac', b'PNST':'pnst', b'DDS ':'dds', b'LIG2':'lig', b'CAM\x00':'cam', b'EVE\x00':'eve', b'POS\x00':'pos', b'SEF\x00':'sef', b'HITS':'hits', b'DCA\x00':'dca', b'MOT\x00':'mot', b'VAGp':'vag', b'VAGP':'vag'}
TAG_NAMES={'ITM\x00':'itm','STE\x00':'ste','EST\x00':'est','TSC':'tsc','TEX':'tex','AFS':'afs','HID':'hid'}
STR_RE=re.compile(rb'[ -~]{4,}')

def entropy(data:bytes)->float:
 if not data:return 0.0
 c=Counter(data); n=len(data); return round(-sum((v/n)*math.log2(v/n) for v in c.values()),4)

def classify(ext:str,head:bytes,sample:bytes,ratio:float,zero:float):
 found=[]
 for m,name in MAGICS.items():
  off=head.find(m)
  if off>=0: found.append((off,name,m.decode('latin1','replace')))
 for t,name in TAG_NAMES.items():
  b=t.encode('latin1'); off=sample.find(b)
  if off>=0: found.append((off,name,t))
 found.sort(key=lambda x:(x[0],x[1]))
 primary=found[0][1] if found else None
 textish=ratio>=0.72 and zero<0.03
 if primary in {'scm','mod','efm','shw','pac','pnst','dds','lig','cam','eve','pos','sef','hits','dca','mot','vag'}: nature=primary
 elif primary in {'tsc','itm','ste','est','hid'}: nature=primary+'-text-or-tagged'
 elif textish: nature='text-like'
 elif ext in {'.pac','.afs'}: nature='container-candidate'
 else: nature='opaque-binary'
 return primary,nature,found[:8],textish

def one(p:Path,src:Path,do_hash:bool):
 try:
  st=p.stat(); size=st.st_size; h=hashlib.sha256() if do_hash else None; chunks=[]; total=0
  with p.open('rb') as f:
   first=f.read(65536); chunks.append(first); total+=len(first)
   if h: h.update(first)
   while h:
    b=f.read(1024*1024)
    if not b: break
    h.update(b); total+=len(b)
  sample=b''.join(chunks); head=sample[:64]
  ratio=sum(32<=b<127 or b in (9,10,13) for b in sample)/len(sample) if sample else 0
  zero=sample.count(0)/len(sample) if sample else 0
  ext=p.suffix.lower() or '<none>'; primary,nature,found,textish=classify(ext,head,sample,ratio,zero)
  strings=[]
  for m in STR_RE.findall(sample):
   s=m.decode('ascii','replace')
   if s not in strings: strings.append(s)
   if len(strings)>=8: break
  row={'path':p.relative_to(src).as_posix(),'family':p.relative_to(src).parts[0] if p.relative_to(src).parts else '<root>','extension':ext,'size':size,'size_bucket':('<16' if size<16 else '16-255' if size<256 else '256-4095' if size<4096 else '4K-64K' if size<65536 else '64K-1M' if size<1048576 else '1M-16M' if size<16777216 else '>=16M'),'head64':head.hex(' '),'prefix4':head[:4].decode('latin1','replace') if all(32<=b<127 for b in head[:4]) else head[:4].hex(' ').upper(),'sample_entropy':entropy(sample),'printable_ratio':round(ratio,4),'zero_ratio':round(zero,4),'detected_format':primary or '<none>','nature':nature,'matches':[{'offset':o,'format':n,'token':t} for o,n,t in found[:8]],'strings':strings,'text_like':textish}
  if h: row['sha256']=h.hexdigest()
  return row
 except Exception as e:
  return {'path':str(p.relative_to(src)),'ok':False,'error':repr(e)}

def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--source',required=True,type=Path); ap.add_argument('--output',required=True,type=Path); ap.add_argument('--workers',type=int,default=8); ap.add_argument('--hash',action='store_true'); a=ap.parse_args(); src=a.source; files=[p for p in src.rglob('*') if p.is_file()]; rows=[]
 with ThreadPoolExecutor(max_workers=max(1,a.workers)) as pool:
  fs=[pool.submit(one,p,src,a.hash) for p in files]
  for f in as_completed(fs): rows.append(f.result())
 rows.sort(key=lambda r:r['path']); agg=Counter(); fm=defaultdict(Counter); nat=Counter(); dup=defaultdict(list); errors=[]
 for r in rows:
  if r.get('ok') is False: errors.append(r); continue
  agg[r['extension']]+=1; fm[r['extension']][r['detected_format']]+=1; nat[r['nature']]+=1
  if 'sha256' in r: dup[r['sha256']].append(r['path'])
 duplicates={k:v for k,v in dup.items() if len(v)>1}
 a.output.parent.mkdir(parents=True,exist_ok=True); jsonl=a.output.with_suffix('.jsonl');
 with jsonl.open('w',encoding='utf-8') as f:
  for r in rows: f.write(json.dumps(r,ensure_ascii=False,separators=(',',':'))+'\n')
 report={'schema_version':1,'source':str(src),'files_seen':len(rows),'files_ok':len(rows)-len(errors),'errors':errors,'extensions':dict(sorted(agg.items(),key=lambda x:(-x[1],x[0]))),'nature_counts':dict(sorted(nat.items(),key=lambda x:(-x[1],x[0]))),'detected_by_extension':{k:dict(v.most_common()) for k,v in sorted(fm.items())},'duplicate_hash_groups':len(duplicates),'duplicate_hashes':duplicates,'jsonl':str(jsonl)}
 a.output.write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8'); print(json.dumps({'files_seen':len(rows),'files_ok':len(rows)-len(errors),'errors':len(errors),'nature_counts':report['nature_counts'],'duplicate_hash_groups':len(duplicates)},ensure_ascii=False))
if __name__=='__main__': main()
