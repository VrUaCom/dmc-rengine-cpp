#!/usr/bin/env python3
"""EXE differential tests for PTX placement, pool initialization and reconfiguration.

Placement itself has no intercepted callees. Integration executes the actual
materializer, allocation, placement, marking, rollback, loaders and cache.
"""
import argparse
import ctypes
import hashlib
import json
import random
import struct
import subprocess
import tempfile
from pathlib import Path

import capstone
import unicorn
from unicorn import x86_const as R
from canonical_image import CANONICAL, CanonicalImage
from verify_crt_lifecycle import emulator, STOP
from verify_ptx_payload import POOL, PAYLOAD, SOURCE, MANAGER, MASK, digest, check
from verify_ptx_materializer import INPUT, state as materializer_state

PROFILE=0x54000040
CONFIG_SLOT=0x140d6d300
ROOTS={0:0x140331520,1:0x1403366e0,2:0x140336bb0,3:0x140336a70,
       4:0x140314e00,5:0x140314fa0,6:0x140315180,7:0x140331910,8:0x140331d90}
WINDOWS={0x140331520:0x1403316c7,0x140331910:0x14033196c,0x140331d90:0x140331de2}


def config(op=0,**kwargs):
    c=[op,0,0,0,POOL,0x1000,1,SOURCE,0x1800,0]
    for key,value in kwargs.items():
        c[{'index':1,'mode':2,'argument':3,'record':4,'profile_start':5,
           'palette_result':6,'key':7,'profile_end':8,'reserve_blocks':9}[key]]=value&MASK
    return c


def fixture(levels=2,palette=False,free=None,seed=1,textures=2):
    s=materializer_state(levels,palette,free,seed,textures)
    s['pool'][-64:-64]=bytes([0xab])*8
    s['profile']=bytearray(bytes([0xcd])*0x100)
    struct.pack_into('<i',s['pool'],64+0xcb0c,0x1800)
    # Only 64 initial cells are free. Outside that interval occupancy is nonzero.
    s['pool'][64+0x2800:64+0xca00]=bytes([0x7f])*0xa200
    s['pool'][64+0x2800:64+0x2840]=bytes(64)
    return s


def prepare_record(s,palette=True,length=3,palette_length=2):
    for off,value in ((0,1),(6,0x7777),(0xe,length),(0x18,0),(0x1c,0 if palette else -1),
                      (0x1e,0x5555),(0x26,palette_length),(0x44,0),(0x46,0)):
        struct.pack_into('<H',s['pool'],64+off,value&0xffff)


def map_bytes(s,index,data):s['pool'][64+0x2800+index:64+0x2800+index+len(data)]=data


