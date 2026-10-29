#!/usr/bin/env python3
"""Compare recovered PTX loaders/pool cleanup with unmodified canonical code.

Parser, record materializer, render finalizer and CRT memset are explicit test
boundaries. Actual cache, loader, pool, span-release and cookie code executes.
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

POOL=0x140d5fb70
POOL_SIZE=0xcb48
PAYLOAD=0x50000040
SOURCE=0x51000040
MANAGER=0x52000040
MASK=(1<<64)-1
ROOTS={0:0x1403313f0,1:0x140330f60,2:0x140331420,3:0x1403317d0,
       4:0x140336bb0,5:0x140336a70,6:0x140314e00,7:0x140314fa0,8:0x140315180}
WINDOWS={0x1403313f0:0x140331418,0x140330f60:0x140330fed,0x140331420:0x14033145d,
         0x1403317d0:0x1403318bc,0x140336bb0:0x140336ce1,0x140336a70:0x140336ba1,
         0x14032d3c0:0x14032d3cf,0x140331a80:0x140331bcd,0x1403366e0:0x1403368ec}


def check(condition,message):
    if not condition:raise AssertionError(message)


def digest(data):
    value=14695981039346656037
    for b in data:value=((value^b)*1099511628211)&MASK
    return value


def native(image,state,cfg):
    u=emulator(image)
    for base in (0x50000000,0x51000000,0x52000000):u.mem_map(base,0x20000)
    for key,base in (('payload',PAYLOAD),('pool',POOL),('source',SOURCE),('manager',MANAGER)):
        u.mem_write(base-64,bytes(state[key]))
    op=cfg[0]
    u.reg_write(R.UC_X86_REG_RCX,POOL if op<3 else PAYLOAD if op==3 else SOURCE if op<6 else MANAGER)
    u.reg_write(R.UC_X86_REG_RDX,cfg[1] if op<3 else PAYLOAD if op<6 else cfg[6])
    u.reg_write(R.UC_X86_REG_R8,cfg[2])
    events=[];reached=set();index=0;descriptor=None;current_payload=PAYLOAD;memsets=[]
    def ret(uc,result):
        rsp=uc.reg_read(R.UC_X86_REG_RSP)
        target=struct.unpack('<Q',uc.mem_read(rsp,8))[0]
        for n,reg in enumerate((R.UC_X86_REG_RCX,R.UC_X86_REG_RDX,R.UC_X86_REG_R8,
                               R.UC_X86_REG_R9,R.UC_X86_REG_R10,R.UC_X86_REG_R11)):
            uc.reg_write(reg,0x61000000+n*0x100)
        uc.reg_write(R.UC_X86_REG_RAX,result&MASK)
        uc.reg_write(R.UC_X86_REG_RSP,rsp+8);uc.reg_write(R.UC_X86_REG_RIP,target)
    def hook(uc,addr,size,_):
        nonlocal index,descriptor,current_payload
        reached.add(addr)
        if addr in (0x140336bb0,0x140336a70):
            descriptor=uc.reg_read(R.UC_X86_REG_RSP)-0x88
            uc.mem_write(descriptor,bytes(state['workspace']))
            current_payload=uc.reg_read(R.UC_X86_REG_RDX)
        if addr==0x1403365b0:
            source=uc.reg_read(R.UC_X86_REG_RCX);desc=uc.reg_read(R.UC_X86_REG_RDX)
            check(desc==descriptor,'parser descriptor points to actual loader workspace')
            count,width=struct.unpack('<II',uc.mem_read(current_payload+0x200,8))
            events.append([1,source,digest(uc.mem_read(desc,64)),count,width])
            uc.mem_write(desc,struct.pack('<Q',source));uc.mem_write(desc+0x38,bytes([index]))
            ret(uc,0 if cfg[3]==index else 1)
        elif addr==0x1403366e0:
            desc=uc.reg_read(R.UC_X86_REG_RCX)
            events.append([2,uc.reg_read(R.UC_X86_REG_RDX)&0xffffffff,
                           uc.reg_read(R.UC_X86_REG_R8),digest(uc.mem_read(desc,64)),index])
            width=cfg[8+index]&0xffffffff
            uc.mem_write(POOL+0xcb20,struct.pack('<I',width));uc.mem_write(POOL+0xcb28,bytes(32))
            signed=struct.unpack('<i',struct.pack('<I',width))[0]
            for j in range(max(0,min(4,signed))):
                slot=index*4+j;record=bytearray((slot*13+k*7+3)&255 for k in range(0x50))
                struct.pack_into('<HH',record,0,1,slot);struct.pack_into('<H',record,0x46,0)
                address=POOL+slot*0x50
                uc.mem_write(address,bytes(record));uc.mem_write(POOL+0xcb28+j*8,struct.pack('<Q',address))
            failed=cfg[4]==index;index+=1;ret(uc,0 if failed else 1)
        elif addr==0x140331a80:
            p=uc.reg_read(R.UC_X86_REG_RCX);raw=uc.mem_read(p,0x208)
            count,width=struct.unpack_from('<II',raw,0x200)
            events.append([3,count,width,digest(raw),0]);ret(uc,cfg[5])
        elif addr==0x140346bea:
            dst=uc.reg_read(R.UC_X86_REG_RCX);value=uc.reg_read(R.UC_X86_REG_RDX)&255
            length=uc.reg_read(R.UC_X86_REG_R8)
            check(value==0 and length==0x50 and POOL<=dst<=POOL+0x27b0,'record memset contract')
            memsets.append(dst);uc.mem_write(dst,bytes(length));ret(uc,dst)
    u.hook_add(unicorn.UC_HOOK_CODE,hook)
    u.emu_start(ROOTS[op],STOP,count=200000)
    check(u.reg_read(R.UC_X86_REG_RIP)==STOP,'normal termination within instruction budget')
    rax=u.reg_read(R.UC_X86_REG_RAX)
    result=rax if op==0 else 0 if op<4 else (rax-MANAGER if rax else -1) if op in (6,7) else rax&255 if op==8 else ctypes.c_int32(rax&0xffffffff).value
    actual={key:bytes(u.mem_read(base-64,len(state[key]))) for key,base in
            (('payload',PAYLOAD),('pool',POOL),('source',SOURCE),('manager',MANAGER))}
    actual['workspace']=bytes(u.mem_read(descriptor,64)) if descriptor is not None else bytes(state['workspace'])
    return actual,result,events,reached,memsets


def fixture(seed=1,count=2):
    rng=random.Random(seed)
    state={k:bytearray(rng.randbytes(n+128)) for k,n in
           (('payload',0x208),('pool',POOL_SIZE),('source',0x8000),('manager',0x4308))}
    state['workspace']=bytearray(rng.randbytes(64))
    state['payload'][64:64+0x208]=bytes(0x208)
    struct.pack_into('<i',state['source'],64,count)
    for i in range(16):struct.pack_into('<I',state['source'],68+i*4,i%3+1)
    struct.pack_into('<I',state['pool'],64+0xcb08,0x1000)
    for i in range(128):struct.pack_into('<H',state['pool'],64+i*0x50+0x46,0)
    struct.pack_into('<Q',state['manager'],64,0x140507b60)
    for i in range(32):struct.pack_into('<QI',state['manager'],72+i*0x218,0,0)
    return state


def configuration(op,**kwargs):
    c=[op,POOL,0x123456789abcdef0,MASK,MASK,1,SOURCE,0]+[2]*16
    for key,value in kwargs.items():c[{'address':1,'argument':2,'parse_fail':3,'build_fail':4,'result':5,'key':6}[key]]=value&MASK
    return c


def record(state,index=0,active=True,palette=True,base=0x1040,length=3):
    p=64+index*0x50
    for off,value in ((0,1),(6,base),(0xe,length),(0x18,0),(0x1c,0 if palette else -1),
                      (0x1e,0x1100),(0x26,2),(0x44,0),(0x46,int(active))):
        struct.pack_into('<H',state['pool'],p+off,value&0xffff)


def body_evidence(image):
    rows=[]
    for root,end in WINDOWS.items():
        # Explicit reviewed windows include chained .pdata fragments, not just
        # the first RUNTIME_FUNCTION entry. Traverse normal control flow only.
        todo=[root];instructions={};external=[]
        while todo:
            va=todo.pop()
            if va in instructions:continue
            check(root<=va<end,f'instruction escaped reviewed window {root:#x}')
            ins=image.instruction(va);instructions[va]=ins
            check(ins.address+ins.size<=end,'instruction stays inside window')
            nxt=va+ins.size
            if ins.group(capstone.CS_GRP_RET):continue
            if ins.group(capstone.CS_GRP_JUMP):
                target=ins.operands[0].imm if ins.operands[0].type==capstone.x86.X86_OP_IMM else None
                if target is not None and root<=target<end:todo.append(target)
                else:external.append(dict(site=hex(va),target=None if target is None else hex(target)))
                if ins.mnemonic=='jmp':continue
            todo.append(nxt)
        ordered=sorted(instructions.values(),key=lambda i:i.address)
        calls=[dict(site=hex(i.address),target=hex(i.operands[0].imm)) for i in ordered
               if i.group(capstone.CS_GRP_CALL) and i.operands[0].type==capstone.x86.X86_OP_IMM]
        rows.append(dict(root=hex(root),end=hex(end),instruction_count=len(ordered),
                         instruction_addresses=[hex(i.address) for i in ordered],
                         code_sha256=hashlib.sha256(b''.join(i.bytes for i in ordered)).hexdigest(),
                         calls=calls,external_jumps=external))
    check(image.import_thunk(0x140346bea)[1]=='VCRUNTIME140.dll!memset','record clear import')
    return dict(status='EXE_CONFIRMED',sha256=CANONICAL,functions=rows,
        payload={'size':0x208,'record_slots':64,'texture_count_offset':0x200,'first_texture_record_count_offset':0x204},
        pool={'base':hex(POOL),'record_count':128,'record_stride':0x50,'last_record':hex(POOL+0x27b0),
              'occupancy_map_offset':0x2800,'base_units_offset':0xcb08,'scratch_count_offset':0xcb20,'scratch_records_offset':0xcb28},
        limitations=['Reviewed normal-flow windows cover chained unwind fragments; no Windows SEH execution claim.',
                     'Function 0x1403366e0 and 0x140331a80 are statically catalogued boundaries, not implemented here.'])


def run(exe,repo,out):
    image=CanonicalImage(exe);facts=body_evidence(image);cases=[];all_reached=set();total_memsets=0
    with tempfile.TemporaryDirectory(prefix='ptx-payload-') as temp:
        library=Path(temp)/'ptx.so'
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Wconversion','-Werror',
            '-I'+str(repo/'include'),str(repo/'src/reverse/ptx_payload_state.cpp'),
            str(repo/'src/reverse/ptx_manager_state.cpp'),str(repo/'tests/reverse/ptx_payload_bridge.cpp'),
            '-o',str(library)],check=True)
        fn=ctypes.CDLL(str(library)).ptx_payload_step
        fn.argtypes=[ctypes.c_void_p]*4+[ctypes.c_size_t,ctypes.POINTER(ctypes.c_uint64),ctypes.POINTER(ctypes.c_uint64),ctypes.c_void_p]
        fn.restype=ctypes.c_int64
        def cpp(state,cfg):
            buffers={k:ctypes.create_string_buffer(bytes(v),len(v)) for k,v in state.items()}
            config=(ctypes.c_uint64*len(cfg))(*cfg);log=(ctypes.c_uint64*1024)()
            result=fn(ctypes.byref(buffers['payload'],64),ctypes.byref(buffers['pool'],64),buffers['workspace'],
                ctypes.byref(buffers['source'],64),len(state['source'])-128,config,log,ctypes.byref(buffers['manager'],64))
            events=[list(log[1+i*5:6+i*5]) for i in range(log[0])]
            return {k:v.raw for k,v in buffers.items()},result,events
        def case(name,state,cfg):
            nonlocal total_memsets
            actual,result,events,reached,clears=native(image,state,cfg)
            expected,cpp_result,cpp_events=cpp(state,cfg)
            check(result==cpp_result,f'{name}: result {result}/{cpp_result}')
            check(events==cpp_events,f'{name}: service events {events}/{cpp_events}')
            for key in actual:
                if actual[key]!=expected[key]:
                    pos=next(i for i,(a,b) in enumerate(zip(actual[key],expected[key])) if a!=b)
                    raise AssertionError(f'{name}: {key} memory mismatch at {pos-64:#x}: {actual[key][pos]:02x}/{expected[key][pos]:02x}')
                if key!='workspace':
                    check(actual[key][:64]==state[key][:64] and actual[key][-64:]==state[key][-64:],f'{name}: {key} guards')
            check(actual['source']==bytes(state['source']),f'{name}: source immutable')
            all_reached.update(reached);total_memsets+=len(clears)
            cases.append(dict(name=name,entry=hex(ROOTS[cfg[0]]),result=result,byte_equal=True,
                              service_events=events,record_clears=[hex(a) for a in clears]))
            return {k:bytearray(v) for k,v in actual.items()}
        for free in (0,1,63,127,None):
            s=fixture()
            if free is not None:struct.pack_into('<H',s['pool'],64+free*0x50,0)
            case(f'allocate-first-free-{free}',s,configuration(0))
        for address in (0,POOL-1,POOL,POOL+0x27b0,POOL+0x27b1,POOL+1):
            s=fixture()
            if address==POOL+1:struct.pack_into('<H',s['pool'],64+1+0x46,0)
            case(f'record-range-{address:x}',s,configuration(2,address=address))
        for op in (1,2):
            for base in (0x1040,0xfff,0xfe1,-5):
                s=fixture();record(s,base=base)
                if base<0:struct.pack_into('<I',s['pool'],64+0xcb08,0)
                case(f'spans-{op}-base-{base}',s,configuration(op))
            for off,value in ((0x1c,-1),(0x18,1),(0x44,1),(0x46,0)):
                s=fixture();record(s);struct.pack_into('<H',s['pool'],64+off,value&0xffff)
                case(f'spans-{op}-gate-{off:x}',s,configuration(op))
        for counts in ((0,4),(1,1),(2,2),(16,4),(0xffffffff,1),(0x40000000,4)):
            s=fixture()
            struct.pack_into('<II',s['payload'],64+0x200,*counts)
            for i in range(64):
                record(s,index=i,palette=(i%2==0))
                struct.pack_into('<Q',s['payload'],64+i*8,POOL+i*0x50)
            case(f'payload-product-{counts[0]:x}-{counts[1]}',s,configuration(3))
        s=fixture();record(s);struct.pack_into('<II',s['payload'],64+0x200,5,1)
        struct.pack_into('<5Q',s['payload'],64,0,POOL-1,POOL,POOL,POOL+0x27b1)
        case('payload-null-invalid-duplicate',s,configuration(3))
        for op in (4,5):
            for count in (0,-1,1,3,16):
                case(f'load-{op}-count-{count}',fixture(count=count),configuration(op))
            cfg=configuration(op);cfg[8:24]=[4]*16
            case(f'load-{op}-all-64-record-slots',fixture(count=16),cfg)
            for stage in ('parse_fail','build_fail'):
                for index in (0,1,2):case(f'load-{op}-{stage}-{index}',fixture(count=3),configuration(op,**{stage:index}))
            for widths in ((4,4,4),(3,1,2),(1,4,2),(0,2,2),(0xffffffff,2,2)):
                cfg=configuration(op);cfg[8:11]=widths
                case(f'load-{op}-widths-{widths}',fixture(count=3),cfg)
            s=fixture();struct.pack_into('<II',s['source'],68,0x200001,0xffffffff)
            case(f'load-{op}-block-shift-wrap',s,configuration(op))
            s=fixture(count=0);struct.pack_into('<II',s['payload'],64+0x200,7,3)
            case(f'load-{op}-zero-count-retains-existing-state',s,configuration(op))
        s=fixture(count=2)
        for i,(op,arg) in enumerate(((6,0),(7,99),(8,0),(8,0),(7,101))):
            s=case(f'cache-through-load-and-release-step-{i}',s,configuration(op,argument=arg))
        for op in (6,7):
            case(f'cache-{op}-partial-build-failure',fixture(count=3),configuration(op,build_fail=1))
        # These are host guards, explicitly not compared to undefined/unbounded EXE paths.
        host_guards=[]
        for length in (0,-1):
            s=fixture();record(s,length=length)
            _,result,_=cpp(s,configuration(1))
            check(result==-0x70000000,'host rejects nonpositive do-while length')
            host_guards.append(dict(span_length=length,status='HOST_GUARD_ONLY'))
        s=fixture();struct.pack_into('<II',s['payload'],64+0x200,65,1)
        for i in range(64):struct.pack_into('<Q',s['payload'],64+i*8,0)
        _,result,_=cpp(s,configuration(3))
        check(result==-0x70000000,'host refuses the 65th payload slot')
        host_guards.append(dict(payload_slots=65,status='HOST_GUARD_ONLY'))
    sources=['include/dmc_rengine/reverse/ptx_payload_state.hpp','src/reverse/ptx_payload_state.cpp',
             'tests/reverse/ptx_payload_bridge.cpp','scripts/reverse/verify_ptx_payload.py',
             'include/dmc_rengine/reverse/ptx_manager_state.hpp','src/reverse/ptx_manager_state.cpp',
             'scripts/reverse/canonical_image.py','scripts/reverse/verify_crt_lifecycle.py']
    verification=dict(status='PASS',sha256=CANONICAL,cases=len(cases),case_results=cases,
        host_guards=host_guards,unicorn_version=unicorn.__version__,
        compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        actual_internal_entries=[hex(a) for a in sorted(set(ROOTS.values())|{0x14032d3c0,0x1403455f0}) if a in all_reached],
        record_memsets=total_memsets,
        limitations=['Texture parser 0x1403365b0, record builder 0x1403366e0 and render finalizer 0x140331a80 are synthetic service boundaries.',
          'CRT memset is intercepted with its checked 80-byte zeroing contract; wrapper 0x14032d3c0 runs unchanged.',
          'Workspace initial bytes are supplied explicitly because loader stack storage is uninitialized.',
          'Mismatched record-count cases test loader mechanics only, not validity for the real finalizer.',
          'No decoded images, GPU acceptance, exception unwinding, aliasing or concurrent/reentrant execution claim.',
          'Host guards for out-of-image accesses/nonpositive spans are not original EXE validation.'])
    out.mkdir(parents=True,exist_ok=True)
    (out/'evidence.json').write_text(json.dumps(facts,indent=2)+'\n')
    (out/'verification.json').write_text(json.dumps(verification,indent=2)+'\n')
    return {k:v for k,v in verification.items() if k not in ('case_results','source_sha256')}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo.resolve(),a.out),indent=2))
