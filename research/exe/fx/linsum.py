"""Linear summary per function: spawn K#id, comiss consts, script play (0x14005a1f0 edx/r8d), event calls."""
import sys, capstone, struct
sys.path.insert(0, '.')
from pe import Image
from resolve import resolve
img = Image(sys.argv[1]); lo = int(sys.argv[2], 16); hi = int(sys.argv[3], 16)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64); md.detail = True
tb, text = img.text()
API = {0x1402E7A90, 0x1402E7AB0, 0x1402E7CA0, 0x1402E7A80}
def fconst(x):
    for op in x.operands:
        if op.type == capstone.x86.X86_OP_MEM and op.mem.base == capstone.x86.X86_REG_RIP:
            o = img.off(x.address + x.size + op.mem.disp)
            if o is not None: return struct.unpack_from('<f', img.data, o)[0]
    return None
for b, e in img.functions:
    va = img.base + b
    if not (lo <= va < hi): continue
    ins = list(md.disasm(text[va - tb: img.base + e - tb], va))
    seq = []; regs = {}
    for x in ins:
        if x.mnemonic == 'mov' and len(x.operands) == 2 and x.operands[1].type == capstone.x86.X86_OP_IMM and x.operands[0].type == capstone.x86.X86_OP_REG:
            regs[x.reg_name(x.operands[0].reg)] = x.operands[1].imm
        if x.mnemonic in ('comiss', 'ucomiss'):
            f = fconst(x)
            if f is not None and f == f and abs(f) < 1e5: seq.append(f'cmp{f:g}')
        if x.mnemonic == 'call' and x.op_str.startswith('0x'):
            t = int(x.op_str, 16)
            if t in API:
                k, i, m = resolve(img, x.address)
                seq.append(f"{'PEGV'[k] if k is not None and k < 4 else '?'}{i}")
            elif t == 0x14005a1f0:
                seq.append(f"play({regs.get('edx')},{regs.get('r8d')})")
        if x.mnemonic == 'call' and x.op_str.endswith('+ 0x80]') and 'edx' in regs: pass
    if any(s[0] in 'PEGV?' and s[1:].isdigit() for s in seq):
        print(hex(va), ' '.join(seq))
