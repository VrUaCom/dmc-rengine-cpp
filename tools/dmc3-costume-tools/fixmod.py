"""Repair a community player PAC (pl011.pac) against the canonical dmc3.exe:
canonical PTX descriptors and full mip chains (slot 0), the model's own SHW
shadow hulls (slot 8) and the missing slot 14 SHW (the player init passes
slot 14 to the shadow object without a null check).

Usage: python3 fixmod.py mod.pac pl000.pac mod_fixed.pac
(pl000.pac, Dante's retail PAC, gives the SHW headers and a DDS header.)"""
import struct, sys
import numpy as np
from scipy.spatial import ConvexHull

def u32(b, o): return struct.unpack_from('<I', b, o)[0]

def pac_slots(b):
    assert b[:4] == b'PAC\0'
    n = u32(b, 4); offs = [u32(b, 8 + 4 * i) for i in range(n)]
    ends = sorted(set(offs + [len(b)]))
    return [b[o:min(x for x in ends if x > o)] for o in offs]

def build_pac(slots):
    n = len(slots)
    head = 8 + 4 * n
    first = (head + 15) & ~15
    out = bytearray(first)
    out[0:4] = b'PAC\0'; struct.pack_into('<I', out, 4, n)
    for i, s in enumerate(slots):
        while len(out) % 16: out.append(0)
        struct.pack_into('<I', out, 8 + 4 * i, len(out))
        out += s
    return bytes(out)

# ---------------------------------------------------------------- DXT5 ----
def rgb565(c):
    r, g, b = (int(c[0]) * 31 + 127) // 255, (int(c[1]) * 63 + 127) // 255, (int(c[2]) * 31 + 127) // 255
    return (r << 11) | (g << 5) | b

def unpack565(v):
    r, g, b = (v >> 11) & 31, (v >> 5) & 63, v & 31
    return np.array([(r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)], dtype=np.float64)

