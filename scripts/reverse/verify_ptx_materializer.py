#!/usr/bin/env python3
"""Differential PTX record materialization, including caller-visible failure state."""
import argparse
import ctypes
import hashlib
import json
import struct
import subprocess
import tempfile
from pathlib import Path

import capstone
import unicorn
from unicorn import x86_const as R
from canonical_image import CANONICAL, CanonicalImage
from verify_crt_lifecycle import emulator, STOP
from verify_ptx_payload import POOL, POOL_SIZE, PAYLOAD, SOURCE, MANAGER, MASK, digest, fixture, record, check

INPUT=0x53000040
ROOTS={0:0x1403366e0,1:0x1403310f0,2:0x140336bb0,3:0x140336a70,
       4:0x140314e00,5:0x140314fa0,6:0x140315180}


def config(op=0,**kwargs):
    c=[op,0xffffffff,0,1,MASK,0,SOURCE,POOL,1,0,0,0]+[2]*16
    for key,value in kwargs.items():
        c[{'selector':1,'argument':2,'palette_result':3,'place_fail':4,'failure_result':5,
           'key':6,'record':7,'place_result':8,'vary_count':9}[key]]=value&MASK
    return c


def state(levels=3,palette=False,free=None,seed=13,textures=2):
    s=fixture(seed,textures);s.pop('workspace')
    s['input']=bytearray(bytes([0xa7])*64+bytes(64)+bytes([0xb6])*64)
    struct.pack_into('<QQ',s['input'],64,0x9100000000,0x9200000000 if palette else MASK)
    for off,value in ((0x10,0x14),(0x12,8),(0x14,256),(0x16,128),(0x18,0x13),
                      (0x1a,0x1111),(0x1c,0x2222),(0x1e,0x3333),(0x3c,levels)):
        struct.pack_into('<H',s['input'],64+off,value&0xffff)
    struct.pack_into('<III',s['input'],64+0x2c,0x800,0x200,0x80)
    s['pool'][64+0x2800:64+0xca00]=bytes(0xa200)
    for i in range(128):struct.pack_into('<H',s['pool'],64+i*0x50,1)
    for i in (range(16) if free is None else free):s['pool'][64+i*0x50:64+(i+1)*0x50]=bytes(0x50)
    return s


