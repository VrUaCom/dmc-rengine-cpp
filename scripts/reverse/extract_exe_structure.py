#!/usr/bin/env python3
"""Canonical PE, CRT initialization and address-backed startup structure."""
import argparse
import csv
import gzip
import hashlib
import io
import json
from pathlib import Path
import struct

HASH='e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'


def run(exe,out):
    b=exe.read_bytes()
    if hashlib.sha256(b).hexdigest()!=HASH: raise ValueError('noncanonical EXE')
    def u(fmt,p):return struct.unpack_from('<'+fmt,b,p)
    pe=u('I',0x3c)[0];opt=pe+24;base=u('Q',opt+24)[0]
    ns=u('H',pe+6)[0];sz=u('H',pe+20)[0]
    sections=[]
    for i in range(ns):
        p=opt+sz+40*i;vs,rva,rawsz,raw=u('IIII',p+8)
        sections.append(dict(name=b[p:p+8].rstrip(b'\0').decode(),rva=rva,virtual_size=vs,raw_size=rawsz,raw_offset=raw,zero_fill=max(0,vs-rawsz)))
    def off(va,n=1):
        for s in sections:
            r=va-base-s['rva']
            if 0<=r and r+n<=s['raw_size']:return s['raw_offset']+r
        raise ValueError(hex(va))
    def hx(x):return hex(x)
    direct=[]
    for site,target in [(0x140346160,0x14034673c),(0x140346169,0x140345ff0),(0x140346102,0x1402c5dc0),(0x1402c5ddf,0x1402c5df0),(0x1402c5e03,0x140030190),(0x140030199,0x1400490d0),(0x140346053,0x140346cb6),(0x140346074,0x140346cb0)]:
        p=off(site,5);assert b[p] in (0xe8,0xe9)
        actual=site+5+u('i',p+1)[0];assert actual==target
        direct.append(dict(site=hx(site),target=hx(target),kind='call' if b[p]==0xe8 else 'tail_jump',status='EXE_CONFIRMED'))
    crt=[];rows=[]
    for name,start,end,rcxsite,rdxsite in [('initterm',0x14034f808,0x14035d250,0x14034606d,0x140346066),('initterm_e',0x14035d258,0x14035d278,0x14034604c,0x140346045)]:
        for site,target in [(rcxsite,start),(rdxsite,end)]:
            p=off(site,7);assert b[p:p+2]==b'\x48\x8d';assert site+7+u('i',p+3)[0]==target
        values=[u('Q',off(a,8))[0] for a in range(start,end,8)]
        crt.append(dict(name=name,start=hx(start),end_exclusive=hx(end),slots=len(values),nulls=values.count(0),nonnull=len([v for v in values if v]),unique_nonnull=len(set(values)-{0})))
        rows.extend((name,i,hx(start+8*i),hx(v)) for i,v in enumerate(values))
    tlsrva,tlssize=u('II',opt+112+9*8)
    tls={}
    if tlsrva:
        start,end,index,callbacks,zero,flags=u('QQQQII',off(base+tlsrva,40))
        targets=[]
        for i in range(1024):
            v=u('Q',off(callbacks+8*i,8))[0]
            if not v:break
            targets.append(hx(v))
        else:raise ValueError('TLS callbacks not terminated')
        tls=dict(directory_va=hx(base+tlsrva),template_start=hx(start),template_end=hx(end),index_slot=hx(index),callback_array=hx(callbacks),callbacks=targets)
    # Reacquire table and body constants, without assigning undocumented ownership.
    arena_arg=u('I',off(0x140030195,4))[0];assert arena_arg==0x10400000
    for site,target in [(0x1400301a1,0x1405d9ea8),(0x1400301af,0x1405d9ef0),(0x140023e27,0x140cf3310),(0x140023e33,0x140508728)]:
        p=off(site,7);assert site+7+u('i',p+3)[0]==target
    assert u('I',off(0x140023e23,4))[0]==0x340
    result=dict(sha256=HASH,status='STRUCTURAL_CONFIRMED',entry=hx(base+u('I',opt+16)[0]),sections=sections,crt=crt,tls=tls,startup_edges=direct,
      arena=dict(initializer='0x140030190',allocator='0x1400490d0',bytes=arena_arg,base_global='0x1405d9ea8',end_global='0x1405d9ef0'),
      application_object=dict(drive_name='CMcAppli',initializer='0x140023e10',global_va='0x140cf3310',zeroed_bytes=0x340,vtable='0x140508728'),
      unresolved_rip_slot=dict(va='0x14034f7f8',on_disk_pointer=hx(u('Q',off(0x14034f7f8,8))[0])),
      limitations=['CRT tables use end-exclusive bounds from argument instructions. Initializer counts are not counts of C++ classes.', 'TLS metadata identifies callbacks, not their full semantics.', 'Subsystem ownership labels require additional constructor/caller evidence.'])
    out.mkdir(parents=True,exist_ok=True)
    s=io.StringIO();w=csv.writer(s,delimiter='\t',lineterminator='\n');w.writerow(['table','index','slot_va','target_va']);w.writerows(rows)
    (out/'crt-initializers.tsv.gz').write_bytes(gzip.compress(s.getvalue().encode(),mtime=0))
    (out/'structure.json').write_text(json.dumps(result,indent=2)+'\n')
    return result


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('exe',type=Path);p.add_argument('out',type=Path);a=p.parse_args();print(json.dumps(run(a.exe,a.out),indent=2))
