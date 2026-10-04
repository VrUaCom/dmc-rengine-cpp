
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031fd30 <.text+0x31ed30>:
   14031fd30:	48 89 5c 24 18       	mov    QWORD PTR [rsp+0x18],rbx
   14031fd35:	48 89 54 24 10       	mov    QWORD PTR [rsp+0x10],rdx
   14031fd3a:	55                   	push   rbp
   14031fd3b:	56                   	push   rsi
   14031fd3c:	57                   	push   rdi
   14031fd3d:	41 54                	push   r12
   14031fd3f:	41 55                	push   r13
   14031fd41:	41 56                	push   r14
   14031fd43:	41 57                	push   r15
   14031fd45:	48 83 ec 20          	sub    rsp,0x20
   14031fd49:	4c 8b 41 28          	mov    r8,QWORD PTR [rcx+0x28]
   14031fd4d:	33 ff                	xor    edi,edi
   14031fd4f:	be 01 00 00 00       	mov    esi,0x1
   14031fd54:	48 8b e9             	mov    rbp,rcx
   14031fd57:	45 0f b6 48 10       	movzx  r9d,BYTE PTR [r8+0x10]
   14031fd5c:	66 44 89 49 04       	mov    WORD PTR [rcx+0x4],r9w
   14031fd61:	41 0f b6 40 11       	movzx  eax,BYTE PTR [r8+0x11]
   14031fd66:	66 89 41 06          	mov    WORD PTR [rcx+0x6],ax
   14031fd6a:	41 0f b6 40 12       	movzx  eax,BYTE PTR [r8+0x12]
   14031fd6f:	4f 8d 34 89          	lea    r14,[r9+r9*4]
   14031fd73:	49 c1 e6 05          	shl    r14,0x5
   14031fd77:	66 89 41 08          	mov    WORD PTR [rcx+0x8],ax
   14031fd7b:	4c 03 f2             	add    r14,rdx
   14031fd7e:	41 8b c1             	mov    eax,r9d
   14031fd81:	c7 41 74 00 00 80 3f 	mov    DWORD PTR [rcx+0x74],0x3f800000
   14031fd88:	c7 41 70 00 00 80 3f 	mov    DWORD PTR [rcx+0x70],0x3f800000
   14031fd8f:	44 8b cf             	mov    r9d,edi
   14031fd92:	89 79 78             	mov    DWORD PTR [rcx+0x78],edi
   14031fd95:	40 88 79 7c          	mov    BYTE PTR [rcx+0x7c],dil
   14031fd99:	66 89 71 0a          	mov    WORD PTR [rcx+0xa],si
   14031fd9d:	48 89 51 20          	mov    QWORD PTR [rcx+0x20],rdx
   14031fda1:	66 3b f8             	cmp    di,ax
   14031fda4:	73 72                	jae    0x14031fe18
   14031fda6:	66 66 0f 1f 84 00 00 	data16 nop WORD PTR [rax+rax*1+0x0]
   14031fdad:	00 00 00 
   14031fdb0:	4c 8b 45 28          	mov    r8,QWORD PTR [rbp+0x28]
   14031fdb4:	49 63 d1             	movsxd rdx,r9d
   14031fdb7:	49 83 c0 20          	add    r8,0x20
   14031fdbb:	48 8b ca             	mov    rcx,rdx
   14031fdbe:	48 c1 e1 06          	shl    rcx,0x6
   14031fdc2:	4c 03 c1             	add    r8,rcx
   14031fdc5:	48 8d 0c 92          	lea    rcx,[rdx+rdx*4]
   14031fdc9:	48 c1 e1 05          	shl    rcx,0x5
   14031fdcd:	48 03 4d 20          	add    rcx,QWORD PTR [rbp+0x20]
   14031fdd1:	66 44 89 09          	mov    WORD PTR [rcx],r9w
   14031fdd5:	41 ff c1             	inc    r9d
   14031fdd8:	41 0f bf 00          	movsx  eax,WORD PTR [r8]
   14031fddc:	89 41 04             	mov    DWORD PTR [rcx+0x4],eax
   14031fddf:	41 0f bf 40 02       	movsx  eax,WORD PTR [r8+0x2]
   14031fde4:	89 41 08             	mov    DWORD PTR [rcx+0x8],eax
   14031fde7:	4c 89 41 10          	mov    QWORD PTR [rcx+0x10],r8
   14031fdeb:	49 8b 40 10          	mov    rax,QWORD PTR [r8+0x10]
   14031fdef:	48 89 41 28          	mov    QWORD PTR [rcx+0x28],rax
   14031fdf3:	49 8b 40 18          	mov    rax,QWORD PTR [r8+0x18]
   14031fdf7:	48 89 41 30          	mov    QWORD PTR [rcx+0x30],rax
   14031fdfb:	49 8b 40 20          	mov    rax,QWORD PTR [r8+0x20]
   14031fdff:	48 89 41 38          	mov    QWORD PTR [rcx+0x38],rax
   14031fe03:	49 8b 40 28          	mov    rax,QWORD PTR [r8+0x28]
   14031fe07:	48 89 41 40          	mov    QWORD PTR [rcx+0x40],rax
   14031fe0b:	66 89 71 02          	mov    WORD PTR [rcx+0x2],si
   14031fe0f:	0f b7 45 04          	movzx  eax,WORD PTR [rbp+0x4]
   14031fe13:	44 3b c8             	cmp    r9d,eax
   14031fe16:	7c 98                	jl     0x14031fdb0
   14031fe18:	bb 02 00 00 00       	mov    ebx,0x2
   14031fe1d:	41 bf 68 00 00 00    	mov    r15d,0x68
   14031fe23:	48 89 5c 24 60       	mov    QWORD PTR [rsp+0x60],rbx
   14031fe28:	49 bc 04 80 00 00 00 	movabs r12,0x1000000000008004
   14031fe2f:	00 00 10 
   14031fe32:	49 bd 80 20 20 20 00 	movabs r13,0x3f80000020202080
   14031fe39:	00 80 3f 
   14031fe3c:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   14031fe40:	8b d6                	mov    edx,esi
   14031fe42:	4d 89 74 2f c8       	mov    QWORD PTR [r15+rbp*1-0x38],r14
   14031fe47:	33 c9                	xor    ecx,ecx
   14031fe49:	e8 d2 0e 00 00       	call   0x140320d20
   14031fe4e:	41 c7 46 08 00 00 00 	mov    DWORD PTR [r14+0x8],0x58000000
   14031fe55:	58 
   14031fe56:	8b d6                	mov    edx,esi
   14031fe58:	41 c7 46 0c 05 00 00 	mov    DWORD PTR [r14+0xc],0x50000005
   14031fe5f:	50 
   14031fe60:	33 c9                	xor    ecx,ecx
   14031fe62:	8b c0                	mov    eax,eax
   14031fe64:	48 c1 e0 20          	shl    rax,0x20
   14031fe68:	48 0d 05 00 00 10    	or     rax,0x10000005
   14031fe6e:	49 89 06             	mov    QWORD PTR [r14],rax
   14031fe71:	4d 89 66 10          	mov    QWORD PTR [r14+0x10],r12
   14031fe75:	49 c7 46 18 0e 00 00 	mov    QWORD PTR [r14+0x18],0xe
   14031fe7c:	00 
   14031fe7d:	49 c7 46 20 44 00 00 	mov    QWORD PTR [r14+0x20],0x44
   14031fe84:	00 
   14031fe85:	49 89 7e 28          	mov    QWORD PTR [r14+0x28],rdi
   14031fe89:	49 89 7e 30          	mov    QWORD PTR [r14+0x30],rdi
   14031fe8d:	49 c7 46 38 14 00 00 	mov    QWORD PTR [r14+0x38],0x14
   14031fe94:	00 
   14031fe95:	49 89 7e 40          	mov    QWORD PTR [r14+0x40],rdi
   14031fe99:	49 89 7e 48          	mov    QWORD PTR [r14+0x48],rdi
   14031fe9d:	4d 89 6e 50          	mov    QWORD PTR [r14+0x50],r13
   14031fea1:	49 89 76 58          	mov    QWORD PTR [r14+0x58],rsi
   14031fea5:	e8 76 0e 00 00       	call   0x140320d20
   14031feaa:	8b c0                	mov    eax,eax
   14031feac:	8b d6                	mov    edx,esi
   14031feae:	48 c1 e0 20          	shl    rax,0x20
   14031feb2:	33 c9                	xor    ecx,ecx
   14031feb4:	48 0d 08 00 00 10    	or     rax,0x10000008
   14031feba:	41 c7 46 68 04 04 00 	mov    DWORD PTR [r14+0x68],0x1000404
   14031fec1:	01 
   14031fec2:	49 89 46 60          	mov    QWORD PTR [r14+0x60],rax
   14031fec6:	41 c7 46 6c 00 00 08 	mov    DWORD PTR [r14+0x6c],0x6c080000
   14031fecd:	6c 
   14031fece:	e8 4d 0e 00 00       	call   0x140320d20
   14031fed3:	41 c7 86 f8 00 00 00 	mov    DWORD PTR [r14+0xf8],0x1000404
   14031feda:	04 04 00 01 
   14031fede:	8b d6                	mov    edx,esi
   14031fee0:	41 c7 86 fc 00 00 00 	mov    DWORD PTR [r14+0xfc],0x6c030008
   14031fee7:	08 00 03 6c 
   14031feeb:	33 c9                	xor    ecx,ecx
   14031feed:	8b c0                	mov    eax,eax
   14031feef:	48 c1 e0 20          	shl    rax,0x20
   14031fef3:	48 0d 03 00 00 10    	or     rax,0x10000003
   14031fef9:	49 89 86 f0 00 00 00 	mov    QWORD PTR [r14+0xf0],rax
   14031ff00:	48 b8 00 80 00 00 00 	movabs rax,0x5000000000008000
   14031ff07:	00 00 50 
   14031ff0a:	49 89 86 00 01 00 00 	mov    QWORD PTR [r14+0x100],rax
   14031ff11:	49 c7 86 08 01 00 00 	mov    QWORD PTR [r14+0x108],0x4444e
   14031ff18:	4e 44 04 00 
   14031ff1c:	49 89 be 10 01 00 00 	mov    QWORD PTR [r14+0x110],rdi
   14031ff23:	49 89 be 18 01 00 00 	mov    QWORD PTR [r14+0x118],rdi
   14031ff2a:	49 89 be 20 01 00 00 	mov    QWORD PTR [r14+0x120],rdi
   14031ff31:	49 89 be 28 01 00 00 	mov    QWORD PTR [r14+0x128],rdi
   14031ff38:	e8 e3 0d 00 00       	call   0x140320d20
   14031ff3d:	8b c0                	mov    eax,eax
   14031ff3f:	44 8b ef             	mov    r13d,edi
   14031ff42:	48 c1 e0 20          	shl    rax,0x20
   14031ff46:	48 0f ba e8 1d       	bts    rax,0x1d
   14031ff4b:	49 89 86 30 01 00 00 	mov    QWORD PTR [r14+0x130],rax
   14031ff52:	49 c7 86 38 01 00 00 	mov    QWORD PTR [r14+0x138],0x0
   14031ff59:	00 00 00 00 
   14031ff5d:	49 81 c6 40 01 00 00 	add    r14,0x140
   14031ff64:	66 3b 7d 04          	cmp    di,WORD PTR [rbp+0x4]
   14031ff68:	0f 83 17 01 00 00    	jae    0x140320085
   14031ff6e:	4c 8b e7             	mov    r12,rdi
   14031ff71:	48 8b 75 20          	mov    rsi,QWORD PTR [rbp+0x20]
   14031ff75:	49 8b ce             	mov    rcx,r14
   14031ff78:	49 03 f4             	add    rsi,r12
   14031ff7b:	ba 01 00 00 00       	mov    edx,0x1
   14031ff80:	4d 89 34 37          	mov    QWORD PTR [r15+rsi*1],r14
   14031ff84:	48 63 46 04          	movsxd rax,DWORD PTR [rsi+0x4]
   14031ff88:	48 c1 e0 04          	shl    rax,0x4
   14031ff8c:	4c 03 f0             	add    r14,rax
   14031ff8f:	4d 89 74 37 10       	mov    QWORD PTR [r15+rsi*1+0x10],r14
   14031ff94:	48 63 46 08          	movsxd rax,DWORD PTR [rsi+0x8]
   14031ff98:	48 c1 e0 04          	shl    rax,0x4
   14031ff9c:	4c 03 f0             	add    r14,rax
   14031ff9f:	4d 89 74 37 e0       	mov    QWORD PTR [r15+rsi*1-0x20],r14
   14031ffa4:	49 8b fe             	mov    rdi,r14
   14031ffa7:	49 83 c6 30          	add    r14,0x30
   14031ffab:	4d 89 74 37 20       	mov    QWORD PTR [r15+rsi*1+0x20],r14
   14031ffb0:	49 8b de             	mov    rbx,r14
   14031ffb3:	49 83 c6 20          	add    r14,0x20
   14031ffb7:	48 89 0b             	mov    QWORD PTR [rbx],rcx
   14031ffba:	33 c9                	xor    ecx,ecx
   14031ffbc:	49 8b 44 37 10       	mov    rax,QWORD PTR [r15+rsi*1+0x10]
   14031ffc1:	48 89 43 08          	mov    QWORD PTR [rbx+0x8],rax
   14031ffc5:	48 8b 46 28          	mov    rax,QWORD PTR [rsi+0x28]
   14031ffc9:	48 89 43 10          	mov    QWORD PTR [rbx+0x10],rax
   14031ffcd:	e8 4e 0d 00 00       	call   0x140320d20
   14031ffd2:	8b c0                	mov    eax,eax
   14031ffd4:	33 c9                	xor    ecx,ecx
   14031ffd6:	48 c1 e0 20          	shl    rax,0x20
   14031ffda:	48 0f ba e8 1c       	bts    rax,0x1c
   14031ffdf:	48 89 07             	mov    QWORD PTR [rdi],rax
   14031ffe2:	c7 47 08 00 00 00 00 	mov    DWORD PTR [rdi+0x8],0x0
   14031ffe9:	c7 47 0c 06 00 00 5c 	mov    DWORD PTR [rdi+0xc],0x5c000006
   14031fff0:	2b 1d b2 9e 2b 00    	sub    ebx,DWORD PTR [rip+0x2b9eb2]        # 0x1405d9ea8
   14031fff6:	8b c3                	mov    eax,ebx
   14031fff8:	33 db                	xor    ebx,ebx
   14031fffa:	48 c1 e0 20          	shl    rax,0x20
   14031fffe:	48 0d 02 00 00 30    	or     rax,0x30000002
   140320004:	89 5f 18             	mov    DWORD PTR [rdi+0x18],ebx
   140320007:	48 89 47 10          	mov    QWORD PTR [rdi+0x10],rax
   14032000b:	8d 53 01             	lea    edx,[rbx+0x1]
   14032000e:	c7 47 1c 08 00 00 5b 	mov    DWORD PTR [rdi+0x1c],0x5b000008
   140320015:	e8 06 0d 00 00       	call   0x140320d20
   14032001a:	8b c0                	mov    eax,eax
   14032001c:	33 c9                	xor    ecx,ecx
   14032001e:	48 c1 e0 20          	shl    rax,0x20
   140320022:	48 0f ba e8 1d       	bts    rax,0x1d
   140320027:	48 89 47 20          	mov    QWORD PTR [rdi+0x20],rax
   14032002b:	48 89 5f 28          	mov    QWORD PTR [rdi+0x28],rbx
   14032002f:	49 8b de             	mov    rbx,r14
   140320032:	4d 89 74 37 f0       	mov    QWORD PTR [r15+rsi*1-0x10],r14
   140320037:	be 01 00 00 00       	mov    esi,0x1
   14032003c:	8b d6                	mov    edx,esi
   14032003e:	49 83 c6 10          	add    r14,0x10
   140320042:	e8 d9 0c 00 00       	call   0x140320d20
   140320047:	8b c0                	mov    eax,eax
   140320049:	33 ff                	xor    edi,edi
   14032004b:	48 c1 e0 20          	shl    rax,0x20
   14032004f:	41 ff c5             	inc    r13d
   140320052:	48 0f ba e8 1d       	bts    rax,0x1d
   140320057:	49 81 c4 a0 00 00 00 	add    r12,0xa0
   14032005e:	48 89 03             	mov    QWORD PTR [rbx],rax
   140320061:	48 c7 43 08 00 00 00 	mov    QWORD PTR [rbx+0x8],0x11000000
   140320068:	11 
   140320069:	0f b7 45 04          	movzx  eax,WORD PTR [rbp+0x4]
   14032006d:	44 3b e8             	cmp    r13d,eax
   140320070:	0f 8c fb fe ff ff    	jl     0x14031ff71
   140320076:	48 8b 5c 24 60       	mov    rbx,QWORD PTR [rsp+0x60]
   14032007b:	49 bc 04 80 00 00 00 	movabs r12,0x1000000000008004
   140320082:	00 00 10 
   140320085:	49 83 c7 08          	add    r15,0x8
   140320089:	49 bd 80 20 20 20 00 	movabs r13,0x3f80000020202080
   140320090:	00 80 3f 
   140320093:	48 83 eb 01          	sub    rbx,0x1
   140320097:	48 89 5c 24 60       	mov    QWORD PTR [rsp+0x60],rbx
   14032009c:	0f 85 9e fd ff ff    	jne    0x14031fe40
   1403200a2:	44 2b 74 24 68       	sub    r14d,DWORD PTR [rsp+0x68]
   1403200a7:	48 8b 5c 24 70       	mov    rbx,QWORD PTR [rsp+0x70]
   1403200ac:	44 89 75 18          	mov    DWORD PTR [rbp+0x18],r14d
   1403200b0:	48 83 c4 20          	add    rsp,0x20
   1403200b4:	41 5f                	pop    r15
   1403200b6:	41 5e                	pop    r14
   1403200b8:	41 5d                	pop    r13
   1403200ba:	41 5c                	pop    r12
   1403200bc:	5f                   	pop    rdi
   1403200bd:	5e                   	pop    rsi
   1403200be:	5d                   	pop    rbp
   1403200bf:	c3                   	ret
