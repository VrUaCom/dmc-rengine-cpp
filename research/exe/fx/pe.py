"""Minimal PE64 reader for dmc3.exe (sections, VA <-> file offset, .pdata)."""
import hashlib
import struct

CANONICAL = 'e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'

class Image:
    def __init__(self, path):
        self.data = open(path, 'rb').read()
        d = self.data
        pe = struct.unpack_from('<I', d, 0x3C)[0]
        nsec = struct.unpack_from('<H', d, pe + 6)[0]
        opt = pe + 24
        opt_size = struct.unpack_from('<H', d, pe + 20)[0]
        self.base = struct.unpack_from('<Q', d, opt + 24)[0]
        self.sections = []
        for i in range(nsec):
            o = opt + opt_size + 40 * i
            name = d[o:o + 8].rstrip(b'\0').decode()
            vs, va, rs, raw = struct.unpack_from('<IIII', d, o + 8)
            self.sections.append((name, va, vs, raw, rs))
        pdata_rva, pdata_size = struct.unpack_from('<II', d, opt + 112 + 8 * 3)
        o = self.off(self.base + pdata_rva)
        self.functions = sorted(struct.unpack_from('<II', d, o + 12 * i)
                                for i in range(pdata_size // 12))
        self.sha256 = hashlib.sha256(d).hexdigest()
        if self.sha256 != CANONICAL:
            print(f'note: not the canonical dmc3.exe ({self.sha256[:16]}...)')

    def off(self, va):
        r = va - self.base
        for name, sva, vs, raw, rs in self.sections:
            if sva <= r < sva + max(vs, rs) and r - sva < rs:
                return raw + r - sva
        return None

    def section_of(self, va):
        r = va - self.base
        for s in self.sections:
            if s[1] <= r < s[1] + max(s[2], s[4]): return s[0]

    def text(self):
        name, va, vs, raw, rs = next(s for s in self.sections if s[0] == '.text')
        return self.base + va, self.data[raw:raw + min(vs, rs)]

    def u32(self, va): return struct.unpack_from('<I', self.data, self.off(va))[0]
    def u64(self, va): return struct.unpack_from('<Q', self.data, self.off(va))[0]

    def function_of(self, va):
        """.pdata RUNTIME_FUNCTION (begin, end) containing va, or None."""
        r = va - self.base
        lo, hi = 0, len(self.functions)
        while lo < hi:
            mid = (lo + hi) // 2
            if self.functions[mid][0] <= r: lo = mid + 1
            else: hi = mid
        if lo and self.functions[lo - 1][0] <= r < self.functions[lo - 1][1]:
            b, e = self.functions[lo - 1]
            return self.base + b, self.base + e
        return None
