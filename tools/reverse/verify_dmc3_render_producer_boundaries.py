"""Hash-bound cropped EXE execution; no game or D3D11 acceptance claim."""
from pathlib import Path
import pefile, struct, json, hashlib, sys
from unicorn import Uc, UC_ARCH_X86, UC_MODE_64, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import *
ROOT=Path(__file__).resolve().parent
if len(sys.argv)!=3: raise SystemExit('Usage: verify_dmc3_render_producer_boundaries.py CANONICAL_EXE OUTPUT_JSON')
OUTPUT=Path(sys.argv[2]); raw=Path(sys.argv[1]).read_bytes()
SHA=hashlib.sha256(raw).hexdigest(); assert SHA=='e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'
p=pefile.PE(data=raw);BASE=p.OPTIONAL_HEADER.ImageBase
def make():
 u=Uc(UC_ARCH_X86,UC_MODE_64);u.mem_map(BASE,(p.OPTIONAL_HEADER.SizeOfImage+4095)&~4095);u.mem_write(BASE,p.get_memory_mapped_image())
 u.mem_map(0x10000000,0x80000);u.reg_write(UC_X86_REG_RSP,0x10070000);return u
def w32(u,a,v):u.mem_write(a,struct.pack('<I',v))
def w64(u,a,v):u.mem_write(a,struct.pack('<Q',v))
def ret(u):
 sp=u.reg_read(UC_X86_REG_RSP);ip=struct.unpack('<Q',u.mem_read(sp,8))[0];u.reg_write(UC_X86_REG_RSP,sp+8);u.reg_write(UC_X86_REG_RIP,ip)
def sub12(a,b):
 u=make();param=0x10001000;w32(u,param+0x2c,a);w32(u,param+0x30,b)
 u.reg_write(UC_X86_REG_RBP,param);u.reg_write(UC_X86_REG_R13,0);u.reg_write(UC_X86_REG_RSP,0x10060000)
 rows=0;records=0;conflict=None;done=False
 def code(u,ip,size,_):
  nonlocal records,conflict,done
  if ip==0x140059390:u.reg_write(UC_X86_REG_RAX,0);ret(u)
  elif ip==0x14031a0b0:
   records+=1
   if records>500: u.emu_stop()
  elif ip==0x14031a19b:done=True;u.emu_stop()
 def write(u,access,address,size,value,_):
  nonlocal conflict
  if address<0x140CD9AF8 and address+size>0x140CD9AF0:
   conflict={'ip':hex(u.reg_read(UC_X86_REG_RIP)),'address':hex(address),'size':size,'recordOrdinal':records};u.emu_stop()
 u.hook_add(UC_HOOK_CODE,code);u.hook_add(UC_HOOK_MEM_WRITE,write)
 u.emu_start(0x140319fe9,0x14031a19c,count=100000)
 out={'input':[a,b],'normalized':list(struct.unpack('<I',u.mem_read(param+0x2c,4)))+list(struct.unpack('<I',u.mem_read(param+0x30,4))), 'steps':[u.reg_read(UC_X86_REG_R12),u.reg_read(UC_X86_REG_R15)],'recordsEntered':records,'completed':done,'knownNeighborWrite':conflict,'endDelta':u.reg_read(UC_X86_REG_RDI)-0x140CD1AF0}
 return out
