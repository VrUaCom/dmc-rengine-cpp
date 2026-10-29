#!/usr/bin/env python3
"""Group bounded CRT entry prefixes; these groups do not prove full semantics."""
import argparse, collections, csv, gzip, hashlib, io, json, struct
from pathlib import Path
import capstone
p=argparse.ArgumentParser(description=__doc__)
for n in ('exe','roots','out'):p.add_argument(n,type=Path)
a=p.parse_args();b=a.exe.read_bytes()
assert hashlib.sha256(b).hexdigest()=='e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'
pe=struct.unpack_from('<I',b,60)[0];opt=pe+24;base=struct.unpack_from('<Q',b,opt+24)[0]
sec=[]
for i in range(struct.unpack_from('<H',b,pe+6)[0]):
 q=opt+struct.unpack_from('<H',b,pe+20)[0]+40*i
 vs,rva,rs,raw=struct.unpack_from('<IIII',b,q+8);sec.append((base+rva,min(vs,rs),raw))
def code(va):
 for start,size,raw in sec:
  if start<=va<start+size:return b[raw+va-start:raw+min(va-start+256,size)]
 raise ValueError(hex(va))
md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);md.detail=True
with gzip.open(a.roots,'rt') as f: roots=sorted({int(r['target_va'],16) for r in csv.DictReader(f,delimiter='\t') if r['kind'].startswith('crt')})
rows=[];groups=collections.Counter();stops=collections.Counter()
for va in roots:
 shape=[];dest=[];stop='budget';count=0
 for ins in md.disasm(code(va),va):
  count+=1;shape.append(ins.mnemonic+':'+','.join(str(o.type) for o in ins.operands))
  for o in ins.operands:
   if o.type==capstone.x86.X86_OP_MEM and o.mem.base==capstone.x86.X86_REG_RIP and o.access&capstone.CS_AC_WRITE:dest.append(hex(ins.address+ins.size+o.mem.disp))
  if any(ins.group(g) for g in (capstone.CS_GRP_CALL,capstone.CS_GRP_JUMP,capstone.CS_GRP_RET,capstone.CS_GRP_INT)):
   stop=ins.mnemonic;break
  if count>=32:break
 key=';'.join(shape);groups[key]+=1;stops[stop]+=1
 rows.append((hex(va),stop,count,','.join(dest),key))
a.out.mkdir(parents=True,exist_ok=True)
s=io.StringIO();w=csv.writer(s,delimiter='\t',lineterminator='\n');w.writerow(['target_va','prefix_stop','instruction_count','rip_write_destinations','mnemonic_operand_type_shape']);w.writerows(rows)
(a.out/'crt-prefixes.tsv.gz').write_bytes(gzip.compress(s.getvalue().encode(),mtime=0))
result=dict(roots=len(roots),shape_count=len(groups),stop_counts=dict(stops),top_shapes=groups.most_common(15),limitations=['Only straight-line prefix through first transfer, at most 32 instructions / 256 bytes.','Shapes omit constants, register identities and operand widths; same shape does not imply same semantics.','RIP write destinations are explicit writes only; aliasing and interprocedural effects are not modeled.'])
(a.out/'crt-prefix-summary.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
