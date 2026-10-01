from dbg2 import *
VT = {1: (0x1404e29c8, 0x1404e2a48), 2: (0x1404e2b18, 0x1404e2b98), 3: (0x1404e2c68, 0x1404e2ce8), 4: (0x1404e2728, 0x1404e27a8), 0: (0x1404e25d0, 0x1404e2650), 5: (0x1404e2878, 0x1404e28f8)}
class Sys:
    def __init__(self, rec, W=None, cam=None, limit=300000, recs=None):
        self.e, self.recs = setup(recs)
        e = self.e
        e.limit = limit
        e.stubs[0x14032d4a0] = lambda em: None
        e.stubs[0x14032d410] = lambda em: None
        self.rec = rec
        recp = e.alloc(len(rec) + 64, 64); e.write(recp, rec)
        defp = e.call(0x140312dc0, recp)
        self.cls = e.read(defp + 1, 1)[0]
        self.vt_o, self.vt_f = VT[self.cls]
        M = e.alloc(64, 16)
        e.write(M, struct.pack('<16f', *(W or (1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1))))
        obj = e.alloc(0xe00, 64)
        e.call(0x140232320, obj)
        e.w64(obj, self.vt_o)
        e.call(0x1402e8170, obj, M, 0, 0)
        e.call(0x1402e81b0, obj, 0, 0)
        e.wf(obj + 0x70, 0.0)
        e.w64(obj + 0x4a0, defp)
        e.w32(obj + 0x114, e.u32(defp + 0x64))
        n = struct.unpack('<H', e.read(defp + 0xc0, 2))[0]
        self.layers = []
        for i in range(n):
            ldef = e.u64(e.u64(defp + 0xc8) + 8 * i)
            f = e.alloc(0x600, 64)
            e.call(0x140232320, f); e.w64(f, self.vt_f)
            e.w64(f + 0x4a0, ldef); e.w64(f + 0x510, obj)
            e.w32(f + 0x114, e.u32(ldef + 0x44))
            e.write(f + 0x80, e.read(obj + 0x80, 64))
            self.layers.append(f)
        self.obj = obj; self.defp = defp
        self.gp = e.u64(0x140D6D300)
        e.write(self.gp + 0xe, b'\0')
        e.call(e.u64(self.vt_o + 64), obj)
        for f in self.layers: e.call(e.u64(self.vt_f + 64), f)
        self.tick = 0
    def update(self):
        e = self.e
        self.tick += 1
        e.write(self.gp + 0xe, bytes([self.tick & 1]))
        e.call(e.u64(self.vt_o + 72), self.obj)
        for f in self.layers: e.call(e.u64(self.vt_f + 72), f)
    def rd(self, a, k): return list(struct.unpack('<%df' % k, self.e.read(a, 4 * k)))
    def verts(self, count, per):
        slot = self.tick & 1
        base = self.obj + 0x540 + slot * 0x330
        return [[self.rd(base + 16 * (per * i + k), 4) for k in range(per)] for i in range(count)]
if __name__ == '__main__':
    import sys
    rec = open(sys.argv[1], 'rb').read()
    s = Sys(rec)
    print('class', s.cls, 'count', s.e.read(s.obj + 0xf4, 1)[0], 'kind', s.e.u32(s.obj + 0xf0))
    for t in range(3):
        v = s.verts(2, 3)
        print('tick', s.tick, 'p0', [[round(x, 2) for x in q] for q in v[0]])
        s.update()