def sub11(width,height,xshift,yshift):
 u=make();param=0x10001000;ctx=0x10020000;globalctx=0x10003000
 w32(u,param+0x58,xshift);w32(u,param+0x5c,yshift);w32(u,ctx+0x15d58,width);w32(u,ctx+0x15d5c,height);w64(u,0x140D6D300,globalctx)
 u.reg_write(UC_X86_REG_RBP,param);u.reg_write(UC_X86_REG_RDI,ctx);u.reg_write(UC_X86_REG_RSI,1)
 u.reg_write(UC_X86_REG_RSP,0x10060000);u.reg_write(UC_X86_REG_XMM9,0x3f800000);u.reg_write(UC_X86_REG_XMM10,0x3f800000)
 conflict=None;done=False
 def code(u,ip,size,_):
  nonlocal done
  if ip==0x1403194d8:done=True;u.emu_stop()
 def write(u,access,address,size,value,_):
  nonlocal conflict
  if address<0x140CD1AD4 and address+size>0x140CD1AD0:
   conflict={'ip':hex(u.reg_read(UC_X86_REG_RIP)),'address':hex(address),'size':size,'recordOrdinal':(u.reg_read(UC_X86_REG_RBX)-0x140CC1AD0)//40+1};u.emu_stop()
 u.hook_add(UC_HOOK_CODE,code);u.hook_add(UC_HOOK_MEM_WRITE,write)
 u.emu_start(0x140318fb3,0x1403194d9,count=2000000)
 return {'display':[width,height],'shifts':[xshift,yshift],'completed':done,'endDelta':u.reg_read(UC_X86_REG_RBX)-0x140CC1AD0,'knownScalarWrite':conflict,'formulaBytes':40*(height//(1<<yshift)+2)*(width//(1<<xshift)+3)}
def shadow(n,edge,active=True,tetra=False):
 u=make();desc=0x10001000;markers=0x10002000;tris=0x10010000;verts=0x10001080
 w64(u,desc,verts);w64(u,desc+8,markers);w64(u,desc+16,tris);w32(u,desc+24,n)
 marker_list=[(0,0,0,0) if i%4==0 else (-1,0,0,0) for i in range(n)] if tetra else [(edge if active else -1,edge,edge,0)]*n
 u.mem_write(markers,b''.join(struct.pack('<iiii',*x) for x in marker_list))
 u.mem_write(tris,struct.pack('<iiii',0,1,2,0)*n)
 u.mem_write(verts,struct.pack('<12f',0,0,0,1,1,0,0,1,0,1,0,1));u.reg_write(UC_X86_REG_RCX,desc)
 conflict=None;draw=None
 def code(u,ip,size,_):
  nonlocal draw
  if ip in [0x140045fe0,0x1400460e0,0x140046910]:ret(u)
  elif ip==0x1400313f0:
   dst=u.reg_read(UC_X86_REG_RCX);src=u.reg_read(UC_X86_REG_R8);u.mem_write(dst,bytes(u.mem_read(src,16)));u.reg_write(UC_X86_REG_RAX,dst);ret(u)
  elif ip==0x140044cdf:
   draw={'bytes':u.reg_read(UC_X86_REG_R9),'flags':u.reg_read(UC_X86_REG_RDX),'mode':u.reg_read(UC_X86_REG_R8),'vertices':u.reg_read(UC_X86_REG_R12)};u.emu_stop()
 def write(u,access,address,size,value,_):
  nonlocal conflict
  if address<0x140C0B3F8 and address+size>0x140C0B3F0:
   conflict={'ip':hex(u.reg_read(UC_X86_REG_RIP)),'address':hex(address),'size':size,'verticesSoFar':u.reg_read(UC_X86_REG_R12)};u.emu_stop()
 u.hook_add(UC_HOOK_CODE,code);u.hook_add(UC_HOOK_MEM_WRITE,write)
 u.emu_start(0x1400446f0,0x140044ce4,count=2000000)
 return {'triangles':n,'edgeMarker':edge,'active':active,'tetraVisibilityPattern':tetra,'activeTriangles':sum(x[0]>=0 for x in marker_list),'draw':draw,'knownRendererPointerWrite':conflict}
def shadow_planner(counts):
 u=make();ctx=0x10001000;source=0x10002000;w64(u,ctx+0x28,source);u.mem_write(source+0x10,bytes([len(counts)]))
 for i,(v,t) in enumerate(counts):u.mem_write(source+0x20+64*i,struct.pack('<HH',v,t))
 u.reg_write(UC_X86_REG_RCX,ctx);u.emu_start(0x14031fc40,0x14031fd22,count=10000)
 signed=lambda x:x if x<32768 else x-65536
 expected=(0x280+0xa0*len(counts)+0x20*sum(signed(v)+signed(t)+6 for v,t in counts))&0xffffffff
 got=u.reg_read(UC_X86_REG_RAX)&0xffffffff;assert got==expected
 return {'hulls':counts,'plannedBytesU32':got,'expectedBytesU32':expected,'signed16CountInterpretation':True}
if __name__=='__main__':
 r={'schema':'dmc-rengine.resource-limits.cropped-emulation.v1','canonicalExeSha256':SHA,'scope':'Unmodified instruction slices; external RNG/state/transform calls stubbed; no original-game execution','subcommand11':[sub11(*x) for x in [(640,360,6,6),(640,360,5,5),(640,360,4,4),(640,360,3,3)]],'subcommand12':[sub12(*x) for x in [(100,100),(60,60),(50,50),(200,200),(201,201),(0,100),(100,0),(1,3)]], 'shadow':[shadow(*x) for x in [(1,0),(1,1),(1,0,False),(455,0),(456,0),(1820,1),(1821,1),(1820,0,True,True),(1824,0,True,True)]]}
 for x in r['subcommand11']:
  assert x['completed'] and x['endDelta']==x['formulaBytes'] or not x['completed'] and x['knownScalarWrite']['recordOrdinal']==1639
 for x in r['subcommand12']:
  if x['completed']:
   sx,sy=x['steps'];assert sx>0 and sy>0 and x['endDelta']==80*((640+sx-1)//sx)*((360+sy-1)//sy)
  else:assert x['knownNeighborWrite']['recordOrdinal']==410
 for x in r['shadow']:
  if x['draw']:assert x['draw']['bytes']==(72 if x['edgeMarker'] else 288)*x['activeTriangles']
  else:assert x['knownRendererPointerWrite']['verticesSoFar']==10926
 r['shadowPlanner']=[shadow_planner(x) for x in [[],[(3,1)],[(4,4),(8,12)],[(4,4),(8,12),(12,20)],[(32767,32767)],[(32768,32768)]]]
 r['verification']={'cases':sum(len(r[k]) for k in ['subcommand11','subcommand12','shadow','shadowPlanner']),'passed':True}
 OUTPUT.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
