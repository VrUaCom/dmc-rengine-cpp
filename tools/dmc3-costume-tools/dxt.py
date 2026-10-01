"""DXT1 / DXT5 codec (numpy, vectorised) and the PTX texture container.

PTX bundle (a PAC slot, e.g. player slot 0, stage slot 1):
  0x000  u32 texture count, then u32 sector span (0x800 units) per texture
  0x800  per texture: 0x70-byte descriptor, 128-byte DDS header, mip chain,
         padded to 0x800
Descriptor (all fields follow from the DDS header; confirmed on retail
pl000/pl001/em028/st000/st001 and the canonical-descriptor check):
  +0x08 0x20000 | mips << 8 | (0x88 DXT5, 0x86 DXT1)
  +0x0C 0xAAE4          +0x10 (h << 16) | w       +0x14 1
  +0x18 w*4 (DXT5) / w*2 (DXT1)                   +0x20 0x40
  +0x38 payload bytes   +0x3C/+0x40 aux pair (0/0 here)
  +0x44 secondary dims (half size) +0x48/+0x4C 1/secondary w, 1/secondary h
  +0x60 format 4 (DXT5) / 0 (DXT1)  +0x64 128 + payload  +0x68 8
DDS header: flags 0x000A1007, caps 0x00401008, FourCC pixel format; retail
linear-size values are inconsistent (the game does not use them), this
module writes the real size of mip 0. The game wants a full mip chain.
"""
import struct

import numpy as np

# ---------------------------------------------------------------- blocks ----
def _unpack565(c):
    c = c.astype(np.int32)
    r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
    return np.stack([(r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)], -1).astype(np.float64)

def _pack565(rgb):
    rgb = np.clip(rgb, 0, 255)
    r = (rgb[..., 0] * 31 + 127.5).astype(np.int32) // 255
    g = (rgb[..., 1] * 63 + 127.5).astype(np.int32) // 255
    b = (rgb[..., 2] * 31 + 127.5).astype(np.int32) // 255
    return (r << 11) | (g << 5) | b

def _color_palette(c0, c1, four):
    p0, p1 = _unpack565(c0), _unpack565(c1)
    p2 = np.where(four[:, None], (2 * p0 + p1) / 3, (p0 + p1) / 2)
    p3 = np.where(four[:, None], (p0 + 2 * p1) / 3, 0)
    return np.stack([p0, p1, p2, p3], 1)          # (N, 4, 3)