def native(image,s,cfg):
    u=emulator(image)
    bases={'pool':POOL,'payload':PAYLOAD,'source':SOURCE,'manager':MANAGER,'input':INPUT}
    for base in (0x50000000,0x51000000,0x52000000,0x53000000):u.mem_map(base,0x20000)
    for key,base in bases.items():u.mem_write(base-64,bytes(s[key]))
    op=cfg[0];events=[];reached=set();placements=0;textures=0;clears=[]
    u.reg_write(R.UC_X86_REG_RCX,INPUT if op==0 else POOL if op==1 else SOURCE if op<4 else MANAGER)
    u.reg_write(R.UC_X86_REG_RDX,cfg[1] if op==0 else cfg[7] if op==1 else PAYLOAD if op<4 else cfg[6])
    u.reg_write(R.UC_X86_REG_R8,cfg[2])
    def ret(result):
        rsp=u.reg_read(R.UC_X86_REG_RSP);target=struct.unpack('<Q',u.mem_read(rsp,8))[0]
        for n,r in enumerate((R.UC_X86_REG_RCX,R.UC_X86_REG_RDX,R.UC_X86_REG_R8,
                               R.UC_X86_REG_R9,R.UC_X86_REG_R10,R.UC_X86_REG_R11)):
            u.reg_write(r,0x61000000+n*0x100)
        u.reg_write(R.UC_X86_REG_RAX,result&MASK)
        u.reg_write(R.UC_X86_REG_RSP,rsp+8);u.reg_write(R.UC_X86_REG_RIP,target)
    def hook(uc,addr,size,_):
        nonlocal placements,textures
        reached.add(addr)
        if addr==0x140331bd0:
            p=uc.reg_read(R.UC_X86_REG_RCX);argument=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([1,p,argument,digest(uc.mem_read(p,0x50)),0])
            uc.mem_write(p+0x1a,struct.pack('<H',7));uc.mem_write(p+0x1e,struct.pack('<H',0x3000))
            ret(cfg[3])
        elif addr==0x140331520:
            check(uc.reg_read(R.UC_X86_REG_RCX)==POOL,'placement pool receiver')
            p=uc.reg_read(R.UC_X86_REG_RDX)
            flags=(uc.reg_read(R.UC_X86_REG_R8)&0xffffffff)|((uc.reg_read(R.UC_X86_REG_R9)&0xffffffff)<<32)
            events.append([2,p,flags,digest(uc.mem_read(p,0x50)),placements])
            index=(p-POOL)//0x50
            uc.mem_write(p+6,struct.pack('<H',0x1000+index*0x80))
            uc.mem_write(p+0xe,struct.pack('<H',2));uc.mem_write(p+0x26,struct.pack('<H',2))
            if struct.unpack('<H',uc.mem_read(p+0x44,2))[0]==0 and struct.unpack('<H',uc.mem_read(p+0x18,2))[0]==0:
                uc.mem_write(p+0x1e,struct.pack('<H',0x2000))
            result=cfg[5] if cfg[4]==placements else cfg[8];placements+=1;ret(result)
        elif addr==0x1403365b0:
            source=uc.reg_read(R.UC_X86_REG_RCX);dest=uc.reg_read(R.UC_X86_REG_RDX)
            events.append([3,source,textures,0,0]);desc=bytearray(uc.mem_read(INPUT,64))
            if cfg[9]:struct.pack_into('<H',desc,0x3c,cfg[12+textures]&0xffff)
            uc.mem_write(dest,bytes(desc));textures+=1;ret(1)
        elif addr==0x140331a80:
            p=uc.reg_read(R.UC_X86_REG_RCX);raw=uc.mem_read(p,0x208)
            count,width=struct.unpack_from('<II',raw,0x200)
            events.append([4,count,width,digest(raw),0]);ret(1)
        elif addr==0x140346bea:
            p=uc.reg_read(R.UC_X86_REG_RCX);length=uc.reg_read(R.UC_X86_REG_R8)
            check(uc.reg_read(R.UC_X86_REG_RDX)&255==0 and length==0x50,'80-byte record clear')
            check(POOL<=p<=POOL+0x27b0,'record memset range');clears.append(p)
            uc.mem_write(p,bytes(length));ret(p)
    u.hook_add(unicorn.UC_HOOK_CODE,hook);u.emu_start(ROOTS[op],STOP,count=250000)
    check(u.reg_read(R.UC_X86_REG_RIP)==STOP,'bounded normal return')
    rax=u.reg_read(R.UC_X86_REG_RAX)
    result=0 if op==1 else (rax-MANAGER if rax else -1) if op in (4,5) else rax&255 if op==6 else ctypes.c_int32(rax&0xffffffff).value
    return {k:bytes(u.mem_read(base-64,len(s[k]))) for k,base in bases.items()},result,events,reached,clears


def evidence(image):
    functions=[]
    for root,end in ((0x1403366e0,0x1403368ec),(0x1403310f0,0x14033117d)):
        ins=list(image.md.disasm(image.raw(root,end-root),root))
        check(ins[-1].address+ins[-1].size==end,'complete static window')
        functions.append(dict(root=hex(root),end=hex(end),instruction_addresses=[hex(i.address) for i in ins],
            code_sha256=hashlib.sha256(image.raw(root,end-root)).hexdigest(),
            calls=[dict(site=hex(i.address),target=hex(i.operands[0].imm)) for i in ins
                   if i.group(capstone.CS_GRP_CALL) and i.operands[0].type==capstone.x86.X86_OP_IMM]))
    return dict(status='EXE_CONFIRMED',sha256=CANONICAL,functions=functions,
        failure_paths=[dict(kind='allocation',test='0x140336757',branch='0x14033675a',exit='0x1403368cd',rollback=False),
                       dict(kind='palette',test='0x14033685a',branch='0x14033685c',exit='0x1403368cd',rollback=False),
                       dict(kind='placement',test='0x140336873',branch='0x140336875',rollback_call='0x1403368ba',rollback=True)],
        descriptor_offsets={'source':0,'palette_source_or_sentinel':8,'format':0x10,'buffer_width':0x12,
                            'width':0x14,'height':0x16,'palette_format':0x18,'source_deltas':0x2c,'signed_level_count':0x3c},
        record_offsets={'format':4,'buffer_width':8,'width':0xa,'height':0xc,'source':0x10,
                        'argument_present':0x18,'selector_low16':0x1a,'palette_format':0x1c,
                        'palette_base_from_first':0x1e,'palette_source':0x28,'level':0x44,'span_marked':0x46},
        naming_limit='Palette naming follows pointer/field flow into 0x140331bd0; complete backend semantics remain open.')


