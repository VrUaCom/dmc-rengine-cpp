"""Check a patched dmc3.exe (coat_patch.py) and a costume PAC by emulating the
game's own code with unicorn, on the real image.

  python3 verify_patch.py dmc3_coat.exe costume.pac [more.pac ...]

For each PAC it builds a fake player with 96 body joints and 39 coat joints,
registers the PAC in the loaded-PAC table the slot getter 0x1401B82C0 reads,
and calls the hook at 0x140DAC000 as CPlDante's coat load would (r14 =
player). It then checks:
  * slot 15 node records -> a mode-1 constraint in coat joint +0x100
    (vtable 0x1404CC1F8, enabled, host = body joint +0x110, offset copied);
  * the game's constraint evaluation 0x1402CBBE0 yields offset x host;
  * slot 15 capsule records -> player +0xB630 + index*0x50 + 0x10;
  * rdx = player +0x1880 on return (the instruction the hook replaced);
  * a PAC without slot 15 changes nothing.
"""
import struct
import sys

import numpy as np
from unicorn import Uc, UC_ARCH_X86, UC_MODE_64
from unicorn.x86_const import UC_X86_REG_R14, UC_X86_REG_RCX, UC_X86_REG_RDX, UC_X86_REG_R8, UC_X86_REG_RSP

from fixmod import pac_slots

BASE = 0x140000000
HOOK = 0x140DAC000
EVAL = 0x1402CBBE0
CVTABLE = 0x1404CC1F8
PACTABLE = 0x140C99D30
GETTER_BASES = 0x140581A20

def run(exe, pac_bytes, name):
    mu = Uc(UC_ARCH_X86, UC_MODE_64)
    pe = struct.unpack_from('<I', exe, 0x3C)[0]
    nsec = struct.unpack_from('<H', exe, pe + 6)[0]
    opt_size = struct.unpack_from('<H', exe, pe + 20)[0]
    image = struct.unpack_from('<I', exe, pe + 24 + 56)[0]
    mu.mem_map(BASE, image)
    mu.mem_write(BASE, exe[:0x400])
    names = []
    for i in range(nsec):
        o = pe + 24 + opt_size + 40 * i
        names.append(exe[o:o + 8].rstrip(b'\0'))
        vs, va, rs, raw = struct.unpack_from('<IIII', exe, o + 8)
        mu.mem_write(BASE + va, exe[raw:raw + min(rs, vs or rs)])
    if b'.dmcx' not in names:
        raise SystemExit('this dmc3.exe has no .dmcx section: run coat_patch.py first')
    HEAP, STACK, STOP = 0x200000000, 0x300000000, 0x400000000
    mu.mem_map(HEAP, 0x400000); mu.mem_map(STACK, 0x100000); mu.mem_map(STOP, 0x1000)
    mu.mem_write(STOP, b'\xf4')
    w64 = lambda a, v: mu.mem_write(a, struct.pack('<Q', v))
    r64 = lambda a: struct.unpack('<Q', mu.mem_read(a, 8))[0]
    heap = [HEAP + 0x20000]
    def new(n):
        a = (heap[0] + 15) & ~15; heap[0] = a + n; return a

    player = HEAP
    mu.mem_write(player + 0x78, struct.pack('<H', 0))
    body_world = {}
    for j in range(96):
        joint = new(0x260); w64(player + 0x1880 + 8 * j, joint)
        world = new(64); w64(joint + 0x110, world)
        m = np.eye(4, dtype=np.float32); m[3, :3] = (j, 100 + j, -j * 0.5)
        mu.mem_write(world, m.tobytes()); body_world[j] = m
    coat = []
    for n in range(39):
        joint = new(0x260); w64(player + 0xA0D0 + 8 * n, joint)
        w64(joint + 0x110, new(64)); coat.append(joint)
    shapes_before = bytes(0x50 * 6)
    mu.mem_write(player + 0xB630, shapes_before)
    pac = new(len(pac_bytes)); mu.mem_write(pac, pac_bytes)
    entry = PACTABLE + struct.unpack('<H', mu.mem_read(GETTER_BASES, 2))[0] * 0x48
    mu.mem_write(entry + 4, struct.pack('<I', 3))
    w64(entry + 0x18, new(16)); w64(entry + 0x20, pac)

    def call(addr, regs):
        rsp = STACK + 0x80000 - 8
        w64(rsp, STOP)
        mu.reg_write(UC_X86_REG_RSP, rsp)
        for r, v in regs.items(): mu.reg_write(r, v)
        mu.emu_start(addr, STOP)

    call(HOOK, {UC_X86_REG_R14: player, UC_X86_REG_RDX: 0})
    assert mu.reg_read(UC_X86_REG_RDX) == player + 0x1880, 'rdx not restored'

    slots = pac_slots(pac_bytes)
    ccns = slots[15] if len(slots) > 15 and slots[15][:4] == b'CCNS' else None
    if ccns is None:
        assert all(r64(j + 0x100) == 0 for j in coat), 'constraint installed without slot 15'
        assert bytes(mu.mem_read(player + 0xB630, 0x50 * 6)) == shapes_before
        print(f'{name}: no slot 15, nothing installed (as the retail game)')
        return
    nodes, caps = struct.unpack_from('<II', ccns, 8)
    records = [(struct.unpack_from('<II', ccns, 0x10 + 0x50 * k),
                np.frombuffer(ccns, '<f4', 16, 0x20 + 0x50 * k).reshape(4, 4)) for k in range(nodes)]
    for (node, joint), off in records:
        c = r64(coat[node] + 0x100)
        assert c and c % 16 == 0, f'node {node}: no constraint'
        assert r64(c) == CVTABLE and mu.mem_read(c + 0x20, 1)[0] == 1
        assert struct.unpack('<I', mu.mem_read(c + 0x28, 4))[0] == 1
        assert r64(c + 0x30) == r64(r64(player + 0x1880 + 8 * joint) + 0x110)
        assert np.array_equal(np.frombuffer(bytes(mu.mem_read(c + 0x80, 64)), '<f4').reshape(4, 4), off)
        call(EVAL, {UC_X86_REG_RCX: c, UC_X86_REG_RDX: coat[node], UC_X86_REG_R8: 0})
        world = np.frombuffer(bytes(mu.mem_read(r64(coat[node] + 0x110), 64)), '<f4').reshape(4, 4)
        assert np.allclose(world, off @ body_world[joint]), f'node {node}: world != offset x host'
        print(f'{name}: coat node {node:2} <- body joint {joint:2}: constraint ok, 0x1402CBBE0 = offset x host')
    listed = {n for (n, _), _ in records}
    assert all(r64(coat[n] + 0x100) == 0 for n in range(39) if n not in listed)
    o = 0x10 + 0x50 * nodes
    for k in range(caps):
        index = struct.unpack_from('<I', ccns, o)[0]
        got = bytes(mu.mem_read(player + 0xB630 + index * 0x50 + 0x10, 48))
        assert got == ccns[o + 0x10:o + 0x40], f'capsule {index}'
        a, b, r = np.frombuffer(got, '<f4').reshape(3, 4)
        print(f'{name}: capsule {index}: A {a[:3].tolist()} B {b[:3].tolist()} r {r[0]}')
        o += 0x40

if __name__ == '__main__':
    exe = open(sys.argv[1], 'rb').read()
    for path in sys.argv[2:]:
        run(exe, open(path, 'rb').read(), path)
    print('OK')
