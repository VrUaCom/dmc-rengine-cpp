"""pl011: move the costume (skirt with its side ribbon, both sleeves) out of
the body MOD into the slot 12 coat MOD and simulate it as cloth.

Needs dmc3.exe patched with coat_patch.py:
  * the coat root hangs from body joint 3 + coat header +0x13; this coat
    writes 11, so the root is joint 14 (pelvis) and the skirt stays on the
    hips in every motion;
  * PAC slot 15 ('CCNS') gives coat nodes constraints to body joints
    (world = offset x body joint world, 0x1402CBBE0 mode 1). Each sleeve has
    three such anchors on body joints 6/7/8 (10/11/12), with identity
    offsets, at the joints' rest positions. The sleeve's own skin weights on
    those joints carry over one to one, so the sleeve follows the arm exactly
    as before, and a cloth chain from the elbow anchor carries the lower half
    of the bell toward the cuff.

CPlDante has room for one cloth chain (+0xA210; +0xA300 holds the next joint
table, 0x1401DE820), so the CLT has one block. It collides with the six
player coat capsules (0x1402151E7), whose shapes slot 15 replaces with ones
that fit this body (CAPSULES), so every skirt chain simulates and the
thighs push the front of the skirt out of the way.

Coat nodes: 0 root, 1..24 skirt (8 chains x anchor + 2 links), 25..36
sleeves (per arm a6/a7/a8 + 3 links).
0x1401DE820 allocates 39 coat joints.

Usage: python3 costume.py mod_fixed.pac pl000.pac mod_costume.pac
"""
import math
import struct
import sys

import numpy as np
from fixmod import build_pac, mod_skin, pac_slots, shw_for
from modwriter import Mesh, Mod, Obj, read_mod, transform_record, write_mod

ROOT_JOINT = 14
SKIRT_CHAINS, SKIRT_LINKS = 8, 2
SLEEVE_LINKS = 3
STIFFNESS = 0.35
# Coat collision capsules for this body (slot 15 replaces the six shapes the
# game writes at player +0xB630; index, host joint, A, B, radius). Dante's
# chest capsule runs 40 units down in front of the hips (z +10, r 15) and
# would shove the front of a short skirt forward; here it is a torso that
# stops above the waist, and the hips and legs fit the girl's body.
CAPSULES = [
    (0, 3, (0.0, 20.0, 1.0), (0.0, -12.0, 1.0), 11.0),
    (1, 2, (0.0, -4.0, 0.0), (0.0, -14.0, 0.0), 12.0),
    (2, 15, (0.0, 0.0, 0.0), (0.0, -50.0, 0.0), 9.0),
    (3, 16, (0.0, 0.0, 0.0), (0.0, -50.0, 0.0), 9.0),
    (4, 19, (0.0, 0.0, 0.0), (0.0, -50.0, 0.0), 9.0),
    (5, 20, (0.0, 0.0, 0.0), (0.0, -50.0, 0.0), 9.0),
]
ARMS = {'right': (6, 7, 8, 9), 'left': (10, 11, 12, 13)}   # clavicle, shoulder, elbow, wrist
COAT_JOINT_CAPACITY = 39

def mesh_arrays(me):
    n = me.count
    pos = np.frombuffer(me.pos, '<f4').reshape(n, 3).astype(np.float64)
    uv = np.frombuffer(me.uv, '<i2').reshape(n, 2).astype(np.float64) / 4096.0
    bl = np.frombuffer(me.blend, 'u1').reshape(n, 4)
    ctl = np.frombuffer(me.ctl, '<u2')
    return pos, uv, bl, ctl

def weights(ctl):
    c = ctl.astype(np.int64) & 0x7fff
    return np.stack([c & 31, (c >> 5) & 31, (c >> 10) & 31], 1)

def take(me, idx):
    out = Mesh(); out.record = me.record
    for kind, size in (('pos', 12), ('nrm', 12), ('uv', 4), ('blend', 4), ('ctl', 2)):
        src = getattr(me, kind)
        setattr(out, kind, b''.join(src[i * size:(i + 1) * size] for i in idx))
    return out

