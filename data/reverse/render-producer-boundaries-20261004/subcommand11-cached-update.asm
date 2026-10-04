
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031a830 <.text+0x319830>:
   14031a830:	40 53                	rex push rbx
   14031a832:	55                   	push   rbp
   14031a833:	56                   	push   rsi
   14031a834:	57                   	push   rdi
   14031a835:	41 54                	push   r12
   14031a837:	41 56                	push   r14
   14031a839:	41 57                	push   r15
   14031a83b:	48 81 ec e0 00 00 00 	sub    rsp,0xe0
   14031a842:	f3 0f 10 4a 2c       	movss  xmm1,DWORD PTR [rdx+0x2c]
   14031a847:	48 8b f2             	mov    rsi,rdx
   14031a84a:	4c 8b f9             	mov    r15,rcx
   14031a84d:	e8 1e c5 ff ff       	call   0x140316d70
   14031a852:	8b 4e 58             	mov    ecx,DWORD PTR [rsi+0x58]
   14031a855:	bb 01 00 00 00       	mov    ebx,0x1
   14031a85a:	8b 46 20             	mov    eax,DWORD PTR [rsi+0x20]
   14031a85d:	8b d3                	mov    edx,ebx
   14031a85f:	01 46 70             	add    DWORD PTR [rsi+0x70],eax
   14031a862:	8b 46 24             	mov    eax,DWORD PTR [rsi+0x24]
   14031a865:	01 46 74             	add    DWORD PTR [rsi+0x74],eax
   14031a868:	44 8b 66 70          	mov    r12d,DWORD PTR [rsi+0x70]
   14031a86c:	8b 7e 74             	mov    edi,DWORD PTR [rsi+0x74]
   14031a86f:	d3 e2                	shl    edx,cl
   14031a871:	8b 4e 5c             	mov    ecx,DWORD PTR [rsi+0x5c]
   14031a874:	d3 e3                	shl    ebx,cl
   14031a876:	8b ca                	mov    ecx,edx
   14031a878:	41 c1 fc 04          	sar    r12d,0x4
   14031a87c:	8d 42 ff             	lea    eax,[rdx-0x1]
   14031a87f:	c1 ff 04             	sar    edi,0x4
   14031a882:	44 23 e0             	and    r12d,eax
   14031a885:	89 54 24 20          	mov    DWORD PTR [rsp+0x20],edx
   14031a889:	8d 43 ff             	lea    eax,[rbx-0x1]
   14031a88c:	44 89 64 24 44       	mov    DWORD PTR [rsp+0x44],r12d
   14031a891:	23 f8                	and    edi,eax
   14031a893:	e8 d8 67 01 00       	call   0x140331070
   14031a898:	44 8b 76 70          	mov    r14d,DWORD PTR [rsi+0x70]
   14031a89c:	8d 48 04             	lea    ecx,[rax+0x4]
   14031a89f:	41 d3 fe             	sar    r14d,cl
   14031a8a2:	8b cb                	mov    ecx,ebx
   14031a8a4:	44 89 74 24 38       	mov    DWORD PTR [rsp+0x38],r14d
   14031a8a9:	e8 c2 67 01 00       	call   0x140331070
   14031a8ae:	49 8b 97 b8 5c 01 00 	mov    rdx,QWORD PTR [r15+0x15cb8]
   14031a8b5:	4c 8d 15 14 72 9a 00 	lea    r10,[rip+0x9a7214]        # 0x140cc1ad0
   14031a8bc:	8b 6e 74             	mov    ebp,DWORD PTR [rsi+0x74]
   14031a8bf:	44 8b 4e 28          	mov    r9d,DWORD PTR [rsi+0x28]
   14031a8c3:	44 8b 46 60          	mov    r8d,DWORD PTR [rsi+0x60]
   14031a8c7:	8d 48 04             	lea    ecx,[rax+0x4]
   14031a8ca:	0f b7 42 5a          	movzx  eax,WORD PTR [rdx+0x5a]
   14031a8ce:	d3 fd                	sar    ebp,cl
   14031a8d0:	0f b7 4c 42 5c       	movzx  ecx,WORD PTR [rdx+rax*2+0x5c]
   14031a8d5:	66 ff c0             	inc    ax
   14031a8d8:	48 c1 e1 04          	shl    rcx,0x4
   14031a8dc:	48 03 4a 50          	add    rcx,QWORD PTR [rdx+0x50]
   14031a8e0:	66 89 42 5a          	mov    WORD PTR [rdx+0x5a],ax
   14031a8e4:	44 89 49 04          	mov    DWORD PTR [rcx+0x4],r9d
   14031a8e8:	44 8b cb             	mov    r9d,ebx
   14031a8eb:	41 f7 d9             	neg    r9d
   14031a8ee:	44 89 01             	mov    DWORD PTR [rcx],r8d
   14031a8f1:	48 c7 41 08 64 00 00 	mov    QWORD PTR [rcx+0x8],0x64
   14031a8f8:	00 
   14031a8f9:	44 89 4c 24 28       	mov    DWORD PTR [rsp+0x28],r9d
   14031a8fe:	45 3b 8f 5c 5d 01 00 	cmp    r9d,DWORD PTR [r15+0x15d5c]
   14031a905:	0f 8f ed 04 00 00    	jg     0x14031adf8
   14031a90b:	44 8b 44 24 20       	mov    r8d,DWORD PTR [rsp+0x20]
   14031a910:	41 03 f9             	add    edi,r9d
   14031a913:	4c 8b 1d e6 29 a5 00 	mov    r11,QWORD PTR [rip+0xa529e6]        # 0x140d6d300
   14031a91a:	41 8b c0             	mov    eax,r8d
   14031a91d:	f3 0f 10 25 0f 38 2c 	movss  xmm4,DWORD PTR [rip+0x2c380f]        # 0x1405de134
   14031a924:	00 
   14031a925:	f7 d8                	neg    eax
   14031a927:	f3 0f 10 2d 01 38 2c 	movss  xmm5,DWORD PTR [rip+0x2c3801]        # 0x1405de130
   14031a92e:	00 
   14031a92f:	8b d7                	mov    edx,edi
   14031a931:	f3 0f 10 1d 27 2c 04 	movss  xmm3,DWORD PTR [rip+0x42c27]        # 0x14035d560
   14031a938:	00 
   14031a939:	8b cb                	mov    ecx,ebx
   14031a93b:	4c 89 ac 24 d8 00 00 	mov    QWORD PTR [rsp+0xd8],r13
   14031a942:	00 
   14031a943:	45 8b ee             	mov    r13d,r14d
   14031a946:	0f 29 b4 24 c0 00 00 	movaps XMMWORD PTR [rsp+0xc0],xmm6
   14031a94d:	00 
   14031a94e:	4c 8d 35 3b 61 2b 00 	lea    r14,[rip+0x2b613b]        # 0x1405d0a90
   14031a955:	f3 0f 10 35 f7 38 2c 	movss  xmm6,DWORD PTR [rip+0x2c38f7]        # 0x1405de254
   14031a95c:	00 
   14031a95d:	41 f7 d5             	not    r13d
   14031a960:	0f 29 bc 24 b0 00 00 	movaps XMMWORD PTR [rsp+0xb0],xmm7
   14031a967:	00 
   14031a968:	41 83 e5 1f          	and    r13d,0x1f
   14031a96c:	f3 0f 10 3d dc 38 2c 	movss  xmm7,DWORD PTR [rip+0x2c38dc]        # 0x1405de250
   14031a973:	00 
   14031a974:	44 0f 29 84 24 a0 00 	movaps XMMWORD PTR [rsp+0xa0],xmm8
   14031a97b:	00 00 
   14031a97d:	f3 44 0f 10 05 4a d3 	movss  xmm8,DWORD PTR [rip+0x1ed34a]        # 0x140507cd0
   14031a984:	1e 00 
   14031a986:	44 0f 29 8c 24 90 00 	movaps XMMWORD PTR [rsp+0x90],xmm9
   14031a98d:	00 00 
   14031a98f:	f3 44 0f 10 0d 70 2c 	movss  xmm9,DWORD PTR [rip+0x42c70]        # 0x14035d608
   14031a996:	04 00 
   14031a998:	44 0f 29 94 24 80 00 	movaps XMMWORD PTR [rsp+0x80],xmm10
   14031a99f:	00 00 
   14031a9a1:	f3 44 0f 10 15 26 71 	movss  xmm10,DWORD PTR [rip+0x9b7126]        # 0x140cd1ad0
   14031a9a8:	9b 00 
   14031a9aa:	89 44 24 34          	mov    DWORD PTR [rsp+0x34],eax
   14031a9ae:	83 c8 ff             	or     eax,0xffffffff
   14031a9b1:	2b c5                	sub    eax,ebp
   14031a9b3:	c1 e2 04             	shl    edx,0x4
   14031a9b6:	44 0f 29 5c 24 70    	movaps XMMWORD PTR [rsp+0x70],xmm11
   14031a9bc:	f3 44 0f 10 1d a7 2b 	movss  xmm11,DWORD PTR [rip+0x42ba7]        # 0x14035d56c
   14031a9c3:	04 00 
   14031a9c5:	c1 e1 04             	shl    ecx,0x4
   14031a9c8:	44 0f 29 64 24 60    	movaps XMMWORD PTR [rsp+0x60],xmm12
   14031a9ce:	f3 44 0f 10 25 a5 60 	movss  xmm12,DWORD PTR [rip+0x2b60a5]        # 0x1405d0a7c
   14031a9d5:	2b 00 
   14031a9d7:	89 4c 24 2c          	mov    DWORD PTR [rsp+0x2c],ecx
   14031a9db:	4c 89 6c 24 50       	mov    QWORD PTR [rsp+0x50],r13
   14031a9e0:	89 44 24 30          	mov    DWORD PTR [rsp+0x30],eax
   14031a9e4:	89 94 24 38 01 00 00 	mov    DWORD PTR [rsp+0x138],edx
   14031a9eb:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]
   14031a9f0:	8b c8                	mov    ecx,eax
   14031a9f2:	48 8d 05 97 60 2b 00 	lea    rax,[rip+0x2b6097]        # 0x1405d0a90
   14031a9f9:	83 e1 1f             	and    ecx,0x1f
   14031a9fc:	8b e9                	mov    ebp,ecx
   14031a9fe:	c1 e5 05             	shl    ebp,0x5
   14031aa01:	49 03 ee             	add    rbp,r14
   14031aa04:	44 8d 71 01          	lea    r14d,[rcx+0x1]
   14031aa08:	41 83 e6 1f          	and    r14d,0x1f
   14031aa0c:	41 c1 e6 05          	shl    r14d,0x5
   14031aa10:	4c 03 f0             	add    r14,rax
   14031aa13:	8d 41 02             	lea    eax,[rcx+0x2]
   14031aa16:	83 e0 1f             	and    eax,0x1f
   14031aa19:	48 8d 0d 70 60 2b 00 	lea    rcx,[rip+0x2b6070]        # 0x1405d0a90
   14031aa20:	c1 e0 05             	shl    eax,0x5
   14031aa23:	48 03 c1             	add    rax,rcx
   14031aa26:	48 89 44 24 48       	mov    QWORD PTR [rsp+0x48],rax
   14031aa2b:	42 0f b6 44 2d 00    	movzx  eax,BYTE PTR [rbp+r13*1+0x0]
   14031aa31:	89 84 24 28 01 00 00 	mov    DWORD PTR [rsp+0x128],eax
   14031aa38:	43 0f b6 04 2e       	movzx  eax,BYTE PTR [r14+r13*1]
   14031aa3d:	89 84 24 20 01 00 00 	mov    DWORD PTR [rsp+0x120],eax
   14031aa44:	8b 44 24 34          	mov    eax,DWORD PTR [rsp+0x34]
   14031aa48:	89 44 24 24          	mov    DWORD PTR [rsp+0x24],eax
   14031aa4c:	41 3b 87 58 5d 01 00 	cmp    eax,DWORD PTR [r15+0x15d58]
   14031aa53:	0f 8f 6a 02 00 00    	jg     0x14031acc3
   14031aa59:	8b 4c 24 2c          	mov    ecx,DWORD PTR [rsp+0x2c]
   14031aa5d:	41 83 cd ff          	or     r13d,0xffffffff
   14031aa61:	44 2b 6c 24 38       	sub    r13d,DWORD PTR [rsp+0x38]
   14031aa66:	03 ca                	add    ecx,edx
   14031aa68:	89 4c 24 3c          	mov    DWORD PTR [rsp+0x3c],ecx
   14031aa6c:	42 8d 0c 20          	lea    ecx,[rax+r12*1]
   14031aa70:	44 8b e1             	mov    r12d,ecx
   14031aa73:	89 8c 24 30 01 00 00 	mov    DWORD PTR [rsp+0x130],ecx
   14031aa7a:	41 c1 e4 04          	shl    r12d,0x4
   14031aa7e:	41 8b c0             	mov    eax,r8d
   14031aa81:	c1 e0 04             	shl    eax,0x4
   14031aa84:	89 44 24 40          	mov    DWORD PTR [rsp+0x40],eax
   14031aa88:	0f 1f 84 00 00 00 00 	nop    DWORD PTR [rax+rax*1+0x0]
   14031aa8f:	00 
   14031aa90:	41 8b c5             	mov    eax,r13d
   14031aa93:	41 0f 28 cb          	movaps xmm1,xmm11
   14031aa97:	83 e0 1f             	and    eax,0x1f
   14031aa9a:	41 0f 28 d3          	movaps xmm2,xmm11
   14031aa9e:	8b d0                	mov    edx,eax
   14031aaa0:	ff c0                	inc    eax
   14031aaa2:	83 e0 1f             	and    eax,0x1f
   14031aaa5:	44 0f b6 04 28       	movzx  r8d,BYTE PTR [rax+rbp*1]
   14031aaaa:	46 0f b6 0c 30       	movzx  r9d,BYTE PTR [rax+r14*1]
   14031aaaf:	41 8b 87 58 5d 01 00 	mov    eax,DWORD PTR [r15+0x15d58]
   14031aab6:	c1 e0 04             	shl    eax,0x4
   14031aab9:	66 0f 6e c0          	movd   xmm0,eax
   14031aabd:	41 8b 87 5c 5d 01 00 	mov    eax,DWORD PTR [r15+0x15d5c]
   14031aac4:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031aac7:	c1 e0 04             	shl    eax,0x4
   14031aaca:	f3 0f 5e c8          	divss  xmm1,xmm0
   14031aace:	66 0f 6e c0          	movd   xmm0,eax
   14031aad2:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   14031aad7:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031aada:	03 c1                	add    eax,ecx
   14031aadc:	8b 0d 9e 5f 2b 00    	mov    ecx,DWORD PTR [rip+0x2b5f9e]        # 0x1405d0a80
   14031aae2:	c1 e0 04             	shl    eax,0x4
   14031aae5:	f3 0f 5e d0          	divss  xmm2,xmm0
   14031aae9:	66 0f 6e c0          	movd   xmm0,eax
   14031aaed:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031aaf0:	f3 41 0f 59 d4       	mulss  xmm2,xmm12
   14031aaf5:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031aaf9:	f3 0f 58 c5          	addss  xmm0,xmm5
   14031aafd:	f3 0f 59 c7          	mulss  xmm0,xmm7
   14031ab01:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   14031ab06:	f3 41 0f 11 02       	movss  DWORD PTR [r10],xmm0
   14031ab0b:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   14031ab10:	03 c7                	add    eax,edi
   14031ab12:	c1 e0 04             	shl    eax,0x4
   14031ab15:	66 0f 6e c0          	movd   xmm0,eax
   14031ab19:	41 8b c0             	mov    eax,r8d
   14031ab1c:	2b 84 24 28 01 00 00 	sub    eax,DWORD PTR [rsp+0x128]
   14031ab23:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ab26:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031ab2a:	f3 0f 58 c4          	addss  xmm0,xmm4
   14031ab2e:	f3 0f 59 c6          	mulss  xmm0,xmm6
   14031ab32:	f3 41 0f 11 42 04    	movss  DWORD PTR [r10+0x4],xmm0
   14031ab38:	41 c7 42 08 00 00 80 	mov    DWORD PTR [r10+0x8],0x3f800000
   14031ab3f:	3f 
   14031ab40:	0f af 46 18          	imul   eax,DWORD PTR [rsi+0x18]
   14031ab44:	d3 f8                	sar    eax,cl
   14031ab46:	83 c0 08             	add    eax,0x8
   14031ab49:	41 03 c4             	add    eax,r12d
   14031ab4c:	66 0f 6e c0          	movd   xmm0,eax
   14031ab50:	8b 84 24 20 01 00 00 	mov    eax,DWORD PTR [rsp+0x120]
   14031ab57:	2b 84 24 28 01 00 00 	sub    eax,DWORD PTR [rsp+0x128]
   14031ab5e:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ab61:	f3 0f 59 c1          	mulss  xmm0,xmm1
   14031ab65:	f3 41 0f 59 c1       	mulss  xmm0,xmm9
   14031ab6a:	f3 41 0f 11 42 0c    	movss  DWORD PTR [r10+0xc],xmm0
   14031ab70:	0f af 46 1c          	imul   eax,DWORD PTR [rsi+0x1c]
   14031ab74:	d3 f8                	sar    eax,cl
   14031ab76:	8b 8c 24 38 01 00 00 	mov    ecx,DWORD PTR [rsp+0x138]
   14031ab7d:	83 c1 08             	add    ecx,0x8
   14031ab80:	03 c1                	add    eax,ecx
   14031ab82:	66 0f 6e c0          	movd   xmm0,eax
   14031ab86:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ab89:	f3 0f 59 c2          	mulss  xmm0,xmm2
   14031ab8d:	f3 41 0f 59 c1       	mulss  xmm0,xmm9
   14031ab92:	f3 41 0f 58 c2       	addss  xmm0,xmm10
   14031ab97:	f3 41 0f 11 42 10    	movss  DWORD PTR [r10+0x10],xmm0
   14031ab9d:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   14031aba2:	03 84 24 30 01 00 00 	add    eax,DWORD PTR [rsp+0x130]
   14031aba9:	c1 e0 04             	shl    eax,0x4
   14031abac:	66 0f 6e c0          	movd   xmm0,eax
   14031abb0:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031abb3:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031abb7:	f3 0f 58 c5          	addss  xmm0,xmm5
   14031abbb:	f3 0f 59 c7          	mulss  xmm0,xmm7
   14031abbf:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   14031abc4:	f3 41 0f 11 42 14    	movss  DWORD PTR [r10+0x14],xmm0
   14031abca:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   14031abcf:	03 c7                	add    eax,edi
   14031abd1:	03 c3                	add    eax,ebx
   14031abd3:	c1 e0 04             	shl    eax,0x4
   14031abd6:	66 0f 6e c0          	movd   xmm0,eax
   14031abda:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031abdd:	8b 0d 9d 5e 2b 00    	mov    ecx,DWORD PTR [rip+0x2b5e9d]        # 0x1405d0a80
   14031abe3:	41 8b c1             	mov    eax,r9d
   14031abe6:	2b 84 24 20 01 00 00 	sub    eax,DWORD PTR [rsp+0x120]
   14031abed:	41 ff c5             	inc    r13d
   14031abf0:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031abf4:	44 89 84 24 28 01 00 	mov    DWORD PTR [rsp+0x128],r8d
   14031abfb:	00 
   14031abfc:	44 8b 44 24 20       	mov    r8d,DWORD PTR [rsp+0x20]
   14031ac01:	f3 0f 58 c4          	addss  xmm0,xmm4
   14031ac05:	f3 0f 59 c6          	mulss  xmm0,xmm6
   14031ac09:	f3 41 0f 11 42 18    	movss  DWORD PTR [r10+0x18],xmm0
   14031ac0f:	41 c7 42 1c 00 00 80 	mov    DWORD PTR [r10+0x1c],0x3f800000
   14031ac16:	3f 
   14031ac17:	0f af 46 18          	imul   eax,DWORD PTR [rsi+0x18]
   14031ac1b:	d3 f8                	sar    eax,cl
   14031ac1d:	83 c0 08             	add    eax,0x8
   14031ac20:	41 03 c4             	add    eax,r12d
   14031ac23:	44 03 64 24 40       	add    r12d,DWORD PTR [rsp+0x40]
   14031ac28:	66 0f 6e c0          	movd   xmm0,eax
   14031ac2c:	48 8b 44 24 48       	mov    rax,QWORD PTR [rsp+0x48]
   14031ac31:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ac34:	0f b6 04 02          	movzx  eax,BYTE PTR [rdx+rax*1]
   14031ac38:	2b 84 24 20 01 00 00 	sub    eax,DWORD PTR [rsp+0x120]
   14031ac3f:	44 89 8c 24 20 01 00 	mov    DWORD PTR [rsp+0x120],r9d
   14031ac46:	00 
   14031ac47:	f3 0f 59 c1          	mulss  xmm0,xmm1
   14031ac4b:	f3 41 0f 59 c1       	mulss  xmm0,xmm9
   14031ac50:	f3 41 0f 11 42 20    	movss  DWORD PTR [r10+0x20],xmm0
   14031ac56:	0f af 46 1c          	imul   eax,DWORD PTR [rsi+0x1c]
   14031ac5a:	d3 f8                	sar    eax,cl
   14031ac5c:	8b 4c 24 3c          	mov    ecx,DWORD PTR [rsp+0x3c]
   14031ac60:	83 c1 09             	add    ecx,0x9
   14031ac63:	03 c1                	add    eax,ecx
   14031ac65:	8b 8c 24 30 01 00 00 	mov    ecx,DWORD PTR [rsp+0x130]
   14031ac6c:	41 03 c8             	add    ecx,r8d
   14031ac6f:	89 8c 24 30 01 00 00 	mov    DWORD PTR [rsp+0x130],ecx
   14031ac76:	66 0f 6e c0          	movd   xmm0,eax
   14031ac7a:	8b 44 24 24          	mov    eax,DWORD PTR [rsp+0x24]
   14031ac7e:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ac81:	41 03 c0             	add    eax,r8d
   14031ac84:	89 44 24 24          	mov    DWORD PTR [rsp+0x24],eax
   14031ac88:	f3 0f 59 c2          	mulss  xmm0,xmm2
   14031ac8c:	f3 41 0f 59 c1       	mulss  xmm0,xmm9
   14031ac91:	f3 41 0f 58 c2       	addss  xmm0,xmm10
   14031ac96:	f3 41 0f 11 42 24    	movss  DWORD PTR [r10+0x24],xmm0
   14031ac9c:	49 83 c2 28          	add    r10,0x28
   14031aca0:	41 3b 87 58 5d 01 00 	cmp    eax,DWORD PTR [r15+0x15d58]
   14031aca7:	0f 8e e3 fd ff ff    	jle    0x14031aa90
   14031acad:	8b 94 24 38 01 00 00 	mov    edx,DWORD PTR [rsp+0x138]
   14031acb4:	44 8b 64 24 44       	mov    r12d,DWORD PTR [rsp+0x44]
   14031acb9:	44 8b 4c 24 28       	mov    r9d,DWORD PTR [rsp+0x28]
   14031acbe:	4c 8b 6c 24 50       	mov    r13,QWORD PTR [rsp+0x50]
   14031acc3:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   14031acc8:	4c 8d 35 c1 5d 2b 00 	lea    r14,[rip+0x2b5dc1]        # 0x1405d0a90
   14031accf:	03 54 24 2c          	add    edx,DWORD PTR [rsp+0x2c]
   14031acd3:	41 03 c4             	add    eax,r12d
   14031acd6:	41 03 87 58 5d 01 00 	add    eax,DWORD PTR [r15+0x15d58]
   14031acdd:	33 c9                	xor    ecx,ecx
   14031acdf:	c1 e0 04             	shl    eax,0x4
   14031ace2:	44 03 cb             	add    r9d,ebx
   14031ace5:	44 89 4c 24 28       	mov    DWORD PTR [rsp+0x28],r9d
   14031acea:	89 94 24 38 01 00 00 	mov    DWORD PTR [rsp+0x138],edx
   14031acf1:	66 0f 6e c0          	movd   xmm0,eax
   14031acf5:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031acf8:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031acfc:	f3 0f 58 c5          	addss  xmm0,xmm5
   14031ad00:	f3 0f 59 c7          	mulss  xmm0,xmm7
   14031ad04:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   14031ad09:	f3 41 0f 11 02       	movss  DWORD PTR [r10],xmm0
   14031ad0e:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   14031ad13:	03 c7                	add    eax,edi
   14031ad15:	03 c3                	add    eax,ebx
   14031ad17:	c1 e0 04             	shl    eax,0x4
   14031ad1a:	66 0f 6e c0          	movd   xmm0,eax
   14031ad1e:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ad21:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031ad25:	f3 0f 58 c4          	addss  xmm0,xmm4
   14031ad29:	f3 0f 59 c6          	mulss  xmm0,xmm6
   14031ad2d:	f3 41 0f 11 42 04    	movss  DWORD PTR [r10+0x4],xmm0
   14031ad33:	49 c7 42 08 00 00 80 	mov    QWORD PTR [r10+0x8],0x3f800000
   14031ad3a:	3f 
   14031ad3b:	41 89 4a 10          	mov    DWORD PTR [r10+0x10],ecx
   14031ad3f:	41 0f bf 43 34       	movsx  eax,WORD PTR [r11+0x34]
   14031ad44:	41 2b c0             	sub    eax,r8d
   14031ad47:	41 03 c4             	add    eax,r12d
   14031ad4a:	c1 e0 04             	shl    eax,0x4
   14031ad4d:	66 0f 6e c0          	movd   xmm0,eax
   14031ad51:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ad54:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031ad58:	f3 0f 58 c5          	addss  xmm0,xmm5
   14031ad5c:	f3 0f 59 c7          	mulss  xmm0,xmm7
   14031ad60:	f3 41 0f 59 c0       	mulss  xmm0,xmm8
   14031ad65:	f3 41 0f 11 42 14    	movss  DWORD PTR [r10+0x14],xmm0
   14031ad6b:	41 0f bf 43 36       	movsx  eax,WORD PTR [r11+0x36]
   14031ad70:	03 c7                	add    eax,edi
   14031ad72:	03 fb                	add    edi,ebx
   14031ad74:	03 c3                	add    eax,ebx
   14031ad76:	c1 e0 04             	shl    eax,0x4
   14031ad79:	66 0f 6e c0          	movd   xmm0,eax
   14031ad7d:	8b 44 24 30          	mov    eax,DWORD PTR [rsp+0x30]
   14031ad81:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031ad84:	ff c0                	inc    eax
   14031ad86:	89 44 24 30          	mov    DWORD PTR [rsp+0x30],eax
   14031ad8a:	f3 0f 59 c3          	mulss  xmm0,xmm3
   14031ad8e:	f3 0f 58 c4          	addss  xmm0,xmm4
   14031ad92:	f3 0f 59 c6          	mulss  xmm0,xmm6
   14031ad96:	f3 41 0f 11 42 18    	movss  DWORD PTR [r10+0x18],xmm0
   14031ad9c:	49 c7 42 1c 00 00 80 	mov    QWORD PTR [r10+0x1c],0x3f800000
   14031ada3:	3f 
   14031ada4:	41 89 4a 24          	mov    DWORD PTR [r10+0x24],ecx
   14031ada8:	49 83 c2 28          	add    r10,0x28
   14031adac:	45 3b 8f 5c 5d 01 00 	cmp    r9d,DWORD PTR [r15+0x15d5c]
   14031adb3:	0f 8e 37 fc ff ff    	jle    0x14031a9f0
   14031adb9:	44 0f 28 64 24 60    	movaps xmm12,XMMWORD PTR [rsp+0x60]
   14031adbf:	44 0f 28 5c 24 70    	movaps xmm11,XMMWORD PTR [rsp+0x70]
   14031adc5:	44 0f 28 94 24 80 00 	movaps xmm10,XMMWORD PTR [rsp+0x80]
   14031adcc:	00 00 
   14031adce:	44 0f 28 8c 24 90 00 	movaps xmm9,XMMWORD PTR [rsp+0x90]
   14031add5:	00 00 
   14031add7:	44 0f 28 84 24 a0 00 	movaps xmm8,XMMWORD PTR [rsp+0xa0]
   14031adde:	00 00 
   14031ade0:	0f 28 bc 24 b0 00 00 	movaps xmm7,XMMWORD PTR [rsp+0xb0]
   14031ade7:	00 
   14031ade8:	0f 28 b4 24 c0 00 00 	movaps xmm6,XMMWORD PTR [rsp+0xc0]
   14031adef:	00 
   14031adf0:	4c 8b ac 24 d8 00 00 	mov    r13,QWORD PTR [rsp+0xd8]
   14031adf7:	00 
   14031adf8:	48 81 c4 e0 00 00 00 	add    rsp,0xe0
   14031adff:	41 5f                	pop    r15
   14031ae01:	41 5e                	pop    r14
   14031ae03:	41 5c                	pop    r12
   14031ae05:	5f                   	pop    rdi
   14031ae06:	5e                   	pop    rsi
   14031ae07:	5d                   	pop    rbp
   14031ae08:	5b                   	pop    rbx
   14031ae09:	c3                   	ret
