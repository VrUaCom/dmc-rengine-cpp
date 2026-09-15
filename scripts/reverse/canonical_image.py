"""Small canonical-only PE reader for address-backed research scripts."""
import bisect
import csv
import gzip
import hashlib
import struct
from pathlib import Path

import capstone

CANONICAL = 'e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'


def read_tsv(path):
    path = Path(path)
    with (gzip.open(path, 'rt') if path.suffix == '.gz' else path.open()) as f:
        return list(csv.DictReader(f, delimiter='\t'))


class CanonicalImage:
    def __init__(self, path):
        self.data = Path(path).read_bytes()
        if hashlib.sha256(self.data).hexdigest() != CANONICAL:
            raise ValueError('canonical dmc3.exe SHA-256 mismatch')
        pe = struct.unpack_from('<I', self.data, 0x3c)[0]
        self.opt = pe + 24
        self.base = struct.unpack_from('<Q', self.data, self.opt + 24)[0]
        n = struct.unpack_from('<H', self.data, pe + 6)[0]
        size = struct.unpack_from('<H', self.data, pe + 20)[0]
        self.sections = []
        for i in range(n):
            p = self.opt + size + i * 40
            vs, rva, rs, raw = struct.unpack_from('<IIII', self.data, p + 8)
            flags = struct.unpack_from('<I', self.data, p + 36)[0]
            self.sections.append((self.base + rva, vs, rs, raw, flags))
        rva, size = struct.unpack_from('<II', self.data, self.opt + 112 + 24)
        self.ranges = [tuple(self.base + v for v in struct.unpack('<III', self.raw(self.base + rva + i, 12)))
                       for i in range(0, size, 12)]
        self.starts = [r[0] for r in self.ranges]
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
        self.md.detail = True
        self.imports = {}
        rva, size = struct.unpack_from('<II', self.data, self.opt + 112 + 8)
        for i in range(0, size, 20):
            oft, stamp, chain, name, ft = struct.unpack('<IIIII', self.raw(self.base + rva + i, 20))
            if not any((oft, stamp, chain, name, ft)):
                break
            dll = self.cstring(self.base + name)
            j = 0
            while True:
                thunk = self.u64(self.base + (oft or ft) + j * 8)
                if not thunk:
                    break
                symbol = 'ordinal:' + str(thunk & 0xffff) if thunk >> 63 else self.cstring(self.base + thunk + 2)
                self.imports[self.base + ft + j * 8] = dll + '!' + symbol
                j += 1

    def offset(self, va, size=1):
        for start, vs, rs, raw, flags in self.sections:
            if start <= va and va + size <= start + min(vs, rs):
                return raw + va - start
        raise ValueError(f'unbacked VA {va:#x}, size {size}')

    def raw(self, va, size):
        p = self.offset(va, size)
        return self.data[p:p + size]

    def u64(self, va):
        return struct.unpack('<Q', self.raw(va, 8))[0]

    def cstring(self, va):
        p = self.offset(va)
        return self.data[p:self.data.index(b'\0', p)].decode('ascii')

    def executable(self, va):
        return any(flags & 0x20000000 and start <= va < start + min(vs, rs)
                   for start, vs, rs, raw, flags in self.sections)

    def owner(self, va):
        i = bisect.bisect_right(self.starts, va) - 1
        return self.ranges[i] if i >= 0 and va < self.ranges[i][1] else None

    def instruction(self, va):
        if not self.executable(va):
            raise ValueError(f'non-executable instruction {va:#x}')
        p = self.offset(va)
        ins = next(self.md.disasm(self.data[p:p + 15], va, count=1), None)
        if ins is None:
            raise ValueError(f'undecodable instruction {va:#x}')
        self.offset(va, ins.size)
        return ins

    def import_thunk(self, va):
        ins = self.instruction(va)
        if ins.mnemonic == 'jmp' and len(ins.operands) == 1:
            op = ins.operands[0]
            if op.type == capstone.x86.X86_OP_MEM and op.mem.base == capstone.x86.X86_REG_RIP:
                slot = ins.address + ins.size + op.mem.disp
                if slot in self.imports:
                    return slot, self.imports[slot]
        return None