def rot_y_toward(u):
    """Euler (rx, ry, 0) whose local +Y row is u (row-vector Rx.Ry.Rz)."""
    u = u / np.linalg.norm(u)
    sx = math.hypot(u[0], u[2])
    return math.atan2(sx, u[1]), math.atan2(u[0], u[2])

def decode_dxt5_rgb(dds, w, h):
    from fixmod import decode_dxt5
    return decode_dxt5(dds[128:], w, h)

def texture_rgb(ptx, index):
    off = 0x800
    for i in range(index): off += struct.unpack_from('<I', ptx, 4 + 4 * i)[0] * 0x800
    dds = ptx[off + 0x70:]
    h, w = struct.unpack_from('<II', dds, 12)
    return decode_dxt5_rgb(dds, w, h)[:, :, :3].astype(int)

def quantize(pairs):
    """[(node, weight)] -> three (node, q) with q summing to 31."""
    acc = {}
    for n, w in pairs:
        if w > 1e-6: acc[n] = acc.get(n, 0.0) + w
    top = sorted(acc.items(), key=lambda x: -x[1])[:3]
    tot = sum(w for _, w in top)
    q = [w / tot * 31 for _, w in top]
    qi = [int(x) for x in q]
    for j in sorted(range(len(q)), key=lambda j: -(q[j] - qi[j]))[:31 - sum(qi)]: qi[j] += 1
    out = [(n, w) for (n, _), w in zip(top, qi) if w > 0]
    while len(out) < 3: out.append((0, 0))
    return out

def knot_weights(s, nodes):
    """Piecewise-linear weights over knots 0 (nodes[0]), 0.5, 1.5, ..."""
    knots = [0.0] + [k + 0.5 for k in range(len(nodes) - 1)]
    if s <= knots[0]: return [(nodes[0], 1.0)]
    if s >= knots[-1]: return [(nodes[-1], 1.0)]
    for k in range(len(knots) - 1):
        if knots[k] <= s <= knots[k + 1]:
            t = (s - knots[k]) / (knots[k + 1] - knots[k])
            return [(nodes[k], 1 - t), (nodes[k + 1], t)]

