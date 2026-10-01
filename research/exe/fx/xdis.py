"""Disassemble a VA range: python3 xdis.py dmc3.exe 0x1402120A7 0x14021210A
(end optional: the .pdata function end, or 0x100 bytes)."""
import sys
import capstone
from pe import Image

img = Image(sys.argv[1])
start = int(sys.argv[2], 16)
if len(sys.argv) > 3:
    end = int(sys.argv[3], 16)
else:
    f = img.function_of(start)
    end = f[1] if f else start + 0x100
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
o = img.off(start)
for ins in md.disasm(img.data[o:o + (end - start)], start):
    print(f'{ins.address:#x} (file {img.off(ins.address):#x}) {ins.bytes.hex():24} {ins.mnemonic} {ins.op_str}')
