#!/usr/bin/env python3
"""Classify opaque resource payloads using recovered structural invariants."""
from __future__ import annotations
import argparse, json, struct
from collections import Counter, defaultdict
from pathlib import Path

EXTS={'.ukn','.bd','.so','.ff','.tsb','.tm2','.bin','.hid','.hid0','.hid1','.hid2','.hid3','.hid4','.hid5','.hid6','.hid7','.hid8','.hid9'}

def u16(b,o): return int.from_bytes(b[o:o+2],'little') if o+2<=len(b) else None
def u32(b,o): return int.from_bytes(b[o:o+4],'little') if o+4<=len(b) else None

def so_graph(b):
    if len(b)<4 or u16(b,0)!=6:return None
    t8=u16(b,2)
    if t8 is None or t8<=8 or t8+10>len(b) or u16(b,t8)!=8:return None
    def block(base,extent,hs):
        if extent<hs+2 or base+extent>len(b):return None
        first=u16(b,base+hs)
        if first is None or first<hs or first>=extent or (first-hs)%2:return None
        count=(first-hs)//2
        vals=[u16(b,base+hs+2*i) for i in range(count)]
        if any(v is None or v>=extent or (i and v<=vals[i-1]) for i,v in enumerate(vals)):return None
        if not vals or vals[0]!=first:return None
        return {'base':base,'extent':extent,'type':u16(b,base),'header_size':hs,'count':count,'offsets':vals[:32]}
    a=block(0,t8,8); c=block(t8,len(b)-t8,8)
    return {'blocks':[a,c]} if a and c else None

def so_volume(b):
    if not b or len(b)%0x50:return None
    rows=[]
    for o in range(0,len(b),0x50):
        typ=u32(b,o)
        if typ not in (2,4) or b[o+4:o+16]!=b'\0'*12:return None
        try:w=struct.unpack_from('<f',b,o+0x1c)[0]
        except struct.error:return None
        if w!=1.0:return None
        rows.append(typ)
    return {'records':len(rows),'types':dict(Counter(map(str,rows)))}

def so_link(b):
    if len(b)<8 or len(b)%4 or u32(b,0)!=6:return None
    for o in range(0,len(b),4):
        if b[o+3]!=0:return None
    return {'records':len(b)//4,'leading':6}

def classify(p,b):
    ext=p.suffix.lower()
    # Strong content signatures first.
    if b.startswith(b'VAGp'): return 'vag-audio', {'chunked':True}
    if b.startswith(b'EVT\0'): return 'event-table', {'declared_size':u32(b,8),'header':'EVT\\0'}
    if b.startswith(b'SPUM') or b.startswith(b'SpuM') or b.startswith(b'SPUMAP'): return 'spu-map', {'header':b[:16].hex()}
    if b.startswith(b'dbsT') and len(b)>=16 and (len(b)-16)%32==0:
        return 'tsb-sound-table', {'records':(len(b)-16)//32,'declared_count':u32(b,12),'record_size':32}
    if ext=='.tm2' and len(b)>112 and b[112:116]==b'DDS ':
        return 'tm2-console-texture', {'header_size':112,'payload':'DDS','payload_size':len(b)-112}
    if ext=='.ff' and len(b)==56*16 and b.startswith(b'ff&A'):
        return 'ff-float-table', {'records':56,'record_size':16}
    if ext.startswith('.hid') and len(b)>=8 and b[4:7]==b'HID':
        return 'hid-dispatch-payload', {'declared_size':u32(b,0),'header':b[4:8].decode('ascii','replace')}
    if ext=='.so':
        g=so_graph(b)
        if g:return 'so-graph',g
        v=so_volume(b)
        if v:return 'so-volume',v
        l=so_link(b)
        if l:return 'so-link',l
    if ext=='.bd' and len(b)==96 and b[:48]==b'\0'*48:
        vals=[u32(b,o) for o in range(48,96,4)]
        return 'bd-effect-record', {'records':6,'record_size':16,'tail_u32':vals}
    if ext=='.ukn':
        if b'.TSC' in b[:256]: return 'ukn-tsc-text', {'magic':'.TSC'}
        if b'HITS' in b[:16]: return 'hits-records', {'magic':'HITS'}
        if b'DCA\0' in b[:16]: return 'dca-records', {'magic':'DCA'}
        if b'ITM' in b[:16]: return 'itm-tagged', {'magic':'ITM'}
        if b'STE' in b[:16]: return 'ste-tagged', {'magic':'STE'}
    return 'opaque-unclassified', {'size':len(b),'prefix':b[:16].hex()}

def main():
    ap=argparse.ArgumentParser();ap.add_argument('root',type=Path);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
    rows=[]; counts=Counter(); examples=defaultdict(list); errors=[]
    for p in a.root.rglob('*'):
        if not p.is_file() or p.suffix.lower() not in EXTS: continue
        try:b=p.read_bytes(); typ,det=classify(p,b)
        except Exception as e: errors.append({'path':str(p),'error':str(e)}); continue
        counts[typ]+=1
        if len(examples[typ])<8: examples[typ].append({'path':str(p),'size':len(b),'detail':det})
        rows.append({'path':str(p),'extension':p.suffix.lower(),'size':len(b),'classification':typ,'detail':det})
    out={'schema_version':1,'source':str(a.root),'files':len(rows),'errors':errors,'classification_counts':dict(counts.most_common()),'examples':dict(examples),'rows':rows}
    a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'files':len(rows),'errors':len(errors),'classification_counts':dict(counts.most_common())},ensure_ascii=False))
if __name__=='__main__':main()
