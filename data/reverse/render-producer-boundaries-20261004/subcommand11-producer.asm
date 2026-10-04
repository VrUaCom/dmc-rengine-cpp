
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140318ec0 <.text+0x317ec0>:
   140318ec0:	40 53                	rex push rbx
   140318ec2:	55                   	push   rbp
   140318ec3:	56                   	push   rsi
   140318ec4:	57                   	push   rdi
   140318ec5:	41 54                	push   r12
   140318ec7:	41 55                	push   r13
   140318ec9:	41 56                	push   r14
   140318ecb:	41 57                	push   r15
   140318ecd:	48 81 ec f8 00 00 00 	sub    rsp,0xf8
   140318ed4:	83 b9 94 5c 01 00 00 	cmp    DWORD PTR [rcx+0x15c94],0x0
   140318edb:	48 8b ea             	mov    rbp,rdx
   140318ede:	44 0f 29 8c 24 b0 00 	movaps XMMWORD PTR [rsp+0xb0],xmm9
   140318ee5:	00 00 
   140318ee7:	48 8b f9             	mov    rdi,rcx
   140318eea:	44 0f 29 94 24 a0 00 	movaps XMMWORD PTR [rsp+0xa0],xmm10
   140318ef1:	00 00 
   140318ef3:	be 01 00 00 00       	mov    esi,0x1
   140318ef8:	75 12                	jne    0x140318f0c
   140318efa:	39 b1 90 5c 01 00    	cmp    DWORD PTR [rcx+0x15c90],esi
   140318f00:	75 0a                	jne    0x140318f0c
   140318f02:	48 83 b9 88 5c 01 00 	cmp    QWORD PTR [rcx+0x15c88],0xe
   140318f09:	0e 
   140318f0a:	74 32                	je     0x140318f3e
   140318f0c:	33 d2                	xor    edx,edx
   140318f0e:	e8 6d cf ff ff       	call   0x140315e80
   140318f13:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140318f1a:	48 89 87 98 5c 01 00 	mov    QWORD PTR [rdi+0x15c98],rax
   140318f21:	48 83 c0 10          	add    rax,0x10
   140318f25:	48 89 87 b0 5c 01 00 	mov    QWORD PTR [rdi+0x15cb0],rax
   140318f2c:	48 89 b7 90 5c 01 00 	mov    QWORD PTR [rdi+0x15c90],rsi
   140318f33:	48 c7 87 88 5c 01 00 	mov    QWORD PTR [rdi+0x15c88],0xe
   140318f3a:	0e 00 00 00 
   140318f3e:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140318f45:	48 8b cf             	mov    rcx,rdi
   140318f48:	48 c7 00 0b 00 00 00 	mov    QWORD PTR [rax],0xb
   140318f4f:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140318f56:	48 c7 40 08 63 00 00 	mov    QWORD PTR [rax+0x8],0x63
   140318f5d:	00 
   140318f5e:	48 83 87 b0 5c 01 00 	add    QWORD PTR [rdi+0x15cb0],0x10
   140318f65:	10 
   140318f66:	f3 0f 10 4d 2c       	movss  xmm1,DWORD PTR [rbp+0x2c]
   140318f6b:	e8 00 de ff ff       	call   0x140316d70
   140318f70:	f3 0f 10 4d 30       	movss  xmm1,DWORD PTR [rbp+0x30]
   140318f75:	48 8b cf             	mov    rcx,rdi
   140318f78:	44 0f 28 d0          	movaps xmm10,xmm0
   140318f7c:	e8 ef dd ff ff       	call   0x140316d70
   140318f81:	0f 57 c9             	xorps  xmm1,xmm1
   140318f84:	44 0f 28 c8          	movaps xmm9,xmm0
   140318f88:	f3 41 0f 5a ca       	cvtss2sd xmm1,xmm10
   140318f8d:	66 0f 2f 0d 2b ed 1e 	comisd xmm1,QWORD PTR [rip+0x1eed2b]        # 0x140507cc0
   140318f94:	00 
   140318f95:	76 09                	jbe    0x140318fa0
   140318f97:	f3 44 0f 10 15 18 ed 	movss  xmm10,DWORD PTR [rip+0x1eed18]        # 0x140507cb8
   140318f9e:	1e 00 
   140318fa0:	41 0f 2f c2          	comiss xmm0,xmm10
   140318fa4:	76 0d                	jbe    0x140318fb3
   140318fa6:	45 0f 28 ca          	movaps xmm9,xmm10
   140318faa:	f3 44 0f 59 0d 79 45 	mulss  xmm9,DWORD PTR [rip+0x44579]        # 0x14035d52c
   140318fb1:	04 00 
   140318fb3:	8b 4d 58             	mov    ecx,DWORD PTR [rbp+0x58]
   140318fb6:	44 8b ee             	mov    r13d,esi
   140318fb9:	8b 45 20             	mov    eax,DWORD PTR [rbp+0x20]
   140318fbc:	01 45 70             	add    DWORD PTR [rbp+0x70],eax
   140318fbf:	8b 45 24             	mov    eax,DWORD PTR [rbp+0x24]
   140318fc2:	01 45 74             	add    DWORD PTR [rbp+0x74],eax
   140318fc5:	44 8b 7d 70          	mov    r15d,DWORD PTR [rbp+0x70]
   140318fc9:	44 8b 75 74          	mov    r14d,DWORD PTR [rbp+0x74]
   140318fcd:	41 d3 e5             	shl    r13d,cl
   140318fd0:	8b 4d 5c             	mov    ecx,DWORD PTR [rbp+0x5c]
   140318fd3:	d3 e6                	shl    esi,cl
   140318fd5:	41 8b cd             	mov    ecx,r13d
   140318fd8:	41 c1 ff 04          	sar    r15d,0x4
   140318fdc:	41 8d 45 ff          	lea    eax,[r13-0x1]
   140318fe0:	41 c1 fe 04          	sar    r14d,0x4
   140318fe4:	44 23 f8             	and    r15d,eax
   140318fe7:	8d 46 ff             	lea    eax,[rsi-0x1]
   140318fea:	44 89 7c 24 34       	mov    DWORD PTR [rsp+0x34],r15d
   140318fef:	44 23 f0             	and    r14d,eax
   140318ff2:	e8 79 80 01 00       	call   0x140331070
   140318ff7:	8d 48 04             	lea    ecx,[rax+0x4]
   140318ffa:	8b 45 70             	mov    eax,DWORD PTR [rbp+0x70]
   140318ffd:	d3 f8                	sar    eax,cl
   140318fff:	8b ce                	mov    ecx,esi
   140319001:	89 44 24 28          	mov    DWORD PTR [rsp+0x28],eax
   140319005:	e8 66 80 01 00       	call   0x140331070
   14031900a:	8b 55 74             	mov    edx,DWORD PTR [rbp+0x74]
   14031900d:	4c 8d 25 bc 8a 9a 00 	lea    r12,[rip+0x9a8abc]        # 0x140cc1ad0
   140319014:	44 8b d6             	mov    r10d,esi
   140319017:	49 8b dc             	mov    rbx,r12
   14031901a:	41 f7 da             	neg    r10d
   14031901d:	8d 48 04             	lea    ecx,[rax+0x4]
   140319020:	44 89 94 24 50 01 00 	mov    DWORD PTR [rsp+0x150],r10d
   140319027:	00 
   140319028:	d3 fa                	sar    edx,cl
   14031902a:	44 3b 97 5c 5d 01 00 	cmp    r10d,DWORD PTR [rdi+0x15d5c]
   140319031:	0f 8f a1 04 00 00    	jg     0x1403194d8
   140319037:	4c 8b 1d c2 42 a5 00 	mov    r11,QWORD PTR [rip+0xa542c2]        # 0x140d6d300
   14031903e:	4c 8d 0d 4b 7a 2b 00 	lea    r9,[rip+0x2b7a4b]        # 0x1405d0a90
   140319045:	f3 0f 10 25 e7 50 2c 	movss  xmm4,DWORD PTR [rip+0x2c50e7]        # 0x1405de134
   14031904c:	00 
   14031904d:	45 03 f2             	add    r14d,r10d
   140319050:	f3 0f 10 2d d8 50 2c 	movss  xmm5,DWORD PTR [rip+0x2c50d8]        # 0x1405de130
   140319057:	00 
   140319058:	41 8b c5             	mov    eax,r13d
   14031905b:	f3 0f 10 1d fd 44 04 	movss  xmm3,DWORD PTR [rip+0x444fd]        # 0x14035d560
   140319062:	00 
   140319063:	f7 d8                	neg    eax
   140319065:	0f 29 b4 24 e0 00 00 	movaps XMMWORD PTR [rsp+0xe0],xmm6
   14031906c:	00 
   14031906d:	44 8b e6             	mov    r12d,esi
   140319070:	f3 0f 10 35 dc 51 2c 	movss  xmm6,DWORD PTR [rip+0x2c51dc]        # 0x1405de254
   140319077:	00 
   140319078:	0f 29 bc 24 d0 00 00 	movaps XMMWORD PTR [rsp+0xd0],xmm7
   14031907f:	00 
   140319080:	f3 0f 10 3d c8 51 2c 	movss  xmm7,DWORD PTR [rip+0x2c51c8]        # 0x1405de250
   140319087:	00 
   140319088:	44 0f 29 84 24 c0 00 	movaps XMMWORD PTR [rsp+0xc0],xmm8
   14031908f:	00 00 
   140319091:	f3 44 0f 10 05 36 ec 	movss  xmm8,DWORD PTR [rip+0x1eec36]        # 0x140507cd0
   140319098:	1e 00 
   14031909a:	44 0f 29 9c 24 90 00 	movaps XMMWORD PTR [rsp+0x90],xmm11
   1403190a1:	00 00 
   1403190a3:	f3 44 0f 10 1d 5c 45 	movss  xmm11,DWORD PTR [rip+0x4455c]        # 0x14035d608
   1403190aa:	04 00 
   1403190ac:	44 0f 29 a4 24 80 00 	movaps XMMWORD PTR [rsp+0x80],xmm12
   1403190b3:	00 00 
   1403190b5:	f3 44 0f 10 25 12 8a 	movss  xmm12,DWORD PTR [rip+0x9b8a12]        # 0x140cd1ad0
   1403190bc:	9b 00 
   1403190be:	89 44 24 24          	mov    DWORD PTR [rsp+0x24],eax
   1403190c2:	83 c8 ff             	or     eax,0xffffffff
   1403190c5:	2b c2                	sub    eax,edx
   1403190c7:	44 0f 29 6c 24 70    	movaps XMMWORD PTR [rsp+0x70],xmm13
   1403190cd:	f3 44 0f 10 2d 96 44 	movss  xmm13,DWORD PTR [rip+0x44496]        # 0x14035d56c
   1403190d4:	04 00 
   1403190d6:	41 8b d6             	mov    edx,r14d
   1403190d9:	c1 e2 04             	shl    edx,0x4
   1403190dc:	41 c1 e4 04          	shl    r12d,0x4
   1403190e0:	44 0f 29 74 24 60    	movaps XMMWORD PTR [rsp+0x60],xmm14
   1403190e6:	f3 44 0f 10 35 8d 79 	movss  xmm14,DWORD PTR [rip+0x2b798d]        # 0x1405d0a7c
   1403190ed:	2b 00 
   1403190ef:	44 89 64 24 38       	mov    DWORD PTR [rsp+0x38],r12d
   1403190f4:	89 44 24 20          	mov    DWORD PTR [rsp+0x20],eax
   1403190f8:	89 94 24 58 01 00 00 	mov    DWORD PTR [rsp+0x158],edx
   1403190ff:	90                   	nop
   140319100:	8b c8                	mov    ecx,eax
   140319102:	48 8d 05 87 79 2b 00 	lea    rax,[rip+0x2b7987]        # 0x1405d0a90
   140319109:	83 e1 1f             	and    ecx,0x1f
   14031910c:	44 8b c1             	mov    r8d,ecx
   14031910f:	41 c1 e0 05          	shl    r8d,0x5
   140319113:	4d 03 c1             	add    r8,r9
   140319116:	44 8d 49 01          	lea    r9d,[rcx+0x1]
   14031911a:	4c 89 44 24 40       	mov    QWORD PTR [rsp+0x40],r8
   14031911f:	41 83 e1 1f          	and    r9d,0x1f
   140319123:	41 c1 e1 05          	shl    r9d,0x5
   140319127:	4c 03 c8             	add    r9,rax
   14031912a:	8d 41 02             	lea    eax,[rcx+0x2]
   14031912d:	83 e0 1f             	and    eax,0x1f
   140319130:	4c 89 4c 24 48       	mov    QWORD PTR [rsp+0x48],r9
   140319135:	c1 e0 05             	shl    eax,0x5
   140319138:	48 8d 0d 51 79 2b 00 	lea    rcx,[rip+0x2b7951]        # 0x1405d0a90
   14031913f:	48 03 c1             	add    rax,rcx
   140319142:	48 89 44 24 50       	mov    QWORD PTR [rsp+0x50],rax
   140319147:	8b 44 24 24          	mov    eax,DWORD PTR [rsp+0x24]
   14031914b:	89 84 24 48 01 00 00 	mov    DWORD PTR [rsp+0x148],eax
   140319152:	3b 87 58 5d 01 00    	cmp    eax,DWORD PTR [rdi+0x15d58]
   140319158:	0f 8f 48 02 00 00    	jg     0x1403193a6
   14031915e:	41 8d 0c 14          	lea    ecx,[r12+rdx*1]
   140319162:	41 83 cc ff          	or     r12d,0xffffffff
   140319166:	44 2b 64 24 28       	sub    r12d,DWORD PTR [rsp+0x28]
   14031916b:	89 4c 24 2c          	mov    DWORD PTR [rsp+0x2c],ecx
   14031916f:	42 8d 0c 38          	lea    ecx,[rax+r15*1]
   140319173:	44 8b f9             	mov    r15d,ecx
   140319176:	89 8c 24 40 01 00 00 	mov    DWORD PTR [rsp+0x140],ecx
   14031917d:	41 c1 e7 04          	shl    r15d,0x4
   140319181:	41 8b c5             	mov    eax,r13d
   140319184:	c1 e0 04             	shl    eax,0x4
   140319187:	89 44 24 30          	mov    DWORD PTR [rsp+0x30],eax
   14031918b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]
   140319190:	41 8b c4             	mov    eax,r12d
   140319193:	41 0f 28 cd          	movaps xmm1,xmm13
   140319197:	83 e0 1f             	and    eax,0x1f
   14031919a:	41 0f 28 d5          	movaps xmm2,xmm13
   14031919e:	44 8b d0             	mov    r10d,eax
   1403191a1:	42 0f b6 14 00       	movzx  edx,BYTE PTR [rax+r8*1]
   1403191a6:	46 0f b6 0c 08       	movzx  r9d,BYTE PTR [rax+r9*1]
   1403191ab:	45 8d 42 01          	lea    r8d,[r10+0x1]
   1403191af:	8b 87 58 5d 01 00    	mov    eax,DWORD PTR [rdi+0x15d58]
   1403191b5:	41 83 e0 1f          	and    r8d,0x1f
   1403191b9:	c1 e0 04             	shl    eax,0x4
   1403191bc:	66 0f 6e c0          	movd   xmm0,eax
   1403191c0:	8b 87 5c 5d 01 00    	mov    eax,DWORD PTR [rdi+0x15d5c]
   1403191c6:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403191c9:	c1 e0 04             	shl    eax,0x4
   1403191cc:	f3 0f 5e c8          	divss  xmm1,xmm0
   1403191d0:	66 0f 6e c0          	movd   xmm0,eax
   1403191d4:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   1403191d9:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403191dc:	03 c1                	add    eax,ecx
   1403191de:	8b 0d 9c 78 2b 00    	mov    ecx,DWORD PTR [rip+0x2b789c]        # 0x1405d0a80
   1403191e4:	c1 e0 04             	shl    eax,0x4
   1403191e7:	f3 0f 5e d0          	divss  xmm2,xmm0
   1403191eb:	66 0f 6e c0          	movd   xmm0,eax
   1403191ef:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403191f2:	f3 41 0f 59 d6       	mulss  xmm2,xmm14
   1403191f7:	f3 0f 59 c3          	mulss  xmm0,xmm3
   1403191fb:	f3 0f 58 c5          	addss  xmm0,xmm5
   1403191ff:	f3 0f 59 c7          	mulss  xmm0,xmm7
   140319203:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   140319208:	f3 0f 11 03          	movss  DWORD PTR [rbx],xmm0
   14031920c:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   140319211:	41 03 c6             	add    eax,r14d
   140319214:	c1 e0 04             	shl    eax,0x4
   140319217:	66 0f 6e c0          	movd   xmm0,eax
   14031921b:	48 8b 44 24 40       	mov    rax,QWORD PTR [rsp+0x40]
   140319220:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319223:	41 0f b6 04 00       	movzx  eax,BYTE PTR [r8+rax*1]
   140319228:	2b c2                	sub    eax,edx
   14031922a:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031922e:	f3 0f 58 c4          	addss  xmm0,xmm4
   140319232:	f3 0f 59 c6          	mulss  xmm0,xmm6
   140319236:	f3 0f 11 43 04       	movss  DWORD PTR [rbx+0x4],xmm0
   14031923b:	c7 43 08 00 00 80 3f 	mov    DWORD PTR [rbx+0x8],0x3f800000
   140319242:	0f af 45 18          	imul   eax,DWORD PTR [rbp+0x18]
   140319246:	d3 f8                	sar    eax,cl
   140319248:	83 c0 08             	add    eax,0x8
   14031924b:	41 03 c7             	add    eax,r15d
   14031924e:	66 0f 6e c0          	movd   xmm0,eax
   140319252:	41 8b c1             	mov    eax,r9d
   140319255:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319258:	2b c2                	sub    eax,edx
   14031925a:	8b 94 24 58 01 00 00 	mov    edx,DWORD PTR [rsp+0x158]
   140319261:	f3 0f 59 c1          	mulss  xmm0,xmm1
   140319265:	f3 41 0f 59 c3       	mulss  xmm0,xmm11
   14031926a:	f3 0f 11 43 0c       	movss  DWORD PTR [rbx+0xc],xmm0
   14031926f:	0f af 45 1c          	imul   eax,DWORD PTR [rbp+0x1c]
   140319273:	d3 f8                	sar    eax,cl
   140319275:	83 c0 08             	add    eax,0x8
   140319278:	03 c2                	add    eax,edx
   14031927a:	66 0f 6e c0          	movd   xmm0,eax
   14031927e:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319281:	f3 0f 59 c2          	mulss  xmm0,xmm2
   140319285:	f3 41 0f 59 c3       	mulss  xmm0,xmm11
   14031928a:	f3 41 0f 58 c4       	addss  xmm0,xmm12
   14031928f:	f3 0f 11 43 10       	movss  DWORD PTR [rbx+0x10],xmm0
   140319294:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   140319299:	03 84 24 40 01 00 00 	add    eax,DWORD PTR [rsp+0x140]
   1403192a0:	c1 e0 04             	shl    eax,0x4
   1403192a3:	66 0f 6e c0          	movd   xmm0,eax
   1403192a7:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403192aa:	f3 0f 59 c3          	mulss  xmm0,xmm3
   1403192ae:	f3 0f 58 c5          	addss  xmm0,xmm5
   1403192b2:	f3 0f 59 c7          	mulss  xmm0,xmm7
   1403192b6:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   1403192bb:	f3 0f 11 43 14       	movss  DWORD PTR [rbx+0x14],xmm0
   1403192c0:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   1403192c5:	41 03 c6             	add    eax,r14d
   1403192c8:	03 c6                	add    eax,esi
   1403192ca:	c1 e0 04             	shl    eax,0x4
   1403192cd:	66 0f 6e c0          	movd   xmm0,eax
   1403192d1:	48 8b 44 24 48       	mov    rax,QWORD PTR [rsp+0x48]
   1403192d6:	41 ff c4             	inc    r12d
   1403192d9:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403192dc:	41 0f b6 04 00       	movzx  eax,BYTE PTR [r8+rax*1]
   1403192e1:	4c 8b 44 24 40       	mov    r8,QWORD PTR [rsp+0x40]
   1403192e6:	41 2b c1             	sub    eax,r9d
   1403192e9:	f3 0f 59 c3          	mulss  xmm0,xmm3
   1403192ed:	f3 0f 58 c4          	addss  xmm0,xmm4
   1403192f1:	f3 0f 59 c6          	mulss  xmm0,xmm6
   1403192f5:	f3 0f 11 43 18       	movss  DWORD PTR [rbx+0x18],xmm0
   1403192fa:	c7 43 1c 00 00 80 3f 	mov    DWORD PTR [rbx+0x1c],0x3f800000
   140319301:	0f af 45 18          	imul   eax,DWORD PTR [rbp+0x18]
   140319305:	d3 f8                	sar    eax,cl
   140319307:	83 c0 08             	add    eax,0x8
   14031930a:	41 03 c7             	add    eax,r15d
   14031930d:	44 03 7c 24 30       	add    r15d,DWORD PTR [rsp+0x30]
   140319312:	66 0f 6e c0          	movd   xmm0,eax
   140319316:	48 8b 44 24 50       	mov    rax,QWORD PTR [rsp+0x50]
   14031931b:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031931e:	41 0f b6 04 02       	movzx  eax,BYTE PTR [r10+rax*1]
   140319323:	41 2b c1             	sub    eax,r9d
   140319326:	4c 8b 4c 24 48       	mov    r9,QWORD PTR [rsp+0x48]
   14031932b:	f3 0f 59 c1          	mulss  xmm0,xmm1
   14031932f:	f3 41 0f 59 c3       	mulss  xmm0,xmm11
   140319334:	f3 0f 11 43 20       	movss  DWORD PTR [rbx+0x20],xmm0
   140319339:	0f af 45 1c          	imul   eax,DWORD PTR [rbp+0x1c]
   14031933d:	d3 f8                	sar    eax,cl
   14031933f:	8b 4c 24 2c          	mov    ecx,DWORD PTR [rsp+0x2c]
   140319343:	83 c1 09             	add    ecx,0x9
   140319346:	03 c1                	add    eax,ecx
   140319348:	8b 8c 24 40 01 00 00 	mov    ecx,DWORD PTR [rsp+0x140]
   14031934f:	41 03 cd             	add    ecx,r13d
   140319352:	89 8c 24 40 01 00 00 	mov    DWORD PTR [rsp+0x140],ecx
   140319359:	66 0f 6e c0          	movd   xmm0,eax
   14031935d:	8b 84 24 48 01 00 00 	mov    eax,DWORD PTR [rsp+0x148]
   140319364:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319367:	41 03 c5             	add    eax,r13d
   14031936a:	89 84 24 48 01 00 00 	mov    DWORD PTR [rsp+0x148],eax
   140319371:	f3 0f 59 c2          	mulss  xmm0,xmm2
   140319375:	f3 41 0f 59 c3       	mulss  xmm0,xmm11
   14031937a:	f3 41 0f 58 c4       	addss  xmm0,xmm12
   14031937f:	f3 0f 11 43 24       	movss  DWORD PTR [rbx+0x24],xmm0
   140319384:	48 83 c3 28          	add    rbx,0x28
   140319388:	3b 87 58 5d 01 00    	cmp    eax,DWORD PTR [rdi+0x15d58]
   14031938e:	0f 8e fc fd ff ff    	jle    0x140319190
   140319394:	44 8b 7c 24 34       	mov    r15d,DWORD PTR [rsp+0x34]
   140319399:	44 8b 94 24 50 01 00 	mov    r10d,DWORD PTR [rsp+0x150]
   1403193a0:	00 
   1403193a1:	44 8b 64 24 38       	mov    r12d,DWORD PTR [rsp+0x38]
   1403193a6:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   1403193ab:	4c 8d 0d de 76 2b 00 	lea    r9,[rip+0x2b76de]        # 0x1405d0a90
   1403193b2:	41 03 c7             	add    eax,r15d
   1403193b5:	33 c9                	xor    ecx,ecx
   1403193b7:	03 87 58 5d 01 00    	add    eax,DWORD PTR [rdi+0x15d58]
   1403193bd:	44 03 d6             	add    r10d,esi
   1403193c0:	c1 e0 04             	shl    eax,0x4
   1403193c3:	41 03 d4             	add    edx,r12d
   1403193c6:	44 89 94 24 50 01 00 	mov    DWORD PTR [rsp+0x150],r10d
   1403193cd:	00 
   1403193ce:	89 94 24 58 01 00 00 	mov    DWORD PTR [rsp+0x158],edx
   1403193d5:	66 0f 6e c0          	movd   xmm0,eax
   1403193d9:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403193dc:	f3 0f 59 c3          	mulss  xmm0,xmm3
   1403193e0:	f3 0f 58 c5          	addss  xmm0,xmm5
   1403193e4:	f3 0f 59 c7          	mulss  xmm0,xmm7
   1403193e8:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   1403193ed:	f3 0f 11 03          	movss  DWORD PTR [rbx],xmm0
   1403193f1:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   1403193f6:	41 03 c6             	add    eax,r14d
   1403193f9:	03 c6                	add    eax,esi
   1403193fb:	c1 e0 04             	shl    eax,0x4
   1403193fe:	66 0f 6e c0          	movd   xmm0,eax
   140319402:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319405:	f3 0f 59 c3          	mulss  xmm0,xmm3
   140319409:	f3 0f 58 c4          	addss  xmm0,xmm4
   14031940d:	f3 0f 59 c6          	mulss  xmm0,xmm6
   140319411:	f3 0f 11 43 04       	movss  DWORD PTR [rbx+0x4],xmm0
   140319416:	48 c7 43 08 00 00 80 	mov    QWORD PTR [rbx+0x8],0x3f800000
   14031941d:	3f 
   14031941e:	89 4b 10             	mov    DWORD PTR [rbx+0x10],ecx
   140319421:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   140319426:	41 2b c5             	sub    eax,r13d
   140319429:	41 03 c7             	add    eax,r15d
   14031942c:	c1 e0 04             	shl    eax,0x4
   14031942f:	66 0f 6e c0          	movd   xmm0,eax
   140319433:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319436:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031943a:	f3 0f 58 c5          	addss  xmm0,xmm5
   14031943e:	f3 0f 59 c7          	mulss  xmm0,xmm7
   140319442:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   140319447:	f3 0f 11 43 14       	movss  DWORD PTR [rbx+0x14],xmm0
   14031944c:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   140319451:	41 03 c6             	add    eax,r14d
   140319454:	44 03 f6             	add    r14d,esi
   140319457:	03 c6                	add    eax,esi
   140319459:	c1 e0 04             	shl    eax,0x4
   14031945c:	66 0f 6e c0          	movd   xmm0,eax
   140319460:	8b 44 24 20          	mov    eax,DWORD PTR [rsp+0x20]
   140319464:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319467:	ff c0                	inc    eax
   140319469:	89 44 24 20          	mov    DWORD PTR [rsp+0x20],eax
   14031946d:	f3 0f 59 c3          	mulss  xmm0,xmm3
   140319471:	f3 0f 58 c4          	addss  xmm0,xmm4
   140319475:	f3 0f 59 c6          	mulss  xmm0,xmm6
   140319479:	f3 0f 11 43 18       	movss  DWORD PTR [rbx+0x18],xmm0
   14031947e:	48 c7 43 1c 00 00 80 	mov    QWORD PTR [rbx+0x1c],0x3f800000
   140319485:	3f 
   140319486:	89 4b 24             	mov    DWORD PTR [rbx+0x24],ecx
   140319489:	48 83 c3 28          	add    rbx,0x28
   14031948d:	44 3b 97 5c 5d 01 00 	cmp    r10d,DWORD PTR [rdi+0x15d5c]
   140319494:	0f 8e 66 fc ff ff    	jle    0x140319100
   14031949a:	44 0f 28 74 24 60    	movaps xmm14,XMMWORD PTR [rsp+0x60]
   1403194a0:	4c 8d 25 29 86 9a 00 	lea    r12,[rip+0x9a8629]        # 0x140cc1ad0
   1403194a7:	44 0f 28 6c 24 70    	movaps xmm13,XMMWORD PTR [rsp+0x70]
   1403194ad:	44 0f 28 a4 24 80 00 	movaps xmm12,XMMWORD PTR [rsp+0x80]
   1403194b4:	00 00 
   1403194b6:	44 0f 28 9c 24 90 00 	movaps xmm11,XMMWORD PTR [rsp+0x90]
   1403194bd:	00 00 
   1403194bf:	44 0f 28 84 24 c0 00 	movaps xmm8,XMMWORD PTR [rsp+0xc0]
   1403194c6:	00 00 
   1403194c8:	0f 28 bc 24 d0 00 00 	movaps xmm7,XMMWORD PTR [rsp+0xd0]
   1403194cf:	00 
   1403194d0:	0f 28 b4 24 e0 00 00 	movaps xmm6,XMMWORD PTR [rsp+0xe0]
   1403194d7:	00 
   1403194d8:	49 8b cc             	mov    rcx,r12
   1403194db:	e8 e0 a1 d1 ff       	call   0x1400336c0
   1403194e0:	48 8b 8f b0 5c 01 00 	mov    rcx,QWORD PTR [rdi+0x15cb0]
   1403194e7:	4c 8d 9c 24 f8 00 00 	lea    r11,[rsp+0xf8]
   1403194ee:	00 
   1403194ef:	49 2b dc             	sub    rbx,r12
   1403194f2:	48 c1 fb 02          	sar    rbx,0x2
   1403194f6:	89 01                	mov    DWORD PTR [rcx],eax
   1403194f8:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   1403194ff:	89 58 04             	mov    DWORD PTR [rax+0x4],ebx
   140319502:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140319509:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319510:	00 
   140319511:	48 83 87 b0 5c 01 00 	add    QWORD PTR [rdi+0x15cb0],0x10
   140319518:	10 
   140319519:	8b 8f b0 5c 01 00    	mov    ecx,DWORD PTR [rdi+0x15cb0]
   14031951f:	44 8b 4d 28          	mov    r9d,DWORD PTR [rbp+0x28]
   140319523:	44 8b 45 60          	mov    r8d,DWORD PTR [rbp+0x60]
   140319527:	48 8b 97 b8 5c 01 00 	mov    rdx,QWORD PTR [rdi+0x15cb8]
   14031952e:	0f b7 42 5a          	movzx  eax,WORD PTR [rdx+0x5a]
   140319532:	2b 4a 50             	sub    ecx,DWORD PTR [rdx+0x50]
   140319535:	48 c1 f9 04          	sar    rcx,0x4
   140319539:	66 89 4c 42 5c       	mov    WORD PTR [rdx+rax*2+0x5c],cx
   14031953e:	48 8b 87 b8 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb8]
   140319545:	66 ff 40 5a          	inc    WORD PTR [rax+0x5a]
   140319549:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140319550:	44 89 00             	mov    DWORD PTR [rax],r8d
   140319553:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   14031955a:	44 89 48 04          	mov    DWORD PTR [rax+0x4],r9d
   14031955e:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140319565:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   14031956c:	00 
   14031956d:	48 83 87 b0 5c 01 00 	add    QWORD PTR [rdi+0x15cb0],0x10
   140319574:	10 
   140319575:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   14031957c:	f3 44 0f 11 10       	movss  DWORD PTR [rax],xmm10
   140319581:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   140319588:	45 0f 28 53 a8       	movaps xmm10,XMMWORD PTR [r11-0x58]
   14031958d:	f3 44 0f 11 48 04    	movss  DWORD PTR [rax+0x4],xmm9
   140319593:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   14031959a:	45 0f 28 4b b8       	movaps xmm9,XMMWORD PTR [r11-0x48]
   14031959f:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   1403195a6:	00 
   1403195a7:	48 83 87 b0 5c 01 00 	add    QWORD PTR [rdi+0x15cb0],0x10
   1403195ae:	10 
   1403195af:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   1403195b6:	48 c7 00 0b 00 00 00 	mov    QWORD PTR [rax],0xb
   1403195bd:	48 8b 87 b0 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb0]
   1403195c4:	48 c7 40 08 65 00 00 	mov    QWORD PTR [rax+0x8],0x65
   1403195cb:	00 
   1403195cc:	48 83 87 b0 5c 01 00 	add    QWORD PTR [rdi+0x15cb0],0x10
   1403195d3:	10 
   1403195d4:	49 8b e3             	mov    rsp,r11
   1403195d7:	41 5f                	pop    r15
   1403195d9:	41 5e                	pop    r14
   1403195db:	41 5d                	pop    r13
   1403195dd:	41 5c                	pop    r12
   1403195df:	5f                   	pop    rdi
   1403195e0:	5e                   	pop    rsi
   1403195e1:	5d                   	pop    rbp
   1403195e2:	5b                   	pop    rbx
   1403195e3:	c3                   	ret
