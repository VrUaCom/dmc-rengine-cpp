"""Who uses an address: python3 xref.py dmc3.exe 0x1402F9570 [--disp 0x7540]
  * direct call/jmp rel32 sites and 8-byte pointers (vtables) to a VA;
  * with --disp, every instruction whose memory operand uses that
    displacement (e.g. a player-object field such as +0x1898)."""
import re
import struct
import sys
import capstone
from pe import Image

img = Image(sys.argv[1])
tbase, text = img.text()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
if '--disp' in sys.argv:
    disp = int(sys.argv[sys.argv.index('--disp') + 1], 16)
    for m in re.finditer(re.escape(struct.pack('<I', disp)), text):
        for back in (3, 4, 2, 5, 6):
            s = m.start() - back
            ins = list(md.disasm(text[s:s + 16], tbase + s, 1))
            if ins and hex(disp) in ins[0].op_str and ins[0].size >= back + 4:
                f = img.function_of(ins[0].address)
                where = f'in {f[0]:#x}' if f else ''
                print(f'{ins[0].address:#x} {ins[0].mnemonic} {ins[0].op_str}  {where}')
                break
    sys.exit()
target = int(sys.argv[2], 16)
for i in range(len(text) - 5):
    if text[i] in (0xE8, 0xE9):
        rel = struct.unpack_from('<i', text, i + 1)[0]
        if tbase + i + 5 + rel == target:
            f = img.function_of(tbase + i)
            kind = 'call' if text[i] == 0xE8 else 'jmp'
            print(f'{kind} at {tbase + i:#x}' + (f'  in {f[0]:#x}' if f else ''))
p = struct.pack('<Q', target)
i = img.data.find(p)
while i != -1:
    for name, va, vs, raw, rs in img.sections:
        if raw <= i < raw + rs:
            print(f'pointer at {img.base + va + i - raw:#x} ({name})')
    i = img.data.find(p, i + 1)