def native(image,s,cfg,branches):
    u=emulator(image);op=cfg[0]
    pool_base=0x55000040 if op==8 else POOL
    manager_base=0x140d5b860 if op==8 else MANAGER
    bases={'pool':pool_base,'payload':PAYLOAD,'source':SOURCE,'manager':manager_base,'input':INPUT,'profile':PROFILE}
    for base in (0x50000000,0x51000000,0x52000000,0x53000000,0x54000000,0x55000000):u.mem_map(base,0x20000)
    for key,base in bases.items():u.mem_write(base-64,bytes(s[key]))
    u.mem_write(CONFIG_SLOT,struct.pack('<Q',PROFILE))
    receiver=pool_base if op in (0,7,8) else INPUT if op==1 else SOURCE if op in (2,3) else manager_base
    second=cfg[4] if op==0 else 0xffffffff if op==1 else PAYLOAD if op in (2,3) else cfg[9] if op==8 else cfg[7]
    u.reg_write(R.UC_X86_REG_RCX,receiver);u.reg_write(R.UC_X86_REG_RDX,second)
    u.reg_write(R.UC_X86_REG_R8,cfg[1] if op==0 else cfg[3]);u.reg_write(R.UC_X86_REG_R9,cfg[2] if op==0 else 0)
    events=[];reached=set();edges=set();pending=None;textures=0;clears=[]
    def ret(result):
        rsp=u.reg_read(R.UC_X86_REG_RSP);target=struct.unpack('<Q',u.mem_read(rsp,8))[0]
        for n,r in enumerate((R.UC_X86_REG_RCX,R.UC_X86_REG_RDX,R.UC_X86_REG_R8,
                               R.UC_X86_REG_R9,R.UC_X86_REG_R10,R.UC_X86_REG_R11)):
            u.reg_write(r,0x61000000+n*0x100)
        u.reg_write(R.UC_X86_REG_RAX,result&MASK)
        u.reg_write(R.UC_X86_REG_RSP,rsp+8);u.reg_write(R.UC_X86_REG_RIP,target)
    def hook(uc,addr,size,_):
        nonlocal pending,textures
        reached.add(addr)
        if pending is not None:edges.add((pending,addr))
        pending=addr if addr in branches else None
        if addr==0x140331520:
            check(uc.reg_read(R.UC_X86_REG_RCX)==POOL,'placement receiver')
            p=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([1,p,uc.reg_read(R.UC_X86_REG_R8)&0xffffffff,
                           uc.reg_read(R.UC_X86_REG_R9)&0xffffffff,digest(uc.mem_read(p,0x50))])
            # Observe this call but run its original instructions, including all branches.
        elif addr==0x140331bd0:
            p=uc.reg_read(R.UC_X86_REG_RCX);arg=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([2,p,arg,digest(uc.mem_read(p,0x50)),0])
            uc.mem_write(p+0x1a,struct.pack('<H',7));uc.mem_write(p+0x1e,struct.pack('<H',0x3000));ret(cfg[6])
        elif addr==0x1403365b0:
            address=uc.reg_read(R.UC_X86_REG_RCX);dest=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([3,address,textures,0,0]);textures+=1
            uc.mem_write(dest,bytes(uc.mem_read(INPUT,64)));ret(1)
        elif addr==0x140331a80:
            raw=uc.mem_read(uc.reg_read(R.UC_X86_REG_RCX),0x208)
            count,width=struct.unpack_from('<II',raw,0x200)
            events.append([4,count,width,digest(raw),0]);ret(1)
        elif addr==0x140346bea:
            p=uc.reg_read(R.UC_X86_REG_RCX);length=uc.reg_read(R.UC_X86_REG_R8)
            check(uc.reg_read(R.UC_X86_REG_RDX)&255==0,'zeroing memset')
            check((op==7 and p==pool_base and length==0xcb50) or
                  (POOL<=p<=POOL+0x27b0 and length==0x50),'represented memset extent')
            clears.append([p,length]);uc.mem_write(p,bytes(length));ret(p)
    u.hook_add(unicorn.UC_HOOK_CODE,hook);u.emu_start(ROOTS[op],STOP,count=250000)
    check(u.reg_read(R.UC_X86_REG_RIP)==STOP,'bounded return')
    rax=u.reg_read(R.UC_X86_REG_RAX)
    result=0 if op==7 else (rax-manager_base if rax else -1) if op in (4,5) else rax&255 if op in (0,6,8) else ctypes.c_int32(rax&0xffffffff).value
    return {k:bytes(u.mem_read(base-64,len(s[k]))) for k,base in bases.items()},result,events,reached,edges,clears


def evidence(image):
    rows=[];branches={}
    for root,end in WINDOWS.items():
        ins=list(image.md.disasm(image.raw(root,end-root),root))
        check(ins[-1].address+ins[-1].size==end,'complete reviewed window')
        calls=[]
        for i in ins:
            if i.group(capstone.CS_GRP_CALL):calls.append(dict(site=hex(i.address),target=hex(i.operands[0].imm)))
            if root==ROOTS[0] and i.group(capstone.CS_GRP_JUMP) and i.mnemonic!='jmp':
                branches[i.address]=(i.address+i.size,i.operands[0].imm)
        rows.append(dict(root=hex(root),end=hex(end),instruction_addresses=[hex(i.address) for i in ins],
            code_sha256=hashlib.sha256(image.raw(root,end-root)).hexdigest(),calls=calls))
    i=image.instruction(0x140331591)
    check(i.address+i.size+i.operands[1].mem.disp==CONFIG_SLOT,'configuration global binding')
    check(not rows[0]['calls'],'placement leaf has no external callees')
    return dict(status='EXE_CONFIRMED',sha256=CANONICAL,functions=rows,
        config_pointer=hex(CONFIG_SLOT),config_start_word_offset=0x4c,config_end_word_offset=0x4e,
        pool_initialization_clear_size=0xcb50,
        pool_offsets={'base_units':0xcb08,'scan_limit_units':0xcb0c,'upper_units':0xcb10,
                      'reserved_units':0xcb14,'remaining_units':0xcb18},
        configuration_resets_global_manager=hex(0x140d5b860),
        limitations=['The clear proves an accessed extent, not the complete original class layout.',
                     'No game-scene ordering or runtime profile values are inferred from synthetic profiles.']),branches


