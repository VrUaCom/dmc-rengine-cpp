
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

00000001403153c0 <.text+0x3143c0>:
   1403153c0:	48 89 5c 24 10       	mov    QWORD PTR [rsp+0x10],rbx
   1403153c5:	48 89 6c 24 18       	mov    QWORD PTR [rsp+0x18],rbp
   1403153ca:	56                   	push   rsi
   1403153cb:	57                   	push   rdi
   1403153cc:	41 56                	push   r14
   1403153ce:	48 83 ec 20          	sub    rsp,0x20
   1403153d2:	48 8b 05 27 7f a5 00 	mov    rax,QWORD PTR [rip+0xa57f27]        # 0x140d6d300
   1403153d9:	48 8b da             	mov    rbx,rdx
   1403153dc:	48 8b f9             	mov    rdi,rcx
   1403153df:	44 0f be 70 0e       	movsx  r14d,BYTE PTR [rax+0xe]
   1403153e4:	0f b7 42 04          	movzx  eax,WORD PTR [rdx+0x4]
   1403153e8:	a8 04                	test   al,0x4
   1403153ea:	74 7c                	je     0x140315468
   1403153ec:	0f bf 42 0c          	movsx  eax,WORD PTR [rdx+0xc]
   1403153f0:	33 f6                	xor    esi,esi
   1403153f2:	44 0f a3 f0          	bt     eax,r14d
   1403153f6:	73 33                	jae    0x14031542b
   1403153f8:	48 8b 42 10          	mov    rax,QWORD PTR [rdx+0x10]
   1403153fc:	48 85 c0             	test   rax,rax
   1403153ff:	74 1d                	je     0x14031541e
   140315401:	4a 8d 3c f0          	lea    rdi,[rax+r14*8]
   140315405:	41 8b ce             	mov    ecx,r14d
   140315408:	48 39 77 40          	cmp    QWORD PTR [rdi+0x40],rsi
   14031540c:	74 10                	je     0x14031541e
   14031540e:	48 c1 e1 05          	shl    rcx,0x5
   140315412:	48 03 c8             	add    rcx,rax
   140315415:	e8 46 0e fb ff       	call   0x1402c6260
   14031541a:	48 89 77 40          	mov    QWORD PTR [rdi+0x40],rsi
   14031541e:	bd 03 00 00 00       	mov    ebp,0x3
   140315423:	44 0f bb f5          	btc    ebp,r14d
   140315427:	66 21 6b 0c          	and    WORD PTR [rbx+0xc],bp
   14031542b:	66 39 73 0c          	cmp    WORD PTR [rbx+0xc],si
   14031542f:	0f 85 7b 02 00 00    	jne    0x1403156b0
   140315435:	80 3b 05             	cmp    BYTE PTR [rbx],0x5
   140315438:	75 18                	jne    0x140315452
   14031543a:	48 8b 8b 18 01 00 00 	mov    rcx,QWORD PTR [rbx+0x118]
   140315441:	e8 5a e4 d1 ff       	call   0x1400338a0
   140315446:	48 8b 8b 20 01 00 00 	mov    rcx,QWORD PTR [rbx+0x120]
   14031544d:	e8 4e e4 d1 ff       	call   0x1400338a0
   140315452:	66 89 33             	mov    WORD PTR [rbx],si
   140315455:	89 73 02             	mov    DWORD PTR [rbx+0x2],esi
   140315458:	89 73 08             	mov    DWORD PTR [rbx+0x8],esi
   14031545b:	48 89 73 10          	mov    QWORD PTR [rbx+0x10],rsi
   14031545f:	66 89 73 0c          	mov    WORD PTR [rbx+0xc],si
   140315463:	e9 48 02 00 00       	jmp    0x1403156b0
   140315468:	a8 01                	test   al,0x1
   14031546a:	0f 85 40 02 00 00    	jne    0x1403156b0
   140315470:	4c 89 7c 24 40       	mov    QWORD PTR [rsp+0x40],r15
   140315475:	a8 02                	test   al,0x2
   140315477:	74 25                	je     0x14031549e
   140315479:	0f b6 42 01          	movzx  eax,BYTE PTR [rdx+0x1]
   14031547d:	83 bc 81 c0 5c 01 00 	cmp    DWORD PTR [rcx+rax*4+0x15cc0],0x0
   140315484:	00 
   140315485:	75 17                	jne    0x14031549e
   140315487:	c7 84 81 c0 5c 01 00 	mov    DWORD PTR [rcx+rax*4+0x15cc0],0x1
   14031548e:	01 00 00 00 
   140315492:	45 33 c0             	xor    r8d,r8d
   140315495:	0f b6 52 01          	movzx  edx,BYTE PTR [rdx+0x1]
   140315499:	e8 f2 04 00 00       	call   0x140315990
   14031549e:	48 8b d3             	mov    rdx,rbx
   1403154a1:	48 8b cf             	mov    rcx,rdi
   1403154a4:	e8 37 0e 00 00       	call   0x1403162e0
   1403154a9:	48 8b 53 10          	mov    rdx,QWORD PTR [rbx+0x10]
   1403154ad:	44 8b f8             	mov    r15d,eax
   1403154b0:	48 85 d2             	test   rdx,rdx
   1403154b3:	74 2b                	je     0x1403154e0
   1403154b5:	bd 03 00 00 00       	mov    ebp,0x3
   1403154ba:	39 43 08             	cmp    DWORD PTR [rbx+0x8],eax
   1403154bd:	74 04                	je     0x1403154c3
   1403154bf:	66 89 6b 0c          	mov    WORD PTR [rbx+0xc],bp
   1403154c3:	0f bf 43 0c          	movsx  eax,WORD PTR [rbx+0xc]
   1403154c7:	44 0f a3 f0          	bt     eax,r14d
   1403154cb:	73 13                	jae    0x1403154e0
   1403154cd:	45 8b c6             	mov    r8d,r14d
   1403154d0:	48 8b cf             	mov    rcx,rdi
   1403154d3:	e8 b8 15 00 00       	call   0x140316a90
   1403154d8:	44 0f bb f5          	btc    ebp,r14d
   1403154dc:	66 21 6b 0c          	and    WORD PTR [rbx+0xc],bp
   1403154e0:	48 8b 53 10          	mov    rdx,QWORD PTR [rbx+0x10]
   1403154e4:	48 8d 2d 15 ab ce ff 	lea    rbp,[rip+0xffffffffffceab15]        # 0x140000000
   1403154eb:	48 85 d2             	test   rdx,rdx
   1403154ee:	74 0c                	je     0x1403154fc
   1403154f0:	4a 83 7c f2 40 00    	cmp    QWORD PTR [rdx+r14*8+0x40],0x0
   1403154f6:	0f 85 09 01 00 00    	jne    0x140315605
   1403154fc:	48 8b cf             	mov    rcx,rdi
   1403154ff:	e8 ac 03 00 00       	call   0x1403158b0
   140315504:	0f b6 03             	movzx  eax,BYTE PTR [rbx]
   140315507:	ff c8                	dec    eax
   140315509:	83 f8 0f             	cmp    eax,0xf
   14031550c:	0f 87 de 00 00 00    	ja     0x1403155f0
   140315512:	48 98                	cdqe
   140315514:	8b 8c 85 c8 56 31 00 	mov    ecx,DWORD PTR [rbp+rax*4+0x3156c8]
   14031551b:	48 03 cd             	add    rcx,rbp
   14031551e:	ff e1                	jmp    rcx
   140315520:	48 8b d3             	mov    rdx,rbx
   140315523:	48 8b cf             	mov    rcx,rdi
   140315526:	e8 45 1f 00 00       	call   0x140317470
   14031552b:	e9 c0 00 00 00       	jmp    0x1403155f0
   140315530:	48 8b d3             	mov    rdx,rbx
   140315533:	48 8b cf             	mov    rcx,rdi
   140315536:	e8 e5 27 00 00       	call   0x140317d20
   14031553b:	e9 b0 00 00 00       	jmp    0x1403155f0
   140315540:	48 8b d3             	mov    rdx,rbx
   140315543:	48 8b cf             	mov    rcx,rdi
   140315546:	e8 e5 32 00 00       	call   0x140318830
   14031554b:	e9 a0 00 00 00       	jmp    0x1403155f0
   140315550:	48 8b d3             	mov    rdx,rbx
   140315553:	48 8b cf             	mov    rcx,rdi
   140315556:	e8 d5 35 00 00       	call   0x140318b30
   14031555b:	e9 90 00 00 00       	jmp    0x1403155f0
   140315560:	48 8b d3             	mov    rdx,rbx
   140315563:	48 8b cf             	mov    rcx,rdi
   140315566:	e8 65 25 00 00       	call   0x140317ad0
   14031556b:	e9 80 00 00 00       	jmp    0x1403155f0
   140315570:	48 8b d3             	mov    rdx,rbx
   140315573:	48 8b cf             	mov    rcx,rdi
   140315576:	e8 75 40 00 00       	call   0x1403195f0
   14031557b:	eb 73                	jmp    0x1403155f0
   14031557d:	48 8b d3             	mov    rdx,rbx
   140315580:	48 8b cf             	mov    rcx,rdi
   140315583:	e8 c8 4d 00 00       	call   0x14031a350
   140315588:	eb 66                	jmp    0x1403155f0
   14031558a:	48 8b d3             	mov    rdx,rbx
   14031558d:	48 8b cf             	mov    rcx,rdi
   140315590:	e8 bb 20 00 00       	call   0x140317650
   140315595:	eb 59                	jmp    0x1403155f0
   140315597:	48 8b d3             	mov    rdx,rbx
   14031559a:	48 8b cf             	mov    rcx,rdi
   14031559d:	e8 ee 22 00 00       	call   0x140317890
   1403155a2:	eb 4c                	jmp    0x1403155f0
   1403155a4:	48 8b d3             	mov    rdx,rbx
   1403155a7:	48 8b cf             	mov    rcx,rdi
   1403155aa:	e8 11 2a 00 00       	call   0x140317fc0
   1403155af:	eb 3f                	jmp    0x1403155f0
   1403155b1:	48 8b d3             	mov    rdx,rbx
   1403155b4:	48 8b cf             	mov    rcx,rdi
   1403155b7:	e8 04 39 00 00       	call   0x140318ec0
   1403155bc:	eb 32                	jmp    0x1403155f0
   1403155be:	48 8b d3             	mov    rdx,rbx
   1403155c1:	48 8b cf             	mov    rcx,rdi
   1403155c4:	e8 97 47 00 00       	call   0x140319d60
   1403155c9:	eb 25                	jmp    0x1403155f0
   1403155cb:	48 8b d3             	mov    rdx,rbx
   1403155ce:	48 8b cf             	mov    rcx,rdi
   1403155d1:	e8 ca 40 00 00       	call   0x1403196a0
   1403155d6:	eb 18                	jmp    0x1403155f0
   1403155d8:	48 8b d3             	mov    rdx,rbx
   1403155db:	48 8b cf             	mov    rcx,rdi
   1403155de:	e8 2d 30 00 00       	call   0x140318610
   1403155e3:	eb 0b                	jmp    0x1403155f0
   1403155e5:	48 8b d3             	mov    rdx,rbx
   1403155e8:	48 8b cf             	mov    rcx,rdi
   1403155eb:	e8 30 2f 00 00       	call   0x140318520
   1403155f0:	33 d2                	xor    edx,edx
   1403155f2:	48 8b cf             	mov    rcx,rdi
   1403155f5:	e8 36 09 00 00       	call   0x140315f30
   1403155fa:	48 83 7b 10 00       	cmp    QWORD PTR [rbx+0x10],0x0
   1403155ff:	75 04                	jne    0x140315605
   140315601:	48 89 43 10          	mov    QWORD PTR [rbx+0x10],rax
   140315605:	48 8b 53 10          	mov    rdx,QWORD PTR [rbx+0x10]
   140315609:	48 85 d2             	test   rdx,rdx
   14031560c:	0f 84 83 00 00 00    	je     0x140315695
   140315612:	4a 83 7c f2 40 00    	cmp    QWORD PTR [rdx+r14*8+0x40],0x0
   140315618:	74 7b                	je     0x140315695
   14031561a:	48 89 97 b8 5c 01 00 	mov    QWORD PTR [rdi+0x15cb8],rdx
   140315621:	33 f6                	xor    esi,esi
   140315623:	48 8b 05 d6 7c a5 00 	mov    rax,QWORD PTR [rip+0xa57cd6]        # 0x140d6d300
   14031562a:	48 0f be 48 0e       	movsx  rcx,BYTE PTR [rax+0xe]
   14031562f:	48 8b 44 ca 40       	mov    rax,QWORD PTR [rdx+rcx*8+0x40]
   140315634:	48 89 42 50          	mov    QWORD PTR [rdx+0x50],rax
   140315638:	48 8b 87 b8 5c 01 00 	mov    rax,QWORD PTR [rdi+0x15cb8]
   14031563f:	66 89 70 5a          	mov    WORD PTR [rax+0x5a],si
   140315643:	0f b6 03             	movzx  eax,BYTE PTR [rbx]
   140315646:	83 c0 fd             	add    eax,0xfffffffd
   140315649:	83 f8 0d             	cmp    eax,0xd
   14031564c:	77 40                	ja     0x14031568e
   14031564e:	48 98                	cdqe
   140315650:	8b 8c 85 08 57 31 00 	mov    ecx,DWORD PTR [rbp+rax*4+0x315708]
   140315657:	48 03 cd             	add    rcx,rbp
   14031565a:	ff e1                	jmp    rcx
   14031565c:	48 8b d3             	mov    rdx,rbx
   14031565f:	48 8b cf             	mov    rcx,rdi
   140315662:	e8 59 5a 00 00       	call   0x14031b0c0
   140315667:	eb 25                	jmp    0x14031568e
   140315669:	48 8b d3             	mov    rdx,rbx
   14031566c:	48 8b cf             	mov    rcx,rdi
   14031566f:	e8 7c 4f 00 00       	call   0x14031a5f0
   140315674:	eb 18                	jmp    0x14031568e
   140315676:	48 8b d3             	mov    rdx,rbx
   140315679:	48 8b cf             	mov    rcx,rdi
   14031567c:	e8 af 51 00 00       	call   0x14031a830
   140315681:	eb 0b                	jmp    0x14031568e
   140315683:	48 8b d3             	mov    rdx,rbx
   140315686:	48 8b cf             	mov    rcx,rdi
   140315689:	e8 82 57 00 00       	call   0x14031ae10
   14031568e:	48 89 b7 b8 5c 01 00 	mov    QWORD PTR [rdi+0x15cb8],rsi
   140315695:	f6 43 04 08          	test   BYTE PTR [rbx+0x4],0x8
   140315699:	75 0c                	jne    0x1403156a7
   14031569b:	48 8b 53 10          	mov    rdx,QWORD PTR [rbx+0x10]
   14031569f:	48 8b cf             	mov    rcx,rdi
   1403156a2:	e8 59 04 00 00       	call   0x140315b00
   1403156a7:	44 89 7b 08          	mov    DWORD PTR [rbx+0x8],r15d
   1403156ab:	4c 8b 7c 24 40       	mov    r15,QWORD PTR [rsp+0x40]
   1403156b0:	48 8b 5c 24 48       	mov    rbx,QWORD PTR [rsp+0x48]
   1403156b5:	33 c0                	xor    eax,eax
   1403156b7:	48 8b 6c 24 50       	mov    rbp,QWORD PTR [rsp+0x50]
   1403156bc:	48 83 c4 20          	add    rsp,0x20
   1403156c0:	41 5e                	pop    r14
   1403156c2:	5f                   	pop    rdi
   1403156c3:	5e                   	pop    rsi
   1403156c4:	c3                   	ret
   1403156c5:	0f 1f 00             	nop    DWORD PTR [rax]
