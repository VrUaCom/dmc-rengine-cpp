"""MOD model tool.

  python3 modtool.py info      file.pac SLOT | file.mod
  python3 modtool.py layout    file.pac SLOT | file.mod
  python3 modtool.py roundtrip file.pac SLOT | file.mod
  python3 modtool.py obj       file.pac SLOT out.obj [--tex-slot 0]
  python3 modtool.py render    file.pac SLOT out.png [--yaw 0] [--size 600] [--tex-slot 0]

MOD layout (retail files, canonical dmc3.exe):
  header 0x40: +0x10 objects, +0x11 nodes, +0x12 texture count (mirror),
    +0x13 default joint (a coat root offset under coat_patch.py), +0x14 u32,
    +0x20 node block
  object 0x40: +0 mesh count, +1 alpha, +2 element sum, +8 mesh table,
    +0x10 flags, +0x30 bounding sphere
  mesh 0x50: +0 vertex count, +2 texture index (into the PAC's PTX, slot 0
    for players), +0x10..+0x30 streams: position f32x3, normal f32x3,
    uv i16x2 / 4096, blend u8x4 (lanes 1..3 = bone*4), control u16
    (bit 15 = strip break, three 5-bit weights summing to 31)
  node block: parent (by order position), order, adapter, transforms
    (translation, length, rotation XYZ radians; row-vector Rx.Ry.Rz)
Triangles: a strip restarted at every break vertex (two leading breaks start
a strip); the winding follows the vertex normals, as the game's post-load.
Vertices are stored in rest model space (a coat's in coat-root space).
The render mirrors X for display: DMC3 data is right-handed (Dante's right
hand is on -X).
"""
import argparse
import math
import os
import struct
import sys

import numpy as np

import dxt
from fixmod import pac_slots
from modwriter import read_mod, write_mod

def load(path, slot):
    b = open(path, 'rb').read()
    if b[:4] == b'PAC\0':
        slots = pac_slots(b)
        if slot is None: raise SystemExit('give the slot of the MOD (1 body, 12 coat)')
        if slots[slot][:4] != b'MOD ': raise SystemExit(f'slot {slot} is not a MOD')
        return slots[slot], slots
    if b[:4] != b'MOD ': raise SystemExit('not a MOD')
    return b, None

def arrays(me):
    n = me.count
    pos = np.frombuffer(me.pos, '<f4').reshape(n, 3).astype(np.float64)
    nrm = np.frombuffer(me.nrm, '<f4').reshape(n, 3).astype(np.float64)
    uv = np.frombuffer(me.uv, '<i2').reshape(n, 2).astype(np.float64) / 4096.0
    bl = np.frombuffer(me.blend, 'u1').reshape(n, 4)
    ctl = np.frombuffer(me.ctl, '<u2')
    return pos, nrm, uv, bl, ctl

def triangles(pos, nrm, ctl):
    out = []
    for i in range(2, len(ctl)):
        if ctl[i] & 0x8000: continue
        a, b, c = i - 2, i - 1, i
        face = np.cross(pos[c] - pos[a], pos[b] - pos[a])
        s = nrm[a] + nrm[b] + nrm[c]
        if not s.any(): s = face
        out.append((a, b, c) if np.dot(s, face) > 0 else (a, c, b))
    return out

def rest_worlds(m):
    """Rest world translation+rotation per node (row-vector)."""
    n = len(m.transforms)
    local = []
    for t in m.transforms:
        tx, ty, tz, _, rx, ry, rz, _ = struct.unpack('<8f', t)
        cx, sx, cy, sy, cz, sz = math.cos(rx), math.sin(rx), math.cos(ry), math.sin(ry), math.cos(rz), math.sin(rz)
        Rx = np.array([[1, 0, 0], [0, cx, sx], [0, -sx, cx]])
        Ry = np.array([[cy, 0, -sy], [0, 1, 0], [sy, 0, cy]])
        Rz = np.array([[cz, sz, 0], [-sz, cz, 0], [0, 0, 1]])
        M = np.eye(4); M[:3, :3] = Rx @ Ry @ Rz; M[3, :3] = (tx, ty, tz)
        local.append(M)
    world = [None] * n
    for pos_i, node in enumerate(m.order):
        p = m.parent[pos_i]
        world[node] = local[node] if p == 255 else local[node] @ world[p]
    return world

