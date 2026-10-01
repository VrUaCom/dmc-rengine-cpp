from dbg2 import *
import json, math, sys

def s16(v): return struct.pack('<h', v)
def mk_record(layers, world_gravity, random_frame):
    rec = bytearray(0x150 + 0xB0 * len(layers) + (0x10 if layers else 0) - (0x10 if layers else 0))
    rec = bytearray(0x160 + 0xB0 * len(layers))
    struct.pack_into('<I', rec, 0, 2)
    struct.pack_into('<4i', rec, 0x10, 0x10, 0x140, 0x150, 0)   # def off, ptr array, layer offs...
    for i in range(len(layers)): struct.pack_into('<i', rec, 0x18 + 4 * i, 0x150 + 0xB0 * i)
    d = 0x20
    rec[d] = 2; rec[d + 1] = 3; rec[d + 2:d + 5] = b'sy0'
    struct.pack_into('<4f', rec, d + 0x20, 12.0, -7.0, 5.0, 1.0)
    struct.pack_into('<4f', rec, d + 0x30, 0.3, -0.5, 1.1, 1.0)
    struct.pack_into('<4f', rec, d + 0x40, 1.2, 0.9, 1.5, 1.0)
    rec[d + 0x50] = 0xf8
    struct.pack_into('<3h', rec, d + 0x52, 5, -9, 3)
    struct.pack_into('<3h', rec, d + 0x58, 150, -200, 90)
    struct.pack_into('<3h', rec, d + 0x5e, -20, 10, 5)
    struct.pack_into('<i', rec, d + 0x68, 20)
    kf = [(6, (200, 100, 50, 255)), (5, (100, 200, 50, 128)), (7, (0, 50, 255, 64)), (0, (10, 20, 30, 0))]
    for k, (dur, col) in enumerate(kf):
        o = d + 0x6c + 20 * k
        struct.pack_into('<h', rec, o, dur); rec[o + 4:o + 8] = bytes(col)
        rec[o + 8:o + 20] = bytes([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12])
    rec[d + 0xbc] = 1; rec[d + 0xbd] = 0; rec[d + 0xbe] = 0
    struct.pack_into('<H', rec, d + 0xc0, len(layers))
    struct.pack_into('<3h', rec, d + 0xe2, 288, 200, 160)
    struct.pack_into('<3h', rec, d + 0xee, 16, -16, 0)
    struct.pack_into('<3f', rec, d + 0xf4, 0.3, 0.2, 0.5)
    struct.pack_into('<3h', rec, d + 0x100, 96, 64, 48)
    struct.pack_into('<H', rec, d + 0x106, 6)
    struct.pack_into('<2H', rec, d + 0x108, 400, 320)
    struct.pack_into('<f', rec, d + 0x10c, -0.25)
    rec[d + 0x110] = 1 if world_gravity else 0
    struct.pack_into('<3h', rec, d + 0x116, 40, 32, 24)
    struct.pack_into('<3f', rec, d + 0x11c, 0.01, 0.0, 0.02)
    rec[d + 0x128] = 1 if random_frame else 0
    for i, L in enumerate(layers):
        l = 0x160 + 0xB0 * i
        struct.pack_into('<4f', rec, l, *L['t'], 1.0)
        struct.pack_into('<4f', rec, l + 0x10, *L['r'], 1.0)
        struct.pack_into('<4f', rec, l + 0x20, *L['s'], 1.0)
        struct.pack_into('<3h', rec, l + 0x30, *L['mt'])
        struct.pack_into('<3h', rec, l + 0x36, *L['mr'])
        struct.pack_into('<3h', rec, l + 0x3c, *L['ms'])
        for k, (dur, col) in enumerate(L['kf']):
            o = l + 0x48 + 20 * k
            struct.pack_into('<h', rec, o, dur); rec[o + 4:o + 8] = bytes(col)
        rec[l + 0x98] = 1; rec[l + 0x99] = L.get('loop', 0)
    return bytes(rec)

