"""Find the vtable(s) of a class by RTTI name or TypeDescriptor VA and list slots.
python3 vtables.py dmc3.exe CEm034Shl02 [--slots 40]"""
import struct, sys
from pe import Image
img = Image(sys.argv[1]); name = sys.argv[2]
nslots = int(sys.argv[sys.argv.index('--slots') + 1]) if '--slots' in sys.argv else 40
d = img.data
rdata = next(s for s in img.sections if s[0] == '.rdata'); data = next(s for s in img.sections if s[0] == '.data')
def td_name(td):
    o = img.off(td + 0x10)
    return d[o:o + 128].split(b'\0')[0].decode(errors='replace') if o else ''
# find TD
if name.startswith('0x'): tds = [int(name, 16)]
else:
    key = f'.?AV{name}@@'.encode() + b'\0'
    tds = []
    for sec in (data, rdata):
        _, va, vs, raw, rs = sec
        i = d.find(key, raw, raw + rs)
        while i != -1:
            tds.append(img.base + va + i - raw - 0x10); i = d.find(key, i + 1, raw + rs)
for td in tds:
    rva = td - img.base
    _, va, vs, raw, rs = rdata
    for o in range(raw, raw + rs - 0x18, 4):
        if struct.unpack_from('<I', d, o)[0] == 1 and struct.unpack_from('<I', d, o + 12)[0] == rva:
            col = img.base + va + o - raw
            if struct.unpack_from('<I', d, o + 0x14)[0] != col - img.base: continue
            off_in_obj = struct.unpack_from('<I', d, o + 4)[0]
            p = struct.pack('<Q', col); i = d.find(p, raw, raw + rs)
            while i != -1:
                vt = img.base + va + i - raw + 8
                print(f'{td_name(td)} COL {col:#x} this-offset {off_in_obj:#x} vtable {vt:#x}')
                for k in range(nslots):
                    f = img.u64(vt + 8 * k)
                    if img.section_of(f) != '.text': break
                    fn = img.function_of(f)
                    print(f'   [{k:2}] +{8*k:#05x} {f:#x}' + ('' if fn and fn[0] == f else ' (thunk/mid)'))
                i = d.find(p, i + 1, raw + rs)
