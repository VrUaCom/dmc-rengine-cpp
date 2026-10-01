"""PAC archive tool.

  python3 pactool.py list    file.pac
  python3 pactool.py extract file.pac outdir
  python3 pactool.py replace file.pac SLOT newfile -o out.pac
  python3 pactool.py append  file.pac newfile -o out.pac
  python3 pactool.py remove  file.pac SLOT -o out.pac      (last slot only)
  python3 pactool.py pack    indir -o out.pac               (slot_00.* ... from extract)

PAC: 'PAC\\0', u32 count, u32 offset per slot; slots aligned to 16. Retail
PACs rebuild byte-identically. Player PACs are indexed by slot number
(getter 0x1401B82C0): 0 PTX, 1 body MOD, 2-4 motion PACs, 5 motion script,
6/7 collision, 8 body SHW, 9-11 tables, 12 coat MOD, 13 coat CLT, 14 coat
SHW (required), 15 'CCNS' (coat_patch.py only). Slot numbers must not move:
replace in place, append only at the end.
"""
import argparse
import glob
import os
import re
import struct

import dxt
from fixmod import build_pac, pac_slots

def kind(b):
    if len(b) == 0: return 'empty'
    if b[:4] == b'PAC\0': return f'PAC ({struct.unpack_from("<I", b, 4)[0]} slots)'
    if b[:4] == b'PNST': return 'PNST'
    if b[:4] == b'MOD ': return f'MOD ({b[0x10]} objects, {b[0x11]} nodes)'
    if b[:4] == b'SCM ': return 'SCM'
    if b[:4] == b'SHW ': return f'SHW ({b[0x10]} hulls, {b[0x11]} nodes)'
    if b[:4] == b'CCNS': return f'CCNS ({struct.unpack_from("<I", b, 8)[0]} nodes, {struct.unpack_from("<I", b, 12)[0]} capsules)'
    if b[:4] == b'EFM ': return 'EFM'
    if b[:4] == b'TSC ': return 'TSC'
    if b[:1] == b';':
        m = re.search(rb'ClothNum\s+(\d+)', b[:512])
        return f'CLT (ClothNum {int(m.group(1))})' if m else 'text'
    if dxt.is_ptx(b): return f'PTX ({struct.unpack_from("<I", b, 0)[0]} textures)'
    return f'data ({b[:4].hex()})'

EXT = {'PAC': 'pac', 'PNST': 'pnst', 'MOD': 'mod', 'SCM': 'scm', 'SHW': 'shw', 'CCNS': 'ccns',
       'EFM': 'efm', 'TSC': 'tsc', 'CLT': 'clt', 'PTX': 'ptx', 'text': 'txt'}

def ext(b):
    return EXT.get(kind(b).split(' ')[0], 'bin')

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('cmd', choices=['list', 'extract', 'replace', 'append', 'remove', 'pack'])
    ap.add_argument('file')
    ap.add_argument('args', nargs='*')
    ap.add_argument('-o', '--out')
    a = ap.parse_args()
    if a.cmd == 'pack':
        files = sorted(glob.glob(os.path.join(a.file, 'slot_*.*')), key=lambda p: int(re.search(r'slot_(\d+)', p).group(1)))
        slots = [open(p, 'rb').read() for p in files]
        open(a.out, 'wb').write(build_pac(slots)); print('written', a.out, len(slots), 'slots'); return
    slots = pac_slots(open(a.file, 'rb').read())
    if a.cmd == 'list':
        for i, s in enumerate(slots): print(f'{i:3}: {len(s):9} bytes  {kind(s)}')
    elif a.cmd == 'extract':
        out = a.args[0] if a.args else os.path.splitext(a.file)[0]
        os.makedirs(out, exist_ok=True)
        for i, s in enumerate(slots):
            p = os.path.join(out, f'slot_{i:02}.{ext(s)}'); open(p, 'wb').write(s); print(p)
    else:
        slots = list(slots)
        if a.cmd == 'replace':
            i = int(a.args[0]); slots[i] = open(a.args[1], 'rb').read()
            print(f'slot {i}: {kind(slots[i])}')
        elif a.cmd == 'append':
            slots.append(open(a.args[0], 'rb').read()); print(f'slot {len(slots) - 1}: {kind(slots[-1])}')
        elif a.cmd == 'remove':
            i = int(a.args[0])
            if i != len(slots) - 1: raise SystemExit('only the last slot can go: slot numbers must not move')
            slots.pop()
        out = a.out or a.file + '.new'
        open(out, 'wb').write(build_pac(slots)); print('written', out, len(slots), 'slots')

if __name__ == '__main__':
    main()
