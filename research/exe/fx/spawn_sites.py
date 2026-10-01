"""Every direct call to the effect spawn API, with (kind, id, prep mode) and the owning class.

python3 spawn_sites.py dmc3.exe > spawn_sites.json

* API: 0x1402E7A90 (A90), 0x1402E7AB0 (AB0), 0x1402E7CA0 (CA0), 0x1402E7A80 (A80);
  kind in ecx, id in edx, prep mode / flags in r9d (resolve.py also follows
  `lea ecx, [rdx - N]`).
* Owner: the class whose own vtable functions (functions in exactly one
  class's vtables) surround the site's function in address order. MSVC lays a
  translation unit out contiguously, so this names the unit; "A|B" when the
  neighbours differ. (Class spans built from *all* vtable entries start at
  shared base functions and over-count; that was the v74 "998 sites" error.)
"""
import collections, bisect, json, struct, sys
from pe import Image
from resolve import resolve

API = {0x1402E7A90: 'A90', 0x1402E7AB0: 'AB0', 0x1402E7CA0: 'CA0', 0x1402E7A80: 'A80'}
img = Image(sys.argv[1]); tbase, text = img.text(); d = img.data
_, va0, vs0, raw0, rs0 = next(s for s in img.sections if s[0] == '.rdata')

def col_ok(c):
    if img.section_of(c) != '.rdata': return False
    try: return img.u32(c) == 1 and img.u32(c + 0x14) == c - img.base
    except Exception: return False

vt_funcs = collections.defaultdict(set)
i = raw0
while i < raw0 + rs0 - 16:
    p = struct.unpack_from('<Q', d, i)[0]
    if img.base <= p < img.base + 0x10000000 and img.section_of(p) == '.rdata' and col_ok(p):
        td = img.base + struct.unpack_from('<I', d, img.off(p + 12))[0]
        o = img.off(td + 0x10); name = d[o:o + 128].split(b'\0')[0].decode(errors='replace')[4:-2]
        vt = img.base + va0 + (i - raw0) + 8; n = 0
        while n < 200:
            f = struct.unpack_from('<Q', d, img.off(vt + 8 * n))[0]
            if not (tbase <= f < tbase + len(text)): break
            vt_funcs[f].add(name); n += 1
        i += 8 * max(n, 1)
    else:
        i += 8
own = sorted((f, next(iter(c))) for f, c in vt_funcs.items() if len(c) == 1)
addrs = [a for a, _ in own]

def owner(fn):
    k = bisect.bisect_right(addrs, fn)
    b = own[k - 1][1] if k else '?'; a = own[k][1] if k < len(own) else '?'
    return b if a == b else f'{b}|{a}'

sites = []
for j in range(len(text) - 5):
    if text[j] != 0xE8: continue
    t = tbase + j + 5 + struct.unpack_from('<i', text, j + 1)[0]
    if t not in API: continue
    at = tbase + j
    fn = img.function_of(at)
    kind, id_, mode = resolve(img, at)
    sites.append({'call': hex(at), 'api': API[t], 'function': hex(fn[0]) if fn else None,
                  'owner': owner(fn[0]) if fn else '?',
                  'kind': 'PEGV'[kind] if kind is not None and kind < 4 else None,
                  'id': id_, 'mode': mode})
json.dump(sites, sys.stdout, indent=1)
