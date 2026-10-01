"""Function bounds from .pdata: python3 pdata.py dmc3.exe 0x14021537A ..."""
import sys
from pe import Image

img = Image(sys.argv[1])
for a in sys.argv[2:]:
    va = int(a, 16)
    f = img.function_of(va)
    print(f'{va:#x}: ' + (f'function {f[0]:#x}..{f[1]:#x}' if f else 'no .pdata entry (padding or leaf)'))
