
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

00000001403204f0 <.text+0x31f4f0>:
   1403204f0:	40 53                	rex push rbx
   1403204f2:	55                   	push   rbp
   1403204f3:	56                   	push   rsi
   1403204f4:	57                   	push   rdi
   1403204f5:	41 54                	push   r12
   1403204f7:	41 56                	push   r14
   1403204f9:	41 57                	push   r15
   1403204fb:	48 81 ec 80 00 00 00 	sub    rsp,0x80
   140320502:	48 8b 05 a7 4b 2b 00 	mov    rax,QWORD PTR [rip+0x2b4ba7]        # 0x1405d50b0
   140320509:	48 33 c4             	xor    rax,rsp
   14032050c:	48 89 44 24 50       	mov    QWORD PTR [rsp+0x50],rax
   140320511:	48 8b 05 e8 cd a4 00 	mov    rax,QWORD PTR [rip+0xa4cde8]        # 0x140d6d300
   140320518:	4c 8b f1             	mov    r14,rcx
   14032051b:	4c 89 6c 24 78       	mov    QWORD PTR [rsp+0x78],r13
   140320520:	b9 10 00 00 00       	mov    ecx,0x10
   140320525:	4d 8b e9             	mov    r13,r9
   140320528:	8b da                	mov    ebx,edx
   14032052a:	4d 8b e0             	mov    r12,r8
   14032052d:	0f be 40 0e          	movsx  eax,BYTE PTR [rax+0xe]
   140320531:	89 44 24 30          	mov    DWORD PTR [rsp+0x30],eax
   140320535:	e8 26 03 01 00       	call   0x140330860
   14032053a:	48 8d 1c 9b          	lea    rbx,[rbx+rbx*4]
   14032053e:	33 f6                	xor    esi,esi
   140320540:	48 c1 e3 05          	shl    rbx,0x5
   140320544:	4c 8b f8             	mov    r15,rax
   140320547:	49 03 5e 20          	add    rbx,QWORD PTR [r14+0x20]
   14032054b:	39 73 08             	cmp    DWORD PTR [rbx+0x8],esi
   14032054e:	0f 8e 98 00 00 00    	jle    0x1403205ec
   140320554:	0f 29 74 24 60       	movaps XMMWORD PTR [rsp+0x60],xmm6
   140320559:	48 8b f8             	mov    rdi,rax
   14032055c:	0f 57 f6             	xorps  xmm6,xmm6
   14032055f:	33 ed                	xor    ebp,ebp
   140320561:	48 8b 43 28          	mov    rax,QWORD PTR [rbx+0x28]
   140320565:	48 8d 54 24 40       	lea    rdx,[rsp+0x40]
   14032056a:	48 63 4c 28 08       	movsxd rcx,DWORD PTR [rax+rbp*1+0x8]
   14032056f:	4c 63 4c 28 04       	movsxd r9,DWORD PTR [rax+rbp*1+0x4]
   140320574:	4c 63 04 28          	movsxd r8,DWORD PTR [rax+rbp*1]
   140320578:	48 c1 e1 04          	shl    rcx,0x4
   14032057c:	49 03 cc             	add    rcx,r12
   14032057f:	49 c1 e1 04          	shl    r9,0x4
   140320583:	49 c1 e0 04          	shl    r8,0x4
   140320587:	4d 03 cc             	add    r9,r12
   14032058a:	48 89 4c 24 20       	mov    QWORD PTR [rsp+0x20],rcx
   14032058f:	4d 03 c4             	add    r8,r12
   140320592:	49 8b ce             	mov    rcx,r14
   140320595:	e8 16 06 00 00       	call   0x140320bb0
   14032059a:	48 8d 54 24 40       	lea    rdx,[rsp+0x40]
   14032059f:	49 8b cd             	mov    rcx,r13
   1403205a2:	e8 89 07 d1 ff       	call   0x140030d30
   1403205a7:	0f 2f f0             	comiss xmm6,xmm0
   1403205aa:	76 26                	jbe    0x1403205d2
   1403205ac:	49 8b ce             	mov    rcx,r14
   1403205af:	c6 07 01             	mov    BYTE PTR [rdi],0x1
   1403205b2:	e8 99 03 00 00       	call   0x140320950
   1403205b7:	a9 ff ff 03 00       	test   eax,0x3ffff
   1403205bc:	75 14                	jne    0x1403205d2
   1403205be:	49 8b ce             	mov    rcx,r14
   1403205c1:	e8 2a 01 00 00       	call   0x1403206f0
   1403205c6:	a9 ff ff 03 00       	test   eax,0x3ffff
   1403205cb:	75 05                	jne    0x1403205d2
   1403205cd:	ff 43 0c             	inc    DWORD PTR [rbx+0xc]
   1403205d0:	eb 03                	jmp    0x1403205d5
   1403205d2:	c6 07 00             	mov    BYTE PTR [rdi],0x0
   1403205d5:	ff c6                	inc    esi
   1403205d7:	48 83 c5 10          	add    rbp,0x10
   1403205db:	48 ff c7             	inc    rdi
   1403205de:	3b 73 08             	cmp    esi,DWORD PTR [rbx+0x8]
   1403205e1:	0f 8c 7a ff ff ff    	jl     0x140320561
   1403205e7:	0f 28 74 24 60       	movaps xmm6,XMMWORD PTR [rsp+0x60]
   1403205ec:	8b 74 24 30          	mov    esi,DWORD PTR [rsp+0x30]
   1403205f0:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   1403205f3:	4c 8b 6c 24 78       	mov    r13,QWORD PTR [rsp+0x78]
   1403205f8:	48 8b 8c f3 88 00 00 	mov    rcx,QWORD PTR [rbx+rsi*8+0x88]
   1403205ff:	00 
   140320600:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   140320603:	83 7b 0c 00          	cmp    DWORD PTR [rbx+0xc],0x0
   140320607:	0f 8e af 00 00 00    	jle    0x1403206bc
   14032060d:	4c 63 43 04          	movsxd r8,DWORD PTR [rbx+0x4]
   140320611:	49 8b d4             	mov    rdx,r12
   140320614:	48 8b 4c f3 68       	mov    rcx,QWORD PTR [rbx+rsi*8+0x68]
   140320619:	49 c1 e0 04          	shl    r8,0x4
   14032061d:	e8 c2 65 02 00       	call   0x140346be4
   140320622:	48 8b 6c f3 48       	mov    rbp,QWORD PTR [rbx+rsi*8+0x48]
   140320627:	45 33 f6             	xor    r14d,r14d
   14032062a:	41 8b fe             	mov    edi,r14d
   14032062d:	44 39 73 08          	cmp    DWORD PTR [rbx+0x8],r14d
   140320631:	7e 6f                	jle    0x1403206a2
   140320633:	45 8b ce             	mov    r9d,r14d
   140320636:	45 8b d6             	mov    r10d,r14d
   140320639:	4d 8b df             	mov    r11,r15
   14032063c:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   140320640:	45 38 33             	cmp    BYTE PTR [r11],r14b
   140320643:	75 0f                	jne    0x140320654
   140320645:	48 8b 44 f3 78       	mov    rax,QWORD PTR [rbx+rsi*8+0x78]
   14032064a:	41 c7 04 01 ff ff ff 	mov    DWORD PTR [r9+rax*1],0xffffffff
   140320651:	ff 
   140320652:	eb 3c                	jmp    0x140320690
   140320654:	48 8b 53 30          	mov    rdx,QWORD PTR [rbx+0x30]
   140320658:	4c 8b 44 f3 78       	mov    r8,QWORD PTR [rbx+rsi*8+0x78]
   14032065d:	41 0f b7 04 12       	movzx  eax,WORD PTR [r10+rdx*1]
   140320662:	42 0f b6 0c 38       	movzx  ecx,BYTE PTR [rax+r15*1]
   140320667:	43 89 0c 01          	mov    DWORD PTR [r9+r8*1],ecx
   14032066b:	41 0f b7 44 12 02    	movzx  eax,WORD PTR [r10+rdx*1+0x2]
   140320671:	42 0f b6 0c 38       	movzx  ecx,BYTE PTR [rax+r15*1]
   140320676:	43 89 4c 01 04       	mov    DWORD PTR [r9+r8*1+0x4],ecx
   14032067b:	41 0f b7 44 12 04    	movzx  eax,WORD PTR [r10+rdx*1+0x4]
   140320681:	42 0f b6 0c 38       	movzx  ecx,BYTE PTR [rax+r15*1]
   140320686:	43 89 4c 01 08       	mov    DWORD PTR [r9+r8*1+0x8],ecx
   14032068b:	47 89 74 01 0c       	mov    DWORD PTR [r9+r8*1+0xc],r14d
   140320690:	ff c7                	inc    edi
   140320692:	49 ff c3             	inc    r11
   140320695:	49 83 c2 08          	add    r10,0x8
   140320699:	49 83 c1 10          	add    r9,0x10
   14032069d:	3b 7b 08             	cmp    edi,DWORD PTR [rbx+0x8]
   1403206a0:	7c 9e                	jl     0x140320640
   1403206a2:	48 8b 44 f3 58       	mov    rax,QWORD PTR [rbx+rsi*8+0x58]
   1403206a7:	48 85 c0             	test   rax,rax
   1403206aa:	75 07                	jne    0x1403206b3
   1403206ac:	b8 01 00 00 00       	mov    eax,0x1
   1403206b1:	eb 06                	jmp    0x1403206b9
   1403206b3:	2b 05 ef 97 2b 00    	sub    eax,DWORD PTR [rip+0x2b97ef]        # 0x1405d9ea8
   1403206b9:	89 45 24             	mov    DWORD PTR [rbp+0x24],eax
   1403206bc:	b9 10 00 00 00       	mov    ecx,0x10
   1403206c1:	e8 fa 01 01 00       	call   0x1403308c0
   1403206c6:	48 8b 4c 24 50       	mov    rcx,QWORD PTR [rsp+0x50]
   1403206cb:	48 33 cc             	xor    rcx,rsp
   1403206ce:	e8 1d 4f 02 00       	call   0x1403455f0
   1403206d3:	48 81 c4 80 00 00 00 	add    rsp,0x80
   1403206da:	41 5f                	pop    r15
   1403206dc:	41 5e                	pop    r14
   1403206de:	41 5c                	pop    r12
   1403206e0:	5f                   	pop    rdi
   1403206e1:	5e                   	pop    rsi
   1403206e2:	5d                   	pop    rbp
   1403206e3:	5b                   	pop    rbx
   1403206e4:	c3                   	ret