def decode_dxt5(data, w, h):
    img = np.zeros((max(4, (h + 3) // 4 * 4), max(4, (w + 3) // 4 * 4), 4), dtype=np.uint8)
    o = 0
    for by in range(0, img.shape[0], 4):
        for bx in range(0, img.shape[1], 4):
            a0, a1 = data[o], data[o + 1]
            abits = int.from_bytes(data[o + 2:o + 8], 'little')
            if a0 > a1:
                pal_a = [a0, a1] + [((7 - k) * a0 + k * a1) // 7 for k in range(1, 7)]
            else:
                pal_a = [a0, a1] + [((5 - k) * a0 + k * a1) // 5 for k in range(1, 5)] + [0, 255]
            c0, c1 = struct.unpack_from('<HH', data, o + 8)
            cbits = u32(data, o + 12)
            p0, p1 = unpack565(c0), unpack565(c1)
            pal_c = [p0, p1, (2 * p0 + p1) / 3, (p0 + 2 * p1) / 3]
            for i in range(16):
                y, x = by + i // 4, bx + i % 4
                img[y, x, :3] = np.round(pal_c[(cbits >> (2 * i)) & 3]).astype(np.uint8)
                img[y, x, 3] = pal_a[(abits >> (3 * i)) & 7]
            o += 16
    return img[:h, :w]

def encode_block(px):
    """px: 16x4 uint8 -> 16 bytes DXT5."""
    px = px.astype(np.float64)
    a = px[:, 3]
    amax, amin = int(a.max()), int(a.min())
    if amax == amin:
        ablock = bytes([amax, amin]) + bytes(6)
    else:
        pal = np.array([amax, amin] + [((7 - k) * amax + k * amin) / 7 for k in range(1, 7)])
        idx = np.abs(a[:, None] - pal[None, :]).argmin(1)
        bits = 0
        for i, v in enumerate(idx): bits |= int(v) << (3 * i)
        ablock = bytes([amax, amin]) + bits.to_bytes(6, 'little')
    rgb = px[:, :3]
    # endpoints along the principal axis of the block's colours
    mean = rgb.mean(0)
    cov = np.cov((rgb - mean).T) if np.ptp(rgb, 0).any() else np.zeros((3, 3))
    axis = np.linalg.eigh(cov)[1][:, -1] if cov.any() else np.array([1.0, 1.0, 1.0]) / np.sqrt(3)
    t = (rgb - mean) @ axis
    hi = np.clip(mean + axis * t.max(), 0, 255); lo = np.clip(mean + axis * t.min(), 0, 255)
    c0, c1 = rgb565(hi), rgb565(lo)
    if c0 < c1: c0, c1 = c1, c0
    if c0 == c1:
        cblock = struct.pack('<HHI', c0, c1, 0)
    else:
        p0, p1 = unpack565(c0), unpack565(c1)
        pal = np.array([p0, p1, (2 * p0 + p1) / 3, (p0 + 2 * p1) / 3])
        idx = ((rgb[:, None, :] - pal[None, :, :]) ** 2).sum(2).argmin(1)
        bits = 0
        for i, v in enumerate(idx): bits |= int(v) << (2 * i)
        cblock = struct.pack('<HHI', c0, c1, bits)
    return ablock + cblock

def encode_dxt5(img):
    h, w = img.shape[:2]
    H, W = max(4, (h + 3) // 4 * 4), max(4, (w + 3) // 4 * 4)
    pad = np.zeros((H, W, 4), dtype=np.uint8)
    pad[:h, :w] = img
    if h < H: pad[h:, :w] = img[-1:, :, :]
    if w < W: pad[:, w:] = pad[:, w - 1:w]
    out = bytearray()
    for by in range(0, H, 4):
        for bx in range(0, W, 4):
            out += encode_block(pad[by:by + 4, bx:bx + 4].reshape(16, 4))
    return bytes(out)

def half(img):
    h, w = img.shape[:2]
    nh, nw = max(1, h // 2), max(1, w // 2)
    f = img.astype(np.float64)
    if h > 1 and w > 1:
        s = f[0:nh * 2:2, 0:nw * 2:2] + f[1:nh * 2:2, 0:nw * 2:2] + f[0:nh * 2:2, 1:nw * 2:2] + f[1:nh * 2:2, 1:nw * 2:2]
        return np.round(s / 4).astype(np.uint8)
    if h > 1: return np.round((f[0::2] + f[1::2]) / 2).astype(np.uint8)
    return np.round((f[:, 0::2] + f[:, 1::2]) / 2).astype(np.uint8)

def full_mips(w, h):
    d = max(w, h); c = 1
    while d > 1: d //= 2; c += 1
    return c

# ----------------------------------------------------------------- PTX ----
def canonical_descriptor(w, h, mips, payload):
    d = bytearray(0x70)
    sw, sh = max(1, w // 2), max(1, h // 2)
    struct.pack_into('<I', d, 0x08, 0x20000 | (mips << 8) | 0x88)  # DXT5
    struct.pack_into('<I', d, 0x0C, 0xAAE4)
    struct.pack_into('<I', d, 0x10, (h << 16) | w)
    struct.pack_into('<I', d, 0x14, 1)
    struct.pack_into('<I', d, 0x18, w * 4)
    struct.pack_into('<I', d, 0x20, 0x40)
    struct.pack_into('<I', d, 0x38, payload)
    struct.pack_into('<I', d, 0x44, (sh << 16) | sw)
    struct.pack_into('<f', d, 0x48, 1.0 / sw)
    struct.pack_into('<f', d, 0x4C, 1.0 / sh)
    struct.pack_into('<I', d, 0x60, 4)
    struct.pack_into('<I', d, 0x64, 128 + payload)
    struct.pack_into('<I', d, 0x68, 8)
    return bytes(d)

def rebuild_ptx(ptx, template_dds_header):
    n = u32(ptx, 0)
    textures = []
    off = 0x800
    for i in range(n):
        span = u32(ptx, 4 + 4 * i)
        dds = ptx[off + 0x70:]
        h, w, m = u32(dds, 12), u32(dds, 16), u32(dds, 28)
        assert dds[84:88] == b'DXT5', dds[84:88]
        want = full_mips(w, h)
        data = []
        ww, hh = w, h
        o = 128
        for level in range(m):
            size = max(1, (ww + 3) // 4) * max(1, (hh + 3) // 4) * 16
            data.append(dds[o:o + size]); o += size
            ww, hh = max(1, ww // 2), max(1, hh // 2)
        if m < want:
            # decode the last stored level and build the rest of the chain
            lw, lh = max(1, w >> (m - 1)), max(1, h >> (m - 1))
            img = decode_dxt5(data[-1], lw, lh)
            for level in range(m, want):
                img = half(img)
                data.append(encode_dxt5(img))
            print(f'  t{i}: {w}x{h} mips {m} -> {want} (generated {want - m})')
        else:
            print(f'  t{i}: {w}x{h} mips {m} kept')
        payload = sum(len(x) for x in data)
        header = bytearray(template_dds_header)
        struct.pack_into('<I', header, 12, h)
        struct.pack_into('<I', header, 16, w)
        struct.pack_into('<I', header, 20, len(data[0]))  # linear size of mip 0
        struct.pack_into('<I', header, 28, want)
        textures.append(canonical_descriptor(w, h, want, payload) + bytes(header) + b''.join(data))
        off += span * 0x800
    out = bytearray(0x800)
    struct.pack_into('<I', out, 0, n)
    for i, t in enumerate(textures):
        sectors = (len(t) + 0x7FF) // 0x800
        struct.pack_into('<I', out, 4 + 4 * i, sectors)
        out += t + bytes(sectors * 0x800 - len(t))
    return bytes(out)

# ----------------------------------------------------------------- SHW ----
DIRS = [np.array(v, dtype=np.float64) for v in (
    (1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1),
    (1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0), (1, 0, 1), (1, 0, -1),
    (-1, 0, 1), (-1, 0, -1), (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1))]

def hull_of(points):
    """Closed convex hull of the extreme points of `points` along DIRS:
    vertices, CCW-outward triangles, neighbour across edge (v_i, v_i+1)."""
    pts = np.unique(np.array([points[(points @ d).argmax()] for d in DIRS]), axis=0)
    if len(pts) < 4: return None
    try:
        hull = ConvexHull(pts)
    except Exception:
        return None
    keep = sorted(set(hull.vertices.tolist()))
    remap = {v: k for k, v in enumerate(keep)}
    verts = pts[keep]
    centre = verts.mean(0)
    tris = []
    for s in hull.simplices:
        a, b, c = (remap[int(x)] for x in s)
        n = np.cross(verts[b] - verts[a], verts[c] - verts[a])
        if np.dot(n, verts[a] - centre) < 0: b, c = c, b
        tris.append((a, b, c))
    edge_owner = {}
    for t, (a, b, c) in enumerate(tris):
        for e in ((a, b), (b, c), (c, a)): edge_owner[e] = t
    adj = []
    for a, b, c in tris:
        adj.append(tuple(edge_owner[(y, x)] for x, y in ((a, b), (b, c), (c, a))))
    assert len(tris) == 2 * len(verts) - 4, (len(tris), len(verts))
    return verts, tris, adj

def build_shw(hulls, header_template, node_count):
    """hulls: list of (verts, tris, adj, joint)."""
    n = len(hulls)
    out = bytearray(header_template[:0x20])
    out[0x10] = n; out[0x11] = node_count
    table = 0x20
    out += bytes(n * 0x40)
    def align():
        while len(out) % 16: out.append(0)
    for h, (verts, tris, adj, joint) in enumerate(hulls):
        r = table + h * 0x40
        struct.pack_into('<HH', out, r, len(verts), len(tris))
        align(); to = len(out)
        for t in tris: out += struct.pack('<4I', *t, 0)
        align(); ao = len(out)
        for a in adj: out += struct.pack('<4H', *a, 0)
        align(); vo = len(out)
        for v in verts: out += struct.pack('<4f', *[float(x) for x in v], 1.0)
        align(); so = len(out)
        out += bytes([joint] * len(verts))
        align()
        struct.pack_into('<QQQQ', out, r + 0x10, to, ao, vo, so)
    return bytes(out)

def mod_skin(mod_bytes, fmt='%.5f'):
    """Rest-space positions of every MOD vertex (objects, meshes in file
    order) and each vertex's dominant skin joint (blend lane k+1 / 4 with the
    largest 5-bit weight). Positions are rounded through `fmt`, as the text
    exports the published outputs were built from."""
    from modwriter import read_mod
    m = read_mod(mod_bytes)
    pos, joints = [], []
    for o in m.objects:
        for me in o.meshes:
            n = me.count
            p = np.frombuffer(me.pos, '<f4').reshape(n, 3)
            bl = np.frombuffer(me.blend, 'u1').reshape(n, 4)
            c = np.frombuffer(me.ctl, '<u2').astype(np.int64) & 0x7fff
            w = np.stack([c & 31, (c >> 5) & 31, (c >> 10) & 31], 1)
            pos += [[float(fmt % x) for x in v] for v in p]
            joints += list((bl[:, 1:4] // 4)[np.arange(n), w.argmax(1)])
    return np.array(pos, np.float64), np.array(joints)

def shw_for(skin, groups, header_template, node_count):
    """skin: (positions, dominant joints). One hull per (joint, members)."""
    pos, joints = skin
    hulls = []
    for joint, members in groups:
        pts = pos[np.isin(joints, members)]
        if len(pts) < 4: continue
        h = hull_of(pts)
        if h is None: continue
        hulls.append((*h, joint))
        print(f'  joint {joint:2} (+{[m for m in members if m != joint]}): {len(pts):5} verts -> hull V{len(h[0])} T{len(h[1])} y {pts[:,1].min():.0f}..{pts[:,1].max():.0f}')
    return build_shw(hulls, header_template, node_count)

if __name__ == '__main__':
    mod_path, dante_path, out_path = sys.argv[1:4]
    mod = pac_slots(open(mod_path, 'rb').read())
    dante = pac_slots(open(dante_path, 'rb').read())
    # Dante t2 (256x256 DXT5, full chain) as the DDS header template.
    dptx = dante[0]
    off = 0x800 + (u32(dptx, 4) + u32(dptx, 8)) * 0x800
    template = dptx[off + 0x70: off + 0x70 + 128]
    print('PTX:')
    ptx = rebuild_ptx(mod[0], template)
    # Body hulls: Dante's 17 joints; small joints join their parent's hull.
    body_groups = [(5, [5, 23]), (3, [3, 6, 10]), (4, [4]), (14, [14]), (15, [15]), (19, [19]),
                   (9, [9]), (16, [16]), (17, [17, 18]), (20, [20]), (21, [21, 22]), (8, [8]),
                   (7, [7]), (11, [11]), (12, [12]), (13, [13]), (2, [2])]
    print('SHW slot 8 (body):')
    shw8 = shw_for(mod_skin(mod[1]), body_groups, dante[8], 24)
    print('SHW slot 14 (slot 12 model):')
    tail_groups = [(j, [j]) for j in range(8)]
    shw14 = shw_for(mod_skin(mod[12]), tail_groups, dante[14], 8)
    slots = [ptx] + mod[1:8] + [shw8] + mod[9:14] + [shw14]
    pac = build_pac(slots)
    open(out_path, 'wb').write(pac)
    print('written', out_path, len(pac), 'bytes,', len(slots), 'slots')