LAYERS = [
    dict(t=(-9.0, 4.0, 2.0), r=(-0.2, 0.4, 0.1), s=(1.6, 1.6, 1.6), mt=(4, 30, 0), mr=(218, 0, 217), ms=(-81, -81, -81),
         kf=[(5, (255, 255, 255, 255)), (6, (128, 64, 32, 90)), (8, (48, 48, 48, 16)), (0, (16, 16, 16, 0))]),
    dict(t=(3.0, -2.0, 1.0), r=(1.0, -0.7, 2.0), s=(0.8, 1.1, 1.0), mt=(-3, 2, 7), mr=(-300, 120, 60), ms=(15, -5, 10),
         kf=[(4, (10, 200, 30, 255)), (4, (30, 60, 90, 120)), (4, (90, 90, 90, 40)), (0, (0, 0, 0, 0))], loop=1),
]

def run(world_gravity, nupd):
    rec = mk_record(LAYERS, world_gravity, False)
    e, recs = setup()
    e.stubs[0x14032d4a0] = lambda em: None
    e.stubs[0x14032d410] = lambda em: None
    obj, defp, layers, cls = make_emitter(e, rec)
    c, s = math.cos(0.7), math.sin(0.7)
    W = (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 100.0, 20.0, -30.0, 1)
    e.write(obj + 0x80, struct.pack('<16f', *W))
    for f, l in layers: e.write(f + 0x80, struct.pack('<16f', *W))
    e.call(e.u64(VT_SPRT + 64), obj)
    for f, l in layers: e.call(e.u64(VT_FSPRT + 64), f)
    n = 12
    def rd(a, k): return list(struct.unpack('<%df' % k, e.read(a, 4 * k)))
    out = dict(record=rec.hex(), world=list(W))
    out['init_pos'] = [rd(obj + 0xb80 + 12 * (n + i), 3) for i in range(n)]
    out['init_vel'] = [rd(obj + 0xb80 + 12 * i, 3) for i in range(n)]
    out['frames'] = []
    for fr in range(nupd):
        e.call(e.u64(VT_SPRT + 72), obj)
        for f, l in layers: e.call(e.u64(VT_FSPRT + 72), f)
        fd = dict(life=e.f32(obj + 0x4d0),
                  tr=rd(obj + 0x120, 3), rot=rd(obj + 0x130, 3), sc=rd(obj + 0x140, 3),
                  col=list(e.read(obj + 0x4a8, 4)),
                  pos=[rd(obj + 0xb80 + 12 * (n + i), 3) for i in range(n)],
                  vel=[rd(obj + 0xb80 + 12 * i, 3) for i in range(n)],
                  layers=[dict(tr=rd(f + 0x120, 3), rot=rd(f + 0x130, 3), sc=rd(f + 0x140, 3), col=list(e.read(f + 0x4a8, 4))) for f, l in layers])
        out['frames'].append(fd)
    # draw capture
    cap = []
    def hook(mu, a, sz, u):
        if a == 0x140313109:
            rbp = mu.reg_read(UC_X86_REG_RBP)
            cap.append(list(struct.unpack('<16f', e.read(rbp + 0x50, 64))))
            mu.emu_stop()
    e.mu.hook_add(UC_HOOK_CODE, hook)
    e.call(e.u64(VT_SPRT + 24), obj)
    for f, l in layers: e.call(e.u64(VT_FSPRT + 24), f)
    slot = e.read(e.u64(0x140D6D300) + 0xe, 1)[0]
    base = obj + 0x540 + slot * 0x330
    out['verts'] = [[rd(base + 64 * i + 16 * k, 4) for k in range(4)] for i in range(n)]
    out['final'] = cap
    return out

if __name__ == '__main__':
    res = {'wg1': run(True, 14), 'wg0': run(False, 14)}
    json.dump(res, open('gt.json', 'w'))
    f = res['wg1']['final']
    print(len(f), [round(x, 3) for x in f[0]])
    v = res['wg1']['verts'][0]
    print(v)
