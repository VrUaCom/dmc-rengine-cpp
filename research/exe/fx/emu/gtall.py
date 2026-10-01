from ptcl2 import *
import json, math, random

LAYERS = [
    dict(t=(-9.0, 4.0, 2.0), r=(-0.2, 0.4, 0.1), s=(1.6, 1.6, 1.6), mt=(4, 30, 0), mr=(218, 0, 217), ms=(-81, -81, -81), blend=0x48),
    dict(t=(3.0, -2.0, 1.0), r=(1.0, -0.7, 2.0), s=(0.8, 1.1, 1.0), mt=(-3, 2, 7), mr=(-300, 120, 60), ms=(15, -5, 10), blend=0x44, loop=1),
]

def mk(cls, wg, eases, seed):
    rnd = random.Random(seed)
    rec = bytearray(0x160 + 0xB0 * 2)
    struct.pack_into('<I', rec, 0, 2)
    struct.pack_into('<4i', rec, 0x10, 0x10, 0x140, 0x150, 0)
    for i in range(2): struct.pack_into('<i', rec, 0x18 + 4 * i, 0x150 + 0xB0 * i)
    d = 0x20
    rec[d] = 2; rec[d + 1] = cls; rec[d + 2:d + 5] = b'sy0'
    struct.pack_into('<4f', rec, d + 0x20, 12.0, -7.0, 5.0, 1.0)
    struct.pack_into('<4f', rec, d + 0x30, 0.3, -0.5, 1.1, 1.0)
    struct.pack_into('<4f', rec, d + 0x40, 1.2, 0.9, 1.5, 1.0)
    struct.pack_into('<3h', rec, d + 0x52, 5, -9, 3)
    struct.pack_into('<3h', rec, d + 0x58, 150, -200, 90)
    struct.pack_into('<3h', rec, d + 0x5e, -20, 10, 5)
    struct.pack_into('<i', rec, d + 0x68, 20)
    rec[d + 0x65] = 0x44
    def kfs(base):
        for k, dur in enumerate((6, 5, 7, 0)):
            o = base + 20 * k
            struct.pack_into('<h', rec, o, dur)
            rec[o + 4:o + 20] = bytes(rnd.randrange(256) for _ in range(16))
    kfs(d + 0x6c)
    rec[d + 0xbc] = 1; rec[d + 0xbd] = 0; rec[d + 0xbe] = eases[0]
    struct.pack_into('<H', rec, d + 0xc0, 2)
    if cls == 3:
        struct.pack_into('<3h', rec, d + 0xe2, 288, 200, 160)
        struct.pack_into('<3h', rec, d + 0xee, 16, -16, 0)
        struct.pack_into('<3f', rec, d + 0xf4, 0.3, 0.2, 0.5)
        struct.pack_into('<3h', rec, d + 0x100, 96, 64, 48)
        struct.pack_into('<H', rec, d + 0x106, 6)
        struct.pack_into('<2H', rec, d + 0x108, 400, 320)
        struct.pack_into('<f', rec, d + 0x10c, -0.25)
        rec[d + 0x110] = 1 if wg else 0
        struct.pack_into('<3h', rec, d + 0x116, 40, 32, 24)
        struct.pack_into('<3f', rec, d + 0x11c, 0.01, 0.0, 0.02)
        rec[d + 0x128] = 0
    elif cls == 1:
        struct.pack_into('<f', rec, d + 0xe0, -0.3)
        struct.pack_into('<3h', rec, d + 0xe8, 300, 220, 140)
        struct.pack_into('<3h', rec, d + 0xee, 16, -32, 8)
        struct.pack_into('<3h', rec, d + 0xf4, 112, 80, 64)
        rec[d + 0xfa] = 1
        struct.pack_into('<h', rec, d + 0xfc, 50)
        rec[d + 0xfe] = 1 if wg else 0
        struct.pack_into('<3h', rec, d + 0x100, 48, 40, 32)
        struct.pack_into('<3f', rec, d + 0x108, 0.25, 0.35, 0.15)
        struct.pack_into('<3f', rec, d + 0x114, 0.01, 0.0, 0.02)
    else:  # class 4
        struct.pack_into('<f', rec, d + 0xe0, -0.2)
        struct.pack_into('<2h', rec, d + 0xe4, 400, 240)       # segment lengths
        struct.pack_into('<3h', rec, d + 0xe8, 280, 200, 120)
        struct.pack_into('<3h', rec, d + 0xee, 16, -32, 8)
        struct.pack_into('<3h', rec, d + 0xf4, 112, 80, 64)
        rec[d + 0xfa] = 1 if wg else 0
        struct.pack_into('<3h', rec, d + 0xfc, 48, 40, 32)
    for i, L in enumerate(LAYERS):
        l = 0x160 + 0xB0 * i
        struct.pack_into('<4f', rec, l, *L['t'], 1.0)
        struct.pack_into('<4f', rec, l + 0x10, *L['r'], 1.0)
        struct.pack_into('<4f', rec, l + 0x20, *L['s'], 1.0)
        struct.pack_into('<3h', rec, l + 0x30, *L['mt'])
        struct.pack_into('<3h', rec, l + 0x36, *L['mr'])
        struct.pack_into('<3h', rec, l + 0x3c, *L['ms'])
        rec[l + 0x45] = L['blend']
        kfs(l + 0x48)
        rec[l + 0x98] = 1; rec[l + 0x99] = L.get('loop', 0); rec[l + 0x9a] = eases[1 + i]
    return bytes(rec)

