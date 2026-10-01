import sys
import json, re
d = json.load(open('gt_poly.json'))
def fl(v): return '{' + ', '.join(('%.9gF' % x) for x in v) + '}'
out = ['// Generated from an emulated run of dmc3.exe CPtclPoly00 on a synthetic record (no game data).']
for key in ('wg1', 'wg0'):
    r = d[key]; n = r['count']
    out.append(f'struct truth_{key} {{')
    out.append('constexpr char kRecordHex[] = "%s";' % r['record'])
    out.append('constexpr float kWorld[16] = %s;' % fl(r['world']))
    out.append(f'constexpr int kCount = {n};')
    def tri(t): return '{' + ', '.join(fl(v[:3]) for v in t) + '}'
    out.append('constexpr float kInitTri[%d][3][3] = {%s};' % (n, ', '.join(tri(t) for t in r['init_tri'])))
    out.append('constexpr float kInitVel[%d][3] = {%s};' % (n, ', '.join(fl(p) for p in r['init_vel'])))
    nf = len(r['frames'])
    out.append(f'constexpr int kFrames = {nf};')
    out.append('struct Frame { float tr[3], rot[3], sc[3]; int col[4]; float life; float tri[%d][3][3]; float vel[%d][3]; struct L { float tr[3], rot[3], sc[3]; int col[4]; } layer[2]; };' % (n, n))
    fr = []
    for f in r['frames']:
        ls = ', '.join('{%s, %s, %s, {%s}}' % (fl(l['tr']), fl(l['rot']), fl(l['sc']), ', '.join(map(str, l['col']))) for l in f['layers'])
        fr.append('{%s, %s, %s, {%s}, %.9gF, {%s}, {%s}, {%s}}' % (fl(f['tr']), fl(f['rot']), fl(f['sc']), ', '.join(map(str, f['col'])), f['life'],
                  ', '.join(tri(t) for t in f['tri']), ', '.join(fl(p) for p in f['vel']), ls))
    out.append('constexpr Frame kFrame[kFrames] = {%s};' % ',\n    '.join(fr))
    out.append('constexpr float kFinal[3][16] = {%s};' % ', '.join(fl(m) for m in r['final']))
    out.append('};')
s = '\n'.join(out) + '\n'
def f(m):
    t = m.group(1)
    if '.' not in t and 'e' not in t: t += '.0'
    return t + 'F'
s = re.sub(r'(-?\d+(?:\.\d+)?(?:e[+-]?\d+)?)F', f, s)
s = s.replace('constexpr ', 'static constexpr ')
open(sys.argv[1] if len(sys.argv) > 1 else 'particle_poly_truth.inc', 'w').write(s)
print(len(s))
