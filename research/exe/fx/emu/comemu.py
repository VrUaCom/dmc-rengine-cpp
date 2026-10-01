"""Run enemy AI command pairs (start, update) of a CCom class with a stub enemy interface.

python3 comemu.py [VTABLE] [FIRST_SLOT LAST_SLOT]     (default: CComEm000 0x1404C6A18)
FX_LENGTHS=lengths.json (from action_lengths.py) FX_TABLES=0x1405A3300,0x1405A31E8

The command object gets an interface pointer at +0xE0 whose vtable slots are
stubs: +0x20 / +0x28 record play(cmd) / play2(cmd) and restart the motion
clock, +0x40 returns the motion frame (ticks since the last play), +0x80
records event codes, +0x48 ("current action finished", 0x140059590 on the
script player) is true before the first play and once the frame reaches the
played action's length; other slots return 1 / 0.0. play(c) is action
table1[c], play2(c) action table2[c] of bank 0 (class tables, see the note).
Each update call is one tick.
Output: per pair, the plays and the events with the frame they fired at.
"""
import json, os, struct, sys
from emu import *
from imports import install_crt

HERE = os.path.dirname(os.path.abspath(__file__))

def setup():
    e = Emu(); install_crt(e)
    e.stubs[0x1403261b0] = lambda em: em.wf(em.mu.reg_read(UC_X86_REG_RCX) + 0x14, 1.0)
    for line in open(os.path.join(HERE, 'init_funcs.txt')).read().split(): e.call(int(line, 16))
    gm = e.alloc(0x2000, 64); b2 = e.alloc(0x2000, 64)
    e.w64(0x140C90E28, gm); e.w64(gm + 0x28, b2)
    e.stubs[0x140338940] = lambda em: em.mu.reg_write(UC_X86_REG_RAX, 0)  # sound
    return e

STUB_BASE = 0x7FD000000
NSLOTS = 96

LENGTHS = json.load(open(os.environ['FX_LENGTHS'])) if os.environ.get('FX_LENGTHS') else {}
TABLES = [int(x, 16) for x in os.environ.get('FX_TABLES', '0x1405A3300,0x1405A31E8').split(',')]

class Run:
    def __init__(self, e):
        self.e = e; self.log = []; self.frame = 0.0; self.tick = 0; self.action = None
        try: e.mu.mem_map(STUB_BASE, 0x4000, UC_PROT_ALL)
        except Exception: pass
        e.write(STUB_BASE, b'\xc3' * 0x4000)
        vt = e.alloc(8 * NSLOTS, 16)
        for i in range(NSLOTS):
            a = STUB_BASE + 16 * i
            e.w64(vt + 8 * i, a)
            e.stubs[a] = self.mk(i)
        self.iface = e.alloc(0x4000, 64); e.w64(self.iface, vt)
        # The command object's own vtable: stubs that record the call (the
        # command hands over to the scheduler through these).
        svt = e.alloc(8 * 300, 16)
        for i in range(300):
            a = STUB_BASE + 0x800 + 16 * i if i < 128 else STUB_BASE + 0x2000 + 16 * i
            e.w64(svt + 8 * i, a)
            e.stubs[a] = self.mk_self(i)
        self.obj = e.alloc(0x2000, 64); e.w64(self.obj, svt)
        actor = e.alloc(0x1000, 64); kind = e.alloc(0x100, 16)
        e.w64(self.obj + 0xE0, self.iface); e.w64(self.obj + 0x20, actor); e.w64(self.obj + 0x08, kind)
        e.w64(self.obj + 0x10, self.iface)
    def mk_self(self, i):
        def f(em):
            em.mu.reg_write(UC_X86_REG_RAX, 0)
            self.log.append(('self', 8 * i, self.tick, self.frame))
        return f
    def mk(self, i):
        def f(em):
            mu = em.mu
            rdx = mu.reg_read(UC_X86_REG_RDX) & 0xffffffff
            mu.reg_write(UC_X86_REG_RAX, 1)
            mu.reg_write(UC_X86_REG_XMM0, int.from_bytes(struct.pack('<f', 0.0) + bytes(12), 'little'))
            off = 8 * i
            if off in (0x20, 0x28):
                t = TABLES[0 if off == 0x20 else 1]
                bank, action = em.read(t + 2 * rdx, 2)
                self.action = f'{bank}/{action}'
                self.log.append(('play' if off == 0x20 else 'play2', rdx, self.tick, self.action)); self.frame = 0.0
            elif off == 0x48:
                info = LENGTHS.get(self.action) if self.action else None
                done = self.action is None or (info is not None and info['loop'] != 1 and self.frame >= info['frames'] - 1)
                mu.reg_write(UC_X86_REG_RAX, 1 if done else 0)
            elif off == 0x40:
                mu.reg_write(UC_X86_REG_XMM0, int.from_bytes(struct.pack('<f', self.frame) + bytes(12), 'little'))
            elif off == 0x80:
                kind = 'event' if rdx < 0x384 else 'control'
                self.log.append((kind, rdx, self.tick, self.frame, self.action))
        return f

def _unused(): pass

def run_pair(update, start, ticks=400):
    e = setup(); r = Run(e); err = None
    try:
        e.call(start, r.obj)
        for t in range(ticks):
            r.tick = t
            e.call(update, r.obj)
            r.frame += 1.0
    except Exception as ex:
        err = str(ex)[:120]
    return r.log, err

if __name__ == '__main__':
    vt = int(sys.argv[1], 16) if len(sys.argv) > 1 else 0x1404C6A18
    lo = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    hi = int(sys.argv[3]) if len(sys.argv) > 3 else 260
    sys.path.insert(0, os.path.join(HERE, '..'))
    from pe import Image
    img = Image(os.environ.get('DMC3_EXE', 'dmc3.exe'))
    tb, text = img.text()
    out = []
    for s in range(lo, hi, 2):
        u = img.u64(vt + 8 * s); st = img.u64(vt + 8 * (s + 1))
        if not (tb <= u < tb + len(text) and tb <= st < tb + len(text)): break
        if u == 0x140066A40: continue  # empty update
        log, err = run_pair(u, st)
        ev = [x for x in log if x[0] in ('event', 'play', 'play2')]
        if not any(x[0] == 'event' for x in log) and not err: continue
        out.append({'slot': s, 'update': hex(u), 'start': hex(st), 'log': log, 'err': err})
        print(s, hex(u), hex(st), (err or '')[:40], [(x[0], hex(x[1]), x[2], x[3] if x[0] != 'event' else f'{x[4]}@{int(x[3])}') for x in ev][:14])
    json.dump(out, open('comemu.json', 'w'), indent=0)
