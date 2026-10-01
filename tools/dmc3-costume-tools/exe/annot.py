"""Annotated disassembly: python3 annot.py dmc3.exe START END  (rip-relative constants resolved, calls named if known)"""
import sys, struct
import capstone
from pe import Image
img = Image(sys.argv[1]); lo = int(sys.argv[2], 16); hi = int(sys.argv[3], 16)
off = img.off(lo)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64); md.detail = True
code = img.data[off:off + (hi - lo)]
for ins in md.disasm(code, lo):
    note = ''
    for op in ins.operands:
        if op.type == capstone.x86.X86_OP_MEM and op.mem.base == capstone.x86.X86_REG_RIP:
            t = ins.address + ins.size + op.mem.disp
            o = img.off(t)
            if o is not None:
                sec = img.section_of(t)
                raw = img.data[o:o + 16]
                f = struct.unpack_from('<f', raw)[0]; d = struct.unpack_from('<I', raw)[0]
                if ins.mnemonic == 'lea':
                    s = raw.split(b'\0')[0]
                    note = f'  ; {t:#x} ' + (repr(s.decode("latin1")) if len(s) > 1 and all(32 <= c < 127 for c in s) else '')
                else:
                    note = f'  ; [{t:#x}] f={f:.6g} u32={d:#x} {raw[:16].hex()}'
    print(f'{ins.address:#x}  {ins.mnemonic} {ins.op_str}{note}')
