"""v27 canonical demo text-clip -> original scheduler -> runtime fixtures.

Usage: verify_dmc3_demo_clip_runtime.py CANONICAL_EXE OUTPUT_JSON
Requires pefile==2024.8.26, unicorn==2.1.4.
Pool/controller state is synthetic. Heap/CRT/security stubs are explicit.
No original Windows game/GPU execution or packed serializer is claimed.
"""
from pathlib import Path
import hashlib,json,math,re,struct,sys
import pefile,unicorn
from unicorn import Uc,UC_ARCH_X86,UC_MODE_64,UC_HOOK_CODE,UC_HOOK_MEM_WRITE_UNMAPPED,UcError,UC_ERR_WRITE_UNMAPPED
from unicorn.x86_const import *

EXPECTED='e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082'
raw=Path(sys.argv[1]).read_bytes();assert hashlib.sha256(raw).hexdigest()==EXPECTED
pe=pefile.PE(data=raw);BASE=pe.OPTIONAL_HEADER.ImageBase
OBJ=0x10001000;PARAM=0x10002000;TEXT=0x10003000;POOL=0x1000A000
ENTRIES=0x10010000;CTX=0x10020000;LOOKUP=CTX+0x3F60
BANK=0x140CD9B80;VTABLE=0x1405085B8;STOP=0x1007E000
ATOI=STOP+0x100;ATOF=STOP+0x200;CMP=STOP+0x300

def w32(u,a,v):u.mem_write(a,struct.pack('<I',v&0xffffffff))
def w64(u,a,v):u.mem_write(a,struct.pack('<Q',v))
def r32(u,a):return struct.unpack('<I',u.mem_read(a,4))[0]
def r64(u,a):return struct.unpack('<Q',u.mem_read(a,8))[0]
def f32(u,a,v):u.mem_write(a,struct.pack('<f',v))
def rf(u,a):return struct.unpack('<f',u.mem_read(a,4))[0]
def ret(u):
 sp=u.reg_read(UC_X86_REG_RSP);u.reg_write(UC_X86_REG_RIP,r64(u,sp));u.reg_write(UC_X86_REG_RSP,sp+8)
def cstr(u,a):
 b=bytearray()
 for i in range(4096):
  c=u.mem_read(a+i,1)[0]
  if c==0:return b.decode('ascii')
  b.append(c)
 raise AssertionError('Unterminated fixture text')

ORIGINAL_ENTRIES=[0x140323930,0x140323FB0,0x1403233F0,0x140329200,0x1403376F0,
 0x1402DA9D0,0x140326CA0,0x1402D6420,0x14031B2D0,0x14031E0B0,0x14031E190,
 0x140323CF0,0x14031E690,0x14031E5D0,0x14031E580,0x1402C8430,
 0x14031EA00,0x14031EA40,0x14031EA50,0x140315BD0,0x140316E10,
 0x14031F050,0x14031EC60,0x14031ED00,0x14031ED10,0x140316C20,0x140326960,0x14031EDB0,0x14031EE00]

