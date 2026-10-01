import os
import collections, sys
from ptcl import *
from imports import install_crt
def setup(recs=None):
    recs = recs if recs is not None else em034_bank()
    e = Emu()
    install_crt(e)
    e.stubs[0x1403261b0] = lambda em: em.wf(em.mu.reg_read(UC_X86_REG_RCX) + 0x14, 1.0)
    POOL = 0x140CEFFD0; k=0
    for (kind, id_), data in recs.items():
        if kind == 'A' and k < 0x80:
            p = e.alloc(len(data) + 32, 16); e.write(p, data)
            e.w32(POOL + 8 + 16 * k, id_); e.w64(POOL + 16 + 16 * k, p); k += 1
    gp = e.alloc(0x100, 64); e.w64(0x140D6D300, gp)
    for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'init_funcs.txt')).read().split():
        e.call(int(line, 16))
    cam = e.alloc(0x400, 64)
    e.write(cam + 0x100, struct.pack('<16f', 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1))
    e.w64(0x140CF2330, cam)
    return e, recs
if __name__ == '__main__':
    e, recs = setup()
    obj, defp, layers, cls = make_emitter(e, recs[('P', 3)])
    e.call(e.u64(VT_SPRT + 64), obj)
    for f, l in layers: e.call(e.u64(VT_FSPRT + 64), f)
    ring = collections.deque(maxlen=40)
    e.mu.hook_add(UC_HOOK_CODE, lambda mu,a,s,u: ring.append(a))
    e.wf(obj + 0x14, 1.0)
    try: e.call(e.u64(VT_SPRT + 72), obj)
    except Exception as ex: print(ex); print([hex(a) for a in ring])
