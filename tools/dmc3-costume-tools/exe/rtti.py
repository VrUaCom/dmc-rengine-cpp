"""MSVC RTTI class of a vtable or of a vtable slot:
python3 rtti.py dmc3.exe 0x1404DF990 0x1404DFA38
Walks back to the vtable start (the slot after a valid CompleteObjectLocator)
and prints the class name and the slot index."""
import sys
from pe import Image

img = Image(sys.argv[1])
def col_ok(c):
    if img.section_of(c) != '.rdata': return False
    return img.u32(c) == 1 and img.u32(c + 0x14) == c - img.base
for a in sys.argv[2:]:
    va = int(a, 16)
    v = va
    while not col_ok(img.u64(v - 8)):
        v -= 8
        if va - v > 0x2000:
            print(f'{va:#x}: no vtable found'); break
    else:
        col = img.u64(v - 8)
        td = img.base + img.u32(col + 12)
        o = img.off(td + 0x10)
        name = img.data[o:o + 128].split(b'\0')[0].decode(errors='replace')
        print(f'{va:#x}: vtable {v:#x} {name} slot {(va - v) // 8}')
