from dbg2 import *
import sys, struct
SPAWN = {0x1403127b0: 'P', 0x1402e3b90: 'E', 0x1403244e0: 'V', 0x1402eba90: 'G?'}
class GSys:
    def __init__(self, grec, W=None, limit=400000):
        self.e, _ = setup({})
        e = self.e; e.limit = limit
        self.events = []; self.tick = 0
        for a, nm in SPAWN.items():
            e.stubs[a] = (lambda nm: (lambda em: self._spawn(nm)))(nm)
        self.grec = e.alloc(len(grec) + 64, 64); e.write(self.grec, grec)
        M = e.alloc(64, 16)
        e.write(M, struct.pack('<16f', *(W or (1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1))))
        obj = e.alloc(0x400, 64)
        e.call(0x140232320, obj)
        e.w64(obj, 0x1405073f8)
        e.call(0x1402e8170, obj, M, 0, 0)
        e.call(0x1402e81b0, obj, 0, 0)
        e.wf(obj + 0x70, 0.0)
        e.w64(obj + 0x168, self.grec)
        self.obj = obj
    def _spawn(self, nm):
        e = self.e; mu = e.mu
        rcx = mu.reg_read(UC_X86_REG_RCX); r8 = mu.reg_read(UC_X86_REG_R8) & 0xffff; r9 = mu.reg_read(UC_X86_REG_R9)
        rsp = mu.reg_read(UC_X86_REG_RSP)
        m = list(struct.unpack('<16f', e.read(rcx, 64)))
        pm = list(struct.unpack('<16f', e.read(r9, 64))) if r9 else None
        self.events.append(dict(tick=self.tick, kind=nm, id=r8, m=m, pm=pm, flag=e.u32(rsp + 0x28 + 0), delay=e.f32(rsp + 0x30), b=e.read(rsp + 0x38, 1)[0]))
        blk = e.alloc(0x400, 64)
        mu.reg_write(UC_X86_REG_RAX, blk)
    def step(self):
        self.tick += 1
        self.e.call(0x1402ec840, self.obj)
if __name__ == '__main__':
    g = open(sys.argv[1], 'rb').read()
    s = GSys(g)
    e = s.e
    for t in range(int(sys.argv[2])):
        s.step()
    print('events', len(s.events))
    for ev in s.events[:12]: print(ev['tick'], ev['kind'], ev['id'], [round(x, 2) for x in ev['m'][12:15]], 'scale', [round(ev['m'][i], 3) for i in (0, 5, 10)], 'pm', ev['pm'] is not None, ev['flag'], ev['delay'], ev['b'])
