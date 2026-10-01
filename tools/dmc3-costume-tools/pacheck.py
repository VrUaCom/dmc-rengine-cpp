"""Check a Dante player PAC against the limits the canonical dmc3.exe sets.

  python3 pacheck.py costume.pac [--vergil]

  * slot count: slot 14 (coat SHW) must exist; the player init passes it to
    the shadow object without a null check (0x140215206 -> 0x14008BC60);
  * MOD skin (slots 1 and 12): 5-bit weights sum to 31, active blend lanes
    are bone*4, bones < node count; topology: triangle lists or strips;
  * coat (slot 12): at most 39 nodes (0x1401DE820 allocates 39 coat joints);
  * CLT (slot 13): ClothNum 1 for Dante (chain +0xA210, next joint table at
    +0xA300), 2 for Vergil (+0xA230 .. +0xA410); bones < coat node count;
  * SHW (slots 8 and 14): hull blocks inside the slot, T = 2V - 4, every
    adjacency entry a valid triangle, joints < the model's node count;
  * slot 15 'CCNS': version 1, <= 16 node records (coat node < 39, body
    joint < 96), <= 6 capsule records (index < 6), sizes consistent.
"""
import re
import struct
import sys

from fixmod import pac_slots
from modwriter import read_mod

def check_mod(name, b, errors):
    m = read_mod(b)
    nodes = len(m.transforms)
    verts = 0
    for oi, o in enumerate(m.objects):
        for k, me in enumerate(o.meshes):
            for i in range(me.count):
                c = struct.unpack_from('<H', me.ctl, 2 * i)[0] & 0x7fff
                w = (c & 31, (c >> 5) & 31, (c >> 10) & 31)
                lanes = me.blend[4 * i + 1:4 * i + 4]
                if sum(w) != 31:
                    errors.append(f'{name} obj {oi} mesh {k} vertex {i}: weights {w} sum {sum(w)}')
                    continue
                for q, lane in zip(w, lanes):
                    if q and (lane % 4 or lane // 4 >= nodes):
                        errors.append(f'{name} obj {oi} mesh {k} vertex {i}: lane {lane} (nodes {nodes})')
            verts += me.count
    print(f'{name}: {len(m.objects)} objects, {verts} vertices, {nodes} nodes, header +0x13 = {m.header[0x13]}')
    return m

def check_shw(name, b, nodes, errors):
    if b[:4] != b'SHW ':
        errors.append(f'{name}: not an SHW'); return
    n = b[0x10]
    if b[0x11] != nodes: errors.append(f'{name}: header node count {b[0x11]}, model has {nodes}')
    for h in range(n):
        r = 0x20 + h * 0x40
        vc, tc = struct.unpack_from('<HH', b, r)
        to, ao, vo, so = struct.unpack_from('<QQQQ', b, r + 0x10)
        if max(to + 16 * tc, ao + 8 * tc, vo + 16 * vc, so + vc) > len(b):
            errors.append(f'{name} hull {h}: block past the slot end'); continue
        if tc != 2 * vc - 4: errors.append(f'{name} hull {h}: T {tc} != 2V-4 ({vc})')
        for t in range(tc):
            if any(i >= vc for i in struct.unpack_from('<3I', b, to + 16 * t)):
                errors.append(f'{name} hull {h}: triangle {t} vertex out of range'); break
            if any(a >= tc for a in struct.unpack_from('<3H', b, ao + 8 * t)):
                errors.append(f'{name} hull {h}: adjacency {t} out of range'); break
        if any(j >= nodes for j in b[so:so + vc]): errors.append(f'{name} hull {h}: joint >= {nodes}')
    print(f'{name}: {n} hulls')

def main():
    b = open(sys.argv[1], 'rb').read()
    vergil = '--vergil' in sys.argv
    slots = pac_slots(b)
    errors = []
    print(f'{len(slots)} slots')
    if len(slots) < 15:
        errors.append('slot 14 (coat SHW) is missing: the player init dereferences it')
    body = check_mod('slot 1 (body)', slots[1], errors)
    coat = check_mod('slot 12 (coat)', slots[12], errors)
    coat_nodes = len(coat.transforms)
    if coat_nodes > 39:
        errors.append(f'coat has {coat_nodes} nodes; the player allocates 39 coat joints')
    check_shw('slot 8 (body SHW)', slots[8], len(body.transforms), errors)
    if len(slots) > 14: check_shw('slot 14 (coat SHW)', slots[14], coat_nodes, errors)
    text = slots[13].rstrip(b'\0').decode('ascii', errors='replace')
    cloth_num = re.search(r'ClothNum\s+(\d+)', text)
    cloth_num = int(cloth_num.group(1)) if cloth_num else 0
    capacity = 2 if vergil else 1
    bones = [int(x) for x in re.findall(r'^Bone\s+(\d+)', text, re.M)]
    print(f'slot 13 (CLT): ClothNum {cloth_num}, {len(bones)} bones')
    if cloth_num > capacity:
        errors.append(f'ClothNum {cloth_num}: {"CPlVergil" if vergil else "CPlDante"} holds {capacity} chain(s); '
                      'the next block overwrites the joint table after the chains')
    for bone in bones:
        if bone >= coat_nodes:
            errors.append(f'CLT bone {bone} >= coat nodes {coat_nodes}')
    if len(slots) > 15:
        s = slots[15]
        if s[:4] != b'CCNS':
            print('slot 15: not CCNS (the patch ignores it)')
        else:
            version, nodes, caps = struct.unpack_from('<III', s, 4)
            print(f'slot 15 (CCNS): version {version}, {nodes} node records, {caps} capsule records')
            if version != 1: errors.append('CCNS version must be 1')
            if nodes > 16: errors.append('CCNS: more than 16 node records (the patch then ignores the slot)')
            if caps > 6: errors.append('CCNS: more than 6 capsule records')
            if len(s) < 0x10 + 0x50 * nodes + 0x40 * caps: errors.append('CCNS: records past the slot end')
            for k in range(min(nodes, 16)):
                node, joint = struct.unpack_from('<II', s, 0x10 + 0x50 * k)
                if node >= 39 or node >= coat_nodes: errors.append(f'CCNS record {k}: coat node {node}')
                if joint >= 96 or joint >= len(body.transforms): errors.append(f'CCNS record {k}: body joint {joint}')
                if node in bones: errors.append(f'CCNS record {k}: node {node} is also a cloth bone')
            o = 0x10 + 0x50 * nodes
            for k in range(min(caps, 6)):
                if struct.unpack_from('<I', s, o + 0x40 * k)[0] >= 6:
                    errors.append(f'CCNS capsule record {k}: index >= 6')
    for e in errors: print('ERROR', e)
    print('OK' if not errors else f'{len(errors)} error(s)')
    return 1 if errors else 0

if __name__ == '__main__':
    sys.exit(main())