def make():
 u=Uc(UC_ARCH_X86,UC_MODE_64)
 u.mem_map(BASE,(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095)
 u.mem_write(BASE,pe.get_memory_mapped_image());u.mem_map(0x10000000,0x80000)
 for a in [STOP,ATOI,ATOF,CMP]:u.mem_write(a,b'\xc3')
 for a,p in [(0x14034F420,ATOI),(0x14034F428,ATOF),(0x14034F600,CMP)]:w64(u,a,p)
 u.mem_write(BANK,b'\0'*(16*0x280))
 u.mem_write(LOOKUP+4,struct.pack('<20h',*([-1]*20)))
 u.mem_write(0x140CF2D90,struct.pack('<8f',*([1.0]*8)))
 info={'stubs':[],'executed':set(),'importCalls':[],'importReturns':[],'lastImportReturn':None}
 def code(u,ip,size,_):
  if ip==STOP:u.emu_stop();return
  if ip in ORIGINAL_ENTRIES:info['executed'].add(hex(ip))
  if ip==0x14031F050:
   info['importCalls'].append({'manager':hex(u.reg_read(UC_X86_REG_RCX)),
    'type':u.reg_read(UC_X86_REG_RDX)&0xffffffff,'source':hex(u.reg_read(UC_X86_REG_R8)),
    'flagR9b':u.reg_read(UC_X86_REG_R9)&255})
   info['lastImportReturn']=r64(u,u.reg_read(UC_X86_REG_RSP))
  if ip==info['lastImportReturn']:
   v=u.reg_read(UC_X86_REG_RAX)&0xffffffff
   info['importReturns'].append(v);info['lastImportReturn']=None
  if ip==0x140337600:
   dst=u.reg_read(UC_X86_REG_RCX);n=u.reg_read(UC_X86_REG_R8)&0xffffffff
   assert (dst,n) in [(POOL+0x20,50*32),(ENTRIES,112)],(hex(dst),n)
   target=ENTRIES if dst==POOL+0x20 else OBJ
   u.mem_write(target,b'\0'*n)
   w64(u,dst+8,0x140CF1180);w64(u,dst+0x10,target)
   u.reg_write(UC_X86_REG_RAX,target)
   info['stubs'].append({'target':hex(ip),'operation':'synthetic heap allocation/descriptor','descriptor':hex(dst),'bytes':n,'result':hex(target)});ret(u)
  elif ip==0x140337710:
   info['stubs'].append({'target':hex(ip),'operation':'heap reset/free no-op'});ret(u)
  elif ip==0x1402C6150:
   n=u.reg_read(UC_X86_REG_RDX)&0xffffffff;assert n in [48,40]
   u.mem_write(PARAM,b'\xA5'*n);u.reg_write(UC_X86_REG_RAX,PARAM)
   info['stubs'].append({'target':hex(ip),'operation':'source allocation','bytes':n,'fill':165});ret(u)
  elif ip==0x140346BEA:
   dst=u.reg_read(UC_X86_REG_RCX);n=u.reg_read(UC_X86_REG_R8);value=u.reg_read(UC_X86_REG_RDX)&255
   assert n==0x280
   u.mem_write(dst,bytes([value])*n);u.reg_write(UC_X86_REG_RAX,dst)
   info['stubs'].append({'target':hex(ip),'operation':'CRT memset','bytes':n,'value':value});ret(u)
  elif ip==0x1403455F0:
   info['stubs'].append({'target':hex(ip),'operation':'security-cookie return'});ret(u)
  elif ip==ATOI:
   text=cstr(u,u.reg_read(UC_X86_REG_RCX));m=re.match(r'^\s*([+-]?\d+)',text);v=int(m.group(1)) if m else 0
   assert -2147483648<=v<=2147483647
   u.reg_write(UC_X86_REG_RAX,v&0xffffffff)
   info['stubs'].append({'iat':'0x14034F420','operation':'atoi','text':text,'u32':v&0xffffffff});ret(u)
  elif ip==ATOF:
   text=cstr(u,u.reg_read(UC_X86_REG_RCX));v=float(text)
   u.reg_write(UC_X86_REG_XMM0,int.from_bytes(struct.pack('<d',v),'little'))
   info['stubs'].append({'iat':'0x14034F428','operation':'atof','text':text,'value':v});ret(u)
  elif ip==CMP:
   a=cstr(u,u.reg_read(UC_X86_REG_RCX));b=cstr(u,u.reg_read(UC_X86_REG_RDX))
   u.reg_write(UC_X86_REG_RAX,(0 if a==b else -1 if a<b else 1)&0xffffffff)
   info['stubs'].append({'iat':'0x14034F600','operation':'strcmp','left':a,'right':b});ret(u)
 u.hook_add(UC_HOOK_CODE,code);return u,info

def run(u,start):
 sp=0x10070008;w64(u,sp,STOP);u.reg_write(UC_X86_REG_RSP,sp)
 u.emu_start(start,STOP,count=250000)
 assert u.reg_read(UC_X86_REG_RIP)==STOP,'Instruction budget exhausted'
 return u.reg_read(UC_X86_REG_RAX)&0xffffffff

def parse(tag,pair):
 kind={'HTH':11,'RDN':12}[tag];u,info=make()
 u.reg_write(UC_X86_REG_RCX,POOL);u.reg_write(UC_X86_REG_RDX,15);u.reg_write(UC_X86_REG_R8,CTX)
 assert run(u,0x140323930)&255==1
 assert r64(u,POOL)==CTX and r32(u,POOL+0x10)==15 and r64(u,POOL+0x40)==ENTRIES
 keys=['DetailH','DetailV'] if kind==11 else ['ScaleX','ScaleY']
 text=f'# Clip ClipScale 1 Life 5 Id {tag} Param {keys[0]} {pair[0]} {keys[1]} {pair[1]} # End'
 u.mem_write(TEXT,text.encode()+b'\0')
 u.reg_write(UC_X86_REG_RCX,CTX);u.reg_write(UC_X86_REG_RDX,TEXT);u.reg_write(UC_X86_REG_R8,POOL)
 assert run(u,0x1402DA9D0)&255==1
 assert r64(u,OBJ)==VTABLE and r64(u,OBJ+0x20)==POOL
 assert r32(u,OBJ+0x64)==kind and r64(u,OBJ+0x58)==PARAM
 assert r32(u,OBJ+8)==1 and rf(u,OBJ+0x1c)==1 and rf(u,OBJ+0x34)==5
 w32(u,POOL+0x18,1) # synthetic selected scheduler population, one actual allocated clip
 return u,info,kind,text

def callback(u,offset):
 target=r64(u,r64(u,OBJ)+offset)
 assert target in [0x14031E690,0x14031E5D0,0x14031E580]
 u.reg_write(UC_X86_REG_RCX,OBJ);return run(u,target)
def schedule(u,time):
 f32(u,CTX+0x18,time);f32(u,CTX+0x1c,0)
 u.reg_write(UC_X86_REG_RCX,POOL);return run(u,0x140323CF0)
def pointer(u):return r64(u,LOOKUP+0x58)
def lanes(u,kind,p):return [r32(u,p+x) for x in ([0x58,0x5c] if kind==11 else [0x2c,0x30])]
def details(info):return {'executedEntries':sorted(info['executed']),'importCalls':info['importCalls'],'importReturnsU32':info['importReturns'],'stubs':info['stubs']}

def full_route(tag,pair,occupied=0):
 u,info,kind,text=parse(tag,pair)
 for i in range(occupied):u.mem_write(BANK+i*0x280,b'\x01')
 result=schedule(u,0);p=pointer(u)
 assert result&255==1 and r32(u,OBJ+8)==2
 assert p==BANK+occupied*0x280 and u.mem_read(p,1)[0]==kind
 got=lanes(u,kind,p);assert got==[x&0xffffffff for x in pair]
 assert info['importReturns']==[0]
 required=[0x140323930,0x140323FB0,0x1403233F0,0x140323CF0,0x14031EA50,0x140315BD0,0x140316E10,0x14031F050]
 assert all(hex(x) in info['executed'] for x in required)
 return {'tag':tag,'text':text,'preoccupiedRuntimeSlots':occupied,'runtimeSlotOrdinal':occupied,
  'lookupIndex':0,'clipState':2,'runtimeLanesU32':got,'schedulerStarted':True,**details(info)}

def lifecycle(tag):
 initial=(6,3) if tag=='HTH' else (100,100)
 u,info,kind,text=parse(tag,initial)
 assert schedule(u,0)&255==1;p=pointer(u)
 assert schedule(u,1)&255==0 and rf(u,OBJ+0x30)==4 and pointer(u)==p
 pair=(0,-1) if tag=='HTH' else (1,3)
 for off,v in zip([0x1c,0x20] if kind==11 else [0,4],pair):w32(u,PARAM+off,v)
 assert schedule(u,2)&255==0 and rf(u,OBJ+0x30)==3
 assert lanes(u,kind,p)==list(initial) and info['importReturns']==[0]
 assert callback(u,0)&255==1 and pointer(u)==p
 assert lanes(u,kind,p)==[x&0xffffffff for x in pair] and rf(u,OBJ+0x30)==5
 assert schedule(u,6)&255==0 and r32(u,OBJ+8)==3
 assert struct.unpack('<h',u.mem_read(LOOKUP+4,2))[0]==-1
 assert struct.unpack('<H',u.mem_read(p+4,2))[0]&4
 assert struct.unpack('<H',u.mem_read(p+0xc,2))[0]==3
 assert u.mem_read(p,1)[0]==kind # retirement is not immediate slot release
 return {'tag':tag,'text':text,'schedulerTimes':[0,1,2,6],'countdownAfterUpdate':[4,3],
  'sourceMutationIsSynthetic':True,'updateWithExistingLookupDoesNotReimport':True,
  'explicitActivationReimportsIntoSameSlot':True,'runtimeLanesAfterExplicitActivationU32':[x&0xffffffff for x in pair],
  'finalClipState':3,'lookupKindAfterStop':-1,'runtimeRetirementFlag':4,'runtimeRetirementCounter':3,
  'runtimeTypeStillNonzero':True,**details(info)}

def missing_lookup_update(tag,scale):
 u,info,kind,text=parse(tag,(6,3) if tag=='HTH' else (100,100))
 assert callback(u,0)&255==1
 callback(u,0x10);assert pointer(u)==BANK
 f32(u,OBJ+0x1c,scale)
 pair=(0,-1) if tag=='HTH' else (1,3)
 for off,v in zip([0x1c,0x20] if kind==11 else [0,4],pair):w32(u,PARAM+off,v)
 callback(u,8)
 assert pointer(u)==BANK+0x280 and lanes(u,kind,pointer(u))==[x&0xffffffff for x in pair]
 assert info['importReturns']==[0,0]
 return {'tag':tag,'scaleAtUpdate':scale,'sequence':'activate -> original stop clears lookup -> direct original update recreates slot',
  'retiredSlotNotImmediatelyReusable':True,'newRuntimeSlotOrdinal':1,
  'updateHasNoActivationScaleMinimumGate':True,'runtimeLanesU32':lanes(u,kind,pointer(u)),**details(info)}

def scale_gate(tag,value):
 u,info,kind,text=parse(tag,(6,3) if tag=='HTH' else (100,100));f32(u,OBJ+0x1c,value)
 stored=rf(u,OBJ+0x1c);threshold=rf(u,0x1404E01E8)
 expected=abs(stored)>=threshold
 result=bool(callback(u,0)&255)
 assert result==expected
 assert bool(pointer(u))==expected
 return {'tag':tag,'inputScale':value,'storedScale':stored,'threshold':threshold,'callbackAccepted':result,
  'scope':'Direct original vtable activation callback; scheduler temporal window bypassed intentionally',**details(info)}

def exhaustion(tag,domain):
 u,info,kind,text=parse(tag,(6,3) if tag=='HTH' else (100,100))
 if domain=='lookup':u.mem_write(LOOKUP+4,struct.pack('<20h',*([1]*20)))
 else:
  for i in range(16):u.mem_write(BANK+i*0x280,b'\x01')
 result=schedule(u,0)&255
 state=r32(u,OBJ+8)
 if domain=='lookup':
  assert result==0 and state==4 and info['importCalls']==[]
 else:
  assert result==1 and state==2 and pointer(u)==0
  assert struct.unpack('<h',u.mem_read(LOOKUP+4,2))[0]==kind
  assert info['importReturns']==[0xffffffff]
 return {'tag':tag,'exhaustedDomain':domain,'lookupCapacity':20,'runtimeCapacity':16,
  'schedulerStarted':bool(result),'clipState':state,'runtimePointer':hex(pointer(u)),**details(info)}

def failed_allocation_recovery(tag):
 u,info,kind,text=parse(tag,(0,-1) if tag=='HTH' else (1,3))
 for i in range(16):u.mem_write(BANK+i*0x280,b'\x01')
 assert callback(u,0)&255==1 and pointer(u)==0
 u.mem_write(BANK,b'\0') # synthetic release, not original render retirement
 assert callback(u,0)&255==1 and pointer(u)==0 # registered null entry prevents retry
 callback(u,0x10)
 assert struct.unpack('<h',u.mem_read(LOOKUP+4,2))[0]==-1
 assert callback(u,0)&255==1 and pointer(u)==BANK
 assert info['importReturns']==[0xffffffff,0xffffffff,0]
 return {'tag':tag,'sequence':'full runtime bank -> accepted null lookup -> synthetic free slot -> no retry -> original stop -> original activation succeeds',
  'syntheticRelease':True,'finalRuntimePointer':hex(pointer(u)),'finalRuntimeLanesU32':lanes(u,kind,pointer(u)),**details(info)}

def dispatch_argument():
 u,info=make();w64(u,CTX+0x3f50,TEXT);u.mem_write(TEXT,b'# End\0')
 for r,v in [(UC_X86_REG_RBX,CTX),(UC_X86_REG_RDI,31),(UC_X86_REG_RBP,5),(UC_X86_REG_RSI,0),(UC_X86_REG_RAX,TEXT),(UC_X86_REG_RSP,0x10070008)]:u.reg_write(r,v)
 u.emu_start(0x1402D6032,0x1402DA9D0,count=100)
 assert u.reg_read(UC_X86_REG_RIP)==0x1402DA9D0
 arg=u.reg_read(UC_X86_REG_R8);assert arg==0x1405CED80 and arg!=POOL
 return {'index':31,'target':'0x1402DA9D0','actualR8':hex(arg),
  'boundary':'Original dispatcher supplies table base in R8. Full text fixtures deliberately supply a valid synthetic pool instead; production third-argument/reachability remains open.'}

def packed_admission(tag,domain='available',other_manager=False):
 u,info,kind,text=parse(tag,(0,-1) if tag=='HTH' else (1,3))
 manager=LOOKUP
 if other_manager:
  assert schedule(u,0)&255==1 and pointer(u)==BANK
  manager=0x10030000
  u.mem_write(manager+4,struct.pack('<20h',*([-1]*20)))
 resource=0x10040000;record=resource+0x20
 w32(u,resource+4,1);w32(u,resource+8,0x20)
 u.mem_write(record+4,struct.pack('<H',1));u.mem_write(record+6,b'\x01')
 w32(u,record+8,kind);w32(u,record+0xc,0x18)
 u.mem_write(record+0x18,bytes(u.mem_read(PARAM,48 if kind==11 else 40)))
 if domain=='runtime':
  for i in range(16):u.mem_write(BANK+i*0x280,b'\x01')
 elif domain=='lookup':u.mem_write(manager+4,struct.pack('<20h',*([1]*20)))
 u.reg_write(UC_X86_REG_RCX,manager);u.reg_write(UC_X86_REG_RDX,resource)
 assert run(u,0x14031EDB0)&255==1
 fault=[]
 def invalid(u,access,address,size,value,_):
  fault.append({'ip':hex(u.reg_read(UC_X86_REG_RIP)),'address':hex(address),'bytes':size,'value':value})
  return False
 u.hook_add(UC_HOOK_MEM_WRITE_UNMAPPED,invalid)
 u.reg_write(UC_X86_REG_RCX,manager);u.reg_write(UC_X86_REG_RDX,0)
 result=None
 try:result=run(u,0x14031EE00)
 except UcError as error:
  assert domain in ['runtime','lookup'] and error.errno==UC_ERR_WRITE_UNMAPPED
 if domain in ['runtime','lookup']:
  expected=(0x14031F0FB,0x2c) if kind==11 else (0x14031F38D,0x2c)
  assert len(fault)==1 and (int(fault[0]['ip'],16),int(fault[0]['address'],16))==expected,fault
  assert info['importCalls'][-1]['flagR9b']==1
 else:
  p=r64(u,manager+0x58);expected_p=BANK+(0x280 if other_manager else 0)
  assert result==0 and fault==[] and p==expected_p
  assert lanes(u,kind,p)==([0,0xffffffff] if kind==11 else [1,3])
 return {'tag':tag,'domain':domain,'secondManagerSharesGlobalBank':other_manager,
  'manager':hex(manager),'runtimePointer':hex(r64(u,manager+0x58)),
  'admissionReturnU32':result,'expectedSyntheticUnmappedWrite':fault,
  'sourcePayloadCopyIsSynthetic':True,'scope':'Actual binder/admission/allocator/initializer/importer; synthetic packed record copied from actual text-constructor payload. No original serializer, retail crash or reachability claim.',**details(info)}

if __name__=='__main__':
 result={'schema':'dmc-rengine.dmc3-demo-clip-runtime-verification.v1','canonicalExeSha256':EXPECTED,
  'dependencies':{'pefile':pefile.__version__,'unicorn':unicorn.__version__},
  'scope':'Original parser, pool constructor, clip constructor, scheduler, callbacks, runtime allocator/initializer and importer; synthetic controller/pool state and explicit heap/CRT/security stubs. No packed serializer or original game/GPU run.',
  'routes':[full_route(t,p) for t,p in [('HTH',(6,3)),('HTH',(0,-1)),('HTH',(31,32)),('HTH',(-2147483648,2147483647)),
   ('RDN',(100,100)),('RDN',(1,3)),('RDN',(0,0)),('RDN',(-1,-1))]],
  'lastFreeSlot':[full_route(t,p,15) for t,p in [('HTH',(6,3)),('RDN',(1,3))]],
  'lifecycle':[lifecycle(t) for t in ['HTH','RDN']],
  'missingLookupUpdate':[missing_lookup_update(t,s) for t in ['HTH','RDN'] for s in [1,0]],
  'scaleGate':[scale_gate(t,v) for t in ['HTH','RDN'] for v in [0,.00005,.0001,1,-1,-.00005,-.0001]],
  'exhaustion':[exhaustion(t,d) for t in ['HTH','RDN'] for d in ['lookup','runtime']],
  'recovery':[failed_allocation_recovery(t) for t in ['HTH','RDN']],
  'packedAdmission':[packed_admission(t,d) for t in ['HTH','RDN'] for d in ['available','runtime','lookup']],
  'sharedBank':[packed_admission(t,other_manager=True) for t in ['HTH','RDN']],
  'dispatchArgument':[dispatch_argument()]}
 groups=['routes','lastFreeSlot','lifecycle','missingLookupUpdate','scaleGate','exhaustion','recovery','packedAdmission','sharedBank','dispatchArgument']
 result['verification']={'cases':sum(len(result[x]) for x in groups),'passed':True}
 Path(sys.argv[2]).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result['verification']))
