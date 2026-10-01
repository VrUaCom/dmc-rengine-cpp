from gt import mk_record, LAYERS
from ptcl2 import *
import json, math

def mk_poly(layers, world_gravity):
    rec = bytearray(mk_record(layers, world_gravity, False))
    d = 0x20
    rec[d + 1] = 1
    rec[d + 0xe0:d + 0x130] = bytes(0x50)
    struct.pack_into('<f', rec, d + 0xe0, -0.3)                  # gravity
    struct.pack_into('<3h', rec, d + 0xe8, 300, 220, 140)        # push
    struct.pack_into('<3h', rec, d + 0xee, 16, -32, 8)           # bias
    struct.pack_into('<3h', rec, d + 0xf4, 112, 80, 64)          # spread
    rec[d + 0xfa] = 1
    struct.pack_into('<h', rec, d + 0xfc, 50)                    # shard
    rec[d + 0xfe] = 1 if world_gravity else 0
    struct.pack_into('<3h', rec, d + 0x100, 48, 40, 32)          # hollow
    struct.pack_into('<3f', rec, d + 0x108, 0.25, 0.35, 0.15)    # friction
    struct.pack_into('<3f', rec, d + 0x114, 0.01, 0.0, 0.02)     # decay
    return bytes(rec)

def run(wg, nupd):
    rec = mk_poly(LAYERS, wg)
    c, s = math.cos(0.7), math.sin(0.7)
    W = (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 100.0, 20.0, -30.0, 1)
    sy = Sys(rec, W=W)
    e = sy.e
    assert sy.cls == 1
    n = e.read(sy.obj + 0xf4, 1)[0]
    out = dict(record=rec.hex(), world=list(W), count=n)
    out['init_tri'] = sy.verts(n, 3)
    out['init_vel'] = [sy.rd(sy.obj + 0xb80 + 12 * i, 3) for i in range(n)]
    out['frames'] = []
    for fr in range(nupd):
        sy.update()
        o = sy.obj
        out['frames'].append(dict(life=e.f32(o + 0x4d0), tr=sy.rd(o + 0x120, 3), rot=sy.rd(o + 0x130, 3), sc=sy.rd(o + 0x140, 3),
            col=list(e.read(o + 0x4a8, 4)), tri=sy.verts(n, 3), vel=[sy.rd(o + 0xb80 + 12 * i, 3) for i in range(n)],
            layers=[dict(tr=sy.rd(f + 0x120, 3), rot=sy.rd(f + 0x130, 3), sc=sy.rd(f + 0x140, 3), col=list(e.read(f + 0x4a8, 4))) for f in sy.layers]))
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
    return out

if __name__ == '__main__':
    res = {'wg1': run(True, 14), 'wg0': run(False, 14)}
    json.dump(res, open('gt_poly.json', 'w'))
    print(len(res['wg1']['final']), res['wg1']['frames'][0]['tri'][0], res['wg1']['frames'][13]['tri'][0])
