
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140032cc0 <.text+0x31cc0>:
   140032cc0:	f8                   	clc
   140032cc1:	2b 03                	sub    eax,DWORD PTR [rbx]
   140032cc3:	00 cc                	add    ah,cl
   140032cc5:	cc                   	int3
   140032cc6:	cc                   	int3
   140032cc7:	cc                   	int3
   140032cc8:	cc                   	int3
   140032cc9:	cc                   	int3
   140032cca:	cc                   	int3
   140032ccb:	cc                   	int3
   140032ccc:	cc                   	int3
   140032ccd:	cc                   	int3
   140032cce:	cc                   	int3
   140032ccf:	cc                   	int3
   140032cd0:	85 c9                	test   ecx,ecx
   140032cd2:	0f 84 8e 05 00 00    	je     0x140033266
   140032cd8:	48 8b c4             	mov    rax,rsp
   140032cdb:	89 48 08             	mov    DWORD PTR [rax+0x8],ecx
   140032cde:	53                   	push   rbx
   140032cdf:	41 55                	push   r13
   140032ce1:	48 83 ec 48          	sub    rsp,0x48
   140032ce5:	44 8b 15 74 ea 5a 00 	mov    r10d,DWORD PTR [rip+0x5aea74]        # 0x1405e1760
   140032cec:	8b d9                	mov    ebx,ecx
   140032cee:	44 8b 1d 0b ea 5a 00 	mov    r11d,DWORD PTR [rip+0x5aea0b]        # 0x1405e1700
   140032cf5:	4c 8b ea             	mov    r13,rdx
   140032cf8:	44 8b 05 f1 e9 5a 00 	mov    r8d,DWORD PTR [rip+0x5ae9f1]        # 0x1405e16f0
   140032cff:	48 b9 00 00 00 00 00 	movabs rcx,0x400000000000
   140032d06:	40 00 00 
   140032d09:	44 8b 0d e4 e9 5a 00 	mov    r9d,DWORD PTR [rip+0x5ae9e4]        # 0x1405e16f4
   140032d10:	48 89 68 10          	mov    QWORD PTR [rax+0x10],rbp
   140032d14:	48 89 70 e8          	mov    QWORD PTR [rax-0x18],rsi
   140032d18:	48 89 78 e0          	mov    QWORD PTR [rax-0x20],rdi
   140032d1c:	48 8d 3d dd d2 fc ff 	lea    rdi,[rip+0xfffffffffffcd2dd]        # 0x140000000
   140032d23:	4c 89 60 d8          	mov    QWORD PTR [rax-0x28],r12
   140032d27:	4c 89 70 d0          	mov    QWORD PTR [rax-0x30],r14
   140032d2b:	4c 89 78 c8          	mov    QWORD PTR [rax-0x38],r15
   140032d2f:	90                   	nop
   140032d30:	83 3d 15 ea 5a 00 00 	cmp    DWORD PTR [rip+0x5aea15],0x0        # 0x1405e174c
   140032d37:	0f 84 3b 02 00 00    	je     0x140032f78
   140032d3d:	83 3d b4 e9 5a 00 00 	cmp    DWORD PTR [rip+0x5ae9b4],0x0        # 0x1405e16f8
   140032d44:	76 34                	jbe    0x140032d7a
   140032d46:	48 8b 0d 03 67 5a 00 	mov    rcx,QWORD PTR [rip+0x5a6703]        # 0x1405d9450
   140032d4d:	49 8b d5             	mov    rdx,r13
   140032d50:	48 c1 e9 20          	shr    rcx,0x20
   140032d54:	81 e1 ff 3f 00 00    	and    ecx,0x3fff
   140032d5a:	e8 11 06 00 00       	call   0x140033370
   140032d5f:	8b 05 93 e9 5a 00    	mov    eax,DWORD PTR [rip+0x5ae993]        # 0x1405e16f8
   140032d65:	29 05 e1 e9 5a 00    	sub    DWORD PTR [rip+0x5ae9e1],eax        # 0x1405e174c
   140032d6b:	c7 05 83 e9 5a 00 00 	mov    DWORD PTR [rip+0x5ae983],0x0        # 0x1405e16f8
   140032d72:	00 00 00 
   140032d75:	e9 d0 01 00 00       	jmp    0x140032f4a
   140032d7a:	83 3d 7b e9 5a 00 00 	cmp    DWORD PTR [rip+0x5ae97b],0x0        # 0x1405e16fc
   140032d81:	76 30                	jbe    0x140032db3
   140032d83:	49 8b cd             	mov    rcx,r13
   140032d86:	41 83 fa 58          	cmp    r10d,0x58
   140032d8a:	74 07                	je     0x140032d93
   140032d8c:	e8 7f 15 01 00       	call   0x140044310
   140032d91:	eb 05                	jmp    0x140032d98
   140032d93:	e8 58 19 01 00       	call   0x1400446f0
   140032d98:	8b 05 5e e9 5a 00    	mov    eax,DWORD PTR [rip+0x5ae95e]        # 0x1405e16fc
   140032d9e:	29 05 a8 e9 5a 00    	sub    DWORD PTR [rip+0x5ae9a8],eax        # 0x1405e174c
   140032da4:	c7 05 4e e9 5a 00 00 	mov    DWORD PTR [rip+0x5ae94e],0x0        # 0x1405e16fc
   140032dab:	00 00 00 
   140032dae:	e9 97 01 00 00       	jmp    0x140032f4a
   140032db3:	49 8b 6d 00          	mov    rbp,QWORD PTR [r13+0x0]
   140032db7:	48 85 e9             	test   rcx,rbp
   140032dba:	74 18                	je     0x140032dd4
   140032dbc:	48 8b c5             	mov    rax,rbp
   140032dbf:	48 c1 e8 2f          	shr    rax,0x2f
   140032dc3:	25 ff 07 00 00       	and    eax,0x7ff
   140032dc8:	48 89 05 01 64 5a 00 	mov    QWORD PTR [rip+0x5a6401],rax        # 0x1405d91d0
   140032dcf:	e8 6c 9f ff ff       	call   0x14002cd40
   140032dd4:	48 8b cd             	mov    rcx,rbp
   140032dd7:	49 8d 5d 10          	lea    rbx,[r13+0x10]
   140032ddb:	48 c1 e9 3a          	shr    rcx,0x3a
   140032ddf:	ba 04 00 00 00       	mov    edx,0x4
   140032de4:	48 8b c1             	mov    rax,rcx
   140032de7:	83 e0 03             	and    eax,0x3
   140032dea:	0f 84 bc 00 00 00    	je     0x140032eac
   140032df0:	48 83 e8 01          	sub    rax,0x1
   140032df4:	74 22                	je     0x140032e18
   140032df6:	48 83 f8 01          	cmp    rax,0x1
   140032dfa:	0f 85 22 01 00 00    	jne    0x140032f22
   140032e00:	48 8d 04 ad 00 00 00 	lea    rax,[rbp*4+0x0]
   140032e07:	00 
   140032e08:	25 fc ff 01 00       	and    eax,0x1fffc
   140032e0d:	89 05 e5 e8 5a 00    	mov    DWORD PTR [rip+0x5ae8e5],eax        # 0x1405e16f8
   140032e13:	e9 0a 01 00 00       	jmp    0x140032f22
   140032e18:	4d 8b 65 08          	mov    r12,QWORD PTR [r13+0x8]
   140032e1c:	4c 8b fd             	mov    r15,rbp
   140032e1f:	48 8b d5             	mov    rdx,rbp
   140032e22:	8b c5                	mov    eax,ebp
   140032e24:	48 c1 ea 3b          	shr    rdx,0x3b
   140032e28:	25 ff 7f 00 00       	and    eax,0x7fff
   140032e2d:	83 e2 1e             	and    edx,0x1e
   140032e30:	41 be 00 00 00 00    	mov    r14d,0x0
   140032e36:	0f af d0             	imul   edx,eax
   140032e39:	83 c2 02             	add    edx,0x2
   140032e3c:	83 e2 fc             	and    edx,0xfffffffc
   140032e3f:	83 c2 04             	add    edx,0x4
   140032e42:	41 81 e7 ff 7f 00 00 	and    r15d,0x7fff
   140032e49:	48 89 54 24 70       	mov    QWORD PTR [rsp+0x70],rdx
   140032e4e:	4c 89 7c 24 78       	mov    QWORD PTR [rsp+0x78],r15
   140032e53:	0f 86 c9 00 00 00    	jbe    0x140032f22
   140032e59:	48 c1 ed 3c          	shr    rbp,0x3c
   140032e5d:	0f 1f 00             	nop    DWORD PTR [rax]
   140032e60:	8b fd                	mov    edi,ebp
   140032e62:	49 8b f4             	mov    rsi,r12
   140032e65:	83 e7 0f             	and    edi,0xf
   140032e68:	7e 35                	jle    0x140032e9f
   140032e6a:	4c 8d 3d 8f d1 fc ff 	lea    r15,[rip+0xfffffffffffcd18f]        # 0x140000000
   140032e71:	48 8b 13             	mov    rdx,QWORD PTR [rbx]
   140032e74:	48 8b c6             	mov    rax,rsi
   140032e77:	83 e0 0f             	and    eax,0xf
   140032e7a:	b9 0e 00 00 00       	mov    ecx,0xe
   140032e7f:	4d 63 84 87 10 d1 55 	movsxd r8,DWORD PTR [r15+rax*4+0x55d110]
   140032e86:	00 
   140032e87:	e8 d4 9e ff ff       	call   0x14002cd60
   140032e8c:	ff cf                	dec    edi
   140032e8e:	48 c1 ee 04          	shr    rsi,0x4
   140032e92:	48 83 c3 08          	add    rbx,0x8
   140032e96:	85 ff                	test   edi,edi
   140032e98:	7f d7                	jg     0x140032e71
   140032e9a:	4c 8b 7c 24 78       	mov    r15,QWORD PTR [rsp+0x78]
   140032e9f:	41 ff c6             	inc    r14d
   140032ea2:	41 8b c6             	mov    eax,r14d
   140032ea5:	49 3b c7             	cmp    rax,r15
   140032ea8:	72 b6                	jb     0x140032e60
   140032eaa:	eb 71                	jmp    0x140032f1d
   140032eac:	4d 8b 65 08          	mov    r12,QWORD PTR [r13+0x8]
   140032eb0:	8b d5                	mov    edx,ebp
   140032eb2:	81 e2 ff 7f 00 00    	and    edx,0x7fff
   140032eb8:	83 e1 3c             	and    ecx,0x3c
   140032ebb:	0f af d1             	imul   edx,ecx
   140032ebe:	4c 8b fd             	mov    r15,rbp
   140032ec1:	41 be 00 00 00 00    	mov    r14d,0x0
   140032ec7:	83 c2 04             	add    edx,0x4
   140032eca:	48 89 54 24 70       	mov    QWORD PTR [rsp+0x70],rdx
   140032ecf:	41 81 e7 ff 7f 00 00 	and    r15d,0x7fff
   140032ed6:	76 4a                	jbe    0x140032f22
   140032ed8:	48 c1 ed 3c          	shr    rbp,0x3c
   140032edc:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   140032ee0:	33 f6                	xor    esi,esi
   140032ee2:	49 8b fc             	mov    rdi,r12
   140032ee5:	48 85 ed             	test   rbp,rbp
   140032ee8:	74 28                	je     0x140032f12
   140032eea:	66 0f 1f 44 00 00    	nop    WORD PTR [rax+rax*1+0x0]
   140032ef0:	4c 8b 43 08          	mov    r8,QWORD PTR [rbx+0x8]
   140032ef4:	8b cf                	mov    ecx,edi
   140032ef6:	48 8b 13             	mov    rdx,QWORD PTR [rbx]
   140032ef9:	83 e1 0f             	and    ecx,0xf
   140032efc:	e8 5f 9e ff ff       	call   0x14002cd60
   140032f01:	ff c6                	inc    esi
   140032f03:	48 c1 ef 04          	shr    rdi,0x4
   140032f07:	8b c6                	mov    eax,esi
   140032f09:	48 83 c3 10          	add    rbx,0x10
   140032f0d:	48 3b c5             	cmp    rax,rbp
   140032f10:	72 de                	jb     0x140032ef0
   140032f12:	41 ff c6             	inc    r14d
   140032f15:	41 8b c6             	mov    eax,r14d
   140032f18:	49 3b c7             	cmp    rax,r15
   140032f1b:	72 c3                	jb     0x140032ee0
   140032f1d:	48 8b 54 24 70       	mov    rdx,QWORD PTR [rsp+0x70]
   140032f22:	29 15 24 e8 5a 00    	sub    DWORD PTR [rip+0x5ae824],edx        # 0x1405e174c
   140032f28:	48 b9 00 00 00 00 00 	movabs rcx,0x400000000000
   140032f2f:	40 00 00 
   140032f32:	8b c2                	mov    eax,edx
   140032f34:	4d 8d 6c 85 00       	lea    r13,[r13+rax*4+0x0]
   140032f39:	0f 85 74 fe ff ff    	jne    0x140032db3
   140032f3f:	8b 5c 24 60          	mov    ebx,DWORD PTR [rsp+0x60]
   140032f43:	48 8d 3d b6 d0 fc ff 	lea    rdi,[rip+0xfffffffffffcd0b6]        # 0x140000000
   140032f4a:	83 3d fb e7 5a 00 00 	cmp    DWORD PTR [rip+0x5ae7fb],0x0        # 0x1405e174c
   140032f51:	0f 84 ea 02 00 00    	je     0x140033241
   140032f57:	44 8b 15 02 e8 5a 00 	mov    r10d,DWORD PTR [rip+0x5ae802]        # 0x1405e1760
   140032f5e:	44 8b 1d 9b e7 5a 00 	mov    r11d,DWORD PTR [rip+0x5ae79b]        # 0x1405e1700
   140032f65:	44 8b 05 84 e7 5a 00 	mov    r8d,DWORD PTR [rip+0x5ae784]        # 0x1405e16f0
   140032f6c:	44 8b 0d 81 e7 5a 00 	mov    r9d,DWORD PTR [rip+0x5ae781]        # 0x1405e16f4
   140032f73:	e9 ed 00 00 00       	jmp    0x140033065
   140032f78:	45 85 db             	test   r11d,r11d
   140032f7b:	0f 85 8d 02 00 00    	jne    0x14003320e
   140032f81:	45 85 c0             	test   r8d,r8d
   140032f84:	0f 84 db 00 00 00    	je     0x140033065
   140032f8a:	41 0f ba e1 0f       	bt     r9d,0xf
   140032f8f:	0f 82 b2 00 00 00    	jb     0x140033047
   140032f95:	41 8b c1             	mov    eax,r9d
   140032f98:	25 ff 3f 00 00       	and    eax,0x3fff
   140032f9d:	3d d2 03 00 00       	cmp    eax,0x3d2
   140032fa2:	77 6f                	ja     0x140033013
   140032fa4:	74 64                	je     0x14003300a
   140032fa6:	3d b0 00 00 00       	cmp    eax,0xb0
   140032fab:	77 46                	ja     0x140032ff3
   140032fad:	74 27                	je     0x140032fd6
   140032faf:	85 c0                	test   eax,eax
   140032fb1:	74 17                	je     0x140032fca
   140032fb3:	83 e8 7c             	sub    eax,0x7c
   140032fb6:	74 1e                	je     0x140032fd6
   140032fb8:	83 f8 08             	cmp    eax,0x8
   140032fbb:	0f 85 86 00 00 00    	jne    0x140033047
   140032fc1:	4c 89 2d 50 e7 5a 00 	mov    QWORD PTR [rip+0x5ae750],r13        # 0x1405e1718
   140032fc8:	eb 7d                	jmp    0x140033047
   140032fca:	41 83 fa 59          	cmp    r10d,0x59
   140032fce:	74 77                	je     0x140033047
   140032fd0:	41 83 fa 58          	cmp    r10d,0x58
   140032fd4:	75 09                	jne    0x140032fdf
   140032fd6:	4c 89 2d 33 e7 5a 00 	mov    QWORD PTR [rip+0x5ae733],r13        # 0x1405e1710
   140032fdd:	eb 68                	jmp    0x140033047
   140032fdf:	8b c3                	mov    eax,ebx
   140032fe1:	4c 89 2d 20 e7 5a 00 	mov    QWORD PTR [rip+0x5ae720],r13        # 0x1405e1708
   140032fe8:	c1 e8 04             	shr    eax,0x4
   140032feb:	89 05 13 e7 5a 00    	mov    DWORD PTR [rip+0x5ae713],eax        # 0x1405e1704
   140032ff1:	eb 54                	jmp    0x140033047
   140032ff3:	3d b8 00 00 00       	cmp    eax,0xb8
   140032ff8:	74 c7                	je     0x140032fc1
   140032ffa:	3d a7 03 00 00       	cmp    eax,0x3a7
   140032fff:	75 46                	jne    0x140033047
   140033001:	4c 89 2d 30 e7 5a 00 	mov    QWORD PTR [rip+0x5ae730],r13        # 0x1405e1738
   140033008:	eb 3d                	jmp    0x140033047
   14003300a:	4c 89 2d 2f e7 5a 00 	mov    QWORD PTR [rip+0x5ae72f],r13        # 0x1405e1740
   140033011:	eb 34                	jmp    0x140033047
   140033013:	2d d3 03 00 00       	sub    eax,0x3d3
   140033018:	74 26                	je     0x140033040
   14003301a:	83 e8 02             	sub    eax,0x2
   14003301d:	74 0f                	je     0x14003302e
   14003301f:	83 e8 07             	sub    eax,0x7
   140033022:	74 13                	je     0x140033037
   140033024:	83 e8 1f             	sub    eax,0x1f
   140033027:	74 17                	je     0x140033040
   140033029:	83 f8 02             	cmp    eax,0x2
   14003302c:	75 19                	jne    0x140033047
   14003302e:	4c 89 2d f3 e6 5a 00 	mov    QWORD PTR [rip+0x5ae6f3],r13        # 0x1405e1728
   140033035:	eb 10                	jmp    0x140033047
   140033037:	4c 89 2d f2 e6 5a 00 	mov    QWORD PTR [rip+0x5ae6f2],r13        # 0x1405e1730
   14003303e:	eb 07                	jmp    0x140033047
   140033040:	4c 89 2d d9 e6 5a 00 	mov    QWORD PTR [rip+0x5ae6d9],r13        # 0x1405e1720
   140033047:	41 8b c0             	mov    eax,r8d
   14003304a:	41 2b d8             	sub    ebx,r8d
   14003304d:	41 b8 00 00 00 00    	mov    r8d,0x0
   140033053:	44 89 05 96 e6 5a 00 	mov    DWORD PTR [rip+0x5ae696],r8d        # 0x1405e16f0
   14003305a:	4d 8d 6c 85 00       	lea    r13,[r13+rax*4+0x0]
   14003305f:	0f 84 dc 01 00 00    	je     0x140033241
   140033065:	41 0f b6 55 03       	movzx  edx,BYTE PTR [r13+0x3]
   14003306a:	8b c2                	mov    eax,edx
   14003306c:	83 e0 7f             	and    eax,0x7f
   14003306f:	83 c0 f8             	add    eax,0xfffffff8
   140033072:	83 f8 67             	cmp    eax,0x67
   140033075:	0f 87 76 01 00 00    	ja     0x1400331f1
   14003307b:	48 98                	cdqe
   14003307d:	0f b6 84 07 a0 32 03 	movzx  eax,BYTE PTR [rdi+rax*1+0x332a0]
   140033084:	00 
   140033085:	8b 8c 87 68 32 03 00 	mov    ecx,DWORD PTR [rdi+rax*4+0x33268]
   14003308c:	48 03 cf             	add    rcx,rdi
   14003308f:	ff e1                	jmp    rcx
   140033091:	66 41 0f 6e 45 04    	movd   xmm0,DWORD PTR [r13+0x4]
   140033097:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14003309a:	f3 0f 11 05 ce e6 5a 	movss  DWORD PTR [rip+0x5ae6ce],xmm0        # 0x1405e1770
   1400330a1:	00 
   1400330a2:	66 41 0f 6e 4d 08    	movd   xmm1,DWORD PTR [r13+0x8]
   1400330a8:	0f 5b c9             	cvtdq2ps xmm1,xmm1
   1400330ab:	f3 0f 11 0d c1 e6 5a 	movss  DWORD PTR [rip+0x5ae6c1],xmm1        # 0x1405e1774
   1400330b2:	00 
   1400330b3:	66 41 0f 6e 45 0c    	movd   xmm0,DWORD PTR [r13+0xc]
   1400330b9:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1400330bc:	f3 0f 11 05 b4 e6 5a 	movss  DWORD PTR [rip+0x5ae6b4],xmm0        # 0x1405e1778
   1400330c3:	00 
   1400330c4:	66 41 0f 6e 4d 10    	movd   xmm1,DWORD PTR [r13+0x10]
   1400330ca:	0f 5b c9             	cvtdq2ps xmm1,xmm1
   1400330cd:	f3 0f 11 0d a7 e6 5a 	movss  DWORD PTR [rip+0x5ae6a7],xmm1        # 0x1405e177c
   1400330d4:	00 
   1400330d5:	e9 17 01 00 00       	jmp    0x1400331f1
   1400330da:	41 8b 45 00          	mov    eax,DWORD PTR [r13+0x0]
   1400330de:	25 ff ff ff 00       	and    eax,0xffffff
   1400330e3:	c1 e0 02             	shl    eax,0x2
   1400330e6:	89 05 60 e6 5a 00    	mov    DWORD PTR [rip+0x5ae660],eax        # 0x1405e174c
   1400330ec:	e9 00 01 00 00       	jmp    0x1400331f1
   1400330f1:	45 8b 4d 00          	mov    r9d,DWORD PTR [r13+0x0]
   1400330f5:	44 89 0d f8 e5 5a 00 	mov    DWORD PTR [rip+0x5ae5f8],r9d        # 0x1405e16f4
   1400330fc:	45 0f b6 45 02       	movzx  r8d,BYTE PTR [r13+0x2]
   140033101:	44 89 05 e8 e5 5a 00 	mov    DWORD PTR [rip+0x5ae5e8],r8d        # 0x1405e16f0
   140033108:	e9 e4 00 00 00       	jmp    0x1400331f1
   14003310d:	45 8b 4d 00          	mov    r9d,DWORD PTR [r13+0x0]
   140033111:	44 89 0d dc e5 5a 00 	mov    DWORD PTR [rip+0x5ae5dc],r9d        # 0x1405e16f4
   140033118:	41 0f b6 45 02       	movzx  eax,BYTE PTR [r13+0x2]
   14003311d:	44 8d 04 40          	lea    r8d,[rax+rax*2]
   140033121:	44 89 05 c8 e5 5a 00 	mov    DWORD PTR [rip+0x5ae5c8],r8d        # 0x1405e16f0
   140033128:	e9 c4 00 00 00       	jmp    0x1400331f1
   14003312d:	45 8b 4d 00          	mov    r9d,DWORD PTR [r13+0x0]
   140033131:	44 89 0d bc e5 5a 00 	mov    DWORD PTR [rip+0x5ae5bc],r9d        # 0x1405e16f4
   140033138:	45 0f b6 45 02       	movzx  r8d,BYTE PTR [r13+0x2]
   14003313d:	41 c1 e0 02          	shl    r8d,0x2
   140033141:	44 89 05 a8 e5 5a 00 	mov    DWORD PTR [rip+0x5ae5a8],r8d        # 0x1405e16f0
   140033148:	e9 a4 00 00 00       	jmp    0x1400331f1
   14003314d:	45 8b 4d 00          	mov    r9d,DWORD PTR [r13+0x0]
   140033151:	44 89 0d 9c e5 5a 00 	mov    DWORD PTR [rip+0x5ae59c],r9d        # 0x1405e16f4
   140033158:	45 0f b6 45 02       	movzx  r8d,BYTE PTR [r13+0x2]
   14003315d:	41 d1 e8             	shr    r8d,1
   140033160:	44 89 05 89 e5 5a 00 	mov    DWORD PTR [rip+0x5ae589],r8d        # 0x1405e16f0
   140033167:	e9 85 00 00 00       	jmp    0x1400331f1
   14003316c:	49 83 c5 04          	add    r13,0x4
   140033170:	ff cb                	dec    ebx
   140033172:	eb 7d                	jmp    0x1400331f1
   140033174:	c7 05 16 65 5a 00 01 	mov    DWORD PTR [rip+0x5a6516],0x1        # 0x1405d9694
   14003317b:	00 00 00 
   14003317e:	44 8b d2             	mov    r10d,edx
   140033181:	89 15 d9 e5 5a 00    	mov    DWORD PTR [rip+0x5ae5d9],edx        # 0x1405e1760
   140033187:	eb 68                	jmp    0x1400331f1
   140033189:	c7 05 01 65 5a 00 02 	mov    DWORD PTR [rip+0x5a6501],0x2        # 0x1405d9694
   140033190:	00 00 00 
   140033193:	44 8b d2             	mov    r10d,edx
   140033196:	89 15 c4 e5 5a 00    	mov    DWORD PTR [rip+0x5ae5c4],edx        # 0x1405e1760
   14003319c:	eb 53                	jmp    0x1400331f1
   14003319e:	41 bb 01 00 00 00    	mov    r11d,0x1
   1400331a4:	c7 05 e6 64 5a 00 03 	mov    DWORD PTR [rip+0x5a64e6],0x3        # 0x1405d9694
   1400331ab:	00 00 00 
   1400331ae:	44 89 1d 4b e5 5a 00 	mov    DWORD PTR [rip+0x5ae54b],r11d        # 0x1405e1700
   1400331b5:	44 8b d2             	mov    r10d,edx
   1400331b8:	89 15 a2 e5 5a 00    	mov    DWORD PTR [rip+0x5ae5a2],edx        # 0x1405e1760
   1400331be:	eb 31                	jmp    0x1400331f1
   1400331c0:	41 bb 02 00 00 00    	mov    r11d,0x2
   1400331c6:	44 89 1d 33 e5 5a 00 	mov    DWORD PTR [rip+0x5ae533],r11d        # 0x1405e1700
   1400331cd:	eb 22                	jmp    0x1400331f1
   1400331cf:	41 8b 45 00          	mov    eax,DWORD PTR [r13+0x0]
   1400331d3:	25 ff ff ff 00       	and    eax,0xffffff
   1400331d8:	89 05 6e e5 5a 00    	mov    DWORD PTR [rip+0x5ae56e],eax        # 0x1405e174c
   1400331de:	89 05 18 e5 5a 00    	mov    DWORD PTR [rip+0x5ae518],eax        # 0x1405e16fc
   1400331e4:	eb 0b                	jmp    0x1400331f1
   1400331e6:	41 0f b6 45 00       	movzx  eax,BYTE PTR [r13+0x0]
   1400331eb:	89 05 57 e5 5a 00    	mov    DWORD PTR [rip+0x5ae557],eax        # 0x1405e1748
   1400331f1:	49 83 c5 04          	add    r13,0x4
   1400331f5:	48 b9 00 00 00 00 00 	movabs rcx,0x400000000000
   1400331fc:	40 00 00 
   1400331ff:	83 c3 ff             	add    ebx,0xffffffff
   140033202:	89 5c 24 60          	mov    DWORD PTR [rsp+0x60],ebx
   140033206:	0f 85 24 fb ff ff    	jne    0x140032d30
   14003320c:	eb 33                	jmp    0x140033241
   14003320e:	41 83 fb 01          	cmp    r11d,0x1
   140033212:	75 0d                	jne    0x140033221
   140033214:	49 8d 45 f0          	lea    rax,[r13-0x10]
   140033218:	48 89 05 31 e5 5a 00 	mov    QWORD PTR [rip+0x5ae531],rax        # 0x1405e1750
   14003321f:	eb 16                	jmp    0x140033237
   140033221:	48 8b 0d 28 e5 5a 00 	mov    rcx,QWORD PTR [rip+0x5ae528]        # 0x1405e1750
   140033228:	49 8b d5             	mov    rdx,r13
   14003322b:	4c 89 2d 26 e5 5a 00 	mov    QWORD PTR [rip+0x5ae526],r13        # 0x1405e1758
   140033232:	e8 59 23 01 00       	call   0x140045590
   140033237:	c7 05 bf e4 5a 00 00 	mov    DWORD PTR [rip+0x5ae4bf],0x0        # 0x1405e1700
   14003323e:	00 00 00 
   140033241:	4c 8b 74 24 28       	mov    r14,QWORD PTR [rsp+0x28]
   140033246:	4c 8b 64 24 30       	mov    r12,QWORD PTR [rsp+0x30]
   14003324b:	48 8b 7c 24 38       	mov    rdi,QWORD PTR [rsp+0x38]
   140033250:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   140033255:	48 8b 6c 24 68       	mov    rbp,QWORD PTR [rsp+0x68]
   14003325a:	4c 8b 7c 24 20       	mov    r15,QWORD PTR [rsp+0x20]
   14003325f:	48 83 c4 48          	add    rsp,0x48
   140033263:	41 5d                	pop    r13
   140033265:	5b                   	pop    rbx
   140033266:	c3                   	ret
