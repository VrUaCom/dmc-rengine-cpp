from dbg2 import *
import random, math
e, _ = setup({})
pts = [(random.uniform(-50, 50), random.uniform(-50, 50), random.uniform(-50, 50)) for _ in range(9)]
n = len(pts) - 1
pp = e.alloc(12 * len(pts) + 16, 16); e.write(pp, b''.join(struct.pack('<3f', *p) for p in pts))
clip = e.alloc(32, 16); e.w32(clip, n); e.wf(clip + 4, 1.0); e.w64(clip + 8, pp)
out = e.alloc(32, 16)
def w(u):
    u = abs(u)
    if u < 1: return (3 * u ** 3 - 6 * u ** 2 + 4) / 6
    if u < 2: return (2 - u) ** 3 / 6
    return 0.0
def mine(t):
    t = min(1, max(0, t)); s = t * (n + 2) - 1.0
    r = [0, 0, 0]
    for j in range(-3, n + 4):
        idx = min(max(j, 0), n)
        ww = w(s - j)
        for a in range(3): r[a] += ww * pts[idx][a]
    return r
worst = 0
for t in [0, 0.05, 0.13, 0.5, 0.77, 0.99, 1.0, 1.3, -0.2]:
    e.mu.reg_write(UC_X86_REG_XMM2, int.from_bytes(struct.pack('<f', float(t)) + bytes(12), 'little')); e.call(0x1402d3660, clip, out, 0)
    got = struct.unpack('<3f', e.read(out, 12))
    exp = mine(t)
    worst = max(worst, max(abs(a - b) for a, b in zip(got, exp)))
    print(t, [round(x, 3) for x in got], [round(x, 3) for x in exp])
print('worst', worst)
