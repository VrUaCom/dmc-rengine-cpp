"""Regenerate init_funcs.txt: the C++ static initializers the emulator must run first.

The initializer table of dmc3.exe (.rdata 0x14034F7F0..0x14035D2B0, function
pointers and zero separators) holds 6989 entries. The emulator needs the ones
that copy a constant from .rdata into a global (e.g. the mask at 0x1405D9F30
that keeps a matrix row's w): frameless leaf functions (no push, call or jmp
before their ret) that load an XMM register from a RIP-relative constant. 6260 entries qualify; the zeroing ones are skipped
(the image's .data is already zero there).

python3 make_init_funcs.py > init_funcs.txt     (DMC3_EXE = path to dmc3.exe)
"""
import os, sys
import capstone
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from pe import Image

LO, HI = 0x14034F7F0, 0x14035D2B0
img = Image(os.environ.get('DMC3_EXE', 'dmc3.exe'))
tb, text = img.text()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64); md.detail = True
out = []
for a in range(LO, HI, 8):
    fn = img.u64(a)
    if not (tb <= fn < tb + len(text)): continue
    loads = False
    for x in md.disasm(text[fn - tb:fn - tb + 0x400], fn):
        if x.mnemonic in ('call', 'jmp', 'push'): loads = False; break
        if x.mnemonic == 'ret': break
        if len(x.operands) == 2 and x.operands[0].type == capstone.x86.X86_OP_REG and \
           x.operands[1].type == capstone.x86.X86_OP_MEM and x.operands[1].mem.base == capstone.x86.X86_REG_RIP and \
           x.reg_name(x.operands[0].reg).startswith('xmm'):
            loads = True
    if loads: out.append(fn)
print('\n'.join(f'{f:#x}' for f in out))
