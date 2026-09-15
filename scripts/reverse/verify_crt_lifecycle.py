#!/usr/bin/env python3
"""Differentially verify two recovered initializers and audit local CRT arguments.

Unicorn executes canonical instructions. The registration service is intercepted.
For the separate caller-argument audit, all callees are stubbed: it validates
argument recovery on those paths, not real initialization or callee behavior.
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

STACK = 0x70000000
STOP = STACK + 0xf000
REGISTRATION = 0x1403458d0
REGS = {name:getattr(R,'UC_X86_REG_'+name.upper()) for name in ('rax','rcx','rdx','r8','r9','r10','r11')}


def emulator(image):
    u=unicorn.Uc(unicorn.UC_ARCH_X86,unicorn.UC_MODE_64)
    size=struct.unpack_from('<I',image.data,image.opt+56)[0]
    u.mem_map(image.base,(size+4095)&~4095)
    for start,vs,rs,raw,flags in image.sections:
        u.mem_write(start,image.data[raw:raw+min(vs,rs)])
    u.mem_map(STACK,0x10000)
    u.reg_write(R.UC_X86_REG_RSP,STACK+0x8008)
    u.mem_write(STACK+0x8008,struct.pack('<Q',STOP))
    return u


def assert_equal(a,b,message):
    if a!=b:raise AssertionError(f'{message}: {a!r} != {b!r}')


def run(exe,repo,out):
    image=CanonicalImage(exe)
    scenarios=[dict(entry=0x140021a20,base=0x140cb9ed0,size=0x3000,cleanup=0x14034ea50,stores=1536,name='initialize_first'),
               dict(entry=0x1400236a0,base=0x140cf2d90,size=0x101,cleanup=0x14034eb50,stores=49,name='initialize_second')]
    cases=[]
    with tempfile.TemporaryDirectory(prefix='dmc-crt-') as temp:
        library=Path(temp)/'crt.so'
        compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0]
        subprocess.run(['g++','-std=c++20','-O2','-shared','-fPIC','-Wall','-Wextra','-Werror',
            '-I'+str(repo/'include'),str(repo/'src/reverse/crt_global_initializers.cpp'),
            str(repo/'tests/reverse/crt_initializer_bridge.cpp'),'-o',str(library)],check=True)
        lib=ctypes.CDLL(str(library))
        for s in scenarios:
            fn=getattr(lib,s['name']);fn.argtypes=[ctypes.c_void_p,ctypes.c_int];fn.restype=ctypes.c_int
            length=s['size']+128;start=s['base']-64
            patterns=[bytes([v])*length for v in (0,0xff,0xa5)]
            patterns += [random.Random(seed).randbytes(length) for seed in (7,41,20260915)]
            for pattern_index,payload in enumerate(patterns):
                for return_status in (0,-1):
                    u=emulator(image);u.mem_write(start,payload)
                    registrations=[];writes=[]
                    def code_hook(uc,addr,size,_):
                        if addr==REGISTRATION:
                            registrations.append((uc.reg_read(R.UC_X86_REG_RCX),bytes(uc.mem_read(start,length))))
                            uc.reg_write(R.UC_X86_REG_RAX,return_status & 0xffffffffffffffff)
                            rsp=uc.reg_read(R.UC_X86_REG_RSP)
                            uc.reg_write(R.UC_X86_REG_RIP,struct.unpack('<Q',uc.mem_read(rsp,8))[0])
                            uc.reg_write(R.UC_X86_REG_RSP,rsp+8)
                    def write_hook(uc,access,addr,size,value,_):
                        if not STACK<=addr<STACK+0x10000:writes.append((addr,size))
                    u.hook_add(unicorn.UC_HOOK_CODE,code_hook)
                    u.hook_add(unicorn.UC_HOOK_MEM_WRITE,write_hook)
                    u.emu_start(s['entry'],STOP,count=50000)
                    assert_equal(u.reg_read(R.UC_X86_REG_RIP),STOP,'initializer termination')
                    assert_equal(len(registrations),1,'exactly one registration')
                    assert_equal(registrations[0][0],s['cleanup'],'callback target')
                    assert_equal(len(writes),s['stores'],'store count')
                    actual=bytes(u.mem_read(start,length))
                    assert_equal(registrations[0][1],actual,'writes precede registration')
                    buf=ctypes.create_string_buffer(payload,length)
                    assert_equal(fn(buf,return_status),1,'C++ registration/callback contract')
                    assert_equal(buf.raw,actual,'EXE vs C++ full memory including untouched bytes')
                    # Repeating the initializer retains bytes and registers again.
                    assert_equal(fn(buf,return_status),1,'C++ repeated registration')
                    assert_equal(buf.raw,actual,'C++ idempotent memory effects')
                    # Execute the original cleanup, independently of the C++ callback.
                    u.reg_write(R.UC_X86_REG_RSP,STACK+0x8008)
                    u.mem_write(STACK+0x8008,struct.pack('<Q',STOP))
                    u.emu_start(s['cleanup'],STOP,count=16)
                    assert_equal(u.reg_read(R.UC_X86_REG_RIP),STOP,'cleanup termination')
                    assert_equal(bytes(u.mem_read(start,length)),actual,'no-op cleanup memory')
                    cases.append(dict(entry=hex(s['entry']),pattern=pattern_index,registration_status=return_status,
                                      byte_equal=True,writes=len(writes)))
    bodies=json.loads((repo/'data/reverse/crt-lifecycle-20260915/crt-bodies.json').read_text())
    argument_sites=set();callbacks=set();vector_sites=set();app_roots=0
    for b in bodies:
        root=int(b['root'],16)
        if root>=0x140030000:continue  # Four CRT library roots require external platform state.
        app_roots+=1
        transfers={int(t['site'],16):t for t in b['transfers']}
        addresses={int(a,16) for a in b['instruction_addresses']}
        u=emulator(image)
        def transfer_hook(uc,addr,size,_):
            if addr not in addresses:
                raise AssertionError(f'unexpected caller address {addr:#x}')
            t=transfers.get(addr)
            if t is None:return
            for name,value in t['local_arguments'].items():
                if value is not None:
                    assert_equal(uc.reg_read(REGS[name]),int(value,16),f'{addr:#x} {name}')
            if t['local_stack_argument5'] is not None:
                rsp=uc.reg_read(R.UC_X86_REG_RSP)
                assert_equal(struct.unpack('<Q',uc.mem_read(rsp+0x20,8))[0],int(t['local_stack_argument5'],16),f'{addr:#x} stack arg5')
            argument_sites.add(addr)
            if t['target']==hex(REGISTRATION):callbacks.add(addr)
            if t['target']=='0x140345b24':vector_sites.add(addr)
            if t['kind']=='call':
                # Defined synthetic callees intentionally destroy every volatile GPR.
                for j,reg in enumerate(REGS.values()):uc.reg_write(reg,0x61000000+j*0x100)
                uc.reg_write(R.UC_X86_REG_RIP,addr+size)
            elif t['kind']=='jmp' and (t['target'] is None or int(t['target'],16) not in addresses):
                uc.reg_write(R.UC_X86_REG_RIP,STOP)
        u.hook_add(unicorn.UC_HOOK_CODE,transfer_hook)
        u.emu_start(root,STOP,count=100000)
        assert_equal(u.reg_read(R.UC_X86_REG_RIP),STOP,'caller skeleton termination')
    assert_equal(app_roots,79,'app-root selection')
    assert_equal(len(vector_sites),11,'all vector construction callers checked')
    expected_callbacks={int(r['registration_site'],16) for r in json.loads((repo/'data/reverse/crt-lifecycle-20260915/exit-registrations.json').read_text()) if int(r['initializer'],16)<0x140030000}
    assert_equal(callbacks,expected_callbacks,'all app registration bindings checked')
    sources=['include/dmc_rengine/reverse/crt_global_initializers.hpp','src/reverse/crt_global_initializers.cpp',
             'tests/reverse/crt_initializer_bridge.cpp','scripts/reverse/canonical_image.py',
             'scripts/reverse/audit_crt_lifecycle.py','scripts/reverse/verify_crt_lifecycle.py']
    result=dict(status='PASS',canonical_exe_sha256=CANONICAL,unicorn_version=unicorn.__version__,compiler=compiler,
        differential_cases=len(cases),cases=cases,caller_skeletons_checked=app_roots,
        argument_sites_checked=len(argument_sites),app_exit_registration_bindings_checked=len(callbacks),
        vector_construction_sites_checked=len(vector_sites),
        source_sha256={p:hashlib.sha256((repo/p).read_bytes()).hexdigest() for p in sources},
        limitations=['The two initializers run through their memory operations; exit registration is intercepted.',
          'The 79 caller-skeleton checks stub all callees and cover reached paths only; they do not run real constructors.',
          'Four CRT platform initializer bodies are statically catalogued, not emulated here.',
          'No canonical game runtime acceptance or full object-lifecycle claim.'])
    out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n')
    return {k:v for k,v in result.items() if k not in ('cases','source_sha256')}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('exe','repo','out'):p.add_argument(name,type=Path)
    a=p.parse_args();print(json.dumps(run(a.exe,a.repo.resolve(),a.out),indent=2))