PER = {3: (12, 4), 1: (16, 4), 4: (12, 4)}   # particles, vertex slots read
def run(cls, wg, eases, seed, nupd=14):
    rec = mk(cls, wg, eases, seed)
    c, s = math.cos(0.7), math.sin(0.7)
    W = (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 100.0, 20.0, -30.0, 1)
    sy = Sys(rec, W=W)
    e = sy.e
    assert sy.cls == cls, (sy.cls, cls)
    n, nv = PER[cls]
    per = {3: 4, 1: 3, 4: 4}[cls]
    def snap():
        o = sy.obj
        d = dict(tr=sy.rd(o + 0x120, 3), rot=sy.rd(o + 0x130, 3), sc=sy.rd(o + 0x140, 3), col=list(e.read(o + 0x4a8, 16)), life=e.f32(o + 0x4d0),
                 vel=[sy.rd(o + 0xb80 + 12 * i, 3) for i in range(n)],
                 layers=[dict(tr=sy.rd(f + 0x120, 3), rot=sy.rd(f + 0x130, 3), sc=sy.rd(f + 0x140, 3), col=list(e.read(f + 0x4a8, 16))) for f in sy.layers])
        if cls == 3:
            d['pos'] = [sy.rd(o + 0xb80 + 12 * (n + i), 3) for i in range(n)]
        else:
            slot = sy.tick & 1
            base = o + 0x540 + slot * 0x330
            d['vert'] = [[sy.rd(base + 16 * (per * i + k), 3) for k in range(per)] + [[0, 0, 0]] * (4 - per) for i in range(n)]
        return d
    out = dict(record=rec.hex(), world=list(W), n=n, init=snap(), frames=[])
    for fr in range(nupd):
        sy.update(); out['frames'].append(snap())
    cap = []
    def hook(mu, a, sz, u):
        if a == 0x140313109:
            rbp = mu.reg_read(UC_X86_REG_RBP)
            cap.append(list(struct.unpack('<16f', e.read(rbp + 0x50, 64))))
            mu.emu_stop()
    e.mu.hook_add(UC_HOOK_CODE, hook)
    e.call(e.u64(sy.vt_o + 24), sy.obj)
    for f in sy.layers: e.call(e.u64(sy.vt_f + 24), f)
    out['final'] = cap
    if cls == 3:
        slot = sy.tick & 1
        base = sy.obj + 0x540 + slot * 0x330
        out['baked'] = [[sy.rd(base + 64 * i + 16 * k, 3) for k in range(4)] for i in range(n)]
    return out

SCEN = {'sw1': (3, True, (0, 1, 2), 11), 'sw0': (3, False, (2, 0, 1), 12),
        'pw1': (1, True, (0, 1, 2), 13), 'pw0': (1, False, (1, 2, 0), 14),
        'lw1': (4, True, (0, 1, 2), 15), 'lw0': (4, False, (2, 0, 1), 16)}
if __name__ == '__main__':
    import sys
    res = {}
    for k, (cls, wg, eases, seed) in SCEN.items():
        res[k] = run(cls, wg, eases, seed)
        print(k, 'ok', len(res[k]['final']))
    json.dump(res, open('gtall.json', 'w'))
