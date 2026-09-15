#!/usr/bin/env python3
"""Recover CPtxManager image-state contracts and compare C++ with canonical code.

Constructors/array helpers/destructors and cookie checks execute unmodified in
Unicorn. Only the named loader/release/deallocator/exit services are intercepted.
These tests do not establish the internals of the intercepted services.
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

import unicorn
from unicorn import x86_const as R
from canonical_image import CANONICAL, CanonicalImage
from audit_crt_lifecycle import body
from verify_crt_lifecycle import emulator, STACK, STOP

BASE=0x140d5b860
SIZE=0x4308
STRIDE=0x218
ENTRY={0:0x140314c50,1:0x140315150,2:0x140314e00,3:0x140314fa0,
       4:0x140315180,5:0x140314d00,6:0x140314d60}
SERVICES={0x140336bb0:1,0x140336a70:2,0x1403317d0:3,0x140345554:4,0x1403458d0:5}


def check(condition,message):
    if not condition:raise AssertionError(message)


def pattern_bytes(pattern):return bytes((pattern+i*17)&255 for i in range(0x208))


def native(image,payload,op,key,arg,flags,loaded,pattern,entry_override=None):
    u=emulator(image);u.mem_write(BASE-64,payload)
    u.reg_write(R.UC_X86_REG_RCX,BASE)
    u.reg_write(R.UC_X86_REG_RDX,flags if op==6 else key)
    u.reg_write(R.UC_X86_REG_R8,arg)
    log=[0]*9;constructed=[];destroyed=[];registrations=[];stores=[]
    reached=set()
    def code_hook(uc,addr,size,_):
        reached.add(addr)
        if addr==0x140314cb0:constructed.append(uc.reg_read(R.UC_X86_REG_RCX))
        if addr==0x14024ea30:
            receiver=uc.reg_read(R.UC_X86_REG_RCX)
            if BASE+8<=receiver<BASE+SIZE:destroyed.append(receiver)
        kind=SERVICES.get(addr)
        if kind is None:return
        rcx=uc.reg_read(R.UC_X86_REG_RCX);rdx=uc.reg_read(R.UC_X86_REG_RDX)
        r8=uc.reg_read(R.UC_X86_REG_R8)
        if kind==5:
            registrations.append(rcx);result=0
        else:
            log[0]+=1;log[1]=kind;result=0
            if kind in (1,2):
                log[2]=rcx;log[3]=r8 if kind==2 else 0
                log[6]=int(bytes(uc.mem_read(rdx,0x208))==bytes(0x208))
                uc.mem_write(rdx,pattern_bytes(pattern));result=loaded & 0xffffffffffffffff
            elif kind==3:
                offset=rcx-BASE
                check(0x18<=offset<SIZE and (offset-0x18)%STRIDE==0,'release payload address')
                log[4]=offset;log[5]=0x208
                log[7]=struct.unpack('<Q',uc.mem_read(rcx-0x10,8))[0]
                log[8]=struct.unpack('<I',uc.mem_read(rcx-8,4))[0]
                uc.mem_write(rcx,pattern_bytes(0x5a))
            else:
                check(rcx==BASE,'delete receiver')
                log[5]=rdx;log[7]=struct.unpack('<Q',uc.mem_read(BASE,8))[0]
        uc.reg_write(R.UC_X86_REG_RAX,result)
        rsp=uc.reg_read(R.UC_X86_REG_RSP)
        uc.reg_write(R.UC_X86_REG_RIP,struct.unpack('<Q',uc.mem_read(rsp,8))[0])
        uc.reg_write(R.UC_X86_REG_RSP,rsp+8)
    def write_hook(uc,access,addr,size,value,_):
        if not STACK<=addr<STACK+0x10000:stores.append((addr,size))
    u.hook_add(unicorn.UC_HOOK_CODE,code_hook)
    u.hook_add(unicorn.UC_HOOK_MEM_WRITE,write_hook)
    u.emu_start(entry_override or ENTRY[op],STOP,count=150000)
    check(u.reg_read(R.UC_X86_REG_RIP)==STOP,'bounded normal return')
    rax=u.reg_read(R.UC_X86_REG_RAX)
    result=(rax-BASE if rax else -1) if op in (2,3) else (rax&255 if op==4 else 0)
    if op in (0,6) and entry_override is None:check(rax==BASE,'returned this identity')
    if op==0:
        check(constructed==[BASE+8+i*STRIDE for i in range(32)],'real array constructor order')
        check(len(stores)==2081,'constructor writes: one vptr plus 32 x 65 qwords')
    if op in (5,6):
        check(destroyed==[BASE+8+i*STRIDE for i in reversed(range(32))],'real reverse destructor order')
        check(len(stores)==2,'destructor only writes derived/base vptr')
    if entry_override==0x140025050:check(registrations==[0x14034eca0],'actual global exit registration')
    return bytes(u.mem_read(BASE-64,len(payload))),result,log,reached


def make_state(seed,full=False):
    payload=bytearray(random.Random(seed).randbytes(SIZE+128))
    struct.pack_into('<Q',payload,64,0x140507b60)
    for i in range(32):put(payload,i,0x1000+8*i if full else 0,1 if full else 0)
    return payload


def put(payload,index,key,count):
    struct.pack_into('<QI',payload,64+8+index*STRIDE,key,count)


def run(exe,repo,out):
    image=CanonicalImage(exe)
    check(image.u64(0x14034f7f8)==0x14024ea30,'on-disk CFG guard target')
    check(image.instruction(0x14024ea30).mnemonic.startswith('ret'),'guard/element callback no-op')
    check(image.u64(0x140507b60)==ENTRY[6],'primary vtable first entry')
    check([image.u64(0x140507b68+8*i) for i in range(4)]==[ENTRY[i] for i in (1,3,2,4)],'four stored method targets')
    facts=dict(schema='dmc3-ptx-manager-state-v1',sha256=CANONICAL,global_va=hex(BASE),
        object_size=SIZE,entry_count=32,entry_stride=STRIDE,payload_size=0x208,
        field_map={'manager+0x00':'canonical vtable address tag','manager+0x08':'32 cache entries',
                   'entry+0x00':'64-bit resource identity key','entry+0x08':'32-bit reference count',
                   'entry+0x0c':'4 bytes preserved, meaning unknown','entry+0x10':'0x208-byte payload, internal fields not decoded'},
        methods={str(op):hex(va) for op,va in ENTRY.items()},
        external_services={hex(k):v for k,v in SERVICES.items()},
        direct_global_consumers=[],body_evidence=[])
    for lea,call,target in [(0x1400897bd,0x1400897c4,ENTRY[4]),(0x140089991,0x140089998,ENTRY[2]),
                            (0x140331dcf,0x140331dd6,ENTRY[1])]:
        i=image.instruction(lea);j=image.instruction(call)
        check(i.mnemonic=='lea' and lea+i.size+i.operands[1].mem.disp==BASE,'global receiver')
        check(j.mnemonic=='call' and j.operands[0].imm==target,'global method edge')
        facts['direct_global_consumers'].append(dict(lea_site=hex(lea),call_site=hex(call),target=hex(target)))
    for va in sorted(set(ENTRY.values())|{0x140314cb0,0x140345b24,0x140345b94,0x140346714,0x14024ea30}):
        ins,succ,terms,end=body(image,va,set())
        facts['body_evidence'].append(dict(entry=hex(va),instruction_addresses=[hex(a) for a in sorted(ins)],
            sha256=hashlib.sha256(b''.join(bytes(i.bytes) for _,i in sorted(ins.items()))).hexdigest()))
    cases=[];all_reached=set()
    with tempfile.TemporaryDirectory(prefix='dmc-ptx-') as temp:
        libpath=Path(temp)/'ptx.so'
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Wconversion','-Werror',
            '-I'+str(repo/'include'),str(repo/'src/reverse/ptx_manager_state.cpp'),
            str(repo/'tests/reverse/ptx_manager_bridge.cpp'),'-o',str(libpath)],check=True)
        fn=ctypes.CDLL(str(libpath)).ptx_step
        fn.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_uint64,ctypes.c_uint64,ctypes.c_uint32,
                     ctypes.c_int,ctypes.c_uint,ctypes.POINTER(ctypes.c_uint64)]
        fn.restype=ctypes.c_int64
        def case(name,payload,op,key=0,arg=0,flags=0,loaded=1,pattern=0xa6,entry_override=None):
            actual,result,events,reached=native(image,bytes(payload),op,key,arg,flags,loaded,pattern,entry_override)
            buf=ctypes.create_string_buffer(bytes(payload),len(payload));log=(ctypes.c_uint64*9)()
            cpp_result=fn(buf,op,key,arg,flags,loaded,pattern,log)
            if actual!=buf.raw:
                pos=next(i for i,(a,b) in enumerate(zip(actual,buf.raw)) if a!=b)
                raise AssertionError(f'{name}: memory mismatch at state offset {pos-64:#x}: EXE={actual[pos]:02x} C++={buf.raw[pos]:02x}')
            check(result==cpp_result,f'{name}: return mismatch {result}/{cpp_result}')
            check(events==list(log),f'{name}: external service mismatch {events}/{list(log)}')
            all_reached.update(reached)
            cases.append(dict(name=name,entry=hex(entry_override or ENTRY[op]),byte_equal=True,
                              result=result,external_service_calls=events[0],service_kind=events[1]))
            return bytearray(actual)
        for seed in (1,20260915,0xffff):
            raw=bytearray(random.Random(seed).randbytes(SIZE+128))
            case(f'construct-preserve-headers-{seed}',raw,0)
            case(f'reset-preserve-payloads-{seed}',make_state(seed,True),1)
        case('global-CRT-initializer',make_state(2),0,entry_override=0x140025050)
        case('registered-global-cleanup',make_state(2,True),5,entry_override=0x14034eca0)
        case('direct-cleanup-preserves-live-payloads',make_state(3,True),5)
        for flags in (0,1,2,3,0x80000001,0xffffffff):
            case(f'deleting-flags-{flags:x}',make_state(flags,True),6,flags=flags)
        for op in (2,3):
            case(f'acquire-{op}-null',make_state(8),op)
            case(f'acquire-{op}-full-miss',make_state(8,True),op,key=0xabcdef)
            for index in (0,5,31):
                for loaded in (0,1,-1):
                    p=make_state(11,True);put(p,index,0,0x11223344)
                    case(f'acquire-{op}-free-{index}-result-{loaded}',p,op,key=0xabcdef,
                         arg=0x123456789abcdef0,loaded=loaded)
            for index in (0,15,31):
                for refs in (0,1,0xffffffff):
                    p=make_state(12,True);put(p,index,0xabcdef,refs)
                    case(f'acquire-{op}-hit-{index}-refs-{refs:x}',p,op,key=0xabcdef,arg=0xfedcba9876543210)
            p=make_state(14);put(p,4,0xabcdef,7);put(p,20,0xabcdef,1)
            case(f'acquire-{op}-first-duplicate',p,op,key=0xabcdef)
        case('release-null',make_state(7,True),4)
        case('release-absent',make_state(7,True),4,key=0xabcdef)
        for index in (0,15,31):
            for refs in (0,1,2,0xffffffff):
                p=make_state(21,True);put(p,index,0xabcdef,refs)
                case(f'release-{index}-refs-{refs:x}',p,4,key=0xabcdef)
        p=make_state(22);put(p,4,0xabcdef,2);put(p,20,0xabcdef,1)
        case('release-first-duplicate',p,4,key=0xabcdef)
        p=make_state(30)
        for n,(op,arg) in enumerate(((2,0),(3,99),(4,0),(4,0),(3,101),(1,0),(2,0))):
            p=case(f'cross-variant-lifecycle-step-{n}',p,op,key=0xabcdef,arg=arg)
    out.mkdir(parents=True,exist_ok=True)
    (out/'evidence.json').write_text(json.dumps(facts,indent=2)+'\n')
    sources=['include/dmc_rengine/reverse/ptx_manager_state.hpp','src/reverse/ptx_manager_state.cpp',
             'tests/reverse/ptx_manager_bridge.cpp','scripts/reverse/verify_ptx_manager.py',
             'scripts/reverse/canonical_image.py','scripts/reverse/audit_crt_lifecycle.py','scripts/reverse/verify_crt_lifecycle.py']
    verification=dict(status='PASS',sha256=CANONICAL,cases=len(cases),case_results=cases,
        unicorn_version=unicorn.__version__,compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        normal_internal_helpers_executed=[hex(a) for a in (0x140345b24,0x140345b94,0x140346714,0x14024ea30,0x1403455f0) if a in all_reached],
        intercepted_service_entries=[hex(a) for a in SERVICES if a in all_reached],
        limitations=['Loaders, payload release, allocator and exit registry are explicit intercepted service boundaries.',
          'Full manager memory, guards, return results and service effects are compared on tested normal paths.',
          'Windows exception unwinding, allocator implementation, texture decoding/GPU effects and concurrent/reentrant calls remain open.',
          'Canonical vtable address is a data tag in the C++ image model, not a native host vtable.'])
    (out/'verification.json').write_text(json.dumps(verification,indent=2)+'\n')
    return {k:v for k,v in verification.items() if k not in ('case_results','source_sha256')}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo.resolve(),a.out),indent=2))
