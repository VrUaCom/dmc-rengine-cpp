"""Linear summary of enemy command functions: play(cmd) / frame>=F / event(code)."""
import sys, capstone, struct
from pe import Image
img = Image(sys.argv[1]); lo = int(sys.argv[2], 16); hi = int(sys.argv[3], 16)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64); md.detail = True
tb, text = img.text()
def fconst(x):
    for op in x.operands:
        if op.type == capstone.x86.X86_OP_MEM and op.mem.base == capstone.x86.X86_REG_RIP:
            a = x.address + x.size + op.mem.disp
            o = img.off(a)
            if o is not None: return struct.unpack_from('<f', img.data, o)[0]
    return None
for b, e in img.functions:
    va = img.base + b
    if not (lo <= va < hi): continue
    ins = list(md.disasm(text[va - tb: img.base + e - tb], va))
    seq = []; edx = None; last_call = None
    for x in ins:
        if x.op_str.startswith('edx, ') and x.mnemonic == 'mov':
            try: edx = int(x.op_str[5:], 0)
            except ValueError: edx = None
        if x.mnemonic in ('call', 'jmp') and 'qword ptr [rax + ' in x.op_str:
            slot = x.op_str.split('+ ')[1].rstrip(']')
            if slot == '0x20': seq.append(f'play({edx:#x})' if edx is not None else 'play(?)')
            elif slot == '0x28': seq.append(f'play2({edx:#x})' if edx is not None else 'play2(?)')
            elif slot == '0x80' and edx is not None and edx < 0x384: seq.append(f'EV{edx:#x}')
            last_call = slot
        if x.mnemonic in ('comiss', 'ucomiss') and last_call == '0x40':
            f = fconst(x)
            if f is not None: seq.append(f'f>={f:g}')
        if x.mnemonic == 'call' and x.op_str.startswith('0x'): last_call = None
        if x.mnemonic == 'call' and x.op_str.endswith('+ 0x40]'): last_call = '0x40'
    if any(s.startswith('EV') for s in seq):
        print(hex(va), ' '.join(seq))
