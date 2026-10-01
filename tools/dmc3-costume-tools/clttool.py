"""CLT cloth parameters (player slot 13, enemy cloth slots).

  python3 clttool.py show file.pac|file.clt [--slot 13]
  python3 clttool.py set  file.pac KEY VALUE... -o out.pac [--slot 13] [--block N]
        e.g. set pl011.pac Stiffness 0.4 -o out.pac
             set pl011.pac Gravity 0 -0.02 0 -o out.pac

Parsed by 0x1402CA1D0 per ClothNo block (keys: Gravity, SpringForce,
MaxSpeed, Stiffness, Wind, WindLocal, WindParent, WindType, FloorLevel,
LimitLength, Bone N AXIS with AXIS X/Y/Z/NX/NY/NZ = the node axis that points
at its parent). Only block 0 collides with the coat capsules. CPlDante holds
one block (ClothNum 1), CPlVergil two.
"""
import argparse
import re

from fixmod import build_pac, pac_slots

def load(path, slot):
    b = open(path, 'rb').read()
    if b[:4] == b'PAC\0':
        slots = pac_slots(b)
        return slots[slot], slots
    return b, None

def blocks(text):
    out, cur = [], None
    for line in text.splitlines():
        w = line.split()
        if not w: continue
        if w[0] == 'ClothNo': cur = {'ClothNo': w[1], 'bones': []}; out.append(cur)
        elif cur is None: continue
        elif w[0] == 'Bone': cur['bones'].append((int(w[1]), w[2] if len(w) > 2 else '?'))
        elif w[0] == 'End': cur = None
        else: cur[w[0]] = ' '.join(w[1:])
    return out

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('cmd', choices=['show', 'set'])
    ap.add_argument('file')
    ap.add_argument('args', nargs='*')
    ap.add_argument('--slot', type=int, default=13)
    ap.add_argument('--block', type=int)
    ap.add_argument('-o', '--out')
    a = ap.parse_args()
    raw, slots = load(a.file, a.slot)
    text = raw.rstrip(b'\0').decode('ascii', errors='replace')
    if a.cmd == 'show':
        head = re.search(r'ClothNum\s+(\d+)', text)
        print(text.splitlines()[0], '| ClothNum', head.group(1) if head else '?')
        for b in blocks(text):
            bones = b.pop('bones')
            print(f"block {b.pop('ClothNo')}: " + ', '.join(f'{k} {v}' for k, v in b.items()))
            print(f'  {len(bones)} bones: ' + ' '.join(f'{n}{ax}' for n, ax in bones))
        return
    key, values = a.args[0], a.args[1:]
    lines, block, changed = text.split('\r\n'), -1, 0
    for i, line in enumerate(lines):
        w = line.split()
        if w and w[0] == 'ClothNo': block += 1
        if w and w[0] == key and (a.block is None or a.block == block):
            vals = '  '.join(f'{float(v):.6f}' if '.' in v or 'e' in v.lower() else v for v in values)
            lines[i] = f'{key:<11} {vals}'; changed += 1
    if not changed: raise SystemExit(f'{key} not found')
    new = '\r\n'.join(lines).encode('ascii')
    new += bytes((-len(new)) % 16 + 16)
    print(f'{key} set in {changed} block(s)')
    out = a.out or a.file + '.new'
    if slots is None: open(out, 'wb').write(new)
    else:
        slots = list(slots); slots[a.slot] = new; open(out, 'wb').write(build_pac(slots))
    print('written', out)

if __name__ == '__main__':
    main()
