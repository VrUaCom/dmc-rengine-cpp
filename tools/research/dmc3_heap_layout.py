#!/usr/bin/env python3
# Usage: dmc3_heap_layout.py <dir holding pe_view.py> <path to your own dmc3.exe>
# Prints where dmc3.exe (HD Collection, x64) places each region of its main
# memory block, by emulating the setup routine at +30190 with the allocator
# stubbed. Reads the executable you point it at; ships no game bytes.
# Emulates dmc3.exe+30190 (main memory setup) with the allocator stubbed,
# and reports every global it writes: where each region starts.
import sys, struct
sys.path.insert(0, sys.argv[1])
from pe_view import load
from unicorn import *
from unicorn.x86_const import *
b,base,secs=load(sys.argv[2])
mu=Uc(UC_ARCH_X86, UC_MODE_64)
img=0xE00000
mu.mem_map(base, img+0x100000)
for n,va,vsz,rp,rsz in secs:
    mu.mem_write(base+va, b[rp:rp+min(rsz,vsz if vsz else rsz)])
STACK=0x7f0000000; mu.mem_map(STACK,0x100000)
HEAP=0x10000000
mu.mem_map(HEAP, 0x10400000)
mu.mem_map(0, 0x100000)
mu.reg_write(UC_X86_REG_RSP, STACK+0x80000)
writes={}
def hook_code(uc, addr, size, ud):
    rva=addr-base
    if rva==0x490D0:  # allocator: return fake heap base
        uc.reg_write(UC_X86_REG_RAX, HEAP)
        rsp=uc.reg_read(UC_X86_REG_RSP); ret=struct.unpack('<Q',uc.mem_read(rsp,8))[0]
        uc.reg_write(UC_X86_REG_RSP,rsp+8); uc.reg_write(UC_X86_REG_RIP,ret)
def hook_mem(uc, access, addr, size, value, ud):
    if base+0x553000<=addr<base+0xD73000 and size==8:
        writes[addr-base]=value
mu.hook_add(UC_HOOK_CODE, hook_code, begin=base+0x490D0, end=base+0x490D0)
mu.hook_add(UC_HOOK_MEM_WRITE, hook_mem)
RET=base+0x30180+1  # 'mov al,1; ret' area is fine as a stop
retaddr=0x6f0000000; mu.mem_map(retaddr,0x1000)
mu.mem_write(STACK+0x80000, struct.pack('<Q',retaddr))
try:
    mu.emu_start(base+0x30190, retaddr, count=200000)
except UcError as e:
    print('stopped:',e, hex(mu.reg_read(UC_X86_REG_RIP)-base))
prev=None
rows=sorted((v,k) for k,v in writes.items() if HEAP<=v<=HEAP+0x20000000)
for i,(v,k) in enumerate(rows):
    nxt=rows[i+1][0] if i+1<len(rows) else None
    print(f"global +{k:06X} = heap+{v-HEAP:#010x}" + (f"   size {nxt-v:#x} ({(nxt-v)/1048576:.2f} MiB)" if nxt else ""))
