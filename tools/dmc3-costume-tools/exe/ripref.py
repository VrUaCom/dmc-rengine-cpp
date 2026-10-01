"""Any rip-relative memory operand to VAs: python3 ripref.py dmc3.exe 0xVA [0xVA..] (accepts imm suffix 0/1/2/4)"""
import struct, sys
from pe import Image
img = Image(sys.argv[1]); tbase, text = img.text()
targets = {int(a, 16) for a in sys.argv[2:]}
for i in range(3, len(text) - 4):
    # modrm with mod=00 rm=101 sits right before the disp32
    if (text[i - 1] & 0xC7) != 0x05: continue
    disp = struct.unpack_from('<i', text, i)[0]
    for extra in (0, 1, 2, 4):
        t = tbase + i + 4 + extra + disp
        if t in targets:
            f = img.function_of(tbase + i)
            print(f'{tbase + i - 2:#x} -> {t:#x} (+{extra}) in {f[0]:#x}' if f else f'{tbase+i-2:#x} -> {t:#x}')
