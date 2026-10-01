import struct, math
from emu import *
def parse_imports():
    d = img.data
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    opt = pe + 24
    dd = opt + 112  # PE32+ data dirs
    rva, size = struct.unpack_from('<II', d, dd + 8)
    def off(r):
        for name, va, vs, raw, rs in img.sections:
            if va <= r < va + max(vs, rs): return raw + r - va
    res = {}
    p = off(rva)
    while True:
        oft, ts, fc, nr, ft = struct.unpack_from('<IIIII', d, p)
        if nr == 0: break
        dll = d[off(nr):].split(b'\0')[0].decode()
        t = off(oft or ft); slot = ft; 
        while True:
            v = struct.unpack_from('<Q', d, t)[0]
            if v == 0: break
            if v >> 63: nm = f'ord{v & 0xffff}'
            else: nm = d[off(v & 0x7fffffff) + 2:].split(b'\0')[0].decode()
            res[img.base + slot] = (dll, nm)
            t += 8; slot += 8
        p += 20
    return res
IMPORTS = parse_imports()
def thunks(e):
    """map thunk address (jmp [rip+x]) -> import name"""
    out = {}
    text = next(s for s in img.sections if s[0].startswith('.text'))
    name, va, vs, raw, rs = text
    for o in range(0, rs - 6):
        if img.data[raw + o] == 0xff and img.data[raw + o + 1] == 0x25:
            disp = struct.unpack_from('<i', img.data, raw + o + 2)[0]
            slotva = img.base + va + o + 6 + disp
            if slotva in IMPORTS: out[img.base + va + o] = IMPORTS[slotva]
    return out
if __name__ == '__main__':
    e = Emu(); t = thunks(e)
    print(len(t)); 
    for a in (0x140346c20,0x140346c26,0x140346c2c,0x140346c32,0x140346c38): print(hex(a), t.get(a))
    names = sorted({n for d,n in t.values()})
    print(names)

def _f(e, i): return struct.unpack('<f', bytes(e.mu.reg_read(UC_X86_REG_XMM0 + i, ) .to_bytes(16, 'little')[:4]) if isinstance(e.mu.reg_read(UC_X86_REG_XMM0 + i), int) else e.mu.reg_read(UC_X86_REG_XMM0 + i)[:4])[0]
def _xmm(e, i):
    v = e.mu.reg_read(UC_X86_REG_XMM0 + i)
    return struct.unpack('<f', (v.to_bytes(16, 'little') if isinstance(v, int) else bytes(v))[:4])[0]
def _setx(e, v): e.mu.reg_write(UC_X86_REG_XMM0, int.from_bytes(struct.pack('<f', v) + b'\0' * 12, 'little'))
def _rax(e, v): e.mu.reg_write(UC_X86_REG_RAX, v & 0xffffffffffffffff)
def _reg(e, r): return e.mu.reg_read(r)
def install_crt(e):
    m = {
        'sinf': lambda e: _setx(e, math.sin(_xmm(e, 0))),
        'cosf': lambda e: _setx(e, math.cos(_xmm(e, 0))),
        'tanf': lambda e: _setx(e, math.tan(_xmm(e, 0))),
        'acosf': lambda e: _setx(e, math.acos(max(-1, min(1, _xmm(e, 0))))),
        'atan2f': lambda e: _setx(e, math.atan2(_xmm(e, 0), _xmm(e, 1))),
        'sqrtf': lambda e: _setx(e, math.sqrt(max(0.0, _xmm(e, 0)))),
        'fmodf': lambda e: _setx(e, math.fmod(_xmm(e, 0), _xmm(e, 1)) if _xmm(e, 1) else 0.0),
        'powf': lambda e: _setx(e, math.pow(_xmm(e, 0), _xmm(e, 1))),
        'strcmp': lambda e: _rax(e, 1),
        'memcpy': lambda e: (e.write(_reg(e, UC_X86_REG_RCX), e.read(_reg(e, UC_X86_REG_RDX), _reg(e, UC_X86_REG_R8))), _rax(e, _reg(e, UC_X86_REG_RCX))),
        'memmove': lambda e: (e.write(_reg(e, UC_X86_REG_RCX), e.read(_reg(e, UC_X86_REG_RDX), _reg(e, UC_X86_REG_R8))), _rax(e, _reg(e, UC_X86_REG_RCX))),
        'memset': lambda e: (e.write(_reg(e, UC_X86_REG_RCX), bytes([_reg(e, UC_X86_REG_RDX) & 255]) * _reg(e, UC_X86_REG_R8)), _rax(e, _reg(e, UC_X86_REG_RCX))),
    }
    for a, (dll, nm) in thunks(e).items():
        if nm in m: e.stubs[a] = m[nm]
