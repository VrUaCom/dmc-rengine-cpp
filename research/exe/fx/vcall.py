"""Find indirect calls [reg+DISP] preceded (within N instrs) by a reference to +SUB: python3 vcall.py exe 0x80 0x110"""
import sys, capstone
from pe import Image
img = Image(sys.argv[1]); disp = int(sys.argv[2], 16); sub = sys.argv[3]
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
tb, text = img.text()
for b, e in img.functions:
    va = img.base + b
    if not (tb <= va < tb + len(text)): continue
    code = text[va - tb: img.base + e - tb]
    ins = list(md.disasm(code, va))
    for k, x in enumerate(ins):
        if x.mnemonic == 'call' and x.op_str.endswith(f'+ {hex(disp)}]') and 'ptr [r' in x.op_str:
            win = ins[max(0, k - 10):k]
            if any(f'+ {sub}]' in y.op_str for y in win):
                edx = [y.op_str for y in win if y.op_str.startswith('edx,')]
                print(hex(x.address), hex(va), edx[-1:] )