def main():
    fixed_path, dante_path, out_path = sys.argv[1:4]
    slots = pac_slots(open(fixed_path, 'rb').read())
    dante = pac_slots(open(dante_path, 'rb').read())
    body = read_mod(slots[1])
    me = body.objects[2].meshes[0]
    pos, uv, bl, ctl = mesh_arrays(me)
    W = weights(ctl); Bn = bl[:, 1:4] // 4
    dom = Bn[np.arange(len(Bn)), W.argmax(1)]
    br = (ctl & 0x8000) != 0
    assert all(list(br[i:i + 3]) == [True, True, False] for i in range(0, me.count, 3))
    tris = [(i, i + 1, i + 2) for i in range(0, me.count, 3)]

    # ---- rest worlds of the body joints (all rest rotations are zero) -----
    T = [struct.unpack('<8f', t) for t in body.transforms]
    assert all(abs(x) < 1e-4 for t in T for x in t[4:7])
    world = {}
    for i, n in enumerate(body.order):
        p = body.parent[i]
        base = world[p] if p != 255 else np.zeros(3, np.float32)
        world[n] = (base + np.array(T[n][:3], np.float32)).astype(np.float32)
    J = {j: world[j].astype(np.float64) for j in world}
    JR = J[ROOT_JOINT]

    # ---- pick the costume triangles ----------------------------------------
    key = {}
    vid = [key.setdefault(tuple(np.round(p, 3)), len(key)) for p in pos]
    par = list(range(len(key)))
    def find(x):
        while par[x] != x: par[x] = par[par[x]]; x = par[x]
        return x
    for a, b, c in tris:
        par[find(vid[a])] = find(vid[b]); par[find(vid[b])] = find(vid[c])
    shells = {}
    for k, t in enumerate(tris): shells.setdefault(find(vid[t[0]]), []).append(k)
    skirt, ribbon = [], []
    for ks in shells.values():
        vs = [i for k in ks for i in tris[k]]
        P = pos[vs]; U = uv[vs]
        if P[:, 1].min() < 80 or P[:, 1].max() > 117: continue
        if U[:, 0].max() < 0.16 and 0.32 < U[:, 1].min() and U[:, 1].max() < 0.51: skirt = ks
        elif len(ks) < 40 and P[:, 0].min() > 10: ribbon = ks
    assert skirt and ribbon
    tex = texture_rgb(slots[0], 2); th, tw = tex.shape[:2]
    def is_skin(t):
        c = uv[list(t)].mean(0)
        r, g, b = tex[int(c[1] % 1 * th) % th, int(c[0] % 1 * tw) % tw]
        return g > 150 and b > 120 and r - g < 80
    sleeves = {side: [] for side in ARMS}
    for k, t in enumerate(tris):
        d = int(np.bincount(dom[list(t)]).argmax())
        for side, (_, j7, j8, _) in ARMS.items():
            if d in (j7, j8) and not is_skin(t): sleeves[side].append(k)
    cut = set(skirt) | set(ribbon) | set(sleeves['right']) | set(sleeves['left'])
    print(f'skirt {len(skirt)} ribbon {len(ribbon)} sleeves {len(sleeves["right"])}/{len(sleeves["left"])} triangles')

    keep = [i for k, t in enumerate(tris) if k not in cut for i in t]
    body.objects[2].meshes[0] = take(me, keep)
    body_bytes = write_mod(body)

    # ---- coat skeleton ------------------------------------------------------
    parent = [255]; transforms = [transform_record(0, 0, 0)]
    def add(p, t, rx=0.0, ry=0.0):
        parent.append(p); transforms.append(transform_record(*t, rx, ry, 0.0))
        return len(parent) - 1

    # Skirt: eight waist chains, local -Y down the skirt.
    sk = sorted({i for k in skirt for i in tris[k]})
    P = pos[sk]
    cx, cz = P[:, 0].mean(), P[:, 2].mean()
    ang = np.degrees(np.arctan2(P[:, 0] - cx, P[:, 2] - cz)) % 360
    rad = np.hypot(P[:, 0] - cx, P[:, 2] - cz)
    ytop = P[:, 1].max()
    skirt_chains = []
    for c in range(SKIRT_CHAINS):
        th_ = c * 360.0 / SKIRT_CHAINS
        m = np.abs(((ang - th_ + 180) % 360) - 180) < 180.0 / SKIRT_CHAINS
        y = P[m, 1]; r = rad[m]
        rt = r[y > y.max() - 3].mean(); rb = r[y < y.min() + 3].mean(); yb = y[y < y.min() + 3].mean()
        h = np.array([math.sin(math.radians(th_)), 0.0, math.cos(math.radians(th_))])
        top = np.array([cx, ytop, cz]) + h * rt
        bot = np.array([cx, yb, cz]) + h * rb
        D = bot - top; L = np.linalg.norm(D) / SKIRT_LINKS; D /= L * SKIRT_LINKS
        anchor = add(0, top - JR, *rot_y_toward(-D))
        nodes = [anchor]
        for _ in range(SKIRT_LINKS): nodes.append(add(nodes[-1], (0.0, -L, 0.0)))
        skirt_chains.append((top, D, L, nodes))
    constraints = []

    # Sleeves: anchors on body joints 6/7/8 (10/11/12) + a chain from the
    # elbow anchor along the bottom of the bell to the cuff.
    sleeve_rigs = {}
    for side, (j6, j7, j8, j9) in ARMS.items():
        anchors = {}
        for j in (j6, j7, j8):
            anchors[j] = add(0, J[j] - JR)
            constraints.append((anchors[j], j))
        vs = sorted({i for k in sleeves[side] for i in tris[k]})
        S = pos[vs]
        axis = J[j9] - J[j8]; axis /= np.linalg.norm(axis)
        along = (S - J[j8]) @ axis
        cuff = along.max()
        # bottom line: lowest points near the elbow and at the cuff
        near = S[(along > -2) & (along < 4)]; far = S[along > cuff - 3]
        eb = near[near[:, 1].argmin()].copy(); cb = far[far[:, 1].argmin()].copy()
        eb[2] = cb[2] = J[j8][2]
        D = cb - eb; L = np.linalg.norm(D) / SLEEVE_LINKS; D /= L * SLEEVE_LINKS
        n1 = eb + D * L
        rx, ry = rot_y_toward(-D)
        links = [add(anchors[j8], n1 - J[j8], rx, ry)]
        for _ in range(SLEEVE_LINKS - 1): links.append(add(links[-1], (0.0, -L, 0.0)))
        sleeve_rigs[side] = dict(anchors=anchors, links=links, eb=eb, D=D, L=L,
                                 axis=axis, elbow=J[j8], cuff=cuff, j=(j6, j7, j8))
        print(f'  {side} sleeve: elbow {np.round(J[j8],1)} cuff {cuff:.1f} bottom {np.round(eb,1)} -> {np.round(cb,1)} link {L:.2f}')
    nodes_total = len(parent)
    assert nodes_total <= COAT_JOINT_CAPACITY, nodes_total

    # ---- coat skin ----------------------------------------------------------
    def skirt_skin(v):
        a = (math.degrees(math.atan2(v[0] - cx, v[2] - cz)) % 360) / (360.0 / SKIRT_CHAINS)
        c0 = int(math.floor(a)) % SKIRT_CHAINS; c1 = (c0 + 1) % SKIRT_CHAINS; f = a - math.floor(a)
        out = []
        for c, wc in ((c0, 1 - f), (c1, f)):
            top, D, L, nodes = skirt_chains[c]
            s = float(np.dot(v - top, D)) / L
            out += [(n, wc * w) for n, w in knot_weights(s, nodes)]
        return out

    def sleeve_skin(v, i_body, rig):
        j6, j7, j8 = rig['j']
        base = [(rig['anchors'][int(Bn[i_body, k])], float(W[i_body, k]) / 31.0)
                for k in range(3) if W[i_body, k] and int(Bn[i_body, k]) in (j6, j7, j8)]
        if not base: base = [(rig['anchors'][j8], 1.0)]
        # Cloth share: bottom half of the bell, growing toward the cuff.
        rel = v - rig['elbow']
        s_axis = float(rel @ rig['axis'])
        radial = rel - s_axis * rig['axis']
        rn = np.linalg.norm(radial)
        bottom = max(0.0, min(1.0, -radial[1] / rn)) if rn > 1e-6 else 0.0
        along = max(0.0, min(1.0, s_axis / rig['cuff']))
        f = bottom * along ** 0.7
        if f <= 0: return base
        s = float(np.dot(v - rig['eb'], rig['D'])) / rig['L']
        chain = knot_weights(s, [rig['anchors'][j8]] + rig['links'])
        return [(n, w * (1 - f)) for n, w in base] + [(n, w * f) for n, w in chain]

    groups = [(sorted(set(skirt) | set(ribbon)), 'skirt')] + [(sleeves[s], s) for s in ARMS]
    coat_idx = [i for ks, _ in groups for k in sorted(ks) for i in tris[k]]
    coat_mesh = take(me, coat_idx)
    blend = bytearray(); ctlw = bytearray(); newpos = bytearray()
    owner = {}
    for ks, name in groups:
        for k in ks:
            for i in tris[k]: owner[i] = name
    for n_out, i in enumerate(coat_idx):
        v = pos[i]
        pairs = quantize(skirt_skin(v) if owner[i] == 'skirt' else sleeve_skin(v, i, sleeve_rigs[owner[i]]))
        blend += bytes([0] + [p[0] * 4 for p in pairs])
        w = pairs[0][1] | (pairs[1][1] << 5) | (pairs[2][1] << 10)
        if n_out % 3 != 2: w |= 0x8000
        ctlw += struct.pack('<H', w)
        newpos += struct.pack('<3f', *(v - JR))
    coat_mesh.pos = bytes(newpos); coat_mesh.blend = bytes(blend); coat_mesh.ctl = bytes(ctlw)

    coat = Mod()
    header = bytearray(read_mod(dante[12]).header)
    header[0x13] = ROOT_JOINT - 3            # coat_patch.py: root = joint 3 + this
    coat.header = bytes(header)
    obj = Obj()
    rec = bytearray(body.objects[2].record)
    local = pos[coat_idx] - JR
    centre = (local.min(0) + local.max(0)) / 2
    struct.pack_into('<4f', rec, 0x30, *centre, float(np.linalg.norm(local - centre, axis=1).max()))
    obj.record = bytes(rec); obj.meshes = [coat_mesh]
    coat.objects = [obj]
    coat.parent = parent; coat.order = list(range(nodes_total)); coat.adapter = [0] * nodes_total
    coat.transforms = transforms
    coat_bytes = write_mod(coat)

    # ---- CLT: one block (CPlDante has one chain) ------------------------------
    simulated = [n for _, _, _, nodes in skirt_chains for n in nodes[1:]]
    simulated += [n for rig in sleeve_rigs.values() for n in rig['links']]
    clt = (';pl011_02.clt\r\n\r\nClothNum\t1\r\n\r\n\r\n\r\n'
           'ClothNo     0\r\nClothId     0\r\n'
           'Gravity     0.000000  -0.010000  0.000000\r\n'
           'SpringForce 0.020000\r\nMaxSpeed    50.000000\r\n'
           f'Stiffness   {STIFFNESS:.6f}\r\n'
           'Wind        0.000000  0.000000  0.000000\r\n'
           'WindLocal   1\r\nWindParent  0\r\nWindType    1\r\n')
    for n in simulated: clt += f'Bone      {n}    Y\r\n'
    clt += 'End\r\n$\r\n'
    clt_bytes = clt.encode('ascii')
    clt_bytes += bytes((-len(clt_bytes)) % 16 + 16)

    # ---- slot 15: coat node constraints ('CCNS') ------------------------------
    ccns = b'CCNS' + struct.pack('<III', 1, len(constraints), len(CAPSULES))
    for node, joint in constraints:
        ccns += struct.pack('<IIQ', node, joint, 0) + np.eye(4, dtype='<f4').tobytes()
    for index, _, a, b, r in CAPSULES:
        ccns += struct.pack('<I12x4f4f4f', index, *a, 1.0, *b, 1.0, r, 0.0, 0.0, 0.0)
    ccns += bytes((-len(ccns)) % 16)
    print(f'coat nodes {nodes_total}, cloth bones {len(simulated)}, constraints {constraints}')

    # ---- shadows ------------------------------------------------------------------
    body_groups = [(5, [5, 23]), (3, [3, 6, 10]), (4, [4]), (14, [14]), (15, [15]), (19, [19]),
                   (9, [9]), (16, [16]), (17, [17, 18]), (20, [20]), (21, [21, 22]), (8, [8]),
                   (7, [7]), (11, [11]), (12, [12]), (13, [13]), (2, [2])]
    print('SHW slot 8 (body):')
    shw8 = shw_for(mod_skin(body_bytes, '%g'), body_groups, dante[8], len(body.transforms))
    coat_groups = [(nodes[1], nodes) for _, _, _, nodes in skirt_chains]
    for rig in sleeve_rigs.values():
        a = rig['anchors']; j6, j7, j8 = rig['j']
        coat_groups.append((a[j7], [a[j6], a[j7]]))
        coat_groups.append((a[j8], [a[j8]] + rig['links']))
    print('SHW slot 14 (coat):')
    shw14 = shw_for(mod_skin(coat_bytes, '%g'), coat_groups, dante[14], nodes_total)

    out = list(slots)
    out[1] = body_bytes; out[8] = shw8; out[12] = coat_bytes; out[13] = clt_bytes; out[14] = shw14
    out.append(ccns)
    assert len(out) == 16
    pac = build_pac(out)
    open(out_path, 'wb').write(pac)
    print('written', out_path, len(pac), 'bytes', len(out), 'slots; coat', len(coat_bytes), 'body', len(body_bytes))

if __name__ == '__main__':
    main()
