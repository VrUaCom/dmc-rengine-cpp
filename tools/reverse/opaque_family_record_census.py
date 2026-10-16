#!/usr/bin/env python3
"""Summarize record-like structure for opaque families across the extracted corpus."""
from __future__ import annotations
import argparse, json, math, re, struct
from collections import Counter, defaultdict
from pathlib import Path

EXTS={'.ukn','.bd','.so','.ff','.tsb','.tm2','.bin','.hid','.hid0','.hid1','.hid2','.hid3','.hid4','.hid5','.hid6','.hid7','.hid8','.hid9'}
STRIDES=(4,8,12,16,24,32,48,64,80,96,112,128,144,160,176,192,208,224,240,256,272,288,304,320,336,352,368,384,400,416,448,480,512,528,544,576,608,640,704,768,896,1024,1072,1264)

def printable(data):
    vals=[]
    for m in re.finditer(rb'[ -~]{4,}',data):
        vals.append(m.group().decode('ascii','replace')[:80])
    return vals[:12]

def u32s(data):
    n=len(data)//4
    return list(struct.unpack('<%dI'%n,data[:n*4])) if n else []

def float_stats(data):
    n=len(data)//4
    if not n:return {'finite':0,'small':0,'nonneg':0,'min':None,'max':None}
    xs=struct.unpack('<%df'%n,data[:n*4]); good=[x for x in xs if math.isfinite(x)]
    small=[x for x in good if abs(x)<=100000]
    return {'finite':len(good),'small':len(small),'nonneg':sum(x>=0 for x in small),'min':min(small) if small else None,'max':max(small) if small else None}

def stride_info(data):
    out=[]; n=len(data)
    us=u32s(data)
    for s in STRIDES:
        if n and n%s==0 and n//s>=2:
            rows=n//s
            # compare first u32 in rows and row-local zero/nonzero density
            first=[struct.unpack_from('<I',data,i*s)[0] for i in range(min(rows,256))]
            mono=sum(first[i]<=first[i+1] for i in range(len(first)-1)) if len(first)>1 else 0
            out.append({'stride':s,'records':rows,'first_u32_unique':len(set(first)),'first_u32_mono_ratio':round(mono/max(1,len(first)-1),3)})
    return out

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('root',type=Path); ap.add_argument('--out',type=Path,required=True); args=ap.parse_args()
    rows=[]; by=defaultdict(lambda:{'files':0,'bytes':0,'sizes':Counter(),'first16':Counter(),'strides':Counter(),'strings':Counter()})
    for p in args.root.rglob('*'):
        if not p.is_file() or p.suffix.lower() not in EXTS: continue
        try:data=p.read_bytes()
        except OSError:continue
        ext=p.suffix.lower(); us=u32s(data); fs=float_stats(data); si=stride_info(data)
        first16=data[:16].hex(); key=(ext,len(data),first16)
        e=by[ext]; e['files']+=1; e['bytes']+=len(data); e['sizes'][str(len(data))]+=1; e['first16'][first16]+=1
        for x in si:e['strides'][str(x['stride'])]+=1
        for s in printable(data):e['strings'][s]+=1
        sib=Counter(x.suffix.lower() for x in p.parent.iterdir() if x.is_file())
        rows.append({'path':str(p),'ext':ext,'size':len(data),'first16':first16,'u32_first8':us[:8],'float':fs,'strides':si,'strings':printable(data),'sibling_exts':dict(sib)})
    def conv(d): return {k:(dict(v) if isinstance(v,Counter) else v) for k,v in d.items()}
    out={'schema_version':1,'source':str(args.root),'extensions':sorted(EXTS),'files':len(rows),'rows':rows,'summary':{k:conv(v) for k,v in by.items()}}
    args.out.parent.mkdir(parents=True,exist_ok=True); args.out.write_text(json.dumps(out,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps({'files':len(rows),'extensions':{k:v['files'] for k,v in by.items()}},ensure_ascii=False))
if __name__=='__main__':main()
