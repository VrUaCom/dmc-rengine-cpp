
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

00000001400446f0 <.text+0x436f0>:
   1400446f0:	40 55                	rex push rbp
   1400446f2:	57                   	push   rdi
   1400446f3:	41 54                	push   r12
   1400446f5:	41 57                	push   r15
   1400446f7:	48 8d ac 24 48 ff ff 	lea    rbp,[rsp-0xb8]
   1400446fe:	ff 
   1400446ff:	48 81 ec b8 01 00 00 	sub    rsp,0x1b8
   140044706:	48 8b 05 a3 09 59 00 	mov    rax,QWORD PTR [rip+0x5909a3]        # 0x1405d50b0
   14004470d:	48 33 c4             	xor    rax,rsp
   140044710:	48 89 45 00          	mov    QWORD PTR [rbp+0x0],rax
   140044714:	ba 0f 00 00 00       	mov    edx,0xf
   140044719:	48 89 4c 24 38       	mov    QWORD PTR [rsp+0x38],rcx
   14004471e:	48 8b f9             	mov    rdi,rcx
   140044721:	48 8d 0d 78 88 bc 00 	lea    rcx,[rip+0xbc8878]        # 0x140c0cfa0
   140044728:	44 8d 42 fb          	lea    r8d,[rdx-0x5]
   14004472c:	e8 af 18 00 00       	call   0x140045fe0
   140044731:	0f 28 05 38 d3 32 00 	movaps xmm0,XMMWORD PTR [rip+0x32d338]        # 0x140371a70
   140044738:	4c 8d 45 d0          	lea    r8,[rbp-0x30]
   14004473c:	0f 28 0d 2d 9b 59 00 	movaps xmm1,XMMWORD PTR [rip+0x599b2d]        # 0x1405de270
   140044743:	48 8d 0d 56 88 bc 00 	lea    rcx,[rip+0xbc8856]        # 0x140c0cfa0
   14004474a:	0f 11 45 d0          	movups XMMWORD PTR [rbp-0x30],xmm0
   14004474e:	ba 30 00 00 00       	mov    edx,0x30
   140044753:	c6 05 86 87 bc 00 00 	mov    BYTE PTR [rip+0xbc8786],0x0        # 0x140c0cee0
   14004475a:	0f 28 05 ef 9a 59 00 	movaps xmm0,XMMWORD PTR [rip+0x599aef]        # 0x1405de250
   140044761:	0f 11 45 e0          	movups XMMWORD PTR [rbp-0x20],xmm0
   140044765:	0f 11 4d f0          	movups XMMWORD PTR [rbp-0x10],xmm1
   140044769:	e8 72 19 00 00       	call   0x1400460e0
   14004476e:	33 d2                	xor    edx,edx
   140044770:	33 c9                	xor    ecx,ecx
   140044772:	44 8d 42 01          	lea    r8d,[rdx+0x1]
   140044776:	e8 95 21 00 00       	call   0x140046910
   14004477b:	45 33 e4             	xor    r12d,r12d
   14004477e:	4c 8d 3d 6b 6c ba 00 	lea    r15,[rip+0xba6c6b]        # 0x140beb3f0
   140044785:	4c 89 25 0c 4f 59 00 	mov    QWORD PTR [rip+0x594f0c],r12        # 0x1405d9698
   14004478c:	41 8b c4             	mov    eax,r12d
   14004478f:	89 44 24 30          	mov    DWORD PTR [rsp+0x30],eax
   140044793:	44 39 67 18          	cmp    DWORD PTR [rdi+0x18],r12d
   140044797:	0f 86 1c 05 00 00    	jbe    0x140044cb9
   14004479d:	4c 89 ac 24 f8 01 00 	mov    QWORD PTR [rsp+0x1f8],r13
   1400447a4:	00 
   1400447a5:	44 0f 29 9c 24 50 01 	movaps XMMWORD PTR [rsp+0x150],xmm11
   1400447ac:	00 00 
   1400447ae:	f3 44 0f 10 1d b5 8d 	movss  xmm11,DWORD PTR [rip+0x318db5]        # 0x14035d56c
   1400447b5:	31 00 
   1400447b7:	48 89 9c 24 e8 01 00 	mov    QWORD PTR [rsp+0x1e8],rbx
   1400447be:	00 
   1400447bf:	48 89 b4 24 f0 01 00 	mov    QWORD PTR [rsp+0x1f0],rsi
   1400447c6:	00 
   1400447c7:	4c 89 b4 24 b0 01 00 	mov    QWORD PTR [rsp+0x1b0],r14
   1400447ce:	00 
   1400447cf:	0f 29 b4 24 a0 01 00 	movaps XMMWORD PTR [rsp+0x1a0],xmm6
   1400447d6:	00 
   1400447d7:	0f 29 bc 24 90 01 00 	movaps XMMWORD PTR [rsp+0x190],xmm7
   1400447de:	00 
   1400447df:	44 0f 29 84 24 80 01 	movaps XMMWORD PTR [rsp+0x180],xmm8
   1400447e6:	00 00 
   1400447e8:	44 0f 29 8c 24 70 01 	movaps XMMWORD PTR [rsp+0x170],xmm9
   1400447ef:	00 00 
   1400447f1:	44 0f 29 94 24 60 01 	movaps XMMWORD PTR [rsp+0x160],xmm10
   1400447f8:	00 00 
   1400447fa:	44 0f 29 a4 24 40 01 	movaps XMMWORD PTR [rsp+0x140],xmm12
   140044801:	00 00 
   140044803:	44 0f 29 ac 24 30 01 	movaps XMMWORD PTR [rsp+0x130],xmm13
   14004480a:	00 00 
   14004480c:	44 0f 29 b4 24 20 01 	movaps XMMWORD PTR [rsp+0x120],xmm14
   140044813:	00 00 
   140044815:	44 0f 29 bc 24 10 01 	movaps XMMWORD PTR [rsp+0x110],xmm15
   14004481c:	00 00 
   14004481e:	66 90                	xchg   ax,ax
   140044820:	44 8b e8             	mov    r13d,eax
   140044823:	48 8b 47 08          	mov    rax,QWORD PTR [rdi+0x8]
   140044827:	4d 03 ed             	add    r13,r13
   14004482a:	42 83 3c e8 00       	cmp    DWORD PTR [rax+r13*8],0x0
   14004482f:	0f 8c f9 03 00 00    	jl     0x140044c2e
   140044835:	48 8b 47 10          	mov    rax,QWORD PTR [rdi+0x10]
   140044839:	48 8d 4c 24 70       	lea    rcx,[rsp+0x70]
   14004483e:	48 8b 15 cb ce 59 00 	mov    rdx,QWORD PTR [rip+0x59cecb]        # 0x1405e1710
   140044845:	46 8b 34 e8          	mov    r14d,DWORD PTR [rax+r13*8]
   140044849:	42 8b 74 e8 04       	mov    esi,DWORD PTR [rax+r13*8+0x4]
   14004484e:	45 8b c6             	mov    r8d,r14d
   140044851:	42 8b 5c e8 08       	mov    ebx,DWORD PTR [rax+r13*8+0x8]
   140044856:	48 8b 44 24 38       	mov    rax,QWORD PTR [rsp+0x38]
   14004485b:	49 c1 e0 04          	shl    r8,0x4
   14004485f:	4c 03 00             	add    r8,QWORD PTR [rax]
   140044862:	e8 89 cb fe ff       	call   0x1400313f0
   140044867:	48 8b 15 a2 ce 59 00 	mov    rdx,QWORD PTR [rip+0x59cea2]        # 0x1405e1710
   14004486e:	48 8d 4d 80          	lea    rcx,[rbp-0x80]
   140044872:	48 c1 e6 04          	shl    rsi,0x4
   140044876:	0f 28 30             	movaps xmm6,XMMWORD PTR [rax]
   140044879:	48 8b 44 24 38       	mov    rax,QWORD PTR [rsp+0x38]
   14004487e:	4c 8b 00             	mov    r8,QWORD PTR [rax]
   140044881:	4c 03 c6             	add    r8,rsi
   140044884:	e8 67 cb fe ff       	call   0x1400313f0
   140044889:	48 8b 7c 24 38       	mov    rdi,QWORD PTR [rsp+0x38]
   14004488e:	48 8d 4d 90          	lea    rcx,[rbp-0x70]
   140044892:	48 8b 15 77 ce 59 00 	mov    rdx,QWORD PTR [rip+0x59ce77]        # 0x1405e1710
   140044899:	48 c1 e3 04          	shl    rbx,0x4
   14004489d:	44 0f 28 08          	movaps xmm9,XMMWORD PTR [rax]
   1400448a1:	4c 8b 07             	mov    r8,QWORD PTR [rdi]
   1400448a4:	4c 03 c3             	add    r8,rbx
   1400448a7:	e8 44 cb fe ff       	call   0x1400313f0
   1400448ac:	48 8b 15 5d ce 59 00 	mov    rdx,QWORD PTR [rip+0x59ce5d]        # 0x1405e1710
   1400448b3:	48 8d 4d a0          	lea    rcx,[rbp-0x60]
   1400448b7:	49 c1 e6 04          	shl    r14,0x4
   1400448bb:	48 83 c2 40          	add    rdx,0x40
   1400448bf:	4c 03 37             	add    r14,QWORD PTR [rdi]
   1400448c2:	0f 28 38             	movaps xmm7,XMMWORD PTR [rax]
   1400448c5:	4d 8b c6             	mov    r8,r14
   1400448c8:	e8 23 cb fe ff       	call   0x1400313f0
   1400448cd:	4c 8b 07             	mov    r8,QWORD PTR [rdi]
   1400448d0:	48 8d 4d b0          	lea    rcx,[rbp-0x50]
   1400448d4:	48 8b 15 35 ce 59 00 	mov    rdx,QWORD PTR [rip+0x59ce35]        # 0x1405e1710
   1400448db:	4c 03 c6             	add    r8,rsi
   1400448de:	48 83 c2 40          	add    rdx,0x40
   1400448e2:	44 0f 28 00          	movaps xmm8,XMMWORD PTR [rax]
   1400448e6:	e8 05 cb fe ff       	call   0x1400313f0
   1400448eb:	4c 8b 07             	mov    r8,QWORD PTR [rdi]
   1400448ee:	48 8d 4d c0          	lea    rcx,[rbp-0x40]
   1400448f2:	48 8b 15 17 ce 59 00 	mov    rdx,QWORD PTR [rip+0x59ce17]        # 0x1405e1710
   1400448f9:	4c 03 c3             	add    r8,rbx
   1400448fc:	48 83 c2 40          	add    rdx,0x40
   140044900:	44 0f 28 10          	movaps xmm10,XMMWORD PTR [rax]
   140044904:	e8 e7 ca fe ff       	call   0x1400313f0
   140044909:	0f 28 c6             	movaps xmm0,xmm6
   14004490c:	41 0f 28 cb          	movaps xmm1,xmm11
   140044910:	0f c6 c6 ff          	shufps xmm0,xmm6,0xff
   140044914:	f3 0f 5e c8          	divss  xmm1,xmm0
   140044918:	0f 28 10             	movaps xmm2,XMMWORD PTR [rax]
   14004491b:	41 0f 28 c1          	movaps xmm0,xmm9
   14004491f:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   140044923:	0f 59 f1             	mulps  xmm6,xmm1
   140044926:	41 0f 28 cb          	movaps xmm1,xmm11
   14004492a:	41 0f c6 c1 ff       	shufps xmm0,xmm9,0xff
   14004492f:	f3 0f 5e c8          	divss  xmm1,xmm0
   140044933:	0f 28 c7             	movaps xmm0,xmm7
   140044936:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   14004493a:	44 0f 59 c9          	mulps  xmm9,xmm1
   14004493e:	41 0f 28 cb          	movaps xmm1,xmm11
   140044942:	0f c6 c7 ff          	shufps xmm0,xmm7,0xff
   140044946:	f3 0f 5e c8          	divss  xmm1,xmm0
   14004494a:	41 0f 28 c0          	movaps xmm0,xmm8
   14004494e:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   140044952:	0f 59 f9             	mulps  xmm7,xmm1
   140044955:	41 0f 28 cb          	movaps xmm1,xmm11
   140044959:	41 0f c6 c0 ff       	shufps xmm0,xmm8,0xff
   14004495e:	f3 0f 5e c8          	divss  xmm1,xmm0
   140044962:	41 0f 28 c2          	movaps xmm0,xmm10
   140044966:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   14004496a:	44 0f 59 c1          	mulps  xmm8,xmm1
   14004496e:	41 0f 28 cb          	movaps xmm1,xmm11
   140044972:	41 0f c6 c2 ff       	shufps xmm0,xmm10,0xff
   140044977:	f3 0f 5e c8          	divss  xmm1,xmm0
   14004497b:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   14004497f:	44 0f 59 d1          	mulps  xmm10,xmm1
   140044983:	41 0f 28 cb          	movaps xmm1,xmm11
   140044987:	f3 41 0f 11 3f       	movss  DWORD PTR [r15],xmm7
   14004498c:	0f 28 c2             	movaps xmm0,xmm2
   14004498f:	0f c6 c2 ff          	shufps xmm0,xmm2,0xff
   140044993:	0f 28 ef             	movaps xmm5,xmm7
   140044996:	f3 0f 5e c8          	divss  xmm1,xmm0
   14004499a:	41 83 c4 06          	add    r12d,0x6
   14004499e:	0f c6 ef 55          	shufps xmm5,xmm7,0x55
   1400449a2:	44 0f 28 df          	movaps xmm11,xmm7
   1400449a6:	f3 41 0f 11 6f 04    	movss  DWORD PTR [r15+0x4],xmm5
   1400449ac:	45 0f 28 f1          	movaps xmm14,xmm9
   1400449b0:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   1400449b4:	45 0f 28 f9          	movaps xmm15,xmm9
   1400449b8:	0f 59 d1             	mulps  xmm2,xmm1
   1400449bb:	0f 28 c6             	movaps xmm0,xmm6
   1400449be:	44 0f c6 df aa       	shufps xmm11,xmm7,0xaa
   1400449c3:	0f 28 ce             	movaps xmm1,xmm6
   1400449c6:	f3 45 0f 11 5f 08    	movss  DWORD PTR [r15+0x8],xmm11
   1400449cc:	45 0f 28 e0          	movaps xmm12,xmm8
   1400449d0:	f3 45 0f 11 4f 0c    	movss  DWORD PTR [r15+0xc],xmm9
   1400449d6:	45 0f 28 e8          	movaps xmm13,xmm8
   1400449da:	45 0f c6 f1 55       	shufps xmm14,xmm9,0x55
   1400449df:	41 0f 28 da          	movaps xmm3,xmm10
   1400449e3:	f3 45 0f 11 77 10    	movss  DWORD PTR [r15+0x10],xmm14
   1400449e9:	0f 28 e2             	movaps xmm4,xmm2
   1400449ec:	41 0f c6 da 55       	shufps xmm3,xmm10,0x55
   1400449f1:	0f 29 5c 24 40       	movaps XMMWORD PTR [rsp+0x40],xmm3
   1400449f6:	45 0f c6 f9 aa       	shufps xmm15,xmm9,0xaa
   1400449fb:	f3 45 0f 11 7f 14    	movss  DWORD PTR [r15+0x14],xmm15
   140044a01:	f3 41 0f 11 77 18    	movss  DWORD PTR [r15+0x18],xmm6
   140044a07:	0f c6 c6 55          	shufps xmm0,xmm6,0x55
   140044a0b:	f3 41 0f 11 47 1c    	movss  DWORD PTR [r15+0x1c],xmm0
   140044a11:	0f c6 ce aa          	shufps xmm1,xmm6,0xaa
   140044a15:	f3 41 0f 11 4f 20    	movss  DWORD PTR [r15+0x20],xmm1
   140044a1b:	f3 45 0f 11 47 24    	movss  DWORD PTR [r15+0x24],xmm8
   140044a21:	45 0f c6 e0 55       	shufps xmm12,xmm8,0x55
   140044a26:	f3 45 0f 11 67 28    	movss  DWORD PTR [r15+0x28],xmm12
   140044a2c:	45 0f c6 e8 aa       	shufps xmm13,xmm8,0xaa
   140044a31:	f3 45 0f 11 6f 2c    	movss  DWORD PTR [r15+0x2c],xmm13
   140044a37:	f3 45 0f 11 57 30    	movss  DWORD PTR [r15+0x30],xmm10
   140044a3d:	f3 41 0f 11 5f 34    	movss  DWORD PTR [r15+0x34],xmm3
   140044a43:	41 0f 28 da          	movaps xmm3,xmm10
   140044a47:	41 0f c6 da aa       	shufps xmm3,xmm10,0xaa
   140044a4c:	f3 41 0f 11 5f 38    	movss  DWORD PTR [r15+0x38],xmm3
   140044a52:	f3 41 0f 11 57 3c    	movss  DWORD PTR [r15+0x3c],xmm2
   140044a58:	0f 29 5c 24 50       	movaps XMMWORD PTR [rsp+0x50],xmm3
   140044a5d:	0f 28 da             	movaps xmm3,xmm2
   140044a60:	0f c6 da 55          	shufps xmm3,xmm2,0x55
   140044a64:	f3 41 0f 11 5f 40    	movss  DWORD PTR [r15+0x40],xmm3
   140044a6a:	0f c6 e2 aa          	shufps xmm4,xmm2,0xaa
   140044a6e:	f3 41 0f 11 67 44    	movss  DWORD PTR [r15+0x44],xmm4
   140044a74:	49 83 c7 48          	add    r15,0x48
   140044a78:	48 8b 47 08          	mov    rax,QWORD PTR [rdi+0x8]
   140044a7c:	0f 29 6c 24 60       	movaps XMMWORD PTR [rsp+0x60],xmm5
   140044a81:	42 83 3c e8 00       	cmp    DWORD PTR [rax+r13*8],0x0
   140044a86:	0f 85 82 00 00 00    	jne    0x140044b0e
   140044a8c:	0f 28 6c 24 40       	movaps xmm5,XMMWORD PTR [rsp+0x40]
   140044a91:	f3 45 0f 11 0f       	movss  DWORD PTR [r15],xmm9
   140044a96:	f3 45 0f 11 77 04    	movss  DWORD PTR [r15+0x4],xmm14
   140044a9c:	f3 45 0f 11 7f 08    	movss  DWORD PTR [r15+0x8],xmm15
   140044aa2:	f3 45 0f 11 57 0c    	movss  DWORD PTR [r15+0xc],xmm10
   140044aa8:	f3 41 0f 11 6f 10    	movss  DWORD PTR [r15+0x10],xmm5
   140044aae:	0f 28 6c 24 50       	movaps xmm5,XMMWORD PTR [rsp+0x50]
   140044ab3:	f3 41 0f 11 6f 14    	movss  DWORD PTR [r15+0x14],xmm5
   140044ab9:	0f 28 6c 24 60       	movaps xmm5,XMMWORD PTR [rsp+0x60]
   140044abe:	f3 45 0f 11 47 18    	movss  DWORD PTR [r15+0x18],xmm8
   140044ac4:	f3 45 0f 11 67 1c    	movss  DWORD PTR [r15+0x1c],xmm12
   140044aca:	f3 45 0f 11 6f 20    	movss  DWORD PTR [r15+0x20],xmm13
   140044ad0:	f3 45 0f 11 47 24    	movss  DWORD PTR [r15+0x24],xmm8
   140044ad6:	f3 45 0f 11 67 28    	movss  DWORD PTR [r15+0x28],xmm12
   140044adc:	f3 45 0f 11 6f 2c    	movss  DWORD PTR [r15+0x2c],xmm13
   140044ae2:	f3 41 0f 11 77 30    	movss  DWORD PTR [r15+0x30],xmm6
   140044ae8:	f3 41 0f 11 47 34    	movss  DWORD PTR [r15+0x34],xmm0
   140044aee:	f3 41 0f 11 4f 38    	movss  DWORD PTR [r15+0x38],xmm1
   140044af4:	f3 45 0f 11 4f 3c    	movss  DWORD PTR [r15+0x3c],xmm9
   140044afa:	f3 45 0f 11 77 40    	movss  DWORD PTR [r15+0x40],xmm14
   140044b00:	f3 45 0f 11 7f 44    	movss  DWORD PTR [r15+0x44],xmm15
   140044b06:	49 83 c7 48          	add    r15,0x48
   140044b0a:	41 83 c4 06          	add    r12d,0x6
   140044b0e:	48 8b 47 08          	mov    rax,QWORD PTR [rdi+0x8]
   140044b12:	42 83 7c e8 04 00    	cmp    DWORD PTR [rax+r13*8+0x4],0x0
   140044b18:	0f 85 88 00 00 00    	jne    0x140044ba6
   140044b1e:	f3 41 0f 11 3f       	movss  DWORD PTR [r15],xmm7
   140044b23:	f3 41 0f 11 6f 04    	movss  DWORD PTR [r15+0x4],xmm5
   140044b29:	0f 28 6c 24 40       	movaps xmm5,XMMWORD PTR [rsp+0x40]
   140044b2e:	f3 45 0f 11 5f 08    	movss  DWORD PTR [r15+0x8],xmm11
   140044b34:	f3 41 0f 11 57 0c    	movss  DWORD PTR [r15+0xc],xmm2
   140044b3a:	f3 41 0f 11 5f 10    	movss  DWORD PTR [r15+0x10],xmm3
   140044b40:	f3 41 0f 11 67 14    	movss  DWORD PTR [r15+0x14],xmm4
   140044b46:	f3 45 0f 11 57 18    	movss  DWORD PTR [r15+0x18],xmm10
   140044b4c:	f3 41 0f 11 6f 1c    	movss  DWORD PTR [r15+0x1c],xmm5
   140044b52:	0f 28 6c 24 50       	movaps xmm5,XMMWORD PTR [rsp+0x50]
   140044b57:	f3 41 0f 11 6f 20    	movss  DWORD PTR [r15+0x20],xmm5
   140044b5d:	f3 45 0f 11 57 24    	movss  DWORD PTR [r15+0x24],xmm10
   140044b63:	44 0f 28 54 24 40    	movaps xmm10,XMMWORD PTR [rsp+0x40]
   140044b69:	f3 45 0f 11 57 28    	movss  DWORD PTR [r15+0x28],xmm10
   140044b6f:	f3 41 0f 11 6f 2c    	movss  DWORD PTR [r15+0x2c],xmm5
   140044b75:	0f 28 6c 24 60       	movaps xmm5,XMMWORD PTR [rsp+0x60]
   140044b7a:	f3 45 0f 11 4f 30    	movss  DWORD PTR [r15+0x30],xmm9
   140044b80:	f3 45 0f 11 77 34    	movss  DWORD PTR [r15+0x34],xmm14
   140044b86:	f3 45 0f 11 7f 38    	movss  DWORD PTR [r15+0x38],xmm15
   140044b8c:	f3 41 0f 11 7f 3c    	movss  DWORD PTR [r15+0x3c],xmm7
   140044b92:	f3 41 0f 11 6f 40    	movss  DWORD PTR [r15+0x40],xmm5
   140044b98:	f3 45 0f 11 5f 44    	movss  DWORD PTR [r15+0x44],xmm11
   140044b9e:	49 83 c7 48          	add    r15,0x48
   140044ba2:	41 83 c4 06          	add    r12d,0x6
   140044ba6:	48 8b 47 08          	mov    rax,QWORD PTR [rdi+0x8]
   140044baa:	42 83 7c e8 08 00    	cmp    DWORD PTR [rax+r13*8+0x8],0x0
   140044bb0:	75 73                	jne    0x140044c25
   140044bb2:	f3 41 0f 11 37       	movss  DWORD PTR [r15],xmm6
   140044bb7:	f3 41 0f 11 47 04    	movss  DWORD PTR [r15+0x4],xmm0
   140044bbd:	f3 41 0f 11 4f 08    	movss  DWORD PTR [r15+0x8],xmm1
   140044bc3:	f3 45 0f 11 47 0c    	movss  DWORD PTR [r15+0xc],xmm8
   140044bc9:	f3 45 0f 11 67 10    	movss  DWORD PTR [r15+0x10],xmm12
   140044bcf:	f3 45 0f 11 6f 14    	movss  DWORD PTR [r15+0x14],xmm13
   140044bd5:	f3 41 0f 11 57 18    	movss  DWORD PTR [r15+0x18],xmm2
   140044bdb:	f3 41 0f 11 5f 1c    	movss  DWORD PTR [r15+0x1c],xmm3
   140044be1:	f3 41 0f 11 67 20    	movss  DWORD PTR [r15+0x20],xmm4
   140044be7:	f3 41 0f 11 57 24    	movss  DWORD PTR [r15+0x24],xmm2
   140044bed:	f3 41 0f 11 5f 28    	movss  DWORD PTR [r15+0x28],xmm3
   140044bf3:	f3 41 0f 11 67 2c    	movss  DWORD PTR [r15+0x2c],xmm4
   140044bf9:	f3 41 0f 11 7f 30    	movss  DWORD PTR [r15+0x30],xmm7
   140044bff:	f3 41 0f 11 6f 34    	movss  DWORD PTR [r15+0x34],xmm5
   140044c05:	f3 45 0f 11 5f 38    	movss  DWORD PTR [r15+0x38],xmm11
   140044c0b:	f3 41 0f 11 77 3c    	movss  DWORD PTR [r15+0x3c],xmm6
   140044c11:	f3 41 0f 11 47 40    	movss  DWORD PTR [r15+0x40],xmm0
   140044c17:	f3 41 0f 11 4f 44    	movss  DWORD PTR [r15+0x44],xmm1
   140044c1d:	49 83 c7 48          	add    r15,0x48
   140044c21:	41 83 c4 06          	add    r12d,0x6
   140044c25:	f3 44 0f 10 1d 3e 89 	movss  xmm11,DWORD PTR [rip+0x31893e]        # 0x14035d56c
   140044c2c:	31 00 
   140044c2e:	8b 44 24 30          	mov    eax,DWORD PTR [rsp+0x30]
   140044c32:	ff c0                	inc    eax
   140044c34:	89 44 24 30          	mov    DWORD PTR [rsp+0x30],eax
   140044c38:	3b 47 18             	cmp    eax,DWORD PTR [rdi+0x18]
   140044c3b:	0f 82 df fb ff ff    	jb     0x140044820
   140044c41:	44 0f 28 bc 24 10 01 	movaps xmm15,XMMWORD PTR [rsp+0x110]
   140044c48:	00 00 
   140044c4a:	44 0f 28 b4 24 20 01 	movaps xmm14,XMMWORD PTR [rsp+0x120]
   140044c51:	00 00 
   140044c53:	44 0f 28 ac 24 30 01 	movaps xmm13,XMMWORD PTR [rsp+0x130]
   140044c5a:	00 00 
   140044c5c:	44 0f 28 a4 24 40 01 	movaps xmm12,XMMWORD PTR [rsp+0x140]
   140044c63:	00 00 
   140044c65:	44 0f 28 9c 24 50 01 	movaps xmm11,XMMWORD PTR [rsp+0x150]
   140044c6c:	00 00 
   140044c6e:	44 0f 28 94 24 60 01 	movaps xmm10,XMMWORD PTR [rsp+0x160]
   140044c75:	00 00 
   140044c77:	44 0f 28 8c 24 70 01 	movaps xmm9,XMMWORD PTR [rsp+0x170]
   140044c7e:	00 00 
   140044c80:	44 0f 28 84 24 80 01 	movaps xmm8,XMMWORD PTR [rsp+0x180]
   140044c87:	00 00 
   140044c89:	0f 28 bc 24 90 01 00 	movaps xmm7,XMMWORD PTR [rsp+0x190]
   140044c90:	00 
   140044c91:	0f 28 b4 24 a0 01 00 	movaps xmm6,XMMWORD PTR [rsp+0x1a0]
   140044c98:	00 
   140044c99:	4c 8b b4 24 b0 01 00 	mov    r14,QWORD PTR [rsp+0x1b0]
   140044ca0:	00 
   140044ca1:	4c 8b ac 24 f8 01 00 	mov    r13,QWORD PTR [rsp+0x1f8]
   140044ca8:	00 
   140044ca9:	48 8b b4 24 f0 01 00 	mov    rsi,QWORD PTR [rsp+0x1f0]
   140044cb0:	00 
   140044cb1:	48 8b 9c 24 e8 01 00 	mov    rbx,QWORD PTR [rsp+0x1e8]
   140044cb8:	00 
   140044cb9:	43 8d 04 64          	lea    eax,[r12+r12*2]
   140044cbd:	ba 02 00 00 00       	mov    edx,0x2
   140044cc2:	c1 e0 02             	shl    eax,0x2
   140044cc5:	48 8d 0d 44 67 bc 00 	lea    rcx,[rip+0xbc6744]        # 0x140c0b410
   140044ccc:	4c 63 c8             	movsxd r9,eax
   140044ccf:	48 8d 05 1a 67 ba 00 	lea    rax,[rip+0xba671a]        # 0x140beb3f0
   140044cd6:	48 89 44 24 20       	mov    QWORD PTR [rsp+0x20],rax
   140044cdb:	44 8d 42 01          	lea    r8d,[rdx+0x1]
   140044cdf:	e8 ac f2 ff ff       	call   0x140043f90
   140044ce4:	c6 05 f5 81 bc 00 00 	mov    BYTE PTR [rip+0xbc81f5],0x0        # 0x140c0cee0
   140044ceb:	48 8b 4d 00          	mov    rcx,QWORD PTR [rbp+0x0]
   140044cef:	48 33 cc             	xor    rcx,rsp
   140044cf2:	e8 f9 08 30 00       	call   0x1403455f0
   140044cf7:	48 81 c4 b8 01 00 00 	add    rsp,0x1b8
   140044cfe:	41 5f                	pop    r15
   140044d00:	41 5c                	pop    r12
   140044d02:	5f                   	pop    rdi
   140044d03:	5d                   	pop    rbp
   140044d04:	c3                   	ret