def run(exe,repo,out):
    image=CanonicalImage(exe);facts,branches=evidence(image);cases=[];seen=set();covered=set()
    with tempfile.TemporaryDirectory(prefix='ptx-placement-') as temp:
        library=Path(temp)/'placement.so'
        sources=['src/reverse/ptx_record_placement.cpp','src/reverse/ptx_record_materializer.cpp',
                 'src/reverse/ptx_payload_state.cpp','src/reverse/ptx_manager_state.cpp','tests/reverse/ptx_placement_bridge.cpp']
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Wconversion','-Werror',
            '-I'+str(repo/'include'),*[str(repo/p) for p in sources],'-o',str(library)],check=True)
        fn=ctypes.CDLL(str(library)).ptx_placement_step
        fn.argtypes=[ctypes.c_void_p]*5+[ctypes.c_size_t,ctypes.POINTER(ctypes.c_uint64),ctypes.POINTER(ctypes.c_uint64)]
        fn.restype=ctypes.c_int64
        def cpp(s,c):
            b={k:ctypes.create_string_buffer(bytes(v),len(v)) for k,v in s.items()}
            cfg=(ctypes.c_uint64*len(c))(*c);log=(ctypes.c_uint64*2048)()
            result=fn(*[ctypes.byref(b[k],64) for k in ('pool','input','payload','manager','source')],
                      len(s['source'])-128,cfg,log)
            return {k:v.raw for k,v in b.items()},result,[list(log[1+5*i:6+5*i]) for i in range(log[0])]
        def case(name,s,c,expected_result=None):
            struct.pack_into('<HH',s['profile'],64+0x4c,c[5]&0xffff,c[8]&0xffff)
            actual,result,events,reached,edges,clears=native(image,s,c,branches);other,cresult,cevents=cpp(s,c)
            check(result==cresult,f'{name}: result {result}/{cresult}')
            if expected_result is not None:check(result==expected_result,f'{name}: expected observation')
            check(events==cevents,f'{name}: events {events}/{cevents}')
            for key in actual:
                if actual[key]!=other[key]:
                    pos=next(i for i,(a,b) in enumerate(zip(actual[key],other[key])) if a!=b)
                    raise AssertionError(f'{name}: {key} at {pos-64:#x}: EXE {actual[key][pos]:02x}, C++ {other[key][pos]:02x}')
                check(actual[key][:64]==s[key][:64] and actual[key][-64:]==s[key][-64:],f'{name}: {key} guards')
            for key in ('input','source','profile'):check(actual[key]==bytes(s[key]),f'{name}: immutable {key}')
            if c[0]==0:check(actual['pool'][64+0x2800:64+0xca00]==bytes(s['pool'][64+0x2800:64+0xca00]),'placement does not mark occupancy')
            fields={hex(o):struct.unpack_from('<H',actual['pool'],64+o)[0] for o in (6,0xe,0x1e,0x26,0x46)}
            seen.update(reached);covered.update(edges)
            cases.append(dict(name=name,entry=hex(ROOTS[c[0]]),result=result,byte_equal=True,
                              record0_fields=fields,events=events,memsets=[[hex(a),n] for a,n in clears]))
            return {k:bytearray(v) for k,v in actual.items()}
        for mode in (0,1):
            for gate in (None,0x1c,0x44,0x18):
                for index in (0,5):
                    s=fixture();prepare_record(s)
                    if gate is not None:struct.pack_into('<H',s['pool'],64+gate,0xffff if gate==0x1c else 1)
                    case(f'mode-{mode}-gate-{gate}-index-{index}',s,config(index=index,mode=mode))
        for pattern in (bytes([1,1,0,0,0,0,0,1]),bytes([0,1,0,1,0,0,0,0]),bytes([1])*64):
            s=fixture();prepare_record(s,palette=False,length=3);map_bytes(s,0,pattern)
            case('automatic-map-'+pattern.hex(),s,config(mode=1))
        s=fixture();prepare_record(s,palette=False,length=8);struct.pack_into('<i',s['pool'],64+0xcb0c,0x1020)
        case('automatic-free-run-extends-past-limit',s,config(mode=1),1)
        s=fixture();prepare_record(s);map_bytes(s,5,bytes([0,0,0,1,1]))
        case('explicit-does-not-check-palette-cells',s,config(index=5,mode=1),1)
        s=fixture();prepare_record(s);struct.pack_into('<H',s['pool'],64+0x18,1)
        case('explicit-overwrites-external-palette-base',s,config(index=5,mode=1),1)
        s=fixture();prepare_record(s,palette=False,length=1);map_bytes(s,200,bytes([0]))
        case('explicit-index-above-limit-is-accepted',s,config(index=200,mode=1),1)
        s=fixture();prepare_record(s,palette=False,length=1);map_bytes(s,-1,bytes([0]))
        case('explicit-negative-index-before-map',s,config(index=-1,mode=1),1)
        s=fixture();prepare_record(s);struct.pack_into('<ii',s['pool'],64+0xcb08,320,160)
        case('explicit-between-inverted-bounds-is-rejected',s,config(index=7),0)
        for index in (0,5):
            s=fixture();prepare_record(s);map_bytes(s,0,bytes([1])*64)
            case(f'failed-search-keeps-default-length-writes-{index}',s,config(index=index),0)
        for start,bias in ((0x1001,0x1000),(0x0fff,0x1000),(-31,0),(-33,0),(0xffe0,0xffe0)):
            s=fixture();prepare_record(s,palette=False,length=2)
            struct.pack_into('<ii',s['pool'],64+0xcb08,start,start+0x800)
            map_bytes(s,-3,bytes(80))
            case(f'signed-start-and-profile-{start}-{bias}',s,config(mode=1,profile_start=bias))
        for seed in range(64):
            rng=random.Random(seed);s=fixture(seed=seed);prepare_record(s,palette=rng.choice((False,True)),length=rng.randrange(1,7),palette_length=rng.randrange(0,4))
            map_bytes(s,0,bytes(rng.choice((0,0,0,1,0xff)) for _ in range(64)))
            index=rng.choice((0,0,0,1,5,17,63))
            case(f'deterministic-map-{seed}',s,config(index=index,mode=rng.choice((0,1,-1))))
        for op in (1,2,3,4,5):
            for palette in (False,True):
                case(f'integration-{op}-palette-{palette}',fixture(palette=palette,textures=3),config(op))
            case(f'integration-{op}-palette-helper',fixture(palette=True,textures=2),config(op,argument=0x9900))
            s=fixture(palette=True,textures=3);available=2 if op==1 else 3
            map_bytes(s,0,bytes(available)+bytes([1])*(64-available))
            case(f'integration-{op}-real-map-exhaustion',s,config(op),0 if op<4 else -1)
        s=fixture(palette=True,textures=2)
        for step,op in enumerate((4,5,6,6,5)):
            s=case(f'cache-full-placement-lifecycle-{step}',s,config(op,argument=0x9900))
        for start,end in ((0,0),(0x1000,0x1800),(0xffff,0),(0x1880,0x4000)):
            case(f'initialize-{start:x}-{end:x}',fixture(seed=start+end),config(7,profile_start=start,profile_end=end),0)
        for blocks in (0,1,0x40,0x10000,0x08000000,0xffffffff):
            s=fixture(seed=blocks)
            for i in range(32):struct.pack_into('<QI',s['manager'],72+i*0x218,0x8000+i,17+i)
            after=case(f'configure-reservation-{blocks:x}',s,config(8,reserve_blocks=blocks),1)
            check(after['pool'][64:64+0x2800]==s['pool'][64:64+0x2800],'reconfiguration preserves records')
            check(after['pool'][64+0x2800:64+0xca00]==s['pool'][64+0x2800:64+0xca00],'reconfiguration preserves occupancy')
            for i in range(32):
                p=72+i*0x218
                check(after['manager'][p+12:p+0x218]==s['manager'][p+12:p+0x218],'reconfiguration preserves entry padding/payload')
        s=case('initialize-before-configure',fixture(),config(7),0)
        check(struct.unpack_from('<I',s['pool'],64+0xcb0c)[0]==0,'initializer does not set scan limit')
        s=case('configure-after-initialize',s,config(8,reserve_blocks=2),1)
        case('materialize-after-initialize-and-configure',s,config(1),1)
        guards=[]
        for length in (0,-1):
            s=fixture();prepare_record(s,palette=False,length=length)
            _,result,_=cpp(s,config(mode=1));check(result==-0x70000000,'host length guard')
            guards.append(dict(image_length=length,status='HOST_GUARD_ONLY'))
        s=fixture();prepare_record(s,palette=False)
        _,result,_=cpp(s,config(index=0x7fffffff,mode=1));check(result==-0x70000000,'host address-view guard')
        guards.append(dict(index=0x7fffffff,status='HOST_GUARD_ONLY'))
    coverage=[dict(site=hex(a),fallthrough=hex(dest[0]),target=hex(dest[1]),
                   fallthrough_observed=(a,dest[0]) in covered,target_observed=(a,dest[1]) in covered)
              for a,dest in sorted(branches.items())]
    missing=[r for r in coverage if not(r['fallthrough_observed'] and r['target_observed'])]
    sources += ['include/dmc_rengine/reverse/ptx_record_placement.hpp',
                'include/dmc_rengine/reverse/ptx_record_materializer.hpp','include/dmc_rengine/reverse/ptx_payload_state.hpp',
                'include/dmc_rengine/reverse/ptx_manager_state.hpp','scripts/reverse/verify_ptx_placement.py',
                'scripts/reverse/verify_ptx_materializer.py','scripts/reverse/verify_ptx_payload.py',
                'scripts/reverse/canonical_image.py','scripts/reverse/verify_crt_lifecycle.py']
    result=dict(status='PASS',sha256=CANONICAL,cases=len(cases),case_results=cases,host_guards=guards,
        placement_conditional_edges=coverage,conditional_sites=len(branches),sites_with_both_outcomes=len(coverage)-len(missing),
        unicorn_version=unicorn.__version__,compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        real_internal_entries=[hex(p) for p in sorted(set(ROOTS.values())|{0x1403313f0,0x1403310f0,0x140330f60,0x140331420,0x1403317d0,0x140315150}) if p in seen],
        limitations=['Direct placement executes original instructions without intercepted callees.',
          'Integration boundaries: palette helper 0x140331bd0, parser 0x1403365b0, render finalizer 0x140331a80 and checked CRT memset.',
          'Initializer memset contract is 0xcb50 zero bytes; record clear contract is 0x50.',
          'Configuration object fields are synthetic explicit inputs; live game profile values and caller ordering remain open.',
          'Host guards reject unrepresented accesses/lengths; they are not original EXE validation.',
          'No GPU acceptance, Windows SEH, aliasing, concurrency or reentrant parity.'])
    out.mkdir(parents=True,exist_ok=True)
    (out/'evidence.json').write_text(json.dumps(facts,indent=2)+'\n')
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    return {k:v for k,v in result.items() if k not in ('case_results','source_sha256','placement_conditional_edges')},missing


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();summary,missing=run(a.exe,a.repo.resolve(),a.out)
    print(json.dumps(dict(**summary,missing_branch_outcomes=missing),indent=2))
