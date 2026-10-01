"""RIP-relative lea/mov references to a VA: python3 leaxref.py dmc3.exe 0x1404D8588 [...]"""
import struct, sys
from pe import Image
img = Image(sys.argv[1]); tbase, text = img.text()
targets = {int(a, 16) for a in sys.argv[2:]}
for i in range(len(text) - 7):
    if text[i] in (0x48, 0x4C) and text[i + 1] in (0x8D, 0x8B) and (text[i + 2] & 0xC7) == 0x05:
        t = tbase + i + 7 + struct.unpack_from('<i', text, i + 3)[0]
        if t in targets:
            f = img.function_of(tbase + i)
            print(f'{tbase + i:#x} -> {t:#x} in {f[0]:#x}' if f else f'{tbase+i:#x} -> {t:#x}')
