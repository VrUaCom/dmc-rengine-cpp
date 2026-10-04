
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

00000001403196a0 <.text+0x3186a0>:
   1403196a0:	40 55                	rex push rbp
   1403196a2:	56                   	push   rsi
   1403196a3:	57                   	push   rdi
   1403196a4:	41 56                	push   r14
   1403196a6:	41 57                	push   r15
   1403196a8:	48 8d 6c 24 c9       	lea    rbp,[rsp-0x37]
   1403196ad:	48 81 ec f0 00 00 00 	sub    rsp,0xf0
   1403196b4:	48 c7 45 97 fe ff ff 	mov    QWORD PTR [rbp-0x69],0xfffffffffffffffe
   1403196bb:	ff 
   1403196bc:	48 89 9c 24 30 01 00 	mov    QWORD PTR [rsp+0x130],rbx
   1403196c3:	00 
   1403196c4:	48 8b 05 e5 b9 2b 00 	mov    rax,QWORD PTR [rip+0x2bb9e5]        # 0x1405d50b0
   1403196cb:	48 33 c4             	xor    rax,rsp
   1403196ce:	48 89 45 27          	mov    QWORD PTR [rbp+0x27],rax
   1403196d2:	4c 8b f2             	mov    r14,rdx
   1403196d5:	48 8b f1             	mov    rsi,rcx
   1403196d8:	45 33 ff             	xor    r15d,r15d
   1403196db:	4c 39 3d 06 84 9b 00 	cmp    QWORD PTR [rip+0x9b8406],r15        # 0x140cd1ae8
   1403196e2:	75 4c                	jne    0x140319730
   1403196e4:	41 8d 4f 60          	lea    ecx,[r15+0x60]
   1403196e8:	e8 23 be 02 00       	call   0x140345510
   1403196ed:	48 8b d8             	mov    rbx,rax
   1403196f0:	48 89 45 87          	mov    QWORD PTR [rbp-0x79],rax
   1403196f4:	48 85 c0             	test   rax,rax
   1403196f7:	74 18                	je     0x140319711
   1403196f9:	33 d2                	xor    edx,edx
   1403196fb:	45 8d 47 60          	lea    r8d,[r15+0x60]
   1403196ff:	48 8b c8             	mov    rcx,rax
   140319702:	e8 e3 d4 02 00       	call   0x140346bea
   140319707:	48 8b cb             	mov    rcx,rbx
   14031970a:	e8 31 cb d2 ff       	call   0x140046240
   14031970f:	eb 03                	jmp    0x140319714
   140319711:	49 8b c7             	mov    rax,r15
   140319714:	48 89 05 cd 83 9b 00 	mov    QWORD PTR [rip+0x9b83cd],rax        # 0x140cd1ae8
   14031971b:	ba 40 00 00 00       	mov    edx,0x40
   140319720:	44 8d 4a dc          	lea    r9d,[rdx-0x24]
   140319724:	44 8d 42 e0          	lea    r8d,[rdx-0x20]
   140319728:	48 8b c8             	mov    rcx,rax
   14031972b:	e8 10 d7 d2 ff       	call   0x140046e40
   140319730:	48 83 3d b8 03 9c 00 	cmp    QWORD PTR [rip+0x9c03b8],0x0        # 0x140cd9af0
   140319737:	00 
   140319738:	75 0c                	jne    0x140319746
   14031973a:	e8 11 a0 d1 ff       	call   0x140033750
   14031973f:	48 89 05 aa 03 9c 00 	mov    QWORD PTR [rip+0x9c03aa],rax        # 0x140cd9af0
   140319746:	66 41 0f 6e 86 30 01 	movd   xmm0,DWORD PTR [r14+0x130]
   14031974d:	00 00 
   14031974f:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319752:	f3 0f 5e 05 ce 81 05 	divss  xmm0,DWORD PTR [rip+0x581ce]        # 0x140371928
   140319759:	00 
   14031975a:	f3 0f 59 05 b6 81 05 	mulss  xmm0,DWORD PTR [rip+0x581b6]        # 0x140371918
   140319761:	00 
   140319762:	f3 0f 58 c0          	addss  xmm0,xmm0
   140319766:	e8 bb d4 02 00       	call   0x140346c26
   14031976b:	f3 41 0f 11 86 50 01 	movss  DWORD PTR [r14+0x150],xmm0
   140319772:	00 00 
   140319774:	66 41 0f 6e 86 30 01 	movd   xmm0,DWORD PTR [r14+0x130]
   14031977b:	00 00 
   14031977d:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319780:	f3 0f 5e 05 a0 81 05 	divss  xmm0,DWORD PTR [rip+0x581a0]        # 0x140371928
   140319787:	00 
   140319788:	f3 0f 59 05 88 81 05 	mulss  xmm0,DWORD PTR [rip+0x58188]        # 0x140371918
   14031978f:	00 
   140319790:	f3 0f 58 c0          	addss  xmm0,xmm0
   140319794:	e8 87 d4 02 00       	call   0x140346c20
   140319799:	f3 41 0f 11 86 58 01 	movss  DWORD PTR [r14+0x158],xmm0
   1403197a0:	00 00 
   1403197a2:	45 89 be 54 01 00 00 	mov    DWORD PTR [r14+0x154],r15d
   1403197a9:	f3 41 0f 10 8e 34 01 	movss  xmm1,DWORD PTR [r14+0x134]
   1403197b0:	00 00 
   1403197b2:	0f c6 c9 00          	shufps xmm1,xmm1,0x0
   1403197b6:	41 0f 28 86 50 01 00 	movaps xmm0,XMMWORD PTR [r14+0x150]
   1403197bd:	00 
   1403197be:	0f 59 c1             	mulps  xmm0,xmm1
   1403197c1:	41 0f 29 86 50 01 00 	movaps XMMWORD PTR [r14+0x150],xmm0
   1403197c8:	00 
   1403197c9:	45 89 be 5c 01 00 00 	mov    DWORD PTR [r14+0x15c],r15d
   1403197d0:	0f 28 05 39 e8 1b 00 	movaps xmm0,XMMWORD PTR [rip+0x1be839]        # 0x1404d8010
   1403197d7:	0f 29 45 d7          	movaps XMMWORD PTR [rbp-0x29],xmm0
   1403197db:	48 8b 15 4e 8b 9d 00 	mov    rdx,QWORD PTR [rip+0x9d8b4e]        # 0x140cf2330
   1403197e2:	48 81 c2 00 01 00 00 	add    rdx,0x100
   1403197e9:	4d 8d 86 60 01 00 00 	lea    r8,[r14+0x160]
   1403197f0:	48 8d 4d b7          	lea    rcx,[rbp-0x49]
   1403197f4:	e8 77 72 d1 ff       	call   0x140030a70
   1403197f9:	48 8b 15 30 8b 9d 00 	mov    rdx,QWORD PTR [rip+0x9d8b30]        # 0x140cf2330
   140319800:	48 81 c2 00 01 00 00 	add    rdx,0x100
   140319807:	4d 8d 86 50 01 00 00 	lea    r8,[r14+0x150]
   14031980e:	48 8d 4d c7          	lea    rcx,[rbp-0x39]
   140319812:	e8 59 72 d1 ff       	call   0x140030a70
   140319817:	48 8b 15 12 8b 9d 00 	mov    rdx,QWORD PTR [rip+0x9d8b12]        # 0x140cf2330
   14031981e:	48 81 c2 00 01 00 00 	add    rdx,0x100
   140319825:	48 8d 4d e7          	lea    rcx,[rbp-0x19]
   140319829:	e8 92 75 d1 ff       	call   0x140030dc0
   14031982e:	4c 8d 45 d7          	lea    r8,[rbp-0x29]
   140319832:	48 8d 55 e7          	lea    rdx,[rbp-0x19]
   140319836:	49 8d 8e 60 01 00 00 	lea    rcx,[r14+0x160]
   14031983d:	e8 2e 72 d1 ff       	call   0x140030a70
   140319842:	48 8d 4d b7          	lea    rcx,[rbp-0x49]
   140319846:	e8 35 56 01 00       	call   0x14032ee80
   14031984b:	f3 0f 10 0d 19 3d 04 	movss  xmm1,DWORD PTR [rip+0x43d19]        # 0x14035d56c
   140319852:	00 
   140319853:	0f 2f c8             	comiss xmm1,xmm0
   140319856:	76 1a                	jbe    0x140319872
   140319858:	0f 57 e4             	xorps  xmm4,xmm4
   14031985b:	f3 0f 11 65 bf       	movss  DWORD PTR [rbp-0x41],xmm4
   140319860:	0f 28 dc             	movaps xmm3,xmm4
   140319863:	f3 0f 11 65 bb       	movss  DWORD PTR [rbp-0x45],xmm4
   140319868:	0f 28 ec             	movaps xmm5,xmm4
   14031986b:	f3 0f 11 65 b7       	movss  DWORD PTR [rbp-0x49],xmm4
   140319870:	eb 0f                	jmp    0x140319881
   140319872:	f3 0f 10 65 bf       	movss  xmm4,DWORD PTR [rbp-0x41]
   140319877:	f3 0f 10 5d bb       	movss  xmm3,DWORD PTR [rbp-0x45]
   14031987c:	f3 0f 10 6d b7       	movss  xmm5,DWORD PTR [rbp-0x49]
   140319881:	f3 41 0f 10 96 20 01 	movss  xmm2,DWORD PTR [r14+0x120]
   140319888:	00 00 
   14031988a:	f3 0f 59 25 1a e4 1e 	mulss  xmm4,DWORD PTR [rip+0x1ee41a]        # 0x140507cac
   140319891:	00 
   140319892:	f3 0f 59 e2          	mulss  xmm4,xmm2
   140319896:	f3 0f 10 45 cf       	movss  xmm0,DWORD PTR [rbp-0x31]
   14031989b:	f3 0f 59 05 09 e4 1e 	mulss  xmm0,DWORD PTR [rip+0x1ee409]        # 0x140507cac
   1403198a2:	00 
   1403198a3:	f3 0f 58 e0          	addss  xmm4,xmm0
   1403198a7:	f3 0f 10 0d f9 e3 1e 	movss  xmm1,DWORD PTR [rip+0x1ee3f9]        # 0x140507ca8
   1403198ae:	00 
   1403198af:	f3 0f 59 e9          	mulss  xmm5,xmm1
   1403198b3:	f3 0f 59 ea          	mulss  xmm5,xmm2
   1403198b7:	f3 0f 10 45 c7       	movss  xmm0,DWORD PTR [rbp-0x39]
   1403198bc:	f3 0f 59 c1          	mulss  xmm0,xmm1
   1403198c0:	f3 0f 58 e8          	addss  xmm5,xmm0
   1403198c4:	f3 41 0f 10 96 40 01 	movss  xmm2,DWORD PTR [r14+0x140]
   1403198cb:	00 00 
   1403198cd:	f3 0f 5c d5          	subss  xmm2,xmm5
   1403198d1:	f3 41 0f 11 96 40 01 	movss  DWORD PTR [r14+0x140],xmm2
   1403198d8:	00 00 
   1403198da:	f3 0f 59 d9          	mulss  xmm3,xmm1
   1403198de:	f3 41 0f 59 9e 20 01 	mulss  xmm3,DWORD PTR [r14+0x120]
   1403198e5:	00 00 
   1403198e7:	f3 0f 10 45 cb       	movss  xmm0,DWORD PTR [rbp-0x35]
   1403198ec:	f3 0f 59 c1          	mulss  xmm0,xmm1
   1403198f0:	f3 0f 58 d8          	addss  xmm3,xmm0
   1403198f4:	f3 41 0f 10 8e 44 01 	movss  xmm1,DWORD PTR [r14+0x144]
   1403198fb:	00 00 
   1403198fd:	f3 0f 5c cb          	subss  xmm1,xmm3
   140319901:	f3 0f 10 1d f3 fa 1b 	movss  xmm3,DWORD PTR [rip+0x1bfaf3]        # 0x1404d93fc
   140319908:	00 
   140319909:	f3 0f 59 d3          	mulss  xmm2,xmm3
   14031990d:	f3 0f 2c c2          	cvttss2si eax,xmm2
   140319911:	25 ff 3f 00 00       	and    eax,0x3fff
   140319916:	66 0f 6e c0          	movd   xmm0,eax
   14031991a:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   14031991d:	f3 0f 10 15 7f e3 1e 	movss  xmm2,DWORD PTR [rip+0x1ee37f]        # 0x140507ca4
   140319924:	00 
   140319925:	f3 0f 59 c2          	mulss  xmm0,xmm2
   140319929:	f3 41 0f 11 86 40 01 	movss  DWORD PTR [r14+0x140],xmm0
   140319930:	00 00 
   140319932:	f3 0f 59 cb          	mulss  xmm1,xmm3
   140319936:	f3 0f 2c c1          	cvttss2si eax,xmm1
   14031993a:	25 ff 3f 00 00       	and    eax,0x3fff
   14031993f:	66 0f 6e c0          	movd   xmm0,eax
   140319943:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319946:	f3 0f 59 c2          	mulss  xmm0,xmm2
   14031994a:	f3 41 0f 11 86 44 01 	movss  DWORD PTR [r14+0x144],xmm0
   140319951:	00 00 
   140319953:	f3 41 0f 10 8e 48 01 	movss  xmm1,DWORD PTR [r14+0x148]
   14031995a:	00 00 
   14031995c:	f3 0f 5c cc          	subss  xmm1,xmm4
   140319960:	f3 0f 59 cb          	mulss  xmm1,xmm3
   140319964:	f3 0f 2c c1          	cvttss2si eax,xmm1
   140319968:	25 ff 3f 00 00       	and    eax,0x3fff
   14031996d:	66 0f 6e c0          	movd   xmm0,eax
   140319971:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   140319974:	f3 0f 59 c2          	mulss  xmm0,xmm2
   140319978:	f3 41 0f 11 86 48 01 	movss  DWORD PTR [r14+0x148],xmm0
   14031997f:	00 00 
   140319981:	f3 41 0f 10 8e 4c 01 	movss  xmm1,DWORD PTR [r14+0x14c]
   140319988:	00 00 
   14031998a:	f3 0f 5c cc          	subss  xmm1,xmm4
   14031998e:	f3 0f 59 cb          	mulss  xmm1,xmm3
   140319992:	f3 0f 2c c1          	cvttss2si eax,xmm1
   140319996:	25 ff 3f 00 00       	and    eax,0x3fff
   14031999b:	66 0f 6e c0          	movd   xmm0,eax
   14031999f:	0f 5b c0             	cvtdq2ps xmm0,xmm0
   1403199a2:	f3 0f 59 c2          	mulss  xmm0,xmm2
   1403199a6:	f3 41 0f 11 86 4c 01 	movss  DWORD PTR [r14+0x14c],xmm0
   1403199ad:	00 00 
   1403199af:	41 8b 96 2c 01 00 00 	mov    edx,DWORD PTR [r14+0x12c]
   1403199b6:	48 8d 0d 4b 01 9c 00 	lea    rcx,[rip+0x9c014b]        # 0x140cd9b08
   1403199bd:	e8 0e 86 ff ff       	call   0x140311fd0
   1403199c2:	83 be 94 5c 01 00 00 	cmp    DWORD PTR [rsi+0x15c94],0x0
   1403199c9:	75 13                	jne    0x1403199de
   1403199cb:	83 be 90 5c 01 00 01 	cmp    DWORD PTR [rsi+0x15c90],0x1
   1403199d2:	75 0a                	jne    0x1403199de
   1403199d4:	48 83 be 88 5c 01 00 	cmp    QWORD PTR [rsi+0x15c88],0xe
   1403199db:	0e 
   1403199dc:	74 39                	je     0x140319a17
   1403199de:	33 d2                	xor    edx,edx
   1403199e0:	48 8b ce             	mov    rcx,rsi
   1403199e3:	e8 98 c4 ff ff       	call   0x140315e80
   1403199e8:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   1403199ef:	48 89 86 98 5c 01 00 	mov    QWORD PTR [rsi+0x15c98],rax
   1403199f6:	48 83 c0 10          	add    rax,0x10
   1403199fa:	48 89 86 b0 5c 01 00 	mov    QWORD PTR [rsi+0x15cb0],rax
   140319a01:	48 c7 86 90 5c 01 00 	mov    QWORD PTR [rsi+0x15c90],0x1
   140319a08:	01 00 00 00 
   140319a0c:	48 c7 86 88 5c 01 00 	mov    QWORD PTR [rsi+0x15c88],0xe
   140319a13:	0e 00 00 00 
   140319a17:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319a1e:	48 c7 00 0d 00 00 00 	mov    QWORD PTR [rax],0xd
   140319a25:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319a2c:	48 c7 40 08 63 00 00 	mov    QWORD PTR [rax+0x8],0x63
   140319a33:	00 
   140319a34:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319a3b:	10 
   140319a3c:	33 c0                	xor    eax,eax
   140319a3e:	48 89 45 9f          	mov    QWORD PTR [rbp-0x61],rax
   140319a42:	48 89 45 a7          	mov    QWORD PTR [rbp-0x59],rax
   140319a46:	45 33 c0             	xor    r8d,r8d
   140319a49:	48 8b 15 98 80 9b 00 	mov    rdx,QWORD PTR [rip+0x9b8098]        # 0x140cd1ae8
   140319a50:	48 8d 4d 9f          	lea    rcx,[rbp-0x61]
   140319a54:	e8 07 cc d2 ff       	call   0x140046660
   140319a59:	90                   	nop
   140319a5a:	48 8b 5d 9f          	mov    rbx,QWORD PTR [rbp-0x61]
   140319a5e:	bf 00 02 00 00       	mov    edi,0x200
   140319a63:	48 8d 0d 9e 00 9c 00 	lea    rcx,[rip+0x9c009e]        # 0x140cd9b08
   140319a6a:	e8 21 f9 d3 ff       	call   0x140059390
   140319a6f:	0f b6 c8             	movzx  ecx,al
   140319a72:	8b c1                	mov    eax,ecx
   140319a74:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319a79:	c1 e0 08             	shl    eax,0x8
   140319a7c:	0b c1                	or     eax,ecx
   140319a7e:	c1 e0 08             	shl    eax,0x8
   140319a81:	0b c1                	or     eax,ecx
   140319a83:	89 03                	mov    DWORD PTR [rbx],eax
   140319a85:	48 8d 0d 7c 00 9c 00 	lea    rcx,[rip+0x9c007c]        # 0x140cd9b08
   140319a8c:	e8 ff f8 d3 ff       	call   0x140059390
   140319a91:	0f b6 c8             	movzx  ecx,al
   140319a94:	8b c1                	mov    eax,ecx
   140319a96:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319a9b:	c1 e0 08             	shl    eax,0x8
   140319a9e:	0b c1                	or     eax,ecx
   140319aa0:	c1 e0 08             	shl    eax,0x8
   140319aa3:	0b c1                	or     eax,ecx
   140319aa5:	89 43 04             	mov    DWORD PTR [rbx+0x4],eax
   140319aa8:	48 8d 0d 59 00 9c 00 	lea    rcx,[rip+0x9c0059]        # 0x140cd9b08
   140319aaf:	e8 dc f8 d3 ff       	call   0x140059390
   140319ab4:	0f b6 c8             	movzx  ecx,al
   140319ab7:	8b c1                	mov    eax,ecx
   140319ab9:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319abe:	c1 e0 08             	shl    eax,0x8
   140319ac1:	0b c1                	or     eax,ecx
   140319ac3:	c1 e0 08             	shl    eax,0x8
   140319ac6:	0b c1                	or     eax,ecx
   140319ac8:	89 43 08             	mov    DWORD PTR [rbx+0x8],eax
   140319acb:	48 8d 0d 36 00 9c 00 	lea    rcx,[rip+0x9c0036]        # 0x140cd9b08
   140319ad2:	e8 b9 f8 d3 ff       	call   0x140059390
   140319ad7:	0f b6 c8             	movzx  ecx,al
   140319ada:	8b c1                	mov    eax,ecx
   140319adc:	0d 00 80 ff ff       	or     eax,0xffff8000
   140319ae1:	c1 e0 08             	shl    eax,0x8
   140319ae4:	0b c1                	or     eax,ecx
   140319ae6:	c1 e0 08             	shl    eax,0x8
   140319ae9:	0b c1                	or     eax,ecx
   140319aeb:	89 43 0c             	mov    DWORD PTR [rbx+0xc],eax
   140319aee:	48 8d 5b 10          	lea    rbx,[rbx+0x10]
   140319af2:	48 83 ef 01          	sub    rdi,0x1
   140319af6:	0f 85 67 ff ff ff    	jne    0x140319a63
   140319afc:	41 8b 9e 14 01 00 00 	mov    ebx,DWORD PTR [r14+0x114]
   140319b03:	48 8b 0d de 7f 9b 00 	mov    rcx,QWORD PTR [rip+0x9b7fde]        # 0x140cd1ae8
   140319b0a:	e8 a1 cf d2 ff       	call   0x140046ab0
   140319b0f:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   140319b16:	89 01                	mov    DWORD PTR [rcx],eax
   140319b18:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319b1f:	89 58 04             	mov    DWORD PTR [rax+0x4],ebx
   140319b22:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319b29:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319b30:	00 
   140319b31:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319b38:	10 
   140319b39:	f3 41 0f 10 8e 44 01 	movss  xmm1,DWORD PTR [r14+0x144]
   140319b40:	00 00 
   140319b42:	f3 41 0f 10 86 40 01 	movss  xmm0,DWORD PTR [r14+0x140]
   140319b49:	00 00 
   140319b4b:	48 8b 96 b8 5c 01 00 	mov    rdx,QWORD PTR [rsi+0x15cb8]
   140319b52:	8b 8e b0 5c 01 00    	mov    ecx,DWORD PTR [rsi+0x15cb0]
   140319b58:	2b 4a 50             	sub    ecx,DWORD PTR [rdx+0x50]
   140319b5b:	48 c1 f9 04          	sar    rcx,0x4
   140319b5f:	0f b7 42 5a          	movzx  eax,WORD PTR [rdx+0x5a]
   140319b63:	66 89 4c 42 5c       	mov    WORD PTR [rdx+rax*2+0x5c],cx
   140319b68:	48 8b 86 b8 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb8]
   140319b6f:	66 ff 40 5a          	inc    WORD PTR [rax+0x5a]
   140319b73:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319b7a:	f3 0f 11 00          	movss  DWORD PTR [rax],xmm0
   140319b7e:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319b85:	f3 0f 11 48 04       	movss  DWORD PTR [rax+0x4],xmm1
   140319b8a:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319b91:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319b98:	00 
   140319b99:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319ba0:	10 
   140319ba1:	f3 41 0f 10 8e 4c 01 	movss  xmm1,DWORD PTR [r14+0x14c]
   140319ba8:	00 00 
   140319baa:	f3 41 0f 10 86 48 01 	movss  xmm0,DWORD PTR [r14+0x148]
   140319bb1:	00 00 
   140319bb3:	48 8b 96 b8 5c 01 00 	mov    rdx,QWORD PTR [rsi+0x15cb8]
   140319bba:	8b 8e b0 5c 01 00    	mov    ecx,DWORD PTR [rsi+0x15cb0]
   140319bc0:	2b 4a 50             	sub    ecx,DWORD PTR [rdx+0x50]
   140319bc3:	48 c1 f9 04          	sar    rcx,0x4
   140319bc7:	0f b7 42 5a          	movzx  eax,WORD PTR [rdx+0x5a]
   140319bcb:	66 89 4c 42 5c       	mov    WORD PTR [rdx+rax*2+0x5c],cx
   140319bd0:	48 8b 86 b8 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb8]
   140319bd7:	66 ff 40 5a          	inc    WORD PTR [rax+0x5a]
   140319bdb:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319be2:	f3 0f 11 00          	movss  DWORD PTR [rax],xmm0
   140319be6:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319bed:	f3 0f 11 48 04       	movss  DWORD PTR [rax+0x4],xmm1
   140319bf2:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319bf9:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319c00:	00 
   140319c01:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319c08:	10 
   140319c09:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   140319c10:	f3 41 0f 10 86 1c 01 	movss  xmm0,DWORD PTR [r14+0x11c]
   140319c17:	00 00 
   140319c19:	41 8b 86 18 01 00 00 	mov    eax,DWORD PTR [r14+0x118]
   140319c20:	89 01                	mov    DWORD PTR [rcx],eax
   140319c22:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319c29:	f3 0f 11 40 04       	movss  DWORD PTR [rax+0x4],xmm0
   140319c2e:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319c35:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319c3c:	00 
   140319c3d:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319c44:	10 
   140319c45:	45 33 c0             	xor    r8d,r8d
   140319c48:	48 8b 15 a1 fe 9b 00 	mov    rdx,QWORD PTR [rip+0x9bfea1]        # 0x140cd9af0
   140319c4f:	48 8d 4d 87          	lea    rcx,[rbp-0x79]
   140319c53:	e8 08 ca d2 ff       	call   0x140046660
   140319c58:	49 8d 8e 80 00 00 00 	lea    rcx,[r14+0x80]
   140319c5f:	49 8d 56 70          	lea    rdx,[r14+0x70]
   140319c63:	4d 8d 4e 5c          	lea    r9,[r14+0x5c]
   140319c67:	4d 8d 46 1c          	lea    r8,[r14+0x1c]
   140319c6b:	41 8b 86 90 00 00 00 	mov    eax,DWORD PTR [r14+0x90]
   140319c72:	89 44 24 38          	mov    DWORD PTR [rsp+0x38],eax
   140319c76:	48 89 4c 24 30       	mov    QWORD PTR [rsp+0x30],rcx
   140319c7b:	48 89 54 24 28       	mov    QWORD PTR [rsp+0x28],rdx
   140319c80:	41 8b 46 6c          	mov    eax,DWORD PTR [r14+0x6c]
   140319c84:	89 44 24 20          	mov    DWORD PTR [rsp+0x20],eax
   140319c88:	48 8b 55 87          	mov    rdx,QWORD PTR [rbp-0x79]
   140319c8c:	48 8b ce             	mov    rcx,rsi
   140319c8f:	e8 bc c9 ff ff       	call   0x140316650
   140319c94:	48 8d 4d 87          	lea    rcx,[rbp-0x79]
   140319c98:	e8 93 4d f3 ff       	call   0x14024ea30
   140319c9d:	41 8b 5e 18          	mov    ebx,DWORD PTR [r14+0x18]
   140319ca1:	48 8b 0d 48 fe 9b 00 	mov    rcx,QWORD PTR [rip+0x9bfe48]        # 0x140cd9af0
   140319ca8:	e8 23 9c d1 ff       	call   0x1400338d0
   140319cad:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   140319cb4:	89 01                	mov    DWORD PTR [rcx],eax
   140319cb6:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319cbd:	89 58 04             	mov    DWORD PTR [rax+0x4],ebx
   140319cc0:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319cc7:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319cce:	00 
   140319ccf:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319cd6:	10 
   140319cd7:	48 8b 8e b0 5c 01 00 	mov    rcx,QWORD PTR [rsi+0x15cb0]
   140319cde:	41 8b 86 28 01 00 00 	mov    eax,DWORD PTR [r14+0x128]
   140319ce5:	89 01                	mov    DWORD PTR [rcx],eax
   140319ce7:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319cee:	44 89 78 04          	mov    DWORD PTR [rax+0x4],r15d
   140319cf2:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319cf9:	48 c7 40 08 64 00 00 	mov    QWORD PTR [rax+0x8],0x64
   140319d00:	00 
   140319d01:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319d08:	10 
   140319d09:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319d10:	48 c7 00 0d 00 00 00 	mov    QWORD PTR [rax],0xd
   140319d17:	48 8b 86 b0 5c 01 00 	mov    rax,QWORD PTR [rsi+0x15cb0]
   140319d1e:	48 c7 40 08 65 00 00 	mov    QWORD PTR [rax+0x8],0x65
   140319d25:	00 
   140319d26:	48 83 86 b0 5c 01 00 	add    QWORD PTR [rsi+0x15cb0],0x10
   140319d2d:	10 
   140319d2e:	48 8d 4d 9f          	lea    rcx,[rbp-0x61]
   140319d32:	e8 f9 4c f3 ff       	call   0x14024ea30
   140319d37:	48 8b 4d 27          	mov    rcx,QWORD PTR [rbp+0x27]
   140319d3b:	48 33 cc             	xor    rcx,rsp
   140319d3e:	e8 ad b8 02 00       	call   0x1403455f0
   140319d43:	48 8b 9c 24 30 01 00 	mov    rbx,QWORD PTR [rsp+0x130]
   140319d4a:	00 
   140319d4b:	48 81 c4 f0 00 00 00 	add    rsp,0xf0
   140319d52:	41 5f                	pop    r15
   140319d54:	41 5e                	pop    r14
   140319d56:	5f                   	pop    rdi
   140319d57:	5e                   	pop    rsi
   140319d58:	5d                   	pop    rbp
   140319d59:	c3                   	ret
