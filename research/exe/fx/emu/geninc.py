import sys
import json, re
d = json.load(open('gtall.json'))
def fl(v): return '{' + ', '.join(('%.9gF' % x) for x in v) + '}'
def il(v): return '{' + ', '.join(str(x) for x in v) + '}'
out = ['// Generated from emulated runs of dmc3.exe CPtclSprt00 / CPtclPoly00 / CPtclLine01 on synthetic',
       '// records (no game data); regenerate with the scratch tool of docs/research/dmc3-particle-sprt00-exe-v75.md.']
for key, r in d.items():
    n = r['n']
    out.append(f'struct truth_{key} {{')
    out.append('constexpr char kRecordHex[] = "%s";' % r['record'])
    out.append('constexpr float kWorld[16] = %s;' % fl(r['world']))
    out.append(f'constexpr int kCount = {n};')
    out.append(f'constexpr int kFrames = {len(r["frames"])};')
    out.append('struct Layer { float tr[3], rot[3], sc[3]; int col[16]; };')
    out.append('struct Frame { float tr[3], rot[3], sc[3]; int col[16]; float life; float pos[%d][3]; float vert[%d][4][3]; float vel[%d][3]; Layer layer[2]; };' % (n, n, n))
    def frame(f):
        pos = f.get('pos', [[0, 0, 0]] * n)
        vert = f.get('vert', [[[0, 0, 0]] * 4] * n)
        ls = ', '.join('{%s, %s, %s, %s}' % (fl(l['tr']), fl(l['rot']), fl(l['sc']), il(l['col'])) for l in f['layers'])
        return '{%s, %s, %s, %s, %.9gF, {%s}, {%s}, {%s}, {%s}}' % (fl(f['tr']), fl(f['rot']), fl(f['sc']), il(f['col']), f['life'],
               ', '.join(fl(p) for p in pos), ', '.join('{' + ', '.join(fl(x) for x in q) + '}' for q in vert), ', '.join(fl(p) for p in f['vel']), ls)
    out.append('constexpr Frame kInit = %s;' % frame(r['init']))
    out.append('constexpr Frame kFrame[kFrames] = {%s};' % ',\n    '.join(frame(f) for f in r['frames']))
    out.append('constexpr float kFinal[3][16] = {%s};' % ', '.join(fl(m) for m in r['final']))
    baked = r.get('baked', [[[0, 0, 0]] * 4] * n)
    out.append('constexpr float kBaked[%d][4][3] = {%s};' % (n, ', '.join('{' + ', '.join(fl(v) for v in p) + '}' for p in baked)))
    out.append('};')
s = '\n'.join(out) + '\n'
def f(m):
    t = m.group(1)
    if '.' not in t and 'e' not in t: t += '.0'
    return t + 'F'
s = re.sub(r'(-?\d+(?:\.\d+)?(?:e[+-]?\d+)?)F', f, s)
s = s.replace('constexpr ', 'static constexpr ')
open(sys.argv[1] if len(sys.argv) > 1 else 'particle_truth.inc', 'w').write(s)
print(len(s))
