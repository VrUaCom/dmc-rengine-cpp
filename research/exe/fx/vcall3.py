import sys, capstone
from pe import Image
img = Image(sys.argv[1]); codes = {int(c, 16) for c in sys.argv[2:]}
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
tb, text = img.text()
for b, e in img.functions:
    va = img.base + b
    if not (tb <= va < tb + len(text)): continue
    ins = list(md.disasm(text[va - tb: img.base + e - tb], va))
    for k, x in enumerate(ins):
        if x.mnemonic == 'call' and x.op_str.endswith('+ 0x80]'):
            win = ins[max(0, k - 8):k]
            for y in win:
                if y.mnemonic == 'mov' and y.op_str.startswith('edx, '):
                    try: v = int(y.op_str[5:], 0)
                    except ValueError: continue
                    if v in codes: print(hex(x.address), hex(va), hex(v), '|', '; '.join(z.mnemonic+' '+z.op_str for z in win[-5:]))
