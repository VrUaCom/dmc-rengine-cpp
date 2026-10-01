import os, sys, struct, math
from ptcl2 import *
PB = os.environ.get('FX_RECORDS', 'records') + '/'
OUT = os.environ.get('FX_OUT', 'preal') + '/'
os.makedirs(OUT, exist_ok=True)
NF = 12
def dump(fname):
    rec = open(PB + fname, 'rb').read()
    c, s = math.cos(0.4), math.sin(0.4)
    W = (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 30.0, 5.0, -20.0, 1)
    try:
        prefix = fname.rsplit('_P', 1)[0]
        recs = {}
        for g in os.listdir(PB):
            if g.startswith(prefix + '_') and '_A' in g and g.endswith('.bin') and g.rsplit('_', 1)[0] == prefix + '_slot' + fname.split('_slot')[1].split('_')[0]:
                recs[('A', int(g.rsplit('_A', 1)[1][:-4]))] = open(PB + g, 'rb').read()
        doff = struct.unpack_from('<i', rec, 0x10)[0] + 0x10
        aid = struct.unpack_from('<H', rec, doff + 0x106)[0]
        if ('A', aid) not in recs:
            dummy = bytearray(336); dummy[0] = 1; dummy[2] = 0; dummy[3] = 0
            struct.pack_into('<4H', dummy, 6, 0, 0, 16, 16)
            recs[('A', aid)] = bytes(dummy)
        sy = Sys(rec, W=W, limit=400000, recs=recs)
    except Exception as ex:
        return 'ERR init ' + str(ex)[:80]
    e = sy.e
    cls = sy.cls
    n = e.read(sy.obj + 0xf4, 1)[0]
    if cls == 4: n //= 2
    per = {3: 4, 1: 3, 4: 4}.get(cls)
    if per is None: return 'skip class %d' % cls
    nl = len(sy.layers)
    def snap():
        o = sy.obj
        v = [sy.tick] if False else []
        toks = []
        toks += sy.rd(o + 0x120, 3) + sy.rd(o + 0x130, 3) + sy.rd(o + 0x140, 3)
        toks += list(e.read(o + 0x4a8, 16)) + [e.f32(o + 0x4d0)]
        for i in range(n): toks += sy.rd(o + 0xb80 + 12 * i, 3)
        if cls == 3:
            for i in range(n): toks += sy.rd(o + 0xb80 + 12 * (n + i), 3)
        else:
            base = o + 0x540 + (sy.tick & 1) * 0x330
            for i in range(n):
                for k in range(4):
                    toks += sy.rd(base + 16 * (per * i + k), 3) if k < per else [0, 0, 0]
        for f in sy.layers:
            toks += sy.rd(f + 0x120, 3) + sy.rd(f + 0x130, 3) + sy.rd(f + 0x140, 3) + list(e.read(f + 0x4a8, 16))
        return toks
    lines = [f'{cls} {n} {nl} {NF}', rec.hex(), ' '.join('%.9g' % x for x in W)]
    lines.append(' '.join('%.9g' % x for x in snap()))
    try:
        for t in range(NF):
            sy.update()
            lines.append(' '.join('%.9g' % x for x in snap()))
    except Exception as ex:
        return 'ERR update ' + str(ex)[:80]
    open(OUT + fname.replace('.bin', '.txt'), 'w').write('\n'.join(lines) + '\n')
    return 'ok cls %d n %d layers %d' % (cls, n, nl)
if __name__ == '__main__':
    names = sorted(os.listdir(PB))
    lo, hi = int(sys.argv[1]), int(sys.argv[2])
    names = [x for x in names if '_P' in x]
    for f in names[lo:hi]:
        print(f, dump(f), flush=True)
