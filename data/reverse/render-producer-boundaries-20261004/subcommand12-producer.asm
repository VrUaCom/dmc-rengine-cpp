
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140319d60 <.text+0x318d60>:
   140319d60:	48 8b c4             	mov    rax,rsp
   140319d63:	55                   	push   rbp
   140319d64:	56                   	push   rsi
   140319d65:	57                   	push   rdi
   140319d66:	41 54                	push   r12
   140319d68:	41 55                	push   r13
   140319d6a:	41 56                	push   r14
   140319d6c:	41 57                	push   r15
   140319d6e:	48 81 ec f0 00 00 00 	sub    rsp,0xf0
   140319d75:	48 c7 44 24 40 fe ff 	mov    QWORD PTR [rsp+0x40],0xfffffffffffffffe
   140319d7c:	ff ff 
   140319d7e:	48 89 58 18          	mov    QWORD PTR [rax+0x18],rbx
   140319d82:	0f 29 70 b8          	movaps XMMWORD PTR [rax-0x48],xmm6
   140319d86:	0f 29 78 a8          	movaps XMMWORD PTR [rax-0x58],xmm7
   140319d8a:	44 0f 29 40 98       	movaps XMMWORD PTR [rax-0x68],xmm8
   140319d8f:	44 0f 29 48 88       	movaps XMMWORD PTR [rax-0x78],xmm9
   140319d94:	44 0f 29 90 78 ff ff 	movaps XMMWORD PTR [rax-0x88],xmm10
   140319d9b:	ff 
   140319d9c:	44 0f 29 98 68 ff ff 	movaps XMMWORD PTR [rax-0x98],xmm11
   140319da3:	ff 
   140319da4:	44 0f 29 a0 58 ff ff 	movaps XMMWORD PTR [rax-0xa8],xmm12
   140319dab:	ff 
   140319dac:	44 0f 29 6c 24 70    	movaps XMMWORD PTR [rsp+0x70],xmm13
   140319db2:	44 0f 29 74 24 60    	movaps XMMWORD PTR [rsp+0x60],xmm14
   140319db8:	48 8b 05 f1 b2 2b 00 	mov    rax,QWORD PTR [rip+0x2bb2f1]        # 0x1405d50b0
   140319dbf:	48 33 c4             	xor    rax,rsp
   140319dc2:	48 89 44 24 58       	mov    QWORD PTR [rsp+0x58],rax
   140319dc7:	48 8b ea             	mov    rbp,rdx
   140319dca:	48 8b f1             	mov    rsi,rcx
   140319dcd:	45 33 ed             	xor    r13d,r13d
   140319dd0:	4c 39 2d 01 7d 9b 00 	cmp    QWORD PTR [rip+0x9b7d01],r13        # 0x140cd1ad8
   140319dd7:	75 4d                	jne    0x140319e26
   140319dd9:	41 8d 4d 60          	lea    ecx,[r13+0x60]
   140319ddd:	e8 2e b7 02 00       	call   0x140345510
   140319de2:	48 8b d8             	mov    rbx,rax
   140319de5:	48 89 44 24 30       	mov    QWORD PTR [rsp+0x30],rax
   140319dea:	48 85 c0             	test   rax,rax
   140319ded:	74 18                	je     0x140319e07
   140319def:	33 d2                	xor    edx,edx
   140319df1:	45 8d 45 60          	lea    r8d,[r13+0x60]
   140319df5:	48 8b c8             	mov    rcx,rax
   140319df8:	e8 ed cd 02 00       	call   0x140346bea
   140319dfd:	48 8b cb             	mov    rcx,rbx
   140319e00:	e8 3b c4 d2 ff       	call   0x140046240
   140319e05:	eb 03                	jmp    0x140319e0a
   140319e07:	49 8b c5             	mov    rax,r13
   140319e0a:	48 89 05 c7 7c 9b 00 	mov    QWORD PTR [rip+0x9b7cc7],rax        # 0x140cd1ad8
   140319e11:	ba 40 00 00 00       	mov    edx,0x40
   140319e16:	44 8d 4a dc          	lea    r9d,[rdx-0x24]
   140319e1a:	44 8d 42 e0          	lea    r8d,[rdx-0x20]
   140319e1e:	48 8b c8             	mov    rcx,rax
   140319e21:	e8 1a d0 d2 ff       	call   0x140046e40
   140319e26:	48 8b 05 b3 7c 9b 00 	mov    rax,QWORD PTR [rip+0x9b7cb3]        # 0x140cd1ae0
   140319e2d:	48 85 c0             	test   rax,rax
   140319e30:	75 0c                	jne    0x140319e3e
   140319e32:	e8 19 99 d1 ff       	call   0x140033750
   140319e37:	48 89 05 a2 7c 9b 00 	mov    QWORD PTR [rip+0x9b7ca2],rax        # 0x140cd1ae0
   140319e3e:	83 bd 88 00 00 00 00 	cmp    DWORD PTR [rbp+0x88],0x0
   140319e45:	75 11                	jne    0x140319e58
   140319e47:	c7 85 88 00 00 00 04 	mov    DWORD PTR [rbp+0x88],0x4
   140319e4e:	00 00 00 
   140319e51:	48 8b 05 88 7c 9b 00 	mov    rax,QWORD PTR [rip+0x9b7c88]        # 0x140cd1ae0
   140319e58:	83 7d 24 02          	cmp    DWORD PTR [rbp+0x24],0x2
   140319e5c:	72 39                	jb     0x140319e97
   140319e5e:	45 33 c0             	xor    r8d,r8d
   140319e61:	48 8b d0             	mov    rdx,rax
   140319e64:	48 8d 4c 24 30       	lea    rcx,[rsp+0x30]
   140319e69:	e8 f2 c7 d2 ff       	call   0x140046660
   140319e6e:	4c 8d 4d 74          	lea    r9,[rbp+0x74]
   140319e72:	4c 8d 45 34          	lea    r8,[rbp+0x34]
   140319e76:	8b 85 84 00 00 00    	mov    eax,DWORD PTR [rbp+0x84]
   140319e7c:	89 44 24 20          	mov    DWORD PTR [rsp+0x20],eax
   140319e80:	48 8b 54 24 30       	mov    rdx,QWORD PTR [rsp+0x30]
   140319e85:	48 8b ce             	mov    rcx,rsi
   140319e88:	e8 a3 c9 ff ff       	call   0x140316830
   140319e8d:	48 8d 4c 24 30       	lea    rcx,[rsp+0x30]
   140319e92:	e8 99 4b f3 ff       	call   0x14024ea30
   140319e97:	83 be 94 5c 01 00 00 	cmp    DWORD PTR [rsi+0x15c94],0x0
   140319e9e:	75 13                	jne    0x140319eb3
   140319ea0:	83 be 90 5c 01 00 01 	cmp    DWORD PTR [rsi+0x15c90],0x1
   140319ea7:	75 0a                	jne    0x140319eb3
   140319ea9:	48 83 be 88 5c 01 00 	cmp    QWORD PTR [rsi+0x15c88],0xe
   140319eb0:	0e 
   140319eb1:	74 39                	je     0x140319eec
   140319eb3:	33 d2                	xor    edx,edx
   140319eb5:	48 8b ce             	mov    rcx,rsi
   140319eb8:	e8 c3 bf ff ff       	call   0x140315e80
   140319ebd:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319ec4:	48 89 86 98 5c 01 00 	mov    QWORD PTR [rsi+0x15c98],rax
   140319ecb:	48 83 c0 10          	add    rax,0x10
   140319ecf:	48 89 86 b0 5c 01 00 	mov    QWORD PTR [rsi+0x15cb0],rax
   140319ed6:	48 c7 86 90 5c 01 00 	mov    QWORD PTR [rsi+0x15c90],0x1
   140319edd:	01 00 00 00 
   140319ee1:	48 c7 86 88 5c 01 00 	mov    QWORD PTR [rsi+0x15c88],0xe
   140319ee8:	0e 00 00 00 
   140319eec:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319ef3:	48 c7 00 0c 00 00 00 	mov    QWORD PTR [rax],0xc
   140319efa:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319f01:	48 c7 40 08 63 00 00 	mov    QWORD PTR [rax+0x8],0x63
   140319f08:	00 
   140319f09:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319f10:	10 
   140319f11:	8b 95 9c 00 00 00    	mov    edx,DWORD PTR [rbp+0x9c]
   140319f17:	48 8d 0d ea fb 9b 00 	lea    rcx,[rip+0x9bfbea]        # 0x140cd9b08
   140319f1e:	e8 ad 80 ff ff       	call   0x140311fd0
   140319f23:	33 c0                	xor    eax,eax
   140319f25:	48 89 44 24 48       	mov    QWORD PTR [rsp+0x48],rax
   140319f2a:	48 89 44 24 50       	mov    QWORD PTR [rsp+0x50],rax
   140319f2f:	45 33 c0             	xor    r8d,r8d
   140319f32:	48 8b 15 9f 7b 9b 00 	mov    rdx,QWORD PTR [rip+0x9b7b9f]        # 0x140cd1ad8
   140319f39:	48 8d 4c 24 48       	lea    rcx,[rsp+0x48]
   140319f3e:	e8 1d c7 d2 ff       	call   0x140046660
   140319f43:	90                   	nop
   140319f44:	48 8b 5c 24 48       	mov    rbx,QWORD PTR [rsp+0x48]
   140319f49:	bf 00 02 00 00       	mov    edi,0x200
   140319f4e:	66 90                	xchg   ax,ax
   140319f50:	48 8d 0d b1 fb 9b 00 	lea    rcx,[rip+0x9bfbb1]        # 0x140cd9b08
   140319f57:	e8 34 f4 d3 ff       	call   0x140059390
   140319f5c:	0f b6 c8             	movzx  ecx,al
   140319f5f:	8b c1                	mov    eax,ecx
   140319f61:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319f66:	c1 e0 08             	shl    eax,0x8
   140319f69:	0b c1                	or     eax,ecx
   140319f6b:	c1 e0 08             	shl    eax,0x8
   140319f6e:	0b c1                	or     eax,ecx
   140319f70:	89 03                	mov    DWORD PTR [rbx],eax
   140319f72:	48 8d 0d 8f fb 9b 00 	lea    rcx,[rip+0x9bfb8f]        # 0x140cd9b08
   140319f79:	e8 12 f4 d3 ff       	call   0x140059390
   140319f7e:	0f b6 c8             	movzx  ecx,al
   140319f81:	8b c1                	mov    eax,ecx
   140319f83:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319f88:	c1 e0 08             	shl    eax,0x8
   140319f8b:	0b c1                	or     eax,ecx
   140319f8d:	c1 e0 08             	shl    eax,0x8
   140319f90:	0b c1                	or     eax,ecx
   140319f92:	89 43 04             	mov    DWORD PTR [rbx+0x4],eax
   140319f95:	48 8d 0d 6c fb 9b 00 	lea    rcx,[rip+0x9bfb6c]        # 0x140cd9b08
   140319f9c:	e8 ef f3 d3 ff       	call   0x140059390
   140319fa1:	0f b6 c8             	movzx  ecx,al
   140319fa4:	8b c1                	mov    eax,ecx
   140319fa6:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319fab:	c1 e0 08             	shl    eax,0x8
   140319fae:	0b c1                	or     eax,ecx
   140319fb0:	c1 e0 08             	shl    eax,0x8
   140319fb3:	0b c1                	or     eax,ecx
   140319fb5:	89 43 08             	mov    DWORD PTR [rbx+0x8],eax
   140319fb8:	48 8d 0d 49 fb 9b 00 	lea    rcx,[rip+0x9bfb49]        # 0x140cd9b08
   140319fbf:	e8 cc f3 d3 ff       	call   0x140059390
   140319fc4:	0f b6 c8             	movzx  ecx,al
   140319fc7:	8b c1                	mov    eax,ecx
   140319fc9:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319fce:	c1 e0 08             	shl    eax,0x8
   140319fd1:	0b c1                	or     eax,ecx
   140319fd3:	c1 e0 08             	shl    eax,0x8
   140319fd6:	0b c1                	or     eax,ecx
   140319fd8:	89 43 0c             	mov    DWORD PTR [rbx+0xc],eax
   140319fdb:	48 8d 5b 10          	lea    rbx,[rbx+0x10]
   140319fdf:	48 83 ef 01          	sub    rdi,0x1
   140319fe3:	0f 85 67 ff ff ff    	jne    0x140319f50
   140319fe9:	81 7d 2c c8 00 00 00 	cmp    DWORD PTR [rbp+0x2c],0xc8
   140319ff0:	76 07                	jbe    0x140319ff9
   140319ff2:	c7 45 2c 64 00 00 00 	mov    DWORD PTR [rbp+0x2c],0x64
   140319ff9:	81 7d 30 c8 00 00 00 	cmp    DWORD PTR [rbp+0x30],0xc8
   14031a000:	76 07                	jbe    0x14031a009
   14031a002:	c7 45 30 64 00 00 00 	mov    DWORD PTR [rbp+0x30],0x64
   14031a009:	8b 4d 2c             	mov    ecx,DWORD PTR [rbp+0x2c]
   14031a00c:	c1 e1 06             	shl    ecx,0x6
   14031a00f:	b8 1f 85 eb 51       	mov    eax,0x51eb851f
   14031a014:	f7 e1                	mul    ecx
   14031a016:	44 8b e2             	mov    r12d,edx
   14031a019:	41 c1 ec 05          	shr    r12d,0x5
   14031a01d:	8b 4d 30             	mov    ecx,DWORD PTR [rbp+0x30]
   14031a020:	c1 e1 05             	shl    ecx,0x5
   14031a023:	b8 1f 85 eb 51       	mov    eax,0x51eb851f
   14031a028:	f7 e1                	mul    ecx
   14031a02a:	44 8b fa             	mov    r15d,edx
   14031a02d:	41 c1 ef 05          	shr    r15d,0x5
   14031a031:	48 8d 3d b8 7a 9b 00 	lea    rdi,[rip+0x9b7ab8]        # 0x140cd1af0
   14031a038:	45 8b f5             	mov    r14d,r13d
   14031a03b:	f3 44 0f 10 25 6c dc 	movss  xmm12,DWORD PTR [rip+0x1edc6c]        # 0x140507cb0
   14031a042:	1e 00 
   14031a044:	f3 44 0f 10 0d 1f 35 	movss  xmm9,DWORD PTR [rip+0x4351f]        # 0x14035d56c
   14031a04b:	04 00 
   14031a04d:	f3 44 0f 10 2d 9a 35 	movss  xmm13,DWORD PTR [rip+0x4359a]        # 0x14035d5f0
   14031a054:	04 00 
   14031a056:	f3 44 0f 10 35 cd 34 	movss  xmm14,DWORD PTR [rip+0x434cd]        # 0x14035d52c
   14031a05d:	04 00 
   14031a05f:	f3 44 0f 10 15 4c dc 	movss  xmm10,DWORD PTR [rip+0x1edc4c]        # 0x140507cb4
   14031a066:	1e 00 
   14031a068:	f3 44 0f 10 1d 97 35 	movss  xmm11,DWORD PTR [rip+0x43597]        # 0x14035d608
   14031a06f:	04 00 
   14031a071:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   14031a075:	66 66 66 0f 1f 84 00 	data16 data16 nop WORD PTR [rax+rax*1+0x0]
   14031a07c:	00 00 00 00 
   14031a080:	41 8b dd             	mov    ebx,r13d
   14031a083:	66 41 0f 6e fe       	movd   xmm7,r14d
   14031a088:	0f 5b ff             	cvtdq2ps xmm7,xmm7
   14031a08b:	f3 41 0f 59 fc       	mulss  xmm7,xmm12
   14031a090:	f3 41 0f 5c f9       	subss  xmm7,xmm9
   14031a095:	45 03 f4             	add    r14d,r12d
   14031a098:	41 8b c6             	mov    eax,r14d
   14031a09b:	45 0f 57 c0          	xorps  xmm8,xmm8
   14031a09f:	f3 4c 0f 2a c0       	cvtsi2ss xmm8,rax
   14031a0a4:	f3 45 0f 59 c4       	mulss  xmm8,xmm12
   14031a0a9:	f3 45 0f 5c c1       	subss  xmm8,xmm9
   14031a0ae:	66 90                	xchg   ax,ax
   14031a0b0:	48 8d 0d 51 fa 9b 00 	lea    rcx,[rip+0x9bfa51]        # 0x140cd9b08
   14031a0b7:	e8 d4 f2 d3 ff       	call   0x140059390
   14031a0bc:	83 e0 3f             	and    eax,0x3f
   14031a0bf:	0f 57 f6             	xorps  xmm6,xmm6
   14031a0c2:	f3 48 0f 2a f0       	cvtsi2ss xmm6,rax
   14031a0c7:	f3 41 0f 59 f5       	mulss  xmm6,xmm13
   14031a0cc:	48 8d 0d 35 fa 9b 00 	lea    rcx,[rip+0x9bfa35]        # 0x140cd9b08
   14031a0d3:	e8 b8 f2 d3 ff       	call   0x140059390
   14031a0d8:	83 e0 1f             	and    eax,0x1f
   14031a0db:	0f 57 d2             	xorps  xmm2,xmm2
   14031a0de:	f3 48 0f 2a d0       	cvtsi2ss xmm2,rax
   14031a0e3:	f3 41 0f 59 d6       	mulss  xmm2,xmm14
   14031a0e8:	f3 0f 11 3f          	movss  DWORD PTR [rdi],xmm7
   14031a0ec:	66 0f 6e c3          	movd   xmm0,ebx
   14031a0f0:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031a0f3:	f3 41 0f 59 c2       	mulss  xmm0,xmm10
   14031a0f8:	f3 41 0f 5c c1       	subss  xmm0,xmm9
   14031a0fd:	f3 0f 11 47 04       	movss  DWORD PTR [rdi+0x4],xmm0
   14031a102:	44 89 6f 08          	mov    DWORD PTR [rdi+0x8],r13d
   14031a106:	f3 0f 11 77 0c       	movss  DWORD PTR [rdi+0xc],xmm6
   14031a10b:	f3 0f 11 57 10       	movss  DWORD PTR [rdi+0x10],xmm2
   14031a110:	f3 44 0f 11 47 14    	movss  DWORD PTR [rdi+0x14],xmm8
   14031a116:	f3 0f 11 47 18       	movss  DWORD PTR [rdi+0x18],xmm0
   14031a11b:	44 89 6f 1c          	mov    DWORD PTR [rdi+0x1c],r13d
   14031a11f:	0f 28 ce             	movaps xmm1,xmm6
   14031a122:	f3 41 0f 58 cb       	addss  xmm1,xmm11
   14031a127:	f3 0f 11 4f 20       	movss  DWORD PTR [rdi+0x20],xmm1
   14031a12c:	f3 0f 11 57 24       	movss  DWORD PTR [rdi+0x24],xmm2
   14031a131:	f3 44 0f 11 47 28    	movss  DWORD PTR [rdi+0x28],xmm8
   14031a137:	41 03 df             	add    ebx,r15d
   14031a13a:	8b c3                	mov    eax,ebx
   14031a13c:	0f 57 c0             	xorps  xmm0,xmm0
   14031a13f:	f3 48 0f 2a c0       	cvtsi2ss xmm0,rax
   14031a144:	f3 41 0f 59 c2       	mulss  xmm0,xmm10
   14031a149:	f3 41 0f 5c c1       	subss  xmm0,xmm9
   14031a14e:	f3 0f 11 47 2c       	movss  DWORD PTR [rdi+0x2c],xmm0
   14031a153:	44 89 6f 30          	mov    DWORD PTR [rdi+0x30],r13d
   14031a157:	f3 0f 11 4f 34       	movss  DWORD PTR [rdi+0x34],xmm1
   14031a15c:	f3 41 0f 58 d3       	addss  xmm2,xmm11
   14031a161:	f3 0f 11 57 38       	movss  DWORD PTR [rdi+0x38],xmm2
   14031a166:	f3 0f 11 7f 3c       	movss  DWORD PTR [rdi+0x3c],xmm7
   14031a16b:	f3 0f 11 47 40       	movss  DWORD PTR [rdi+0x40],xmm0
   14031a170:	44 89 6f 44          	mov    DWORD PTR [rdi+0x44],r13d
   14031a174:	f3 0f 11 77 48       	movss  DWORD PTR [rdi+0x48],xmm6
   14031a179:	f3 0f 11 57 4c       	movss  DWORD PTR [rdi+0x4c],xmm2
   14031a17e:	48 83 c7 50          	add    rdi,0x50
   14031a182:	81 fb 68 01 00 00    	cmp    ebx,0x168
   14031a188:	0f 8c 22 ff ff ff    	jl     0x14031a0b0
   14031a18e:	41 81 fe 80 02 00 00 	cmp    r14d,0x280
   14031a195:	0f 8c e5 fe ff ff    	jl     0x14031a080
   14031a19b:	83 be 94 5c 01 00 00 	cmp    DWORD PTR [rsi+0x15c94],0x0
   14031a1a2:	75 13                	jne    0x14031a1b7
   14031a1a4:	83 be 90 5c 01 00 01 	cmp    DWORD PTR [rsi+0x15c90],0x1
   14031a1ab:	75 0a                	jne    0x14031a1b7
   14031a1ad:	48 83 be 88 5c 01 00 	cmp    QWORD PTR [rsi+0x15c88],0xe
   14031a1b4:	0e 
   14031a1b5:	74 39                	je     0x14031a1f0
   14031a1b7:	33 d2                	xor    edx,edx
   14031a1b9:	48 8b ce             	mov    rcx,rsi
   14031a1bc:	e8 bf bc ff ff       	call   0x140315e80
   14031a1c1:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a1c8:	48 89 86 98 5c 01 00 	mov    QWORD PTR [rsi+0x15c98],rax
   14031a1cf:	48 83 c0 10          	add    rax,0x10
   14031a1d3:	48 89 86 b0 5c 01 00 	mov    QWORD PTR [rsi+0x15cb0],rax
   14031a1da:	48 c7 86 90 5c 01 00 	mov    QWORD PTR [rsi+0x15c90],0x1
   14031a1e1:	01 00 00 00 
   14031a1e5:	48 c7 86 88 5c 01 00 	mov    QWORD PTR [rsi+0x15c88],0xe
   14031a1ec:	0e 00 00 00 
   14031a1f0:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a1f7:	48 c7 00 0c 00 00 00 	mov    QWORD PTR [rax],0xc
   14031a1fe:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a205:	48 c7 40 08 63 00 00 	mov    QWORD PTR [rax+0x8],0x63
   14031a20c:	00 
   14031a20d:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   14031a214:	10 
   14031a215:	48 8b 0d c4 78 9b 00 	mov    rcx,QWORD PTR [rip+0x9b78c4]        # 0x140cd1ae0
   14031a21c:	e8 af 96 d1 ff       	call   0x1400338d0
   14031a221:	8b d8                	mov    ebx,eax
   14031a223:	48 8b 0d ae 78 9b 00 	mov    rcx,QWORD PTR [rip+0x9b78ae]        # 0x140cd1ad8
   14031a22a:	e8 81 c8 d2 ff       	call   0x140046ab0
   14031a22f:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   14031a236:	89 01                	mov    DWORD PTR [rcx],eax
   14031a238:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   14031a23f:	89 59 04             	mov    DWORD PTR [rcx+0x4],ebx
   14031a242:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a249:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   14031a250:	00 
   14031a251:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   14031a258:	10 
   14031a259:	48 8d 1d 90 78 9b 00 	lea    rbx,[rip+0x9b7890]        # 0x140cd1af0
   14031a260:	48 8b cb             	mov    rcx,rbx
   14031a263:	e8 58 94 d1 ff       	call   0x1400336c0
   14031a268:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   14031a26f:	89 01                	mov    DWORD PTR [rcx],eax
   14031a271:	2b fb                	sub    edi,ebx
   14031a273:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a27a:	89 78 04             	mov    DWORD PTR [rax+0x4],edi
   14031a27d:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a284:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   14031a28b:	00 
   14031a28c:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   14031a293:	10 
   14031a294:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   14031a29b:	8b 55 28             	mov    edx,DWORD PTR [rbp+0x28]
   14031a29e:	8b 45 24             	mov    eax,DWORD PTR [rbp+0x24]
   14031a2a1:	89 01                	mov    DWORD PTR [rcx],eax
   14031a2a3:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a2aa:	89 50 04             	mov    DWORD PTR [rax+0x4],edx
   14031a2ad:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a2b4:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   14031a2bb:	00 
   14031a2bc:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   14031a2c3:	10 
   14031a2c4:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a2cb:	48 c7 00 0c 00 00 00 	mov    QWORD PTR [rax],0xc
   14031a2d2:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   14031a2d9:	48 c7 40 08 65 00 00 	mov    QWORD PTR [rax+0x8],0x65
   14031a2e0:	00 
   14031a2e1:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   14031a2e8:	10 
   14031a2e9:	48 8d 4c 24 48       	lea    rcx,[rsp+0x48]
   14031a2ee:	e8 3d 47 f3 ff       	call   0x14024ea30
   14031a2f3:	48 8b 4c 24 58       	mov    rcx,QWORD PTR [rsp+0x58]
   14031a2f8:	48 33 cc             	xor    rcx,rsp
   14031a2fb:	e8 f0 b2 02 00       	call   0x1403455f0
   14031a300:	4c 8d 9c 24 f0 00 00 	lea    r11,[rsp+0xf0]
   14031a307:	00 
   14031a308:	49 8b 5b 50          	mov    rbx,QWORD PTR [r11+0x50]
   14031a30c:	41 0f 28 73 f0       	movaps xmm6,XMMWORD PTR [r11-0x10]
   14031a311:	41 0f 28 7b e0       	movaps xmm7,XMMWORD PTR [r11-0x20]
   14031a316:	45 0f 28 43 d0       	movaps xmm8,XMMWORD PTR [r11-0x30]
   14031a31b:	45 0f 28 4b c0       	movaps xmm9,XMMWORD PTR [r11-0x40]
   14031a320:	45 0f 28 53 b0       	movaps xmm10,XMMWORD PTR [r11-0x50]
   14031a325:	45 0f 28 5b a0       	movaps xmm11,XMMWORD PTR [r11-0x60]
   14031a32a:	45 0f 28 63 90       	movaps xmm12,XMMWORD PTR [r11-0x70]
   14031a32f:	45 0f 28 6b 80       	movaps xmm13,XMMWORD PTR [r11-0x80]
   14031a334:	44 0f 28 74 24 60    	movaps xmm14,XMMWORD PTR [rsp+0x60]
   14031a33a:	49 8b e3             	mov    rsp,r11
   14031a33d:	41 5f                	pop    r15
   14031a33f:	41 5e                	pop    r14
   14031a341:	41 5d                	pop    r13
   14031a343:	41 5c                	pop    r12
   14031a345:	5f                   	pop    rdi
   14031a346:	5e                   	pop    rsi
   14031a347:	5d                   	pop    rbp
   14031a348:	c3                   	ret