def cmd_info(b):
    m = read_mod(b)
    h = m.header
    print(f'MOD version {struct.unpack_from("<f", h, 4)[0]:.2f}: {h[0x10]} objects, {h[0x11]} nodes, '
          f'{h[0x12]} texture slots, +0x13 = {h[0x13]}, +0x14 = {struct.unpack_from("<I", h, 0x14)[0]}')
    tex_use, bones = {}, {}
    for oi, o in enumerate(m.objects):
        r = o.record
        cx, cy, cz, rad = struct.unpack_from('<4f', r, 0x30)
        print(f'object {oi:2}: meshes {len(o.meshes)}, alpha {r[1]}, flags {r[0x10:0x14].hex()}, '
              f'sphere ({cx:.1f}, {cy:.1f}, {cz:.1f}) r {rad:.1f}')
        for k, me in enumerate(o.meshes):
            pos, nrm, uv, bl, ctl = arrays(me)
            tris = triangles(pos, nrm, ctl) if me.count >= 3 else []
            tex = me.record[2]
            tex_use[tex] = tex_use.get(tex, 0) + len(tris)
            w = np.stack([ctl & 31, (ctl >> 5) & 31, (ctl >> 10) & 31], 1)
            for i in range(me.count):
                for q in range(3):
                    if w[i, q]: bones[bl[i, q + 1] // 4] = bones.get(bl[i, q + 1] // 4, 0) + 1
            lo, hi = (pos.min(0), pos.max(0)) if me.count else (np.zeros(3), np.zeros(3))
            print(f'   mesh {k}: {me.count:5} verts, {len(tris):5} tris, texture {tex}, '
                  f'y {lo[1]:.1f}..{hi[1]:.1f} x {lo[0]:.1f}..{hi[0]:.1f}')
    print('triangles per texture:', dict(sorted(tex_use.items())))
    print('vertex influences per bone:', dict(sorted(bones.items())))
    world = rest_worlds(m)
    print('nodes (parent, rest world position):')
    parent_of = {node: m.parent[i] for i, node in enumerate(m.order)}
    for n in range(len(m.transforms)):
        p = parent_of[n]
        print(f'   {n:2} <- {"root" if p == 255 else p:>4}  {np.round(world[n][3, :3], 2).tolist()}')

def cmd_layout(b):
    u64 = lambda o: struct.unpack_from('<Q', b, o)[0]
    regions = [(0, 0x40, 'header')]
    nobj, nodes, doc = b[0x10], b[0x11], u64(0x20)
    regions.append((0x40, 0x40 + nobj * 0x40, 'object records'))
    for oi in range(nobj):
        r = 0x40 + oi * 0x40; cnt, tab = b[r], u64(r + 8)
        regions.append((tab, tab + cnt * 0x50, f'mesh records o{oi}'))
        for k in range(cnt):
            mr = tab + k * 0x50; n = struct.unpack_from('<H', b, mr)[0]
            for name, off, size in (('pos', 0x10, 12), ('nrm', 0x18, 12), ('uv', 0x20, 4), ('blend', 0x28, 4), ('ctl', 0x30, 2)):
                p = u64(mr + off); regions.append((p, p + size * n, f'{name} o{oi}m{k}'))
            ws = mr + u64(mr + 0x40)
            size = (6 * max(0, n - 2) + 15) & ~15
            regions.append((ws, ws + size, f'workspace o{oi}m{k}'))
    rel = [struct.unpack_from('<I', b, doc + 4 * i)[0] for i in range(4)]
    regions.append((doc, doc + 0x10, 'node block header'))
    for name, rr, size in zip(('parent', 'order', 'adapter', 'transforms'), rel, (nodes, nodes, nodes, 0x20 * nodes)):
        regions.append((doc + rr, doc + rr + size, name))
    regions.sort()
    end = 0
    for a, z, name in regions:
        gap = a - end
        print(f'{a:#8x}..{z:#8x} {z - a:7} {name}' + (f'   (gap {gap:#x})' if gap > 0 else ''))
        end = max(end, z)
    print(f'end {len(b):#x}, after last region {len(b) - end:#x}')

def textures_of(slots, tex_slot):
    if slots is None: return {}
    if tex_slot is None:
        tex_slot = next((i for i, s in enumerate(slots) if dxt.is_ptx(s)), None)
    if tex_slot is None or not dxt.is_ptx(slots[tex_slot]): return {}
    return {i: t.image() for i, t in enumerate(dxt.read_ptx(slots[tex_slot]))}

def cmd_obj(b, slots, out, tex_slot):
    from PIL import Image
    m = read_mod(b)
    textures = textures_of(slots, tex_slot)
    base = os.path.splitext(out)[0]; stem = os.path.basename(base)
    used = set()
    with open(out, 'w') as f:
        f.write(f'# DMC3 MOD export (rest pose, model space)\nmtllib {stem}.mtl\n')
        vbase = 1
        for oi, o in enumerate(m.objects):
            for k, me in enumerate(o.meshes):
                if me.count < 3: continue
                pos, nrm, uv, bl, ctl = arrays(me)
                tris = triangles(pos, nrm, ctl)
                if not tris: continue
                tex = me.record[2]; used.add(tex)
                f.write(f'o object{oi}_mesh{k}\nusemtl tex{tex}\n')
                for p in pos: f.write(f'v {p[0]:.5f} {p[1]:.5f} {p[2]:.5f}\n')
                for t in uv: f.write(f'vt {t[0]:.6f} {1 - t[1]:.6f}\n')
                for n_ in nrm: f.write(f'vn {n_[0]:.5f} {n_[1]:.5f} {n_[2]:.5f}\n')
                for a, b_, c in tris:
                    ia, ib, ic = a + vbase, b_ + vbase, c + vbase
                    f.write(f'f {ia}/{ia}/{ia} {ib}/{ib}/{ib} {ic}/{ic}/{ic}\n')
                vbase += me.count
    with open(base + '.mtl', 'w') as f:
        for tex in sorted(used):
            f.write(f'newmtl tex{tex}\nKd 1 1 1\n')
            if tex in textures:
                png = f'{stem}_tex{tex}.png'
                Image.fromarray(textures[tex], 'RGBA').save(os.path.join(os.path.dirname(out) or '.', png))
                f.write(f'map_Kd {png}\n')
    print('written', out, base + '.mtl', f'({len(used)} materials)')

def cmd_render(b, slots, out, yaw, size, tex_slot):
    from PIL import Image
    m = read_mod(b)
    textures = textures_of(slots, tex_slot)
    tris, uvs, texs = [], [], []
    for o in m.objects:
        for me in o.meshes:
            if me.count < 3: continue
            pos, nrm, uv, bl, ctl = arrays(me)
            for t in triangles(pos, nrm, ctl):
                tris.append(pos[list(t)]); uvs.append(uv[list(t)]); texs.append(me.record[2])
    img = np.full((size, size, 3), 40, np.uint8); zb = np.full((size, size), -1e9)
    c, s = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
    R = np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]])
    allp = np.concatenate(tris) @ R.T
    lo, hi = allp.min(0), allp.max(0)
    span = max(hi[0] - lo[0], hi[1] - lo[1]) * 1.05
    cx, cy = (lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2
    sc = size / span
    L = np.array([0.3, 0.5, 0.8]); L /= np.linalg.norm(L)
    for P, UV, tex in zip(tris, uvs, texs):
        Q = P @ R.T
        X = -(Q[:, 0] - cx) * sc + size / 2
        Y = -(Q[:, 1] - cy) * sc + size / 2
        x0, x1 = int(max(0, np.floor(X.min()))), int(min(size - 1, np.ceil(X.max())))
        y0, y1 = int(max(0, np.floor(Y.min()))), int(min(size - 1, np.ceil(Y.max())))
        if x1 < x0 or y1 < y0: continue
        d = (X[1] - X[0]) * (Y[2] - Y[0]) - (X[2] - X[0]) * (Y[1] - Y[0])
        if abs(d) < 1e-9: continue
        gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
        w1 = ((gx - X[0]) * (Y[2] - Y[0]) - (X[2] - X[0]) * (gy - Y[0])) / d
        w2 = ((X[1] - X[0]) * (gy - Y[0]) - (gx - X[0]) * (Y[1] - Y[0])) / d
        w0 = 1 - w1 - w2
        msk = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
        if not msk.any(): continue
        z = w0 * Q[0, 2] + w1 * Q[1, 2] + w2 * Q[2, 2]
        sub = zb[y0:y1 + 1, x0:x1 + 1]; msk &= z > sub
        if not msk.any(): continue
        fn = np.cross(P[1] - P[0], P[2] - P[0]); nn = np.linalg.norm(fn)
        shade = 0.45 + 0.55 * abs(fn @ R.T @ L) / nn if nn > 0 else 1.0
        t = textures.get(tex)
        if t is None:
            col = np.array([[200.0, 200.0, 200.0]])
        else:
            u = w0 * UV[0, 0] + w1 * UV[1, 0] + w2 * UV[2, 0]
            v = w0 * UV[0, 1] + w1 * UV[1, 1] + w2 * UV[2, 1]
            th, tw = t.shape[:2]
            col = t[(np.mod(v[msk], 1) * th).astype(int) % th, (np.mod(u[msk], 1) * tw).astype(int) % tw, :3].astype(float)
        img[y0:y1 + 1, x0:x1 + 1][msk] = np.clip(col * shade, 0, 255).astype(np.uint8)
        sub[msk] = z[msk]
    Image.fromarray(img).save(out)
    print('written', out, f'({len(tris)} triangles)')

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('cmd', choices=['info', 'layout', 'roundtrip', 'obj', 'render'])
    ap.add_argument('file')
    ap.add_argument('rest', nargs='*')
    ap.add_argument('--yaw', type=float, default=0.0)
    ap.add_argument('--size', type=int, default=600)
    ap.add_argument('--tex-slot', type=int)
    a = ap.parse_args()
    is_pac = open(a.file, 'rb').read(4) == b'PAC\0'
    slot = int(a.rest[0]) if is_pac and a.rest else None
    rest = a.rest[1:] if is_pac else a.rest
    b, slots = load(a.file, slot)
    if a.cmd == 'info': cmd_info(b)
    elif a.cmd == 'layout': cmd_layout(b)
    elif a.cmd == 'roundtrip':
        same = write_mod(read_mod(b)) == b
        print('byte-identical' if same else 'differs (not written by the retail tool layout)')
        sys.exit(0 if same else 1)
    elif a.cmd == 'obj': cmd_obj(b, slots, rest[0], a.tex_slot)
    elif a.cmd == 'render': cmd_render(b, slots, rest[0], a.yaw, a.size, a.tex_slot)

if __name__ == '__main__':
    main()
