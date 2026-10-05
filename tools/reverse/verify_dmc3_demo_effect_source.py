"""v26 hash-bound demo effect source-parser and packed-selection fixtures.

Usage: verify_dmc3_demo_effect_source.py CANONICAL_EXE OUTPUT_JSON
Requires pefile==2024.8.26, unicorn==2.1.4. No retail-game/GPU execution.
Original tokenizer, typed parser, defaults, mapper and importer execute.
Heap/CRT/security/pool calls are explicitly replaced by fixture hooks.
"""
from pathlib import Path
import hashlib,json,re,struct,sys
import pefile,unicorn
from unicorn import Uc,UC_ARCH_X86,UC_MODE_64,UC_HOOK_CODE
from unicorn.x86_const import *

EXPECTED='e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'
raw=Path(sys.argv[1]).read_bytes()
assert hashlib.sha256(raw).hexdigest()==EXPECTED
pe=pefile.PE(data=raw); BASE=pe.OPTIONAL_HEADER.ImageBase
OBJ=0x10001000;PARAM=0x10002000;TEXT=0x10003000;CURSOR=0x10005000
LOOKUP=0x10006000;RUNTIME=0x10008000;POOL=0x1000A000
STOP=0x1007E000;ATOI=STOP+0x100;ATOF=STOP+0x200;CMP=STOP+0x300

def w32(u,a,v):u.mem_write(a,struct.pack('<I',v&0xffffffff))
def w64(u,a,v):u.mem_write(a,struct.pack('<Q',v))
def r32(u,a):return struct.unpack('<I',u.mem_read(a,4))[0]
def r64(u,a):return struct.unpack('<Q',u.mem_read(a,8))[0]
def cstr(u,a):
 out=bytearray()
 for i in range(4096):
  v=u.mem_read(a+i,1)[0]
  if not v:return out.decode('ascii')
  out.append(v)
 raise AssertionError('Unterminated fixture string')
def ret(u):
 sp=u.reg_read(UC_X86_REG_RSP);ip=r64(u,sp)
 u.reg_write(UC_X86_REG_RSP,sp+8);u.reg_write(UC_X86_REG_RIP,ip)

