import sys
from gen_gt import *
import json, random
def mk(b0, mode, clipid, speed, decel, yaw, roll, d0, interval, life, kind, cid, ang, angr, endless, mask, s0, s1, steps, jit, lo, hi, aj):
    r = bytearray(96)
    r[0] = b0; r[1] = mode; struct.pack_into('<H', r, 2, clipid)
    struct.pack_into('<f', r, 4, speed); struct.pack_into('<f', r, 8, decel)
    struct.pack_into('<f', r, 0x10, yaw); struct.pack_into('<f', r, 0x14, roll)
    struct.pack_into('<iii', r, 0x18, d0, interval, life)
    r[0x24] = kind; struct.pack_into('<H', r, 0x26, cid)
    struct.pack_into('<ff', r, 0x28, ang, angr)
    r[0x30] = endless; struct.pack_into('<I', r, 0x34, mask)
    struct.pack_into('<ff', r, 0x38, s0, s1); struct.pack_into('<H', r, 0x40, steps)
    struct.pack_into('<3f', r, 0x44, *jit); struct.pack_into('<ff', r, 0x50, lo, hi)
    r[0x59] = aj
    return bytes(r)
SC = {
 'g1': mk(0, 0, 0, 6.5, 0.2, 30.0, -20.0, 3, 4, 60, 3, 77, 15.0, 40.0, 0, 3, 2.0, 0.5, 10, (20.0, 0.0, 10.0), 0.0, 3.0, 30),
 'g2': mk(1, 2, 0, 0, 0, 0, 0, 0, 5, 40, 0, 5, -170.0, 0.0, 1, 7, 1.0, 1.0, 0, (0, 0, 0), 0.2, 2.5, 90),
 'g3': mk(0, 1, 9, 0.02, 0, 0, 0, 2, 3, 50, 1, 12, 0.0, 10.0, 0, 1, 1.0, 1.0, 0, (4, 4, 4), 0, 0, 0),
}
CLIP = struct.pack('<If', 6, 1.0) + b''.join(struct.pack('<3f', *p) for p in [(0,0,0),(10,5,0),(20,-5,10),(30,0,20),(25,15,25),(5,20,30)])
def run(name, g, ticks=100, seed=777):
    c, s = math.cos(0.6), math.sin(0.6)
    W = (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 15.0, -3.0, 22.0, 1)
    sy = GSys(g, W=W, limit=2000000)
    e = sy.e
    if g[1] == 1:
        pool = 0x140caa190
        for i in range(10): e.write(pool + 2 * i, b'\xff\xff')
        e.write(pool, struct.pack('<H', struct.unpack_from('<H', g, 2)[0]))
        blk = e.alloc(len(CLIP) + 16, 16); e.write(blk, CLIP)
        e.w32(pool + 0xb8, 1); e.w32(pool + 0x18, 5); e.write(pool + 0x1c, CLIP[4:8]); e.w64(pool + 0x20, blk + 8)
    lcg = Lcg(seed)
    sy.e.stubs[0x140059390] = lambda em: em.mu.reg_write(UC_X86_REG_RAX, lcg.next())
    for t in range(ticks): sy.step()
    evs = [dict(tick=ev['tick'], kind={'P':0,'E':1,'G?':2,'V':3}[ev['kind']], id=ev['id'], follow=1 if ev['pm'] else 0, m=ev['m']) for ev in sy.events]
    return dict(record=g.hex(), world=list(W), ticks=ticks, seed=seed, events=evs, clip=CLIP.hex() if g[1] == 1 else '')
res = {k: run(k, v) for k, v in SC.items()}
for k, v in res.items(): print(k, len(v['events']))
# spline samples
e, _ = setup({})
pts = [struct.unpack_from('<3f', CLIP, 8 + 12 * i) for i in range(6)]
pp = e.alloc(100, 16); e.write(pp, CLIP[8:]); clip = e.alloc(32, 16); e.w32(clip, 5); e.w64(clip + 8, pp); out = e.alloc(32, 16)
samples = []
for t in (0.0, 0.1, 0.33, 0.5, 0.71, 0.97, 1.0):
    e.mu.reg_write(UC_X86_REG_XMM2, int.from_bytes(struct.pack('<f', t) + bytes(12), 'little')); e.call(0x1402d3660, clip, out, 0)
    samples.append((t, list(struct.unpack('<3f', e.read(out, 12)))))
res['spline'] = samples
json.dump(res, open('gsynth.json', 'w'))
def fl(v): return '{' + ', '.join('%.9gF' % x for x in v) + '}'
import re
o = ['// Generated from emulated runs of dmc3.exe CGenerator on synthetic records (no game data).']
for k in ('g1', 'g2', 'g3'):
    r = res[k]
    o.append(f'struct truth_{k} {{')
    o.append('constexpr char kRecordHex[] = "%s";' % r['record'])
    o.append('constexpr char kClipHex[] = "%s";' % r['clip'])
    o.append('constexpr float kWorld[16] = %s;' % fl(r['world']))
    o.append(f'constexpr int kTicks = {r["ticks"]}; constexpr unsigned kSeed = {r["seed"]}U; constexpr int kEvents = {len(r["events"])};')
    o.append('struct Event { int tick, kind, id, follow; float m[16]; };')
    o.append('constexpr Event kEvent[kEvents] = {%s};' % ',\n    '.join('{%d, %d, %d, %d, %s}' % (e['tick'], e['kind'], e['id'], e['follow'], fl(e['m'])) for e in r['events']))
    o.append('};')
o.append('struct truth_spline { constexpr float kPoint[7][4] = {%s}; };' % ', '.join(fl([t] + v) for t, v in res['spline']))
s = '\n'.join(o) + '\n'
def f(m):
    t = m.group(1)
    if '.' not in t and 'e' not in t: t += '.0'
    return t + 'F'
s = re.sub(r'(-?\d+(?:\.\d+)?(?:e[+-]?\d+)?)F', f, s).replace('constexpr ', 'static constexpr ')
open(sys.argv[1] if len(sys.argv) > 1 else 'generator_truth.inc', 'w').write(s)
print(len(s))
