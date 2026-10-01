"""Canonical MOD reader/writer for authoring (Python, analysis tooling).

Layout (retail pl000 files, confirmed against the canonical dmc3.exe):
  0x00  header 0x40 (+0x10 objects, +0x11 nodes, +0x12 texture slots,
        +0x13 default joint, +0x14 u32, +0x20 u64 node-domain block)
  0x40  object records 0x40 each (+0 mesh count, +1 alpha, +2 u16 elements,
        +8 u64 mesh table, +0x10 flags, +0x30 sphere centre, +0x3C radius)
  per object: its mesh records (0x50 each), then the streams grouped by
        kind (all positions f32x3, all normals f32x3, all uv i16x2, all blend
        u8x4, all control u16), every block aligned to 16
  node block (aligned 16): 0x20 header of relative offsets, parent u8[n],
        order u8[n], adapter u8[n] (each align4), transforms 0x20[n] (align16)
  per mesh a generated-topology workspace of align16(6 * (n - 2)) bytes,
        mesh +0x40 = workspace - mesh record (record-relative)
"""
import struct

def a16(x): return (x + 15) & ~15
def a4(x): return (x + 3) & ~3
def u16(b, o): return struct.unpack_from('<H', b, o)[0]
def u32(b, o): return struct.unpack_from('<I', b, o)[0]
def u64(b, o): return struct.unpack_from('<Q', b, o)[0]

class Mesh:
    def __init__(self):
        self.record = bytes(0x50)
        self.pos = b''; self.nrm = b''; self.uv = b''; self.blend = b''; self.ctl = b''
    @property
    def count(self): return len(self.pos) // 12

class Obj:
    def __init__(self):
        self.record = bytes(0x40)
        self.meshes = []

class Mod:
    def __init__(self):
        self.header = bytes(0x40)
        self.objects = []
        self.parent = []; self.order = []; self.adapter = []; self.transforms = []

def read_mod(b):
    m = Mod()
    m.header = b[:0x40]
    nobj, nodes = b[0x10], b[0x11]
    for oi in range(nobj):
        r = 0x40 + oi * 0x40
        o = Obj(); o.record = b[r:r + 0x40]
        cnt, tab = b[r], u64(b, r + 8)
        for k in range(cnt):
            mr = tab + k * 0x50
            me = Mesh(); me.record = b[mr:mr + 0x50]
            n = u16(b, mr)
            p, nr, uv, bl, ct = (u64(b, mr + x) for x in (0x10, 0x18, 0x20, 0x28, 0x30))
            me.pos, me.nrm, me.uv, me.blend, me.ctl = b[p:p + 12 * n], b[nr:nr + 12 * n], b[uv:uv + 4 * n], b[bl:bl + 4 * n], b[ct:ct + 2 * n]
            o.meshes.append(me)
        m.objects.append(o)
    doc = u64(b, 0x20)
    pr, orr, ar, tr = (u32(b, doc + 4 * i) for i in range(4))
    m.parent = list(b[doc + pr:doc + pr + nodes])
    m.order = list(b[doc + orr:doc + orr + nodes])
    m.adapter = list(b[doc + ar:doc + ar + nodes])
    m.transforms = [b[doc + tr + 0x20 * i:doc + tr + 0x20 * (i + 1)] for i in range(nodes)]
    return m

def write_mod(m):
    out = bytearray(m.header)
    nobj = len(m.objects); nodes = len(m.transforms)
    out[0x10] = nobj; out[0x11] = nodes
    out += bytes(0x40 * nobj)
    mesh_records = []  # (record offset, mesh)
    def pad16():
        while len(out) % 16: out.append(0)
    for oi, o in enumerate(m.objects):
        pad16()
        tab = len(out)
        rec = bytearray(o.record)
        rec[0] = len(o.meshes)
        struct.pack_into('<H', rec, 2, sum(me.count for me in o.meshes))
        struct.pack_into('<Q', rec, 8, tab)
        out[0x40 + oi * 0x40:0x40 + (oi + 1) * 0x40] = rec
        recs = []
        for me in o.meshes:
            recs.append(len(out)); out += bytes(0x50)
        # Streams are grouped by kind: every mesh's positions, then normals...
        ptrs = [[] for _ in o.meshes]
        for kind in ('pos', 'nrm', 'uv', 'blend', 'ctl'):
            for k, me in enumerate(o.meshes):
                pad16(); ptrs[k].append(len(out)); out += getattr(me, kind)
        for ro, me, mp in zip(recs, o.meshes, ptrs):
            ptrs_ = mp
            r = bytearray(me.record)
            struct.pack_into('<H', r, 0, me.count)
            for k, p in enumerate(ptrs_): struct.pack_into('<Q', r, 0x10 + 8 * k, p)
            struct.pack_into('<I', r, 0x48, 0)
            out[ro:ro + 0x50] = r
            mesh_records.append((ro, me))
    pad16()
    doc = len(out)
    struct.pack_into('<Q', out, 0x20, doc)
    pr = 0x20; orr = pr + a4(nodes); ar = orr + a4(nodes); tr = a16(ar + a4(nodes))
    block = bytearray(tr + 0x20 * nodes)
    struct.pack_into('<4I', block, 0, pr, orr, ar, tr)
    block[pr:pr + nodes] = bytes(m.parent)
    block[orr:orr + nodes] = bytes(m.order)
    block[ar:ar + nodes] = bytes(m.adapter)
    for i, t in enumerate(m.transforms): block[tr + 0x20 * i:tr + 0x20 * (i + 1)] = t
    out += block
    pad16()
    for ro, me in mesh_records:
        ws = len(out)
        struct.pack_into('<Q', out, ro + 0x40, ws - ro)
        # Workspaces carry the 0x12 fill of the retail corpus; the file's
        # last two bytes are zero.
        out += b'\x12' * a16(max(0, 6 * (me.count - 2)))
    if len(out) >= 2 and mesh_records:
        out[-2:] = b'\x00\x00'
    return bytes(out)

def transform_record(tx, ty, tz, rx=0.0, ry=0.0, rz=0.0):
    mag = (tx * tx + ty * ty + tz * tz) ** 0.5
    return struct.pack('<8f', tx, ty, tz, mag, rx, ry, rz, 0.0)
