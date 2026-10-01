import capstone, struct
from pe import Image
N64 = {'eax':'rax','ebx':'rbx','ecx':'rcx','edx':'rdx','esi':'rsi','edi':'rdi','ebp':'rbp','esp':'rsp','cl':'rcx','dl':'rdx','al':'rax','r8d':'r8','r9d':'r9','r10d':'r10','r11d':'r11'}
def norm(r): return N64.get(r, r)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64); md.detail = True
def window(img, at, back=120):
    tb, text = img.text(); j = at - tb
    for s0 in range(max(0, j - back), j):
        ins = list(md.disasm(text[s0:j + 5], tb + s0))
        if ins and ins[-1].address == at and sum(x.size for x in ins) == j + 5 - s0:
            return ins
    return []
def resolve(img, at):
    consts = {}
    for x in window(img, at)[:-1]:
        if len(x.operands) != 2 or x.operands[0].type != capstone.x86.X86_OP_REG: 
            continue
        r = norm(x.reg_name(x.operands[0].reg)); o = x.operands[1]
        if x.mnemonic == 'xor' and o.type == capstone.x86.X86_OP_REG and norm(x.reg_name(o.reg)) == r: consts[r] = 0
        elif x.mnemonic == 'mov' and o.type == capstone.x86.X86_OP_IMM: consts[r] = o.imm & 0xFFFFFFFF
        elif x.mnemonic == 'lea' and o.type == capstone.x86.X86_OP_MEM and o.mem.base and norm(x.reg_name(o.mem.base)) in consts and not o.mem.index:
            consts[r] = (consts[norm(x.reg_name(o.mem.base))] + o.mem.disp) & 0xFFFFFFFF
        else: consts.pop(r, None)
    return consts.get('rcx'), consts.get('rdx'), consts.get('r9')
