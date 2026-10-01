"""Unicorn harness: run dmc3.exe functions on fabricated objects (ground truth for ports)."""
import struct, sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from unicorn import *
from unicorn.x86_const import *
from pe import Image

EXE = os.environ.get('DMC3_EXE', 'dmc3.exe')  # local copy, never committed
img = Image(EXE)
PAGE = 0x1000
def align_down(a): return a & ~(PAGE - 1)
def align_up(a): return (a + PAGE - 1) & ~(PAGE - 1)

class Emu:
    def __init__(self):
        self.mu = Uc(UC_ARCH_X86, UC_MODE_64)
        mu = self.mu
        # headers + sections
        lo = img.base; hi = img.base
        for name, va, vs, raw, rs in img.sections:
            hi = max(hi, img.base + va + max(vs, rs))
        mu.mem_map(img.base, align_up(hi - img.base), UC_PROT_ALL)
        mu.mem_write(img.base, img.data[:0x1000])
        for name, va, vs, raw, rs in img.sections:
            mu.mem_write(img.base + va, img.data[raw:raw + rs])
        # stack and heap
        self.stack_top = 0x7ff000000
        mu.mem_map(self.stack_top - 0x400000, 0x400000, UC_PROT_ALL)
        self.heap = 0x200000000
        self.heap_size = 0x10000000
        mu.mem_map(self.heap, self.heap_size, UC_PROT_ALL)
        self.heap_ptr = self.heap
        self.ret_addr = 0x7fe000000
        mu.mem_map(self.ret_addr, PAGE, UC_PROT_ALL)
        mu.mem_write(self.ret_addr, b'\xf4')  # hlt
        self.stubs = {}
        self.log = []
        self.insn_count = 0
        mu.hook_add(UC_HOOK_MEM_UNMAPPED, self._unmapped)
        mu.hook_add(UC_HOOK_CODE, self._code)
        self.trace = False
    def alloc(self, size, align=16):
        self.heap_ptr = (self.heap_ptr + align - 1) & ~(align - 1)
        p = self.heap_ptr; self.heap_ptr += size
        return p
    def write(self, addr, data): self.mu.mem_write(addr, bytes(data))
    def read(self, addr, n): return bytes(self.mu.mem_read(addr, n))
    def u32(self, a): return struct.unpack('<I', self.read(a, 4))[0]
    def u64(self, a): return struct.unpack('<Q', self.read(a, 8))[0]
    def f32(self, a): return struct.unpack('<f', self.read(a, 4))[0]
    def wf(self, a, v): self.write(a, struct.pack('<f', v))
    def w32(self, a, v): self.write(a, struct.pack('<I', v & 0xffffffff))
    def w64(self, a, v): self.write(a, struct.pack('<Q', v))
    def _unmapped(self, mu, access, address, size, value, ud):
        base = align_down(address)
        if address < 0x10000:
            self.log.append(f'NULL access {address:#x} at rip={mu.reg_read(UC_X86_REG_RIP):#x}')
            return False
        try: mu.mem_map(base, PAGE, UC_PROT_ALL)
        except Exception: return False
        self.log.append(f'lazy map {base:#x} rip={mu.reg_read(UC_X86_REG_RIP):#x}')
        return True
    def _code(self, mu, address, size, ud):
        self.insn_count += 1
        if self.insn_count > getattr(self, "limit", 5_000_000): mu.emu_stop(); self.log.append("LIMIT")
        s = self.stubs.get(address)
        if s is not None:
            s(self)
            # return to caller
            rsp = mu.reg_read(UC_X86_REG_RSP)
            ret = struct.unpack('<Q', mu.mem_read(rsp, 8))[0]
            mu.reg_write(UC_X86_REG_RSP, rsp + 8)
            mu.reg_write(UC_X86_REG_RIP, ret)
    def call(self, addr, *args, xmm=None, stack=()):
        mu = self.mu
        regs = [UC_X86_REG_RCX, UC_X86_REG_RDX, UC_X86_REG_R8, UC_X86_REG_R9]
        for r, a in zip(regs, args): mu.reg_write(r, a & 0xffffffffffffffff)
        rsp = self.stack_top - 0x1000
        rsp -= 0x100 + 8 * len(stack); rsp &= ~0xf
        for i, v in enumerate(stack): self.w64(rsp + 0x20 + 8 * i, v)
        rsp -= 8
        self.w64(rsp, self.ret_addr)
        mu.reg_write(UC_X86_REG_RSP, rsp)
        if xmm:
            for i, v in xmm.items():
                mu.reg_write(UC_X86_REG_XMM0 + i, struct.pack('<f', v) + b'\0' * 12 if isinstance(v, float) else v)
        self.insn_count = 0
        try:
            mu.emu_start(addr, self.ret_addr)
        except UcError as e:
            rip = mu.reg_read(UC_X86_REG_RIP)
            raise RuntimeError(f'emulation error {e} at rip={rip:#x} after {self.insn_count} insns; log tail {self.log[-5:]}')
        return mu.reg_read(UC_X86_REG_RAX)
