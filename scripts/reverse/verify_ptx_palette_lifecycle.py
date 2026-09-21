#!/usr/bin/env python3
"""EXE parity for palette lifecycle and its shared block allocator dependency.

Only CRT memory operations and callback queue draining are modeled. Context,
route selection, run search, handle allocation/free, placement and spans run
their canonical instructions. Callback queue internals remain a named boundary.
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
from verify_ptx_payload import POOL, MASK, digest, check

CONTEXT=0x55000040
ARENAS=0x140ca8910
PROFILE=0x54000040
DEST=0x56000040
MAPS=[0x57000040,0x58000040,0x59000040]
DATA=[0x5a000040,0x5b000040,0x5c000040]
KEYS=('pool','context','arenas','map0','map1','map2','data0','data1','data2','profile','destination')
BASES=dict(zip(KEYS,[POOL,CONTEXT,ARENAS,*MAPS,*DATA,PROFILE,DEST]))
ROOTS={0:0x140331180,1:0x140331460,2:0x1402c6150,3:0x140337600,
       4:0x1402c6260,5:0x1403374a0,6:0x140331bd0}
WINDOWS={0x140331180:0x140331341,0x140331460:0x140331514,0x1402c6150:0x1402c6225,
         0x1402c6260:0x1402c6265,0x1403374a0:0x1403375f1,0x140337600:0x1403376ce,0x140337710:0x140337777}


def put(s,key,offset,fmt,*values):struct.pack_into('<'+fmt,s[key],64+offset,*values)
def get(s,key,offset,fmt):return struct.unpack_from('<'+fmt,s[key],64+offset)[0]


def config(op=0,a=16,b=32,arena=1,fallback=0,forced=0,node=0,record=POOL+0xa0):
    return [v&MASK for v in (op,a,b,arena,fallback,forced,node,record)]


def fixture(seed=13):
    rng=random.Random(seed)
    sizes=[0xcb50,0x58,0x78,*([0x400]*3),*([0x20000]*3),0x100,0x1000]
    s={k:bytearray(rng.randbytes(n+128)) for k,n in zip(KEYS,sizes)}
    for i in range(128):
        put(s,'pool',i*0x50,'H',1);put(s,'pool',i*0x50+0x46,'H',0)
    s['pool'][64:64+16*0x50]=bytes(16*0x50)
    s['pool'][64+0x2800:64+0xca00]=bytes([1])*0xa200
    s['pool'][64+0x2800:64+0x2840]=bytes(64)
    put(s,'pool',0xcb08,'II',0x1000,0x1800)
    s['context'][64:64+0x58]=bytes(0x58)
    for i in range(2):put(s,'context',0x18+i*0x20+0x18,'i',-1)
    for i in range(3):
        put(s,'arenas',i*0x28,'QQiII',MAPS[i],DATA[i],256,256,1)
        put(s,'arenas',i*0x28+0x20,'I',0)
        s[f'map{i}'][64:64+0x400]=bytes(0x400)
    put(s,'profile',0x4c,'HH',0x1000,0x1800)
    return s


def native(image,s,c,branches):
    u=emulator(image)
    for base in range(0x54000000,0x5d000000,0x1000000):u.mem_map(base,0x40000)
    u.mem_map(0x60000000,0x1000)
    for key,base in BASES.items():u.mem_write(base-64,bytes(s[key]))
    u.mem_write(0x140d6d300,struct.pack('<Q',PROFILE))
    u.mem_write(0x140ca8ab2,bytes([c[4]&255]));u.mem_write(0x140ca8ab1,bytes([c[5]&255]))
    slot,symbol=image.import_thunk(0x14032d3d4)
    check(symbol=='VCRUNTIME140.dll!memmove','palette import')
    u.mem_write(slot,struct.pack('<Q',0x60000000))
    op=c[0];node=CONTEXT+0x18+c[6]*0x20
    arena=0 if c[3]==MASK else ARENAS+c[3]*0x28
    u.reg_write(R.UC_X86_REG_RCX,CONTEXT if op<2 else c[7] if op==6 else node)
    u.reg_write(R.UC_X86_REG_RDX,c[1] if op in (0,2,5) else arena if op==3 else CONTEXT if op==6 else 0)
    u.reg_write(R.UC_X86_REG_R8,c[2] if op in (0,2) else c[1] if op==3 else 0)
    if op==5:u.mem_write(node+8,struct.pack('<Q',arena))
    events=[];reached=set();edges=set();pending=None;attempts=[];clears=[]
    def event(kind,a,b,count,extra=0):
        events.append([kind,a,b,count,digest(u.mem_read(CONTEXT,0x58)),digest(u.mem_read(ARENAS,0x78)),
                       extra if kind==3 else digest(u.mem_read(POOL,0xcb50))])
    def ret(result):
        rsp=u.reg_read(R.UC_X86_REG_RSP);target=struct.unpack('<Q',u.mem_read(rsp,8))[0]
        for n,r in enumerate((R.UC_X86_REG_RCX,R.UC_X86_REG_RDX,R.UC_X86_REG_R8,
                               R.UC_X86_REG_R9,R.UC_X86_REG_R10,R.UC_X86_REG_R11)):
            u.reg_write(r,0x61000000+n*0x100)
        u.reg_write(R.UC_X86_REG_RAX,result&MASK);u.reg_write(R.UC_X86_REG_RSP,rsp+8);u.reg_write(R.UC_X86_REG_RIP,target)
    def bounded(address,count):
        check(any(base<=address and address+count<=base+len(s[k])-128 for k,base in BASES.items()),'memory service bounds')
    def hook(uc,addr,size,_):
        nonlocal pending
        reached.add(addr)
        if pending is not None:edges.add((pending,addr))
        pending=addr if addr in branches else None
        if addr==0x140337600:
            attempts.append([uc.reg_read(R.UC_X86_REG_RCX),uc.reg_read(R.UC_X86_REG_RDX),uc.reg_read(R.UC_X86_REG_R8)&0xffffffff])
        elif addr==0x140346bea:
            dest=uc.reg_read(R.UC_X86_REG_RCX);value=uc.reg_read(R.UC_X86_REG_RDX)&255;count=uc.reg_read(R.UC_X86_REG_R8)
            bounded(dest,count)
            if POOL<=dest<=POOL+0x27b0:
                check(value==0 and count==0x50,'pool record clear');clears.append(dest)
            else:event(1,dest,value,count)
            uc.mem_write(dest,bytes([value])*count);ret(dest)
        elif addr==0x1403292a0:
            p=uc.reg_read(R.UC_X86_REG_RCX)
            head=struct.unpack('<Q',uc.mem_read(p,8))[0];first=struct.unpack('<I',uc.mem_read(p+0x18,4))[0]
            event(2,p,head,first);uc.mem_write(p,bytes(8));ret(0)
        elif addr==0x60000000:
            dest=uc.reg_read(R.UC_X86_REG_RCX);source=uc.reg_read(R.UC_X86_REG_RDX);count=uc.reg_read(R.UC_X86_REG_R8)
            bounded(dest,count);bounded(source,count);data=bytes(uc.mem_read(source,count))
            event(3,dest,source,count,digest(data));uc.mem_write(dest,data);ret(dest)
    u.hook_add(unicorn.UC_HOOK_CODE,hook);u.emu_start(ROOTS[op],STOP,count=250000)
    check(u.reg_read(R.UC_X86_REG_RIP)==STOP,'normal bounded return')
    rax=u.reg_read(R.UC_X86_REG_RAX)
    result=0 if op==1 else ctypes.c_int32(rax&0xffffffff).value if op==5 else rax&255 if op in (0,4,6) else rax
    check(bytes(u.mem_read(0x140ca8ab1,2))==bytes([c[5]&255,c[4]&255]),'routing globals unchanged')
    return {k:bytes(u.mem_read(base-64,len(s[k]))) for k,base in BASES.items()},result,events,reached,edges,attempts,clears


def evidence(image):
    facts=[];branches={}
    for root,end in WINDOWS.items():
        ins=list(image.md.disasm(image.raw(root,end-root),root))
        check(ins[-1].address+ins[-1].size==end,'complete reviewed function window')
        for i in ins:
            if i.group(capstone.CS_GRP_JUMP) and i.mnemonic!='jmp':branches[i.address]=(i.address+i.size,i.operands[0].imm)
        facts.append(dict(root=hex(root),end=hex(end),file_offset=hex(image.offset(root,end-root)),
            code_sha256=hashlib.sha256(image.raw(root,end-root)).hexdigest(),
            pdata_fragments=[[hex(v) for v in row] for row in image.ranges if row[0]<end and root<row[1]],
            instructions=[dict(address=hex(i.address),bytes=i.bytes.hex(),mnemonic=i.mnemonic,operands=i.op_str) for i in ins]))
    return dict(sha256=CANONICAL,functions=facts,
        globals={'arena_table':'0x140ca8910','fallback_byte':'0x140ca8ab2','force_arena2_byte':'0x140ca8ab1'},
        unresolved_callback_boundary='0x1403292a0: tests clear callback_head synthetically; actual callback dispatch/recycling is not claimed.',
        initializer_format_dispatch='0x140331201 writes format=0; real placement/mark preserve it under the nonaliasing model. Only format-zero size branch is reachable.',
        status='EXE_CONFIRMED',scope='Named functions under the stated memory/callback boundaries'),branches


def run(exe,repo,out):
    image=CanonicalImage(exe);facts,branches=evidence(image);cases=[];seen=set();covered=set()
    with tempfile.TemporaryDirectory(prefix='ptx-palette-lifecycle-') as temp:
        library=Path(temp)/'lifecycle.so'
        sources=['src/reverse/runtime_block_allocator.cpp','src/reverse/ptx_palette_lifecycle.cpp',
            'src/reverse/ptx_palette_state.cpp','src/reverse/ptx_record_placement.cpp',
            'src/reverse/ptx_record_materializer.cpp','src/reverse/ptx_payload_state.cpp',
            'src/reverse/ptx_manager_state.cpp','tests/reverse/ptx_palette_lifecycle_bridge.cpp']
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Wconversion','-Werror',
            '-I'+str(repo/'include'),*[str(repo/p) for p in sources],'-o',str(library)],check=True)
        fn=ctypes.CDLL(str(library)).ptx_palette_lifecycle_step
        fn.argtypes=[ctypes.POINTER(ctypes.c_void_p),ctypes.POINTER(ctypes.c_size_t),
                     ctypes.POINTER(ctypes.c_uint64),ctypes.POINTER(ctypes.c_uint64)]
        fn.restype=ctypes.c_int64
        def cpp(s,c):
            b={k:ctypes.create_string_buffer(bytes(s[k]),len(s[k])) for k in KEYS}
            pointers=(ctypes.c_void_p*len(KEYS))(*(ctypes.addressof(b[k])+64 for k in KEYS))
            sizes=(ctypes.c_size_t*len(KEYS))(*(len(s[k])-128 for k in KEYS))
            cfg=(ctypes.c_uint64*len(c))(*c);log=(ctypes.c_uint64*2048)()
            result=fn(pointers,sizes,cfg,log)
            return {k:v.raw for k,v in b.items()},result,[list(log[1+7*i:8+7*i]) for i in range(log[0])]
        def case(name,s,c,expected=None,expected_attempts=None):
            if c[0]==5:put(s,'context',0x18+c[6]*0x20+8,'Q',ARENAS+c[3]*0x28)
            actual,result,events,reached,edges,attempts,clears=native(image,s,c,branches)
            other,cresult,cevents=cpp(s,c)
            check(result==cresult,f'{name}: return EXE {result}/C++ {cresult}')
            if expected is not None:check(result==expected,f'{name}: expected {expected}, got {result}')
            if expected_attempts is not None:check(len(attempts)==expected_attempts,f'{name}: attempts {attempts}')
            check(events==cevents,f'{name}: memory/callback event order {events}/{cevents}')
            for key in KEYS:
                if actual[key]!=other[key]:
                    pos=next(i for i,(a,b) in enumerate(zip(actual[key],other[key])) if a!=b)
                    raise AssertionError(f'{name}: {key} offset {pos-64:#x}, EXE {actual[key][pos]:02x}/C++ {other[key][pos]:02x}')
                check(actual[key][:64]==s[key][:64] and actual[key][-64:]==s[key][-64:],f'{name}: guards {key}')
            check(actual['profile']==bytes(s['profile']),f'{name}: immutable profile')
            seen.update(reached);covered.update(edges)
            cases.append(dict(name=name,entry=hex(ROOTS[c[0]]),result=result,byte_equal=True,
                config=c,events=events,allocation_attempts=attempts,record_clears=[hex(v) for v in clears],
                bank_records=[hex(get(actual,'context',8*i,'Q')) for i in range(2)],
                live_allocations=[get(actual,'arenas',0x28*i+0x20,'I') for i in range(3)],
                state_sha256={k:hashlib.sha256(v).hexdigest() for k,v in actual.items()}))
            return {k:bytearray(v) for k,v in actual.items()}

        for capacity in (-8,0,1,7,8,9,16,31,32):
            for needed in (0,1,2,8,9,33,0xffffffff):
                s=fixture();put(s,'arenas',0x28+0x10,'i',capacity)
                case(f'run-capacity-{capacity}-needed-{needed}',s,config(5,a=needed))
        for seed in range(64):
            rng=random.Random(seed);s=fixture(seed);capacity=rng.randrange(1,129)
            put(s,'arenas',0x28+0x10,'i',capacity)
            s['map1'][64:64+capacity]=bytes(rng.choice((0,0,1)) for _ in range(capacity))
            case(f'run-binary-map-{seed}',s,config(5,a=rng.randrange(1,25)))
        for pattern in (bytes([1])*32,bytes([2])*8+bytes(24),bytes([0])+bytes([2])*31,
                        bytes([1,2,255,1,2,255,1,2])+bytes(24),bytes([0,1])*16,
                        bytes(9)+bytes([1])+bytes(22),bytes([1])*8+bytes(7)+bytes([1])+bytes(16)):
            for needed in (1,8,10):
                s=fixture();put(s,'arenas',0x28+0x10,'i',len(pattern));s['map1'][64:64+len(pattern)]=pattern
                case(f'run-special-{pattern.hex()}-{needed}',s,config(5,a=needed))
        s=fixture();put(s,'arenas',0x28,'Q',MAPS[1]+1)
        case('run-unaligned-map',s,config(5,a=1),-1)
        for count in (0,1,255,256,257,1024,2049,0x80000000,0xffffffff):
            s=case(f'allocate-bytes-{count}',fixture(),config(3,a=count))
            case(f'release-after-bytes-{count}',s,config(4))
        for gate in ('null-arena','null-data','disabled','busy','full','unaligned'):
            s=fixture();c=config(3,a=513)
            put(s,'context',0x18,'Q',0x99990000);put(s,'context',0x28,'Q',0x88880000);put(s,'context',0x34,'I',23)
            if gate=='null-arena':c[3]=MASK
            elif gate=='null-data':put(s,'arenas',0x28+8,'Q',0)
            elif gate=='disabled':put(s,'arenas',0x28+0x18,'I',0)
            elif gate=='busy':put(s,'context',0x20,'Q',ARENAS)
            elif gate=='full':s['map1'][64:64+256]=bytes([1])*256
            else:put(s,'arenas',0x28,'Q',MAPS[1]+1)
            case('allocate-gate-'+gate,s,c,0)
        for mode in (-2,-1,0,1,2):
            for count in (512,2048):
                for fallback in (0,1):
                    for forced in (0,1,2):
                        s=fixture();s['map1'][64:64+256]=bytes([1])*256;s['map2'][64:64+256]=bytes([1])*256
                        case(f'route-{mode}-{count}-{fallback}-{forced}',s,config(2,a=count,b=mode,fallback=fallback,forced=forced))
        s=fixture();s['map0'][64:64+256]=bytes([1])*256
        case('route-auto-large-falls-back-to-one',s,config(2,a=2048,b=-1,fallback=255),DATA[1],2)
        s=fixture();s['map2'][64:64+256]=bytes([1])*256
        case('route-forced-two-retries-same-arena',s,config(2,a=512,b=1,fallback=1,forced=1),0,2)
        case('route-original-mode-zero-disables-forced-fallback',s,config(2,a=512,b=0,fallback=1,forced=1),0,1)
        for count in (0x80000000,0xffffffff):case(f'route-auto-signed-size-{count}',fixture(),config(2,a=count,b=-1,fallback=1),0,2)
        s=fixture();put(s,'arenas',0x28+0x20,'I',0xffffffff);put(s,'context',0x18,'Q',0x88889999)
        s=case('allocation-counter-wrap',s,config(3,a=1),DATA[1])
        s=case('release-callback-order-counter-wrap',s,config(4),1)
        case('release-again',s,config(4),0)
        s=fixture();put(s,'context',0x18,'Q',0x12345678)
        after=case('release-empty-retains-callback-head',s,config(4),0)
        check(get(after,'context',0x18,'Q')==0x12345678,'empty release does not drain callbacks')

        for count13,count14 in ((0,0),(-1,0),(0,-1),(1,0),(0,1),(1,1),(8,32),(9,33),(16,32),
                                (8192,1),(8193,1),(16384,1),(65535,0),(65536,0),(0x7fffffff,0),
                                (0,32768),(0,32769),(0,65536),(0,0x7fffffff)):
            s=case(f'initialize-counts-{count13}-{count14}',fixture(),config(a=count13,b=count14))
            case(f'destroy-counts-{count13}-{count14}',s,config(1))
        s=fixture();s['context'][64:64+0x18]=bytes(range(24))
        after=case('initialize-skipped-banks-retain-state',s,config(a=0,b=-3),1)
        check(after==s,'nonpositive bank counts cause no writes')
        for available in (0,1):
            s=fixture()
            for i in range(available,16):put(s,'pool',i*0x50,'H',1)
            s=case(f'initialize-record-exhaustion-{available}',s,config(a=1,b=1),0)
            case(f'cleanup-record-exhaustion-{available}',s,config(1))
        for free_cells in (0,1):
            s=fixture();s['pool'][64+0x2800:64+0x2840]=bytes(free_cells)+bytes([1])*(64-free_cells)
            s=case(f'initialize-placement-failure-{free_cells}',s,config(a=1,b=1),0)
            s=case(f'cleanup-placement-failure-{free_cells}',s,config(1))
            check(get(s,'pool',free_cells*0x50,'H')==1,'unpublished placement-failed record survives context cleanup')
        for first_capacity in (0,32):
            s=fixture();put(s,'arenas',0x28+0x10,'i',first_capacity)
            s=case(f'initialize-allocation-failure-capacity-{first_capacity}',s,config(a=1,b=1),0)
            s=case(f'cleanup-allocation-failure-capacity-{first_capacity}',s,config(1))
            check(get(s,'pool',0,'H')==0 and get(s,'pool',0x50,'H')==0,'published records recover after data allocation failure')
        s=fixture();put(s,'arenas',0x28+0x10,'i',32)
        s=case('initialize-second-bank-fallback',s,config(a=1,b=1,fallback=1),1)
        case('cleanup-banks-from-different-arenas',s,config(1))
        for pointer in (0,POOL-1,POOL+0x27b1,POOL+0x27b0,POOL+1):
            s=fixture();put(s,'context',0,'Q',pointer)
            if POOL<=pointer<=POOL+0x27b0:put(s,'pool',pointer-POOL+0x46,'H',0)
            case(f'cleanup-pointer-{pointer:x}',s,config(1))
        s=fixture();put(s,'context',0,'QQ',POOL,POOL)
        case('cleanup-duplicate-bank-records',s,config(1))
        s=case('lifecycle-initialize',fixture(),config(),1)
        for fmt in (0x13,0x14):
            put(s,'pool',0xa0+4,'H',fmt);put(s,'pool',0xa0+0x1a,'H',0xffff);put(s,'pool',0xa0+0x28,'Q',DEST)
            s=case(f'lifecycle-copy-palette-{fmt:x}',s,config(6),1)
        s=case('lifecycle-destroy',s,config(1))
        s=case('lifecycle-destroy-again',s,config(1))
        case('lifecycle-reinitialize-after-cleanup',s,config(),1)
        s=case('reinitialize-dirty-setup',fixture(),config(a=1,b=1),1)
        s=case('reinitialize-with-live-handles',s,config(a=1,b=1),0)
        s=case('cleanup-after-dirty-reinitialize',s,config(1))
        check(get(s,'pool',0,'H')==1 and get(s,'pool',0x50,'H')==0 and get(s,'pool',0xa0,'H')==0,
              'reinitialize replaces bank pointer before busy handle failure, leaving the old record')
        guards=[]
        s=fixture();put(s,'arenas',0x28+0x14,'I',0)
        _,result,_=cpp(s,config(3,a=1));check(result==-0x70000000,'division guard')
        guards.append(dict(name='zero-block-size',status='HOST_GUARD_ONLY; original CPU division fault not modeled'))
        _,result,_=cpp(fixture(),config(2,a=1,b=3));check(result==-0x70000000,'arena view guard')
        guards.append(dict(name='unrepresented-arena-index',status='HOST_GUARD_ONLY; original has no range check'))
    coverage=[dict(site=hex(a),fallthrough=hex(d[0]),target=hex(d[1]),
        fallthrough_observed=(a,d[0]) in covered,target_observed=(a,d[1]) in covered) for a,d in sorted(branches.items())]
    missing=[r for r in coverage if not(r['fallthrough_observed'] and r['target_observed'])]
    constant_format_sites={hex(a) for a in (0x140331291,0x140331296,0x14033129b,0x1403312a0,0x1403312a5)}
    check({r['site'] for r in missing}==constant_format_sites,
          'all remaining conditional outcomes belong to the proven constant-format dispatch')
    sources += ['include/dmc_rengine/reverse/runtime_block_allocator.hpp','include/dmc_rengine/reverse/ptx_palette_lifecycle.hpp',
        'include/dmc_rengine/reverse/ptx_palette_state.hpp','include/dmc_rengine/reverse/ptx_record_placement.hpp',
        'include/dmc_rengine/reverse/ptx_record_materializer.hpp','include/dmc_rengine/reverse/ptx_payload_state.hpp',
        'include/dmc_rengine/reverse/ptx_manager_state.hpp','scripts/reverse/verify_ptx_palette_lifecycle.py',
        'scripts/reverse/verify_ptx_payload.py','scripts/reverse/canonical_image.py','scripts/reverse/verify_crt_lifecycle.py']
    result=dict(status='PASS',sha256=CANONICAL,cases=len(cases),case_results=cases,host_guards=guards,
        conditional_edges=coverage,conditional_sites=len(branches),sites_with_both_outcomes=len(coverage)-len(missing),
        remaining_branch_outcomes=missing,unicorn_version=unicorn.__version__,
        compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        real_internal_entries=[hex(p) for p in sorted(set(ROOTS.values())|set(WINDOWS)|{0x140331520,0x1403310f0,
            0x140330f60,0x14032d3c0,0x14032d3d0}) if p in seen],
        limitations=['Only CRT fill/move and callback drain 0x1403292a0 are intercepted; callback queue internals remain unrecovered.',
          'Context record-size format branches after format=0 are unreachable with actual placement/mark and nonaliasing state.',
          'Three represented arena descriptors are a test view, not a proven global array bound.',
          'Synthetic arena initialization; production arena setup, callback ownership, GPU validity and caller scheduling remain open.',
          'No aliasing of handles/arena metadata/pool/data, concurrent mutation, reentrant callbacks, SEH or CPU fault parity.',
          'Failure residue describes these functions; it is not a claim of a permanent game resource leak.'])
    out.mkdir(parents=True,exist_ok=True)
    (out/'evidence.json').write_text(json.dumps(facts,indent=2)+'\n')
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    return {k:v for k,v in result.items() if k not in ('case_results','source_sha256','conditional_edges')}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo.resolve(),a.out),indent=2))
