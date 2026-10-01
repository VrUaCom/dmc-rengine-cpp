from gen import *
import os, math, sys
GB = os.environ.get('FX_RECORDS', 'records') + '/'
OUT = os.environ.get('FX_OUT', 'greal') + '/'
os.makedirs(OUT, exist_ok=True)
class Lcg:
    def __init__(self, s): self.s = s
    def next(self):
        self.s = (self.s * 1664525 + 1013904223) & 0xffffffff
        return self.s
def dump(fname, ticks=90, seed=12345):
    g = open(GB + fname, 'rb').read()
    c, s = math.cos(0.5), math.sin(0.5)
    W = (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 40.0, 7.0, -25.0, 1)
    sy = GSys(g, W=W, limit=2000000)
    clip = None
    if g[1] == 1:
        cfile = GB + fname.rsplit('_G', 1)[0] + '_C' + str(struct.unpack_from('<H', g, 2)[0]) + '.bin'
        clip = open(cfile, 'rb').read()
        e = sy.e
        pool = 0x140caa190
        for i in range(10): e.write(pool + 2 * i, b'\xff\xff')
        e.write(pool, struct.pack('<H', struct.unpack_from('<H', g, 2)[0]))
        blk = e.alloc(len(clip) + 16, 16); e.write(blk, clip)
        e.w32(pool + 0xb8, 1)
        e.w32(pool + 0x18, struct.unpack_from('<I', clip, 0)[0] - 1)
        e.write(pool + 0x1c, clip[4:8])
        e.w64(pool + 0x20, blk + 8)
    lcg = Lcg(seed)
    def rng(em):
        em.mu.reg_write(UC_X86_REG_RAX, lcg.next())
    sy.e.stubs[0x140059390] = rng
    for t in range(ticks):
        try: sy.step()
        except Exception as ex: return 'ERR ' + str(ex)[:100]
    lines = [f'{len(sy.events)} {ticks} {seed}', g.hex(), ' '.join('%.9g' % x for x in W), clip.hex() if clip else '-']
    for ev in sy.events:
        k = {'P': 0, 'E': 1, 'G?': 2, 'V': 3}[ev['kind']]
        lines.append('%d %d %d %d %s' % (ev['tick'], k, ev['id'], 1 if ev['pm'] else 0, ' '.join('%.9g' % x for x in ev['m'])))
    open(OUT + fname.replace('.bin', '.txt'), 'w').write('\n'.join(lines) + '\n')
    return 'ok events %d' % len(sy.events)
if __name__ == '__main__':
    names = sorted(x for x in os.listdir(GB) if '_G' in x)
    for f in names:
        print(f, dump(f), flush=True)
