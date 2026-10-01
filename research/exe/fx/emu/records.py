import struct, sys
import os
def pnst(s):
    n = struct.unpack_from('<I', s, 4)[0]; offs = list(struct.unpack_from('<%dI' % n, s, 8))
    out = []
    for i, o in enumerate(offs):
        e = next((x for x in offs[i + 1:] if x), len(s)) if o else 0
        out.append(s[o:e] if o else b'')
    return out
def bank_from_bytes(s):
    outer = pnst(s); txt = outer[0].split(b'\0')[0].decode('latin1'); inner = pnst(outer[1])
    toks = txt.split(); recs = {}; k = 0; i = 0
    while i + 1 < len(toks):
        if toks[i].startswith('#'): break
        kind, id_ = toks[i], int(toks[i + 1]); recs[(kind, id_)] = inner[k] if k < len(inner) else b''
        k += 2 if kind == 'M' else 1; i += 2
    return recs
def pac_slots(b):
    assert b[:4] == b'PAC\0'
    n = struct.unpack_from('<I', b, 4)[0]; offs = [struct.unpack_from('<I', b, 8 + 4 * i)[0] for i in range(n)]
    ends = sorted(set(offs + [len(b)]))
    return [b[o:min(x for x in ends if x > o)] for o in offs]
def em034_bank():
    """FXBANK of em034.pac (slot 28); path from $DMC3_EM034_PAC."""
    b = open(os.environ['DMC3_EM034_PAC'], 'rb').read()
    return bank_from_bytes(pac_slots(b)[28])
def pac_bank(path, slot):
    return bank_from_bytes(pac_slots(open(path, 'rb').read())[slot])
