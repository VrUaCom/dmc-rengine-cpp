"""Frame count of every action of an enemy motion script (mode 1), from a local PAC.

python3 action_lengths.py em000.pac 38 35 > lengths.json
SCRIPT_SLOT: the class's motion script; MOT_SLOT: its motion PAC. An action's
length is the frame count (float at MOT +0x0C) of the MOT its first opcode-1
plays for object 0, and whether that resource loops.
"""
import json, struct, sys
from records import pac_slots

pac = open(sys.argv[1], 'rb').read()
slots = pac_slots(pac)
s = slots[int(sys.argv[2])]
mots = pac_slots(slots[int(sys.argv[3])])
u16 = lambda o: struct.unpack_from('<H', s, o)[0]
A, B = u16(0), u16(2)
def table(base):
    out = []; o = base
    while u16(o) != 0xFFFF: out.append(base + u16(o)); o += 2
    return out
LEN = {1: 8, 3: 6, 4: 6, 5: 2, 6: 2, 7: 2, 8: 2, 32: 4, 33: 2, 34: 6, 35: 2, 36: 4}
res = {}
for bank, sub in enumerate(table(A)):
    n = 0; o = sub
    while u16(o) != 0xFFFF: n += 1; o += 2
    for action in range(n):
        p = sub + u16(sub + 2 * action)
        for _ in range(64):
            op = s[p]
            if op == 1: break
            if op == 0 or op == 2: p = None; break
            p += (op - 15) * 2 + 4 if 16 <= op <= 31 else LEN.get(op, 2)
        if p is None or s[p] != 1: continue
        rb, ra = s[p + 4], s[p + 5]
        bsub = B + u16(B + 2 * rb); r = bsub + u16(bsub + 2 * ra)
        cnt = s[r]
        recs = [struct.unpack_from('<BBBBH', s, r + 2 + 6 * i) for i in range(cnt)]
        body = [x for x in recs if x[0] == 0]
        if not body: continue
        mot = mots[body[0][4] % 100] if body[0][4] % 100 < len(mots) else b''
        if len(mot) < 16: continue
        res[f'{bank}/{action}'] = {'mot': body[0][4], 'frames': struct.unpack_from('<f', mot, 12)[0], 'loop': body[0][1]}
json.dump(res, sys.stdout, indent=0)
