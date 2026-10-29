#!/usr/bin/env python3
"""Differential palette preparation with real EXE copy loops and PTX callers.

Only the imported memmove is implemented by a host byte snapshot/move; the
original count-conversion wrapper runs. Outer boundaries remain parser,
render finalizer and checked CRT record memset. No palette/placement stubs.
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
from verify_crt_lifecycle import emulator, STACK, STOP
from verify_ptx_payload import POOL, PAYLOAD, SOURCE, MANAGER, MASK, digest, check
from verify_ptx_materializer import INPUT
from verify_ptx_placement import PROFILE, CONFIG_SLOT, fixture as placement_fixture

CONTEXT=0x55000040
DEST=0x56000040
BANK13=0x57000040
BANK14=0x58000040
MEMMOVE=0x60000000
KEYS=('pool','input','payload','manager','source','context','destination','bank13','bank14','profile')
BASES=dict(zip(KEYS,(POOL,INPUT,PAYLOAD,MANAGER,SOURCE,CONTEXT,DEST,BANK13,BANK14,PROFILE)))
ROOTS={0:0x140331bd0,1:0x1403366e0,2:0x140336bb0,3:0x140336a70,
       4:0x140314e00,5:0x140314fa0,6:0x140315180}
WINDOWS={0x140331bd0:0x140331d90,0x14032d3d0:0x14032d3db}


def config(op=0,**kwargs):
    c=[op,0xffffffff,0,CONTEXT,POOL,0x1000,0x1800,SOURCE]
    for key,value in kwargs.items():
        c[{'selector':1,'home':2,'argument':3,'record':4,'profile_start':5,'profile_end':6,'key':7}[key]]=value&MASK
    return c


def word(s,key,offset,value):struct.pack_into('<H',s[key],64+offset,value&0xffff)
def qword(s,key,offset,value):struct.pack_into('<Q',s[key],64+offset,value&MASK)
def u16(s,key,offset):return struct.unpack_from('<H',s[key],64+offset)[0]


def fixture(fmt=0x14,selector=-1,next_index=0,capacity=128,levels=2,textures=2,seed=19):
    s=placement_fixture(levels=levels,palette=True,seed=seed,textures=textures)
    rng=random.Random(seed+271)
    for key,size in (('context',0x58),('destination',0x2000),('bank13',0x8000),('bank14',0x8000)):
        s[key]=bytearray(rng.randbytes(size+128))
    struct.pack_into('<HH',s['profile'],64+0x4c,0x1000,0x1800)
    for bank,base in enumerate((BANK13,BANK14)):
        rec=(126+bank)*0x50
        s['pool'][64+rec:64+rec+0x50]=bytes(0x50)
        word(s,'pool',rec,1);word(s,'pool',rec+2,126+bank)
        word(s,'pool',rec+6,0x2000+bank*0x400)
        qword(s,'pool',rec+0x10,base+0x2000)
        qword(s,'context',8*bank,POOL+rec)
        word(s,'context',0x10+2*bank,next_index);word(s,'context',0x14+2*bank,capacity)
    word(s,'pool',4,fmt);word(s,'pool',0x1a,selector);word(s,'pool',0x1e,0x7777)
    qword(s,'pool',0x28,DEST+0x200)
    word(s,'input',0x10,fmt);qword(s,'input',8,DEST+0x200)
    return s


def copy_delta(fmt,value):
    # Fixture placement only: keeps even extreme signed-index accesses inside
    # small mapped buffers; differential assertions never derive output here.
    index=ctypes.c_int16(value&0xffff).value
    divisor=8 if fmt==0x14 else 4
    quotient=abs(index)//divisor*(-1 if index<0 else 1)
    remainder=index-quotient*divisor
    return quotient*(2048 if fmt==0x14 else 4096)+remainder*(32 if fmt==0x14 else 64)


def position_bank(s,fmt,index):
    bank=int(fmt==0x14)
    qword(s,'pool',(126+bank)*0x50+0x10,(BANK14 if bank else BANK13)+0x2000-copy_delta(fmt,index))


def native(image,s,cfg,branches):
    u=emulator(image)
    for base in range(0x50000000,0x59000000,0x1000000):u.mem_map(base,0x20000)
    u.mem_map(MEMMOVE,0x1000)
    for key,base in BASES.items():u.mem_write(base-64,bytes(s[key]))
    u.mem_write(CONFIG_SLOT,struct.pack('<Q',PROFILE))
    slot,symbol=image.import_thunk(0x14032d3d4)
    check(symbol=='VCRUNTIME140.dll!memmove','canonical runtime import')
    u.mem_write(slot,struct.pack('<Q',MEMMOVE)) # Normal simulated IAT resolution, no code patch.
    # Explicit initial machine state, before running any caller instructions.
    u.mem_write(STACK,struct.pack('<I',cfg[2]&0xffffffff)*(0x10000//4))
    u.mem_write(STACK+0x8008,struct.pack('<Q',STOP))
    op=cfg[0]
    u.reg_write(R.UC_X86_REG_RCX,cfg[4] if op==0 else INPUT if op==1 else SOURCE if op<4 else MANAGER)
    u.reg_write(R.UC_X86_REG_RDX,cfg[3] if op==0 else cfg[1] if op==1 else PAYLOAD if op<4 else cfg[7])
    u.reg_write(R.UC_X86_REG_R8,cfg[3])
    events=[];reached=set();edges=set();pending=None;textures=0;clears=[];homes=[]
    def ret(result):
        rsp=u.reg_read(R.UC_X86_REG_RSP);target=struct.unpack('<Q',u.mem_read(rsp,8))[0]
        for n,r in enumerate((R.UC_X86_REG_RCX,R.UC_X86_REG_RDX,R.UC_X86_REG_R8,
                               R.UC_X86_REG_R9,R.UC_X86_REG_R10,R.UC_X86_REG_R11)):
            u.reg_write(r,0x61000000+n*0x100)
        u.reg_write(R.UC_X86_REG_RAX,result&MASK)
        u.reg_write(R.UC_X86_REG_RSP,rsp+8);u.reg_write(R.UC_X86_REG_RIP,target)
    def buffer(address,length):
        check(any(BASES[k]<=address and address+length<=BASES[k]+len(s[k])-128
                  for k in ('destination','bank13','bank14')),'bounded imported memmove')
        return bytes(u.mem_read(address,length))
    def hook(uc,addr,size,_):
        nonlocal pending,textures
        reached.add(addr)
        if pending is not None:edges.add((pending,addr))
        pending=addr if addr in branches else None
        if addr==ROOTS[0]:
            p=uc.reg_read(R.UC_X86_REG_RCX);argument=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([2,p,argument,digest(uc.mem_read(p,0x50)),0])
            home=struct.unpack('<I',uc.mem_read(uc.reg_read(R.UC_X86_REG_RSP)+8,4))[0]
            homes.append(home)
            fmt,selector=struct.unpack('<H',uc.mem_read(p+4,2))[0],struct.unpack('<h',uc.mem_read(p+0x1a,2))[0]
            if op==0 or selector>=0 or fmt not in (0x13,0x14):
                check(home==cfg[2]&0xffffffff,'incoming home state matches explicit C++ input')
        elif addr==MEMMOVE:
            dest=uc.reg_read(R.UC_X86_REG_RCX);source=uc.reg_read(R.UC_X86_REG_RDX);length=uc.reg_read(R.UC_X86_REG_R8)
            check(length in (32,64),'executed wrapper byte conversion')
            before=buffer(source,length);buffer(dest,length)
            events.append([5,dest,source,length,digest(before)])
            uc.mem_write(dest,before);ret(dest)
        elif addr==0x140331520:
            check(uc.reg_read(R.UC_X86_REG_RCX)==POOL,'real placement receiver')
            p=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([1,p,uc.reg_read(R.UC_X86_REG_R8)&0xffffffff,
                           uc.reg_read(R.UC_X86_REG_R9)&0xffffffff,digest(uc.mem_read(p,0x50))])
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
            check(uc.reg_read(R.UC_X86_REG_RDX)&255==0 and length==0x50 and POOL<=p<=POOL+0x27b0,'record memset contract')
            clears.append(p);uc.mem_write(p,bytes(length));ret(p)
    u.hook_add(unicorn.UC_HOOK_CODE,hook);u.emu_start(ROOTS[op],STOP,count=250000)
    check(u.reg_read(R.UC_X86_REG_RIP)==STOP,'bounded normal return')
    rax=u.reg_read(R.UC_X86_REG_RAX)
    result=(rax-MANAGER if rax else -1) if op in (4,5) else rax&255 if op in (0,6) else ctypes.c_int32(rax&0xffffffff).value
    return {k:bytes(u.mem_read(base-64,len(s[k]))) for k,base in BASES.items()},result,events,reached,edges,clears,homes


def evidence(image):
    rows=[];branches={}
    for root,end in WINDOWS.items():
        ins=list(image.md.disasm(image.raw(root,end-root),root))
        check(ins[-1].address+ins[-1].size==end,'complete reviewed instruction window')
        for i in ins:
            if i.group(capstone.CS_GRP_JUMP) and i.mnemonic!='jmp':branches[i.address]=(i.address+i.size,i.operands[0].imm)
        rows.append(dict(root=hex(root),end=hex(end),file_offset=hex(image.offset(root,end-root)),
            code_sha256=hashlib.sha256(image.raw(root,end-root)).hexdigest(),
            instructions=[dict(address=hex(i.address),bytes=i.bytes.hex(),mnemonic=i.mnemonic,operands=i.op_str) for i in ins]))
    tables=[dict(address=hex(a),file_offset=hex(image.offset(a,n)),bytes=image.raw(a,n).hex(),
                 sha256=hashlib.sha256(image.raw(a,n)).hexdigest(),nominal_start=hex(start))
            for a,n,start in ((0x1405d16d1,63,0x1405d16f0),(0x1405d1709,15,0x1405d1710))]
    slot,symbol=image.import_thunk(0x14032d3d4)
    support=[]
    for root,end in ((0x140331180,0x140331341),(0x140331460,0x140331514),
                     (0x140245d40,0x140245da0),(0x1402c0350,0x1402c03d8),(0x1402c051c,0x1402c05ab)):
        support.append(dict(root=hex(root),end=hex(end),status='STATIC_ONLY_NOT_DIFFERENTIALLY_EXECUTED',
            code_sha256=hashlib.sha256(image.raw(root,end-root)).hexdigest(),
            instructions=[dict(address=hex(i.address),bytes=i.bytes.hex(),mnemonic=i.mnemonic,operands=i.op_str)
                for i in image.md.disasm(image.raw(root,end-root),root)]))
    callers=[]
    for site,target in ((0x140336855,0x140331bd0),(0x140245d67,0x140331460),
                        (0x1402c035b,0x140331460),(0x1402c0594,0x140331180)):
        i=image.instruction(site)
        check(i.mnemonic=='call' and i.operands[0].imm==target,'reviewed direct call site')
        callers.append(dict(site=hex(site),target=hex(target),bytes=i.bytes.hex()))
    return dict(sha256=CANONICAL,functions=rows,tables=tables,
        copy_import=dict(slot=hex(slot),symbol=symbol,wrapper='0x14032d3d0'),
        home_reads=['0x140331c18','0x140331c95','0x140331d73'],
        home_origin='ENTRY_RSP+8; prologue delta -0x38; reads [RSP+0x40]; no preceding local store.',
        caller='0x140336855 -> 0x140331bd0; RCX=allocated record, RDX=materializer argument.',
        context_prefix={'bank_records':'+0x00/+0x08','next_u16':'+0x10/+0x12','capacity_u16':'+0x14/+0x16'},
        context_lifecycle_leads={'initialize':'0x140331180','destroy':'0x140331460','allocator_members':'+0x18/+0x38'},
        static_context_support=support,direct_calls=callers,
        global_context_path={'address':'0x140cf1030','initialize_site':'0x1402c0594',
            'initialize_arguments':[16,32],'destroy_site':'0x1402c035b',
            'status':'STATIC_CONFIRMED; reachability and runtime ordering remain open'},
        embedded_context_path={'context_offset':'0x5e0','guard_byte_offset':'0x638','destroy_site':'0x140245d67'},
        status='EXE_CONFIRMED_WITH_RUNTIME_BOUNDARIES'),branches


def run(exe,repo,out):
    image=CanonicalImage(exe);facts,branches=evidence(image);cases=[];seen=set();covered=set()
    with tempfile.TemporaryDirectory(prefix='ptx-palette-') as temp:
        library=Path(temp)/'palette.so'
        sources=['src/reverse/ptx_palette_state.cpp','src/reverse/ptx_record_placement.cpp',
            'src/reverse/ptx_record_materializer.cpp','src/reverse/ptx_payload_state.cpp',
            'src/reverse/ptx_manager_state.cpp','tests/reverse/ptx_palette_bridge.cpp']
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Wconversion','-Werror',
            '-I'+str(repo/'include'),*[str(repo/p) for p in sources],'-o',str(library)],check=True)
        fn=ctypes.CDLL(str(library)).ptx_palette_step
        fn.argtypes=[ctypes.POINTER(ctypes.c_void_p),ctypes.POINTER(ctypes.c_size_t),
                     ctypes.POINTER(ctypes.c_uint64),ctypes.POINTER(ctypes.c_uint64)]
        fn.restype=ctypes.c_int64
        def cpp(s,c):
            b={k:ctypes.create_string_buffer(bytes(s[k]),len(s[k])) for k in KEYS}
            pointers=(ctypes.c_void_p*len(KEYS))(*(ctypes.addressof(b[k])+64 for k in KEYS))
            sizes=(ctypes.c_size_t*len(KEYS))(*(len(s[k])-128 for k in KEYS))
            cfg=(ctypes.c_uint64*len(c))(*c);log=(ctypes.c_uint64*2048)()
            result=fn(pointers,sizes,cfg,log)
            return {k:v.raw for k,v in b.items()},result,[list(log[1+5*i:6+5*i]) for i in range(log[0])]
        def case(name,s,c,expected_result=None):
            actual,result,events,reached,edges,clears,homes=native(image,s,c,branches)
            other,cresult,cevents=cpp(s,c)
            check(result==cresult,f'{name}: return {result}/{cresult}')
            if expected_result is not None:check(result==expected_result,f'{name}: expected observation {result}/{expected_result}')
            check(events==cevents,f'{name}: call/memmove events differ {events}/{cevents}')
            for key in KEYS:
                if actual[key]!=other[key]:
                    pos=next(i for i,(a,b) in enumerate(zip(actual[key],other[key])) if a!=b)
                    raise AssertionError(f'{name}: {key} at {pos-64:#x}: EXE {actual[key][pos]:02x}, C++ {other[key][pos]:02x}')
                check(actual[key][:64]==s[key][:64] and actual[key][-64:]==s[key][-64:],f'{name}: {key} guards')
            for key in ('input','source','profile'):check(actual[key]==bytes(s[key]),f'{name}: immutable fixture {key}')
            check(actual['context'][64+0x18:]==bytes(s['context'][64+0x18:]),f'{name}: untouched allocator tail')
            if c[0]==0 and result==0:check(all(actual[k]==bytes(s[k]) for k in KEYS),f'{name}: exhausted helper has no writes')
            seen.update(reached);covered.update(edges)
            cases.append(dict(name=name,entry=hex(ROOTS[c[0]]),result=result,byte_equal=True,
                selector=u16(actual,'pool',0x1a),palette_base=u16(actual,'pool',0x1e),
                next=[u16(actual,'context',0x10+2*i) for i in range(2)],events=events,
                input_home_words=homes,record_clears=[hex(p) for p in clears],
                state_sha256={k:hashlib.sha256(v).hexdigest() for k,v in actual.items()}))
            return {k:bytearray(v) for k,v in actual.items()}
        for fmt,period in ((0x14,32),(0x13,8)):
            for index in list(range(period))+[period,period+1,255,32767]:
                s=fixture(fmt,next_index=index,capacity=0xffff);position_bank(s,fmt,index)
                case(f'auto-format-{fmt:x}-index-{index}',s,config(home=0x1234ffff),1)
            for index in range(period):
                home=0x13570000|((index+3)&0xffff)
                s=fixture(fmt,selector=index,next_index=0xffff,capacity=0);position_bank(s,fmt,home)
                case(f'explicit-format-{fmt:x}-selector-{index}-different-home',s,config(home=home),1)
            for next_index,capacity in ((0,0),(7,7),(8,7),(0xffff,0xffff),(0xfffe,0xffff),(0x8000,0x8001),
                                         (0x801f,0xffff),(0xffdf,0xffff),(0xfff7,0xffff)):
                s=fixture(fmt,next_index=next_index,capacity=capacity);position_bank(s,fmt,next_index)
                case(f'unsigned-capacity-{fmt:x}-{next_index:x}-{capacity:x}',s,config(),int(next_index<capacity))
            for index in range(-31,-1):
                s=fixture(fmt,next_index=index,capacity=0xffff);position_bank(s,fmt,index)
                case(f'negative-auto-index-{fmt:x}-{index}',s,config(),1)
            for home in (0x8000,0xfffd,0xffff,0xdead0000):
                s=fixture(fmt,selector=5);position_bank(s,fmt,home)
                case(f'signed-home-{fmt:x}-{home:x}',s,config(home=home),1)
            for base in (0x7fff,0x8000,0xffff):
                s=fixture(fmt,next_index=33);position_bank(s,fmt,33)
                word(s,'pool',(126+int(fmt==0x14))*0x50+6,base)
                case(f'base-word-wrap-{fmt:x}-{base:x}',s,config(),1)
            for delta in (-8,0,8,0x80,0x100):
                s=fixture(fmt,next_index=3);position_bank(s,fmt,3)
                qword(s,'pool',0x28,(BANK14 if fmt==0x14 else BANK13)+0x2000+delta)
                case(f'overlapping-ordered-memmoves-{fmt:x}-{delta}',s,config(),1)
        for fmt in (0,1,2,0x12,0x15,0xffff):
            s=fixture(fmt);qword(s,'context',0,0);qword(s,'context',8,MASK);qword(s,'pool',0x28,0)
            case(f'other-format-{fmt:x}-home-fallback-no-dereference',s,config(home=0x89abcdef),1)
        for fmt in (0x13,0x14):
            for op in (1,3,5):
                case(f'integration-{op}-{fmt:x}-success',fixture(fmt,textures=3),config(op),0x18 if op==5 else 1)
                case(f'integration-{op}-{fmt:x}-first-palette-exhausted',fixture(fmt,capacity=0),config(op),-1 if op==5 else 0)
                if op!=1:
                    after=case(f'integration-{op}-{fmt:x}-second-palette-exhausted',fixture(fmt,capacity=1),config(op),-1 if op==5 else 0)
                    check(u16(after,'pool',0)==0 and u16(after,'pool',0x50)==0 and u16(after,'pool',0xa0)==1,
                          'outer cleanup releases previous texture but retains current failed record')
                    check(u16(after,'context',0x10+2*int(fmt==0x14))==1,'previous palette reservation is retained')
                s=fixture(fmt);s['pool'][64+0x2800:64+0x2840]=bytes([1])*64
                after=case(f'integration-{op}-{fmt:x}-placement-failure-after-copy',s,config(op),-1 if op==5 else 0)
                check(u16(after,'pool',0)==0,'failed placement releases record')
                check(u16(after,'context',0x10+2*int(fmt==0x14))==1,'placement rollback does not undo palette counter')
                check(after['destination']!=s['destination'],'placement rollback retains copied bytes')
            for selector,home in ((0,5),(5,0xfffe)):
                s=fixture(fmt);position_bank(s,fmt,home)
                case(f'materializer-explicit-{fmt:x}-{selector}-{home}',s,config(1,selector=selector,home=home),1)
            s=fixture(fmt,textures=2)
            for step,op in enumerate((5,4,6,6,5)):
                s=case(f'cache-palette-lifecycle-{fmt:x}-{step}',s,config(op))
            check(u16(s,'context',0x10+2*int(fmt==0x14))==4,'cache hit and release do not reclaim palette counter')
        guards=[]
        for name,offset,value in (('unrepresented-bank-record',8,POOL-1),):
            s=fixture();qword(s,'context',offset,value)
            _,result,_=cpp(s,config());check(result==-0x70000000,name)
            guards.append(dict(name=name,status='HOST_GUARD_ONLY'))
        s=fixture();qword(s,'pool',0x28,DEST+0x2000)
        _,result,_=cpp(s,config());check(result==-0x70000000,'unrepresented-copy-destination')
        guards.append(dict(name='unrepresented-copy-destination',status='HOST_GUARD_ONLY'))
    coverage=[dict(site=hex(a),fallthrough=hex(d[0]),target=hex(d[1]),
        fallthrough_observed=(a,d[0]) in covered,target_observed=(a,d[1]) in covered) for a,d in sorted(branches.items())]
    check(all(c['fallthrough_observed'] and c['target_observed'] for c in coverage),'all helper conditional outcomes observed')
    sources += ['include/dmc_rengine/reverse/ptx_palette_state.hpp','include/dmc_rengine/reverse/ptx_record_placement.hpp',
        'include/dmc_rengine/reverse/ptx_record_materializer.hpp','include/dmc_rengine/reverse/ptx_payload_state.hpp',
        'include/dmc_rengine/reverse/ptx_manager_state.hpp','scripts/reverse/verify_ptx_palette.py',
        'scripts/reverse/verify_ptx_placement.py','scripts/reverse/verify_ptx_materializer.py',
        'scripts/reverse/verify_ptx_payload.py','scripts/reverse/canonical_image.py','scripts/reverse/verify_crt_lifecycle.py']
    result=dict(status='PASS',sha256=CANONICAL,cases=len(cases),case_results=cases,host_guards=guards,
        conditional_edges=coverage,conditional_sites=len(branches),sites_with_both_outcomes=len(coverage),
        unicorn_version=unicorn.__version__,compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        real_internal_entries=[hex(p) for p in sorted(set(ROOTS.values())|{0x14032d3d0,0x140331520,0x1403313f0,
            0x1403310f0,0x140330f60,0x140331420,0x1403317d0}) if p in seen],
        limitations=['Original palette helper and copy wrapper execute; only imported memmove is modeled with ordered byte moves.',
          'Outer boundaries: parser 0x1403365b0, render finalizer 0x140331a80 and checked CRT 80-byte memset.',
          'Home slot is explicit initial machine state; it is not normalized into a valid source-language parameter.',
          'Signed negative table indexes preserve canonical neighboring bytes, without assigning them palette semantics.',
          'Caller fixtures keep input/source separate from writable palette data; source immutability is not a universal backend claim.',
          'Data-buffer overlap is covered; aliasing with pool/context, native faults, Windows SEH, GPU runtime and concurrency are not.',
          'Palette context constructor/allocator/destructor are statically located, not yet C++/EXE verified.',
          'No claim of global permanent resource leakage; later context/scene teardown remains open.'])
    out.mkdir(parents=True,exist_ok=True)
    (out/'evidence.json').write_text(json.dumps(facts,indent=2)+'\n')
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    return {k:v for k,v in result.items() if k not in ('case_results','source_sha256','conditional_edges')}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo.resolve(),a.out),indent=2))
