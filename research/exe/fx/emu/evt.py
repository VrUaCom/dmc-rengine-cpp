import os
"""Emulate the enemy event handler 0x1401C3130 for every code; record effect spawns and parents."""
import struct, sys, json
from emu import *
from imports import install_crt
API = {0x1402E7AB0: 'AB0', 0x1402E7A90: 'A90', 0x1402E7CA0: 'CA0', 0x1402E7A80: 'A80'}
def setup():
    e = Emu(); install_crt(e)
    e.stubs[0x1403261b0] = lambda em: em.wf(em.mu.reg_read(UC_X86_REG_RCX) + 0x14, 1.0)
    for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'init_funcs.txt')).read().split(): e.call(int(line, 16))
    # Game manager 0x140C90E28 with an empty +0x28 block: the "no player" branch.
    gm = e.alloc(0x2000, 64); b2 = e.alloc(0x2000, 64)
    e.w64(0x140C90E28, gm); e.w64(gm + 0x28, b2)
    return e
def run(code, etype=0, handler=0x1401C3130, vt_main=0x1404c9af8, vt_sub=0x1404c9cd0):
    e = setup()
    spawns = []
    def mk(api):
        def f(em):
            mu = em.mu
            k = mu.reg_read(UC_X86_REG_RCX) & 0xffffffff; i = mu.reg_read(UC_X86_REG_RDX) & 0xffffffff
            m = mu.reg_read(UC_X86_REG_R8); r9 = mu.reg_read(UC_X86_REG_R9) & 0xffffffff
            rsp = mu.reg_read(UC_X86_REG_RSP)
            flags = em.u32(rsp + 0x28)
            mat = list(struct.unpack('<16f', em.read(m, 64))) if m else None
            eff = em.alloc(0x1000, 16)
            spawns.append({'api': api, 'kind': k, 'id': i, 'mode': r9, 'flags': flags, 'matrix': mat, 'eff': eff})
            mu.reg_write(UC_X86_REG_RAX, eff)
        return f
    for a, n in API.items(): e.stubs[a] = mk(n)
    e.stubs[0x140338940] = lambda em: em.mu.reg_write(UC_X86_REG_RAX, 0)  # sound
    obj = e.alloc(0x8000, 64); sub = obj + 0x110
    e.w64(obj, vt_main); e.w64(sub, vt_sub)
    parts = {}
    for off in range(0x5a0, 0x700, 8):
        p = e.alloc(0x400, 64); mat = e.alloc(64, 64)
        k = (off - 0x5c8) // 8
        e.write(mat, struct.pack('<16f', 1,0,0,0, 0,1,0,0, 0,0,1,0, 10000 + 1000 * k, 0, 0, 1))
        e.w64(p + 0x110, mat); e.w64(sub + off, p); parts[mat] = f'sub+{off:#x}'
    e.write(sub - 0x50, struct.pack('<h', 0x4000))
    e.w32(sub + 0x560, etype)
    e.write(sub - 0x90, struct.pack('<4f', 1, 2, 3, 1))
    pos = e.alloc(64, 16); e.write(pos, struct.pack('<4f', 100, 200, 300, 1))
    err = None
    try: e.call(handler, sub, code, pos)
    except Exception as ex: err = str(ex)[:160]
    for s in spawns:
        eff = s.pop('eff')
        for po, mo in ((0xC0, 0xD8), (0xC8, 0xD4)):
            pp = e.u64(eff + po)
            if pp: s[f'parent{po:#x}'] = (parts.get(pp, hex(pp)), e.u32(eff + mo))
        if s['matrix']: s['matrix'] = [round(x, 4) for x in s['matrix']]
    return spawns, err
if __name__ == '__main__':
    out = {}
    codes = [int(c, 0) for c in sys.argv[1:]] or list(range(0, 0x2d)) + [0x66, 0x67, 0x68, 0x69, 0x6a] + list(range(0xc8, 0xcf))
    for c in codes:
        for et in (0, 0x1a):
            sp, err = run(c, et)
            if et and c not in range(0xc8, 0xcf): continue
            out[f'{c:#x}/{et}'] = {'spawns': sp, 'err': err}
            print(f'{c:#x} type{et}', err or '', [(s['api'], 'PEGV'[s['kind']] if s['kind'] < 4 else s['kind'], s['id'], s.get('parent0xc8') or s.get('parent0xc0'), s['matrix'][12:15] if s['matrix'] else None, [s['matrix'][0], s['matrix'][5], s['matrix'][10]] if s['matrix'] else None) for s in sp])
    json.dump(out, open('evt.json', 'w'), indent=0)