def run(exe,repo,out):
    image=CanonicalImage(exe);facts=evidence(image);cases=[];reached=set()
    with tempfile.TemporaryDirectory(prefix='ptx-records-') as temp:
        library=Path(temp)/'records.so'
        sources=['src/reverse/ptx_manager_state.cpp','src/reverse/ptx_payload_state.cpp',
                 'src/reverse/ptx_record_materializer.cpp','tests/reverse/ptx_materializer_bridge.cpp']
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Wconversion','-Werror',
                        '-I'+str(repo/'include'),*[str(repo/p) for p in sources],'-o',str(library)],check=True)
        fn=ctypes.CDLL(str(library)).ptx_materializer_step
        fn.argtypes=[ctypes.c_void_p]*5+[ctypes.c_size_t,ctypes.POINTER(ctypes.c_uint64),ctypes.POINTER(ctypes.c_uint64)]
        fn.restype=ctypes.c_int64
        def cpp(s,c):
            buffers={k:ctypes.create_string_buffer(bytes(v),len(v)) for k,v in s.items()}
            cfg=(ctypes.c_uint64*len(c))(*c);log=(ctypes.c_uint64*1024)()
            result=fn(*[ctypes.byref(buffers[k],64) for k in ('pool','input','payload','manager','source')],
                      len(s['source'])-128,cfg,log)
            return {k:v.raw for k,v in buffers.items()},result,[list(log[1+5*i:6+5*i]) for i in range(log[0])]
        def case(name,s,c):
            actual,result,events,seen,clears=native(image,s,c);expected,cpp_result,cpp_events=cpp(s,c)
            check(result==cpp_result,f'{name}: return {result}/{cpp_result}')
            check(events==cpp_events,f'{name}: service events {events}/{cpp_events}')
            for key in actual:
                if actual[key]!=expected[key]:
                    pos=next(i for i,(a,b) in enumerate(zip(actual[key],expected[key])) if a!=b)
                    raise AssertionError(f'{name}: {key} mismatch at {pos-64:#x}: {actual[key][pos]:02x}/{expected[key][pos]:02x}')
                check(actual[key][:64]==s[key][:64] and actual[key][-64:]==s[key][-64:],f'{name}: {key} guards')
            for key in ('input','source'):check(actual[key]==bytes(s[key]),f'{name}: immutable {key}')
            newly_reserved=[i for i in range(128) if struct.unpack_from('<H',s['pool'],64+i*0x50)[0]==0
                and struct.unpack_from('<H',actual['pool'],64+i*0x50)[0]!=0]
            scratch_count=struct.unpack_from('<I',actual['pool'],64+0xcb20)[0]
            reached.update(seen)
            cases.append(dict(name=name,entry=hex(ROOTS[c[0]]),result=result,byte_equal=True,
                newly_reserved_records=newly_reserved,scratch_count=scratch_count,service_events=events,
                record_clears=[hex(p) for p in clears]))
            return {k:bytearray(v) for k,v in actual.items()}
        for levels in (-32768,-1,0,1,2,3,4):
            case(f'count-{levels}',state(levels=levels),config())
        for dims in ((0,0,0),(1,1,1),(3,15,7),(0xffff,0xffff,0x8000),(64,1024,512)):
            s=state(levels=4)
            struct.pack_into('<HHH',s['input'],64+0x12,*dims)
            case(f'dimension-shifts-{dims}',s,config())
        s=state(levels=4,free=[0,5,126,127]);struct.pack_into('<Q',s['input'],64,MASK-0x80)
        struct.pack_into('<III',s['input'],64+0x2c,0xffffffff,0x80000000,0x100)
        case('sparse-pool-and-unsigned-source-deltas',s,config())
        for palette in (False,True):
            for arg in (0,0x123456789abcdef0):
                for selector in (-1,0,0x12345678):
                    case(f'palette-{palette}-argument-{arg:x}-selector-{selector}',state(palette=palette),config(argument=arg,selector=selector))
        for retval in (0,0x100,0x101,0xff):
            case(f'palette-low-byte-result-{retval:x}',state(palette=True),config(argument=0x9900,palette_result=retval))
        for free in (0,1,2,3):
            case(f'allocation-failure-after-{free}',state(levels=4,free=range(free)),config())
        for where in range(4):
            for retval in (0,0x100):
                case(f'placement-failure-{where}-return-{retval:x}',state(levels=4,palette=True),config(place_fail=where,failure_result=retval))
        case('placement-low-byte-nonzero',state(),config(place_result=0x101))
        for off,value in ((0x1c,-1),(0x18,1),(0x44,1),(0x46,0)):
            s=state();record(s);struct.pack_into('<H',s['pool'],64+off,value&0xffff)
            case(f'mark-span-gate-{off:x}',s,config(1))
        for base in (0x1040,0xfff,0xfe1,-5):
            s=state();record(s,base=base)
            if base<0:struct.pack_into('<I',s['pool'],64+0xcb08,0)
            case(f'mark-span-base-{base}',s,config(1))
        for op in (2,3,4,5):
            case(f'outer-{op}-success',state(levels=2,palette=True,textures=3),config(op,argument=0x9900))
            s=case(f'outer-{op}-pool-exhaustion-leaves-current-prefix',state(levels=2,free=range(3),textures=3),config(op))
            check([i for i in range(3) if struct.unpack_from('<H',s['pool'],64+i*0x50)[0]]==[2],
                  'outer loader cleans previous texture but leaves current partial allocation')
            s=case(f'outer-{op}-placement-failure-cleans-all',state(levels=2,free=range(4),textures=3),config(op,place_fail=3))
            check(all(struct.unpack_from('<H',s['pool'],64+i*0x50)[0]==0 for i in range(4)),
                  'placement failure rollback plus outer cleanup clears all reserved records')
            case(f'outer-{op}-palette-failure-retains-first-record',state(levels=2,palette=True,textures=1),config(op,argument=0x9900,palette_result=0))
        s=state(levels=2,palette=True,textures=2)
        for i,(op,arg) in enumerate(((4,0),(5,0x9900),(6,0),(6,0),(5,0x9900))):
            s=case(f'cache-real-materializer-lifecycle-{i}',s,config(op,argument=arg))
        guards=[]
        for levels in (5,):
            _,result,_=cpp(state(levels=levels),config())
            check(result==-0x70000000,'host scratch capacity guard')
            guards.append(dict(levels=levels,status='HOST_GUARD_ONLY'))
        for length in (0,-1):
            s=state();record(s,length=length)
            _,result,_=cpp(s,config(1));check(result==-0x70000000,'host mark span guard')
            guards.append(dict(mark_span_length=length,status='HOST_GUARD_ONLY'))
    sources += ['include/dmc_rengine/reverse/ptx_record_materializer.hpp',
                'include/dmc_rengine/reverse/ptx_payload_state.hpp','include/dmc_rengine/reverse/ptx_manager_state.hpp',
                'scripts/reverse/verify_ptx_materializer.py','scripts/reverse/verify_ptx_payload.py',
                'scripts/reverse/canonical_image.py','scripts/reverse/verify_crt_lifecycle.py']
    result=dict(status='PASS',sha256=CANONICAL,cases=len(cases),case_results=cases,host_guards=guards,
        unicorn_version=unicorn.__version__,compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        real_internal_entries=[hex(p) for p in sorted(set(ROOTS.values())|{0x1403313f0,0x140331420,0x140330f60,0x1403317d0,0x14032d3c0}) if p in reached],
        limitations=['Palette helper 0x140331bd0 and placement helper 0x140331520 have explicit synthetic return/state contracts.',
            'Outer integration also intercepts parser 0x1403365b0, render finalizer 0x140331a80 and checked 80-byte CRT memset.',
            'Actual pool exhaustion needs no injected allocation failure; real allocator, marker, rollback and caller run.',
            'Synthetic dimensions/palette responses test caller behavior, not full resource validity or backend acceptance.',
            'Failure residue is bounded to these functions; no claim that a later scene/global reset cannot recover it.',
            'No Windows SEH, GPU runtime, aliasing, concurrency or reentrant callback parity.'])
    out.mkdir(parents=True,exist_ok=True)
    (out/'evidence.json').write_text(json.dumps(facts,indent=2)+'\n')
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    return {k:v for k,v in result.items() if k not in ('case_results','source_sha256')}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo.resolve(),a.out),indent=2))