def make():
 u=Uc(UC_ARCH_X86,UC_MODE_64)
 u.mem_map(BASE,(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095)
 u.mem_write(BASE,pe.get_memory_mapped_image())
 u.mem_map(0x10000000,0x80000)
 u.mem_write(STOP,b'\xc3');u.mem_write(ATOI,b'\xc3');u.mem_write(ATOF,b'\xc3');u.mem_write(CMP,b'\xc3')
 w64(u,0x14034F420,ATOI);w64(u,0x14034F428,ATOF);w64(u,0x14034F600,CMP)
 trace=[];executed=set()
 def hook(u,ip,size,_):
  if ip==STOP:u.emu_stop();return
  if ip in [0x140326CA0,0x1402D6420,0x14031B2D0,0x14031E0B0,0x14031E190,0x14031E730,0x14031F050,0x14031EE00]:executed.add(hex(ip))
  if ip==0x140337710:
   trace.append({'target':hex(ip),'operation':'fixture heap descriptor reset/free no-op'});ret(u)
  elif ip==0x1402C6150:
   n=u.reg_read(UC_X86_REG_RDX)&0xffffffff
   assert n in [0x30,0x28],n
   u.mem_write(PARAM,b'\xA5'*n)
   u.reg_write(UC_X86_REG_RAX,PARAM)
   trace.append({'target':hex(ip),'operation':'fixture allocation','bytes':n,'fill':165});ret(u)
  elif ip==0x140323FB0:
   u.mem_write(OBJ,b'\0'*0x100);w32(u,OBJ+0x64,-1)
   u.reg_write(UC_X86_REG_RAX,OBJ)
   trace.append({'target':hex(ip),'operation':'fixture clip pool returns zeroed clip with type=-1'});ret(u)
  elif ip==0x1403455F0:
   trace.append({'target':hex(ip),'operation':'security-cookie return'});ret(u)
  elif ip==ATOI:
   t=cstr(u,u.reg_read(UC_X86_REG_RCX));m=re.match(r'^\s*([+-]?\d+)',t)
   v=int(m.group(1)) if m else 0
   assert -2147483648<=v<=2147483647,'Fixture excludes CRT atoi overflow'
   u.reg_write(UC_X86_REG_RAX,v&0xffffffff)
   trace.append({'iat':'0x14034f420','operation':'atoi fixture','text':t,'resultU32':v&0xffffffff});ret(u)
  elif ip==ATOF:
   t=cstr(u,u.reg_read(UC_X86_REG_RCX));v=float(t)
   u.reg_write(UC_X86_REG_XMM0,int.from_bytes(struct.pack('<d',v),'little'))
   trace.append({'iat':'0x14034f428','operation':'atof fixture','text':t,'value':v});ret(u)
  elif ip==CMP:
   a=cstr(u,u.reg_read(UC_X86_REG_RCX));b=cstr(u,u.reg_read(UC_X86_REG_RDX))
   u.reg_write(UC_X86_REG_RAX,(0 if a==b else -1 if a<b else 1)&0xffffffff)
   trace.append({'iat':'0x14034f600','operation':'strcmp fixture','left':a,'right':b});ret(u)
 u.hook_add(UC_HOOK_CODE,hook)
 return u,trace,executed

def run(u,start):
 sp=0x10070008;w64(u,sp,STOP);u.reg_write(UC_X86_REG_RSP,sp)
 u.emu_start(start,STOP,count=250000)
 assert u.reg_read(UC_X86_REG_RIP)==STOP,'Instruction budget exhausted'

def import_source(u,kind):
 u.mem_write(LOOKUP+4,struct.pack('<20h',kind,*([-1]*19)))
 w64(u,LOOKUP+0x58,RUNTIME)
 u.mem_write(RUNTIME,b'\0'*0x280)
 u.reg_write(UC_X86_REG_RCX,LOOKUP);u.reg_write(UC_X86_REG_RDX,kind)
 u.reg_write(UC_X86_REG_R8,PARAM);u.reg_write(UC_X86_REG_R9,0)
 run(u,0x14031F050)
 assert u.reg_read(UC_X86_REG_RAX)&0xffffffff==0
 offsets=[0x58,0x5c] if kind==11 else [0x2c,0x30]
 return [r32(u,RUNTIME+x) for x in offsets]

def parse_parameters(kind,pair):
 u,trace,executed=make();w32(u,OBJ+0x64,kind)
 keys=['DetailH','DetailV'] if kind==11 else ['ScaleX','ScaleY']
 text=('# End' if pair is None else f'{keys[0]} {pair[0]} {keys[1]} {pair[1]} # End')
 u.mem_write(TEXT,text.encode()+b'\0');w64(u,CURSOR,TEXT)
 u.reg_write(UC_X86_REG_RCX,OBJ);u.reg_write(UC_X86_REG_RDX,CURSOR)
 run(u,0x14031B2D0)
 assert u.reg_read(UC_X86_REG_RAX)&255==1
 offsets=[0x1c,0x20] if kind==11 else [0,4]
 result=[r32(u,PARAM+x) for x in offsets]
 expected=([4,3] if kind==11 else [100,100]) if pair is None else [x&0xffffffff for x in pair]
 assert result==expected,(kind,pair,result,expected)
 runtime=import_source(u,kind);assert runtime==expected
 return {'kind':kind,'text':text,'sourceOffsets':offsets,'sourceValuesU32':result,
   'sourcePayloadHex':bytes(u.mem_read(PARAM,0x30 if kind==11 else 0x28)).hex(),
   'runtimeValuesU32':runtime,'sourceParserSuccess':True,'executedEntries':sorted(executed),'stubs':trace}

def parse_clip(tag,pair):
 kind={'HTH':11,'RDN':12}[tag];keys=['DetailH','DetailV'] if kind==11 else ['ScaleX','ScaleY']
 u,trace,executed=make()
 text=f'# Clip Id {tag} Param {keys[0]} {pair[0]} {keys[1]} {pair[1]} # End'
 u.mem_write(TEXT,text.encode()+b'\0')
 u.reg_write(UC_X86_REG_RCX,0);u.reg_write(UC_X86_REG_RDX,TEXT);u.reg_write(UC_X86_REG_R8,POOL)
 run(u,0x1402DA9D0)
 assert u.reg_read(UC_X86_REG_RAX)&255==1
 assert r32(u,OBJ+0x64)==kind
 assert r64(u,OBJ+0x58)==PARAM
 runtime=import_source(u,kind);assert runtime==[x&0xffffffff for x in pair]
 return {'tag':tag,'kind':kind,'text':text,'runtimeValuesU32':runtime,
   'executedEntries':sorted(executed),'stubs':trace,'scope':'Actual text parser and importer, synthetic clip pool/runtime slot'}

def select_resource(start,stop,packed,count,offset):
 u,trace,executed=make()
 root=OBJ;descriptor=OBJ+0x200;header=OBJ+0x400;payload=OBJ+0x600
 w64(u,root+0x18,descriptor);w64(u,descriptor+0x18,header);w64(u,descriptor+0x20,payload)
 u.mem_write(header,struct.pack('<H',0 if packed else 1))
 w32(u,payload+4,count);w32(u,payload+0x10,offset)
 u.reg_write(UC_X86_REG_R14,root);u.reg_write(UC_X86_REG_R13,root)
 u.reg_write(UC_X86_REG_RSI,0);u.reg_write(UC_X86_REG_RDI,0x10020000)
 u.reg_write(UC_X86_REG_RSP,0x10070008)
 u.emu_start(start,stop,count=10000)
 assert u.reg_read(UC_X86_REG_RIP)==stop
 got=u.reg_read(UC_X86_REG_RDX)
 expected=payload+offset if packed and count>=3 and offset else 0 if packed else payload
 assert got==expected,(hex(start),packed,count,offset,hex(got),hex(expected))
 return {'sliceStart':hex(start),'stopBeforeBinder':hex(stop),'packedSelector':packed,
   'tableCount':count,'thirdEntryOffset':offset,'selectedVA':hex(got)}

def binder(count,offset=0x20,kindByte=1):
 u,trace,executed=make();resource=PARAM
 w32(u,resource+4,count);w32(u,resource+8,offset)
 u.mem_write(resource+offset+6,bytes([kindByte]))
 w64(u,OBJ+0xf8,0x12345678)
 u.reg_write(UC_X86_REG_RCX,OBJ);u.reg_write(UC_X86_REG_RDX,resource)
 run(u,0x14031EDB0)
 signed=lambda x:(x&65535) if x&65535<32768 else (x&65535)-65536
 expected=signed(count)>0 and count>=1 and offset!=0 and kindByte==1
 got=bool(u.reg_read(UC_X86_REG_RAX)&255);ptr=r64(u,OBJ+0xf8)
 assert got==expected and ptr==(resource if expected else 0),(count,got,ptr)
 return {'countU32':count,'low16Signed':signed(count),'offset':offset,'kindByte':kindByte,'accepted':got,'streamPointer':hex(ptr)}

def null_rebind():
 u,trace,executed=make()
 w32(u,PARAM+4,1);w32(u,PARAM+8,0x20)
 u.mem_write(PARAM+0x20+6,b'\x01')
 u.mem_write(PARAM+0x20+4,b'\0\0') # valid stream record with no effects
 u.reg_write(UC_X86_REG_RCX,OBJ);u.reg_write(UC_X86_REG_RDX,PARAM)
 run(u,0x14031EDB0)
 assert u.reg_read(UC_X86_REG_RAX)&255==1
 u.reg_write(UC_X86_REG_RCX,OBJ);u.reg_write(UC_X86_REG_RDX,0)
 run(u,0x14031EDB0)
 assert u.reg_read(UC_X86_REG_RAX)&255==0
 assert r64(u,OBJ+0xf8)==PARAM and r32(u,OBJ)&65535==1
 u.reg_write(UC_X86_REG_RCX,OBJ);u.reg_write(UC_X86_REG_RDX,0)
 run(u,0x14031EE00)
 assert u.reg_read(UC_X86_REG_RAX)&0xffffffff==0
 return {'sequence':'valid bind -> null rebind -> actual empty-stream admission',
   'nullBindReturned':False,'previousPointerPreserved':True,'previousCountPreserved':True,
   'admissionReturnedU32':0,'executedEntries':sorted(executed),'stubs':trace,
   'scope':'Synthetic retained valid resource; not retail reachability or stale freed memory'}

def parser_dispatch(index,target):
 u,trace,executed=make();w64(u,OBJ+0x3f50,TEXT)
 u.mem_write(TEXT,b'# End\0')
 u.reg_write(UC_X86_REG_RBX,OBJ);u.reg_write(UC_X86_REG_RDI,index)
 u.reg_write(UC_X86_REG_RBP,5);u.reg_write(UC_X86_REG_RSI,0)
 u.reg_write(UC_X86_REG_RAX,TEXT)
 u.reg_write(UC_X86_REG_RSP,0x10070008)
 u.emu_start(0x1402D6032,target,count=100)
 assert u.reg_read(UC_X86_REG_RIP)==target
 assert u.reg_read(UC_X86_REG_RCX)==OBJ and u.reg_read(UC_X86_REG_RDX)==TEXT
 return {'index':index,'target':hex(target),'sliceStart':'0x1402d6032','scope':'Actual indirect call setup; synthetic controller/load state'}

if __name__=='__main__':
 ps11=[None,(4,3),(6,3),(0,0),(-1,-1),(31,32),(-2147483648,2147483647)]
 ps12=[None,(100,100),(0,0),(1,3),(2,4),(201,201),(-1,-1),(2147483647,-2147483648)]
 result={'schema':'dmc-rengine.dmc3-demo-effect-source-verification.v1','canonicalExeSha256':EXPECTED,
   'scope':'Unmodified EXE code with explicit CRT/heap/pool/security stubs; controlled inputs, no retail corpus or original-game frames',
   'dependencies':{'pefile':pefile.__version__,'unicorn':unicorn.__version__},
   'parameters':[parse_parameters(11,x) for x in ps11]+[parse_parameters(12,x) for x in ps12],
   'clips':[parse_clip(t,p) for t,p in [('HTH',(6,3)),('HTH',(0,-1)),('RDN',(100,100)),('RDN',(1,3))]],
   'resourceSelection':[select_resource(a,b,*x) for a,b in [(0x14023B98A,0x14023B9C3),(0x14023CB8E,0x14023CBC7)]
       for x in [(True,2,0x100),(True,3,0),(True,3,0x100),(True,8,0x200),(False,0,0)]],
   'binder':[binder(n,o,k) for n,o,k in [(0,0x20,1),(1,0x20,1),(32767,0x20,1),(32768,0x20,1),
      (65535,0x20,1),(65536,0x20,1),(65537,0x20,1),(1,0,1),(1,0x20,2)]],
   'nullRebind':[null_rebind()],
   'parserDispatch':[parser_dispatch(i,t) for i,t in [(28,0x1402DA750),(31,0x1402DA9D0),(16,0x1402DAD60)]]}
 result['verification']={'cases':sum(len(result[k]) for k in ['parameters','clips','resourceSelection','binder','nullRebind','parserDispatch']),'passed':True}
 Path(sys.argv[2]).write_text(json.dumps(result,indent=2)+'\n')
 print(json.dumps(result['verification']))
