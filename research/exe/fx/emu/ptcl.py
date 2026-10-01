from emu import *
from records import *
import struct, sys
VT_SPRT = 0x1404e2c68; VT_FSPRT = 0x1404e2ce8
def make_emitter(e, rec, at=(0.0, 0.0, 0.0), dt=1.0):
    recp = e.alloc(len(rec) + 64, 64); e.write(recp, rec)
    defp = e.call(0x140312dc0, recp)
    cls = e.read(defp + 1, 1)[0]
    # matrix
    M = e.alloc(64, 16)
    e.write(M, struct.pack('<16f', 1,0,0,0, 0,1,0,0, 0,0,1,0, at[0],at[1],at[2],1))
    obj = e.alloc(0xd00 + 0x100, 64)
    e.call(0x140232320, obj)           # CEffectBase ctor
    e.w64(obj, VT_SPRT)
    e.call(0x1402e8170, obj, M, 0, 0)  # matrix copy, parent, mode
    e.call(0x1402e81b0, obj, 0, 0)
    e.wf(obj + 0x70, 0.0)
    e.w64(obj + 0x4a0, defp)
    e.w32(obj + 0x114, e.u32(defp + 0x64))
    e.wf(obj + 0x14, dt)
    layers = []
    n = struct.unpack('<H', e.read(defp + 0xc0, 2))[0]
    for i in range(n):
        ldef = e.u64(e.u64(defp + 0xc8) + 8 * i)
        f = e.alloc(0x520 + 0x100, 64)
        e.call(0x140232320, f); e.w64(f, VT_FSPRT)
        e.w64(f + 0x4a0, ldef); e.w64(f + 0x510, obj)
        e.w32(f + 0x114, e.u32(ldef + 0x44))
        layers.append((f, ldef))
    return obj, defp, layers, cls
if __name__ == '__main__':
    recs = em034_bank()
    e = Emu()
    e.stubs[0x140346c38] = lambda em: em.mu.reg_write(UC_X86_REG_RAX, 1)
    POOL = 0x140CEFFD0
    k = 0
    for (kind, id_), data in recs.items():
        if kind == 'A' and k < 0x80:
            p = e.alloc(len(data) + 32, 16); e.write(p, data)
            e.w32(POOL + 8 + 16 * k, id_); e.w64(POOL + 16 + 16 * k, p); k += 1
    # globals that the update reads
    gp = e.alloc(0x100, 64)
    e.w64(0x140D6D300, gp)
    e.mu.mem_write(0x1405d50b0, struct.pack('<Q', 0x2b992ddfa232))  # security cookie
    obj, defp, layers, cls = make_emitter(e, recs[('P', 3)])
    print('emitter', hex(obj), 'class', cls, 'layers', [hex(a) for a, b in layers])
    # init (vtable slot 8)
    e.call(e.u64(VT_SPRT + 8 * 8), obj)
    print('init ok; count', e.read(obj + 0xf4, 1)[0], 'life', e.f32(obj + 0x4d0), 'log', e.log[-6:])
    for f, l in layers:
        e.call(e.u64(VT_FSPRT + 8 * 8), f)
    print('layers init ok', e.log[-6:])
    for frame in range(3):
        e.wf(obj + 0x14, 1.0)
        try:
            e.call(e.u64(VT_SPRT + 8 * 9), obj)
            for f, l in layers: e.call(e.u64(VT_FSPRT + 8 * 9), f)
        except Exception as ex:
            print('frame', frame, ex); break
        print('frame', frame, 'ok', e.log[-3:])
