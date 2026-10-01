"""PTX texture tool (player slot 0, stage slot 1, enemy slot 0 ...).

  python3 ptxtool.py list    file.pac|file.ptx [--slot N]
  python3 ptxtool.py export  file.pac outdir [--slot N] [--mips]
  python3 ptxtool.py replace file.pac INDEX image.png -o out.pac [--slot N] [--format dxt1|dxt5]
  python3 ptxtool.py add     file.pac image.png -o out.pac [--slot N] [--format dxt1|dxt5]
  python3 ptxtool.py fix     file.pac -o out.pac [--slot N]

Without --slot every PTX slot of the PAC is used (list/export/fix) or the
first one (replace/add). A plain PTX bundle works in place of a PAC.
replace/add encode the PNG with a full mip chain (2x2 box filter) and a
canonical descriptor; the format defaults to the replaced texture's (DXT5 for
add). Untouched textures keep their bytes. `fix` rewrites non-canonical
descriptors and completes mip chains (as fixmod.py does for pl011), leaving
canonical textures byte-identical.

Replacing a texture keeps its index, so the MODs that use it (mesh record
+0x02 = texture index) need no change. After `add`, point a mesh at the new
index yourself; the MOD header +0x12 mirrors the texture count, but the game
takes the live count from the PTX (0x1402F9570).
"""
import argparse
import os
import sys

import numpy as np
from PIL import Image

import dxt
from fixmod import build_pac, pac_slots

def load(path):
    b = open(path, 'rb').read()
    if b[:4] == b'PAC\0':
        return pac_slots(b), True
    if dxt.is_ptx(b):
        return [b], False
    raise SystemExit(f'{path}: neither a PAC nor a PTX bundle')

def ptx_slots(slots, wanted):
    found = [i for i, s in enumerate(slots) if dxt.is_ptx(s)]
    if wanted is not None:
        if wanted not in found: raise SystemExit(f'slot {wanted} is not a PTX bundle (PTX slots: {found})')
        return [wanted]
    if not found: raise SystemExit('no PTX slot')
    return found

def save(path, slots, is_pac):
    open(path, 'wb').write(build_pac(slots) if is_pac else slots[0])
    print('written', path)

def read_png(path):
    return np.asarray(Image.open(path).convert('RGBA'))

def fourcc_of(name, default):
    if name is None: return default
    return {'dxt1': b'DXT1', 'dxt5': b'DXT5'}[name.lower()]

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('cmd', choices=['list', 'export', 'replace', 'add', 'fix'])
    ap.add_argument('file')
    ap.add_argument('args', nargs='*')
    ap.add_argument('--slot', type=int)
    ap.add_argument('--format')
    ap.add_argument('--mips', action='store_true')
    ap.add_argument('-o', '--out')
    a = ap.parse_args()
    slots, is_pac = load(a.file)
    targets = ptx_slots(slots, a.slot)

    if a.cmd == 'list':
        for k in targets:
            tex = dxt.read_ptx(slots[k])
            print(f'slot {k}: {len(tex)} textures, {len(slots[k])} bytes')
            for i, t in enumerate(tex):
                pr = t.problems()
                print(f'  {i:2}: {t.w:4}x{t.h:<4} {t.fourcc.decode()} mips {len(t.levels):2}/{dxt.full_mips(t.w, t.h):2}'
                      f"  {'canonical' if not pr else '; '.join(pr)}")
    elif a.cmd == 'export':
        out = a.args[0] if a.args else 'textures'
        os.makedirs(out, exist_ok=True)
        stem = os.path.splitext(os.path.basename(a.file))[0]
        for k in targets:
            for i, t in enumerate(dxt.read_ptx(slots[k])):
                for lv in (range(len(t.levels)) if a.mips else [0]):
                    name = f'{stem}_s{k}_t{i}_{t.fourcc.decode().lower()}' + (f'_m{lv}' if a.mips else '') + '.png'
                    Image.fromarray(t.image(lv), 'RGBA').save(os.path.join(out, name))
                    print(os.path.join(out, name))
    elif a.cmd in ('replace', 'add'):
        k = targets[0]
        tex = dxt.read_ptx(slots[k])
        if a.cmd == 'replace':
            index, png = int(a.args[0]), a.args[1]
            old = tex[index]
            img = read_png(png)
            if img.shape[:2] != (old.h, old.w):
                print(f'note: size {img.shape[1]}x{img.shape[0]} replaces {old.w}x{old.h}')
            tex[index] = dxt.Texture.from_image(img, fourcc_of(a.format, old.fourcc))
            print(f'slot {k} texture {index}: {tex[index].w}x{tex[index].h} {tex[index].fourcc.decode()}, {len(tex[index].levels)} mips')
        else:
            img = read_png(a.args[0])
            tex.append(dxt.Texture.from_image(img, fourcc_of(a.format, b'DXT5')))
            print(f'slot {k}: added texture {len(tex) - 1} ({img.shape[1]}x{img.shape[0]})')
        slots = list(slots); slots[k] = dxt.write_ptx(tex, canonical=False)
        save(a.out or a.file + '.new', slots, is_pac)
    elif a.cmd == 'fix':
        slots = list(slots)
        for k in targets:
            tex = dxt.read_ptx(slots[k])
            for i, t in enumerate(tex):
                pr = t.problems()
                if not pr: continue
                added = t.complete_mips()
                t.descriptor = t.dds = None       # rewrite canonically
                print(f'slot {k} texture {i}: fixed ({"; ".join(pr)}){f", +{added} mips" if added else ""}')
            slots[k] = dxt.write_ptx(tex, canonical=False)
        save(a.out or a.file + '.fixed', slots, is_pac)

if __name__ == '__main__':
    main()
