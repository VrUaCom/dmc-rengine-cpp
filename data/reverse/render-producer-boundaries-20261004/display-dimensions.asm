
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140316360 <.text+0x315360>:
   140316360:	40 53                	rex push rbx
   140316362:	48 83 ec 20          	sub    rsp,0x20
   140316366:	33 d2                	xor    edx,edx
   140316368:	41 b8 40 5c 00 00    	mov    r8d,0x5c40
   14031636e:	48 8b d9             	mov    rbx,rcx
   140316371:	e8 74 08 03 00       	call   0x140346bea
   140316376:	33 d2                	xor    edx,edx
   140316378:	48 8d 8b 88 5c 01 00 	lea    rcx,[rbx+0x15c88]
   14031637f:	44 8d 42 58          	lea    r8d,[rdx+0x58]
   140316383:	e8 62 08 03 00       	call   0x140346bea
   140316388:	c6 83 60 5d 01 00 00 	mov    BYTE PTR [rbx+0x15d60],0x0
   14031638f:	48 8d 83 80 5c 00 00 	lea    rax,[rbx+0x5c80]
   140316396:	48 89 83 80 5c 01 00 	mov    QWORD PTR [rbx+0x15c80],rax
   14031639d:	48 8b 05 5c 6f a5 00 	mov    rax,QWORD PTR [rip+0xa56f5c]        # 0x140d6d300
   1403163a4:	0f bf 50 20          	movsx  edx,WORD PTR [rax+0x20]
   1403163a8:	89 93 58 5d 01 00    	mov    DWORD PTR [rbx+0x15d58],edx
   1403163ae:	33 d2                	xor    edx,edx
   1403163b0:	48 8b 05 49 6f a5 00 	mov    rax,QWORD PTR [rip+0xa56f49]        # 0x140d6d300
   1403163b7:	0f bf 48 22          	movsx  ecx,WORD PTR [rax+0x22]
   1403163bb:	89 8b 5c 5d 01 00    	mov    DWORD PTR [rbx+0x15d5c],ecx
   1403163c1:	89 93 4c 5d 01 00    	mov    DWORD PTR [rbx+0x15d4c],edx
   1403163c7:	89 93 44 5d 01 00    	mov    DWORD PTR [rbx+0x15d44],edx
   1403163cd:	48 8b 05 2c 6f a5 00 	mov    rax,QWORD PTR [rip+0xa56f2c]        # 0x140d6d300
   1403163d4:	0f bf 48 20          	movsx  ecx,WORD PTR [rax+0x20]
   1403163d8:	89 8b 50 5d 01 00    	mov    DWORD PTR [rbx+0x15d50],ecx
   1403163de:	48 8b 05 1b 6f a5 00 	mov    rax,QWORD PTR [rip+0xa56f1b]        # 0x140d6d300
   1403163e5:	0f bf 48 24          	movsx  ecx,WORD PTR [rax+0x24]
   1403163e9:	ff c9                	dec    ecx
   1403163eb:	89 8b 48 5d 01 00    	mov    DWORD PTR [rbx+0x15d48],ecx
   1403163f1:	48 89 93 c0 5c 01 00 	mov    QWORD PTR [rbx+0x15cc0],rdx
   1403163f8:	48 89 93 c8 5c 01 00 	mov    QWORD PTR [rbx+0x15cc8],rdx
   1403163ff:	48 89 93 d0 5c 01 00 	mov    QWORD PTR [rbx+0x15cd0],rdx
   140316406:	c7 83 38 5d 01 00 08 	mov    DWORD PTR [rbx+0x15d38],0x8
   14031640d:	00 00 00 
   140316410:	8b 05 6a 62 a5 00    	mov    eax,DWORD PTR [rip+0xa5626a]        # 0x140d6c680
   140316416:	89 83 54 5d 01 00    	mov    DWORD PTR [rbx+0x15d54],eax
   14031641c:	48 83 c4 20          	add    rsp,0x20
   140316420:	5b                   	pop    rbx
   140316421:	c3                   	ret
