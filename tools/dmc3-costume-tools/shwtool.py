"""SHW shadow hulls (player slot 8 = body, slot 14 = coat).

  python3 shwtool.py show    file.pac
  python3 shwtool.py rebuild file.pac -o out.pac [--min-verts 8]

rebuild regenerates slot 8 from the body MOD (slot 1) and slot 14 from the
coat MOD (slot 12): one closed convex hull per joint over the rest-space
vertices whose dominant skin joint it is; joints with fewer than --min-verts
vertices join their parent's hull (a 24-node Dante-type body uses Dante's 17
body groups). Hull: extreme points along 18 directions -> convex hull, CCW
outward triangles, neighbour across edge (v_i, v_i+1), w = 1, blocks aligned
to 16, T = 2V - 4. The 0x20-byte header is written from scratch: 'SHW ',
f32 0.5, +0x10 hull count, +0x11 node count, +0x12 = 1 (body) / +0x14 = 2
(coat), as in Dante's files.
Run it after editing a MOD so the floor shadow matches the model.
"""
import argparse
import struct

import numpy as np

from fixmod import build_pac, build_shw, hull_of, mod_skin, pac_slots
from modwriter import read_mod

DANTE_BODY = [(5, [5, 23]), (3, [3, 6, 10]), (4, [4]), (14, [14]), (15, [15]), (19, [19]),
              (9, [9]), (16, [16]), (17, [17, 18]), (20, [20]), (21, [21, 22]), (8, [8]),
              (7, [7]), (11, [11]), (12, [12]), (13, [13]), (2, [2])]

def header(body):
    h = bytearray(0x20); h[0:4] = b'SHW '; struct.pack_into('<f', h, 4, 0.5)
    if body: h[0x12] = 1
    else: h[0x14] = 2
    return bytes(h)

def groups_for(mod_bytes, min_verts, body):
    m = read_mod(mod_bytes)
    nodes = len(m.transforms)
    if body and nodes == 24:
        return DANTE_BODY, nodes
    _, joints = mod_skin(mod_bytes)
    parent = {node: m.parent[i] for i, node in enumerate(m.order)}
    owner = {j: j for j in range(nodes)}
    counts = {j: int((joints == j).sum()) for j in range(nodes)}
    for j in sorted(range(nodes), key=lambda j: -j):     # children before parents
        if counts[j] < min_verts and parent[j] != 255:
            p = parent[j]
            while owner[p] != p: p = owner[p]
            owner[j] = p; counts[p] += counts[j]
    groups = {}
    for j in range(nodes):
        r = j
        while owner[r] != r: r = owner[r]
        groups.setdefault(r, []).append(j)
    return [(j, members) for j, members in sorted(groups.items())], nodes

def build(mod_bytes, min_verts, body):
    pos, joints = mod_skin(mod_bytes, '%.6f')
    groups, nodes = groups_for(mod_bytes, min_verts, body)
    hulls = []
    for joint, members in groups:
        pts = pos[np.isin(joints, members)]
        if len(pts) < 4: continue
        h = hull_of(pts)
        if h is not None: hulls.append((*h, joint))
    print(f"  {'body' if body else 'coat'}: {len(hulls)} hulls over {nodes} nodes")
    return build_shw(hulls, header(body), nodes)

def show(shw, label):
    n = shw[0x10]
    print(f'{label}: {n} hulls, node count {shw[0x11]}, header {shw[:0x20].hex()}')
    for h in range(n):
        r = 0x20 + h * 0x40
        vc, tc = struct.unpack_from('<HH', shw, r)
        vo, so = struct.unpack_from('<QQ', shw, r + 0x20)
        ys = [struct.unpack_from('<f', shw, vo + 16 * i + 4)[0] for i in range(vc)]
        print(f'  {h:2}: joint {shw[so]:2} V {vc:2} T {tc:2}  y {min(ys):.1f}..{max(ys):.1f}')

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('cmd', choices=['show', 'rebuild'])
    ap.add_argument('file')
    ap.add_argument('-o', '--out')
    ap.add_argument('--min-verts', type=int, default=8)
    a = ap.parse_args()
    slots = list(pac_slots(open(a.file, 'rb').read()))
    if a.cmd == 'show':
        for k in (8, 14):
            if k < len(slots) and slots[k][:4] == b'SHW ': show(slots[k], f'slot {k}')
        return
    slots[8] = build(slots[1], a.min_verts, True)
    shw14 = build(slots[12], a.min_verts, False)
    if len(slots) > 14: slots[14] = shw14
    else:
        while len(slots) < 14: slots.append(b'')
        slots.append(shw14)
    out = a.out or a.file + '.new'
    open(out, 'wb').write(build_pac(slots)); print('written', out)

if __name__ == '__main__':
    main()
