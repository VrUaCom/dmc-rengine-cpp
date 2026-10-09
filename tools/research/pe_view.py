import struct, sys, capstone
def load(path):
    b=open(path,'rb').read()
    pe=struct.unpack_from('<I',b,0x3c)[0]
    nsec=struct.unpack_from('<H',b,pe+6)[0]; opt=struct.unpack_from('<H',b,pe+20)[0]
    base=struct.unpack_from('<Q',b,pe+24+24)[0]
    secs=[]
    off=pe+24+opt
    for i in range(nsec):
        name=b[off:off+8].rstrip(b'\0').decode(); vsz,va,rsz,rp=struct.unpack_from('<IIII',b,off+8)
        secs.append((name,va,vsz,rp,rsz)); off+=40
    return b,base,secs
def r2o(secs,rva):
    for n,va,vsz,rp,rsz in secs:
        if va<=rva<va+max(vsz,rsz): return rp+rva-va
def dis(b,secs,base,rva,n=0x40):
    o=r2o(secs,rva); md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64)
    for i in md.disasm(b[o:o+n], base+rva):
        print(f"  +{i.address-base:06X}  {i.bytes.hex():<20} {i.mnemonic} {i.op_str}")
if __name__=='__main__':
    b,base,secs=load(sys.argv[1]); print(hex(base),[(s[0],hex(s[1])) for s in secs])
    for a in sys.argv[2:]:
        print('==',a); dis(b,secs,base,int(a,16))