def _blocks_to_image(px, w, h):
    bw, bh = max(1, (w + 3) // 4), max(1, (h + 3) // 4)
    img = px.reshape(bh, bw, 4, 4, 4).transpose(0, 2, 1, 3, 4).reshape(bh * 4, bw * 4, 4)
    return img[:h, :w]

def decode(data, w, h, fourcc):
    """-> (h, w, 4) uint8 RGBA."""
    bw, bh = max(1, (w + 3) // 4), max(1, (h + 3) // 4)
    n = bw * bh
    if fourcc == b'DXT1':
        b = np.frombuffer(data, np.uint8, n * 8).reshape(n, 8)
        c0 = b[:, 0].astype(np.int32) | (b[:, 1].astype(np.int32) << 8)
        c1 = b[:, 2].astype(np.int32) | (b[:, 3].astype(np.int32) << 8)
        bits = b[:, 4:8].copy().view('<u4')[:, 0]
        four = c0 > c1
        pal = _color_palette(c0, c1, four)
        idx = (bits[:, None] >> (2 * np.arange(16))) & 3
        rgb = np.take_along_axis(pal, idx[..., None].astype(np.int64), 1)
        a = np.where(~four[:, None] & (idx == 3), 0, 255)
    elif fourcc == b'DXT5':
        b = np.frombuffer(data, np.uint8, n * 16).reshape(n, 16)
        a0, a1 = b[:, 0].astype(np.float64), b[:, 1].astype(np.float64)
        abits = np.zeros(n, np.uint64)
        for k in range(6): abits |= b[:, 2 + k].astype(np.uint64) << np.uint64(8 * k)
        aidx = ((abits[:, None] >> (np.uint64(3) * np.arange(16, dtype=np.uint64))) & np.uint64(7)).astype(np.int64)
        eight = a0 > a1
        ap = np.zeros((n, 8))
        ap[:, 0], ap[:, 1] = a0, a1
        for k in range(2, 8):
            e = ((8 - k) * a0 + (k - 1) * a1) // 7
            six = ((6 - k) * a0 + (k - 1) * a1) // 5 if k <= 5 else np.full(n, 0.0 if k == 6 else 255.0)
            ap[:, k] = np.where(eight, e, six)
        a = np.take_along_axis(ap, aidx, 1)
        c0 = b[:, 8].astype(np.int32) | (b[:, 9].astype(np.int32) << 8)
        c1 = b[:, 10].astype(np.int32) | (b[:, 11].astype(np.int32) << 8)
        bits = b[:, 12:16].copy().view('<u4')[:, 0]
        pal = _color_palette(c0, c1, np.ones(n, bool))
        idx = (bits[:, None] >> (2 * np.arange(16))) & 3
        rgb = np.take_along_axis(pal, idx[..., None].astype(np.int64), 1)
    else:
        raise ValueError(f'unsupported FourCC {fourcc!r}')
    px = np.concatenate([np.round(rgb), a[..., None]], -1).clip(0, 255).astype(np.uint8)
    return _blocks_to_image(px, w, h)

def _image_to_blocks(img):
    h, w = img.shape[:2]
    H, W = max(4, (h + 3) // 4 * 4), max(4, (w + 3) // 4 * 4)
    pad = np.zeros((H, W, 4), np.uint8)
    pad[:h, :w] = img
    if h < H: pad[h:, :w] = img[-1:, :, :]
    if w < W: pad[:, w:] = pad[:, w - 1:w]
    return pad.reshape(H // 4, 4, W // 4, 4, 4).transpose(0, 2, 1, 3, 4).reshape(-1, 16, 4).astype(np.float64)

def _fit_colors(rgb, four):
    """Principal-axis endpoints per block -> c0, c1, 2-bit indices."""
    mean = rgb.mean(1, keepdims=True)
    d = rgb - mean
    cov = np.einsum('nki,nkj->nij', d, d)
    axis = np.ones((len(rgb), 3)) / np.sqrt(3)
    for _ in range(8):
        axis = np.einsum('nij,nj->ni', cov, axis)
        norm = np.linalg.norm(axis, axis=1, keepdims=True)
        axis = np.where(norm > 1e-9, axis / np.maximum(norm, 1e-12), np.ones_like(axis) / np.sqrt(3))
    t = np.einsum('nki,ni->nk', d, axis)
    hi = mean[:, 0] + axis * t.max(1, keepdims=True)
    lo = mean[:, 0] + axis * t.min(1, keepdims=True)
    c0, c1 = _pack565(hi), _pack565(lo)
    swap = c0 < c1
    c0, c1 = np.where(swap, c1, c0), np.where(swap, c0, c1)
    if four:
        c1 = np.where(c0 == c1, np.maximum(c1 - 1, 0), c1)   # keep 4-colour mode
        c0 = np.where(c0 == c1, c0 + 1, c0)
    pal = _color_palette(c0, c1, c0 > c1)
    err = ((rgb[:, :, None, :] - pal[:, None, :, :]) ** 2).sum(-1)
    return c0, c1, err.argmin(-1)

def encode(img, fourcc):
    """(h, w, 4) uint8 -> DXT1 or DXT5 bytes (DXT1 is opaque, 4-colour)."""
    blk = _image_to_blocks(img)
    rgb, a = blk[..., :3], blk[..., 3]
    c0, c1, idx = _fit_colors(rgb, True)
    cbits = (idx.astype(np.uint64) << (2 * np.arange(16, dtype=np.uint64))).sum(1).astype(np.uint32)
    colour = np.zeros((len(blk), 8), np.uint8)
    colour[:, 0:2] = np.stack([c0 & 255, c0 >> 8], 1)
    colour[:, 2:4] = np.stack([c1 & 255, c1 >> 8], 1)
    colour[:, 4:8] = cbits.view(np.uint8).reshape(-1, 4)
    if fourcc == b'DXT1':
        return colour.tobytes()
    amax, amin = a.max(1), a.min(1)
    ap = np.stack([amax, amin] + [((7 - k) * amax + k * amin) / 7 for k in range(1, 7)], 1)
    aidx = np.abs(a[:, :, None] - ap[:, None, :]).argmin(-1)
    aidx = np.where((amax == amin)[:, None], 0, aidx)
    abits = (aidx.astype(np.uint64) << (np.uint64(3) * np.arange(16, dtype=np.uint64))).sum(1)
    alpha = np.zeros((len(blk), 8), np.uint8)
    alpha[:, 0], alpha[:, 1] = amax.astype(np.uint8), amin.astype(np.uint8)
    for k in range(6): alpha[:, 2 + k] = ((abits >> np.uint64(8 * k)) & np.uint64(255)).astype(np.uint8)
    return np.concatenate([alpha, colour], 1).tobytes()

def half(img):
    """2x2 box filter (one mip step)."""
    h, w = img.shape[:2]
    f = img.astype(np.float64)
    nh, nw = max(1, h // 2), max(1, w // 2)
    if h > 1 and w > 1:
        s = f[0:nh*2:2, 0:nw*2:2] + f[1:nh*2:2, 0:nw*2:2] + f[0:nh*2:2, 1:nw*2:2] + f[1:nh*2:2, 1:nw*2:2]
        return np.round(s / 4).astype(np.uint8)
    if h > 1: return np.round((f[0::2] + f[1::2]) / 2).astype(np.uint8)
    return np.round((f[:, 0::2] + f[:, 1::2]) / 2).astype(np.uint8)

def full_mips(w, h):
    d, c = max(w, h), 1
    while d > 1: d //= 2; c += 1
    return c

def mip_size(w, h, level, fourcc):
    w, h = max(1, w >> level), max(1, h >> level)
    return max(1, (w + 3) // 4) * max(1, (h + 3) // 4) * (8 if fourcc == b'DXT1' else 16)

# ---------------------------------------------------------------- PTX ----
class Texture:
    def __init__(self, w, h, fourcc, mips, levels, descriptor=None, dds=None):
        self.w, self.h, self.fourcc, self.mips = w, h, fourcc, mips
        self.levels = levels            # list of mip payloads (bytes)
        self.descriptor = descriptor    # original 0x70 bytes, if read
        self.dds = dds                  # original 128-byte DDS header, if read

    @classmethod
    def from_image(cls, img, fourcc):
        levels, cur = [], img
        for _ in range(full_mips(img.shape[1], img.shape[0])):
            levels.append(encode(cur, fourcc)); cur = half(cur)
        return cls(img.shape[1], img.shape[0], fourcc, len(levels), levels)

    def image(self, level=0):
        return decode(self.levels[level], max(1, self.w >> level), max(1, self.h >> level), self.fourcc)

    def complete_mips(self):
        """Generate missing mip levels from the last stored one."""
        want = full_mips(self.w, self.h)
        if len(self.levels) >= want: return 0
        added = want - len(self.levels)
        cur = self.image(len(self.levels) - 1)
        while len(self.levels) < want:
            cur = half(cur); self.levels.append(encode(cur, self.fourcc))
        self.mips = want
        return added

    def canonical_descriptor(self):
        w, h, dxt5 = self.w, self.h, self.fourcc == b'DXT5'
        payload = sum(len(x) for x in self.levels)
        sw, sh = max(1, w // 2), max(1, h // 2)
        d = bytearray(0x70)
        struct.pack_into('<I', d, 0x08, 0x20000 | (len(self.levels) << 8) | (0x88 if dxt5 else 0x86))
        struct.pack_into('<I', d, 0x0C, 0xAAE4)
        struct.pack_into('<I', d, 0x10, (h << 16) | w)
        struct.pack_into('<I', d, 0x14, 1)
        struct.pack_into('<I', d, 0x18, w * (4 if dxt5 else 2))
        struct.pack_into('<I', d, 0x20, 0x40)
        struct.pack_into('<I', d, 0x38, payload)
        struct.pack_into('<I', d, 0x44, (sh << 16) | sw)
        struct.pack_into('<f', d, 0x48, 1.0 / sw)
        struct.pack_into('<f', d, 0x4C, 1.0 / sh)
        struct.pack_into('<I', d, 0x60, 4 if dxt5 else 0)
        struct.pack_into('<I', d, 0x64, 128 + payload)
        struct.pack_into('<I', d, 0x68, 8)
        return bytes(d)

    def canonical_dds(self):
        d = bytearray(128)
        d[0:4] = b'DDS '
        struct.pack_into('<IIIIIII', d, 4, 124, 0x000A1007, self.h, self.w, len(self.levels[0]), 0, len(self.levels))
        struct.pack_into('<II', d, 76, 32, 4)
        d[84:88] = self.fourcc
        struct.pack_into('<I', d, 108, 0x00401008)
        return bytes(d)

    def problems(self):
        out = []
        if self.descriptor is not None:
            # +0x3C/+0x40 is an aux pair the retail files use as 0/0 or
            # non-zero/non-zero; every other field follows from the DDS header.
            canon = self.canonical_descriptor()
            diff = [hex(o) for o in range(0, 0x70, 4)
                    if o not in (0x3C, 0x40) and self.descriptor[o:o + 4] != canon[o:o + 4]]
            aux = struct.unpack_from('<II', self.descriptor, 0x3C)
            if (aux[0] == 0) != (aux[1] == 0): diff.append('0x3c/0x40 (aux pair half zero)')
            # +0x44..+0x4C: secondary dims are half size or (retail st001) the same.
            if set(diff) >= {'0x44', '0x48', '0x4c'} or any(x in diff for x in ('0x44', '0x48', '0x4c')):
                same = bytearray(canon)
                struct.pack_into('<I', same, 0x44, (self.h << 16) | self.w)
                struct.pack_into('<ff', same, 0x48, 1.0 / self.w, 1.0 / self.h)
                if self.descriptor[0x44:0x50] == bytes(same[0x44:0x50]):
                    diff = [x for x in diff if x not in ('0x44', '0x48', '0x4c')]
            if diff: out.append('descriptor differs at ' + ','.join(diff))
        if len(self.levels) < full_mips(self.w, self.h):
            out.append(f'{len(self.levels)} of {full_mips(self.w, self.h)} mip levels')
        return out

def is_ptx(b):
    if len(b) < 0x880 or len(b) % 0x800: return False
    n = struct.unpack_from('<I', b, 0)[0]
    return 0 < n < 256 and b[0x870:0x874] == b'DDS '

def read_ptx(b):
    n = struct.unpack_from('<I', b, 0)[0]
    out, off = [], 0x800
    for i in range(n):
        span = struct.unpack_from('<I', b, 4 + 4 * i)[0]
        desc, dds = b[off:off + 0x70], b[off + 0x70:off + 0xF0]
        h, w, mips = struct.unpack_from('<I', dds, 12)[0], struct.unpack_from('<I', dds, 16)[0], struct.unpack_from('<I', dds, 28)[0]
        fourcc = dds[84:88]
        levels, o = [], off + 0xF0
        for lv in range(max(1, mips)):
            size = mip_size(w, h, lv, fourcc)
            levels.append(b[o:o + size]); o += size
        out.append(Texture(w, h, fourcc, mips, levels, desc, dds))
        off += span * 0x800
    return out

def write_ptx(textures, canonical=True):
    """canonical=False keeps each read texture's original descriptor and DDS
    header (untouched textures stay byte-identical)."""
    out = bytearray(0x800)
    struct.pack_into('<I', out, 0, len(textures))
    for i, t in enumerate(textures):
        keep = not canonical and t.descriptor is not None and t.dds is not None
        body = (t.descriptor if keep else t.canonical_descriptor()) + \
               (t.dds if keep else t.canonical_dds()) + b''.join(t.levels)
        sectors = (len(body) + 0x7FF) // 0x800
        struct.pack_into('<I', out, 4 + 4 * i, sectors)
        out += body + bytes(sectors * 0x800 - len(body))
    return bytes(out)
