#!/usr/bin/env python3
"""Field and anomaly census for BD/SO payloads and PNST offset tables."""
from __future__ import annotations
import argparse, json, math, struct
from collections import Counter, defaultdict
from pathlib import Path

def u32(b,o): return int.from_bytes(b[o:o+4],'little') if o+4<=len(b) else None
def f32(b,o):
    try:return struct.unpack_from('<f',b,o)[0]
    except struct.error:return None

def bd_census(root):
    files=sorted(p for p in root.rglob('*.bd') if p.is_file() and p.stat().st_size==96)
    offsets=[]; rows=[]; groups=defaultdict(list)
    for p in files:
        b=p.read_bytes(); vals=[u32(b,o) for o in range(0,96,4)]; groups[p.parts[-2] if len(p.parts)>1 else 'root'].append(p)
        rows.append({'path':str(p),'u32':vals,'prefix_zero_bytes':sum(x==0 for x in b[:28]),'suffix_zero_bytes':sum(x==0 for x in b[88:96])})
    for o in range(0,96,4):
        vals=[r['u32'][o//4] for r in rows]; fs=[struct.unpack('<f',struct.pack('<I',v))[0] for v in vals]
        finite=[x for x in fs if x is not None and math.isfinite(x)]
        offsets.append({'offset':o,'zero_ratio':round(sum(v==0 for v in vals)/len(vals),4),'unique':len(set(vals)),'top_values':[(str(v),n) for v,n in Counter(vals).most_common(8)],'finite_float_ratio':round(len(finite)/len(vals),4),'small_float_ratio':round(sum(abs(x)<=100000 for x in finite)/len(vals),4)})
    by_root=Counter()
    for p in files: by_root[p.parts[-4] if len(p.parts)>=4 else p.parts[0]]+=1
    return {'files':len(files),'record_size':96,'field_offsets':offsets,'top_level_counts':dict(by_root.most_common()),'rows':rows[:32]}

def so_graph(b):
    if len(b)<4 or int.from_bytes(b[:2],'little')!=6:return None
    t8=u16(b,2)
    if t8 is None or t8<=8 or t8+10>len(b) or u16(b,t8)!=8:return None
    def block(base,extent,hs):
        first=u16(b,base+hs) if base+hs+2<=len(b) else None
        if first is None or first<hs or first>=extent or (first-hs)%2:return None
        n=(first-hs)//2; vals=[u16(b,base+hs+2*i) for i in range(n)]
        if not vals or any(v is None or v>=extent or (i and v<=vals[i-1]) for i,v in enumerate(vals)) or vals[0]!=first:return None
        return {'type':u16(b,base),'extent':extent,'count':n}
    a=block(0,t8,8); c=block(t8,len(b)-t8,8)
    return {'blocks':[a,c]} if a and c else None

def u16(b,o): return int.from_bytes(b[o:o+2],'little') if o+2<=len(b) else None
def so_volume(b):
    if not b or len(b)%80:return None
    types=[]
    for o in range(0,len(b),80):
        if u32(b,o) not in (2,4) or b[o+4:o+16]!=b'\0'*12 or f32(b,o+28)!=1.0:return None
        types.append(u32(b,o))
    return {'records':len(types),'types':dict(Counter(map(str,types)))}
def so_link(b):
    if len(b)<8 or len(b)%4 or u32(b,0)!=6:return None
    if any(b[o+3] for o in range(0,len(b),4)):return None
    return {'records':len(b)//4}

def so_census(root):
    files=sorted(p for p in root.rglob('*.so') if p.is_file()); counts=Counter(); sizes=Counter(); rows=[]; by_parent=Counter()
    for p in files:
        b=p.read_bytes();sizes[str(len(b))]+=1; by_parent[p.parent.name]+=1
        typ='opaque-so'
        det={}
        if so_graph(b): typ='so-graph';det=so_graph(b)
        elif so_volume(b): typ='so-volume';det=so_volume(b)
        elif so_link(b): typ='so-link';det=so_link(b)
        elif len(b)==368: typ='so-23x16-effect';det={'records':23,'record_size':16}
        counts[typ]+=1
        if len(rows)<80 or typ!='opaque-so':
            rows.append({'path':str(p),'size':len(b),'classification':typ,'prefix':b[:16].hex(),'first_u16':[u16(b,o) for o in range(0,min(16,len(b)),2)],'detail':det,'sibling_extensions':dict(Counter(x.suffix.lower() for x in p.parent.iterdir() if x.is_file()))})
    return {'files':len(files),'size_counts':dict(sizes.most_common()),'classification_counts':dict(counts.most_common()),'parent_counts':dict(by_parent.most_common(40)),'rows':rows}

def pnst_census(root):
    rows=[]; failures=[]
    for p in sorted(root.rglob('*.pac')):
        b=p.read_bytes()
        if not b.startswith(b'PNST') or len(b)<8:continue
        n=u32(b,4); table=8+4*n
        if n is None or table>len(b):continue
        offs=[u32(b,8+4*i) for i in range(n)]; eq=sum(x==len(b) for x in offs); gt=sum(x>len(b) for x in offs); lt=sum(0<x<len(b) for x in offs); zero=sum(x==0 for x in offs)
        if eq or gt:
            good=[x for x in offs if 0<x<len(b)]; idx_eq=[i for i,x in enumerate(offs) if x==len(b)]
            txt=p.with_name(p.stem.replace('_012','_011')+'.txt')
            rows.append({'path':str(p),'size':len(b),'slot_count':n,'table_end':table,'positive_in_bounds':lt,'equal_size':eq,'out_of_bounds':gt,'zero_offsets':zero,'max_offset':max(offs),'max_over_size':round(max(offs)/len(b),6),'max_over_3size':round(max(offs)/(3*len(b)),6),'volume_bins':[sum(0<x<=len(b) for x in offs),sum(len(b)<x<=2*len(b) for x in offs),sum(2*len(b)<x<=3*len(b) for x in offs),sum(x>3*len(b) for x in offs)],'equal_size_indices':idx_eq,'manifest_exists':txt.exists(),'manifest_size':txt.stat().st_size if txt.exists() else None,'manifest_end_markers':txt.read_text(encoding='utf-8',errors='replace').count('# End') if txt.exists() else None})
    return {'anomalous_pnst_files':len(rows),'rows':rows}

def main():
    ap=argparse.ArgumentParser();ap.add_argument('root',type=Path);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
    out={'schema_version':1,'source':str(a.root),'bd_96':bd_census(a.root),'so':so_census(a.root),'pnst_anomalies':pnst_census(a.root)}
    a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'bd_96':out['bd_96']['files'],'so':out['so']['files'],'so_classes':out['so']['classification_counts'],'pnst_anomalies':out['pnst_anomalies']['anomalous_pnst_files']},ensure_ascii=False))
if __name__=='__main__':main()
