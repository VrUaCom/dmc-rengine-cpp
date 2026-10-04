
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014008bc60 <.text+0x8ac60>:
   14008bc60:	48 89 5c 24 08       	mov    QWORD PTR [rsp+0x8],rbx
   14008bc65:	48 89 6c 24 10       	mov    QWORD PTR [rsp+0x10],rbp
   14008bc6a:	48 89 74 24 18       	mov    QWORD PTR [rsp+0x18],rsi
   14008bc6f:	57                   	push   rdi
   14008bc70:	48 83 ec 20          	sub    rsp,0x20
   14008bc74:	48 8b e9             	mov    rbp,rcx
   14008bc77:	48 83 c1 30          	add    rcx,0x30
   14008bc7b:	e8 30 48 29 00       	call   0x1403204b0
   14008bc80:	48 8d 4d 30          	lea    rcx,[rbp+0x30]
   14008bc84:	e8 b7 3f 29 00       	call   0x14031fc40
   14008bc89:	48 8d 4d 08          	lea    rcx,[rbp+0x8]
   14008bc8d:	41 b8 fe ff ff ff    	mov    r8d,0xfffffffe
   14008bc93:	8b d0                	mov    edx,eax
   14008bc95:	8b f8                	mov    edi,eax
   14008bc97:	e8 b4 a4 23 00       	call   0x1402c6150
   14008bc9c:	48 8b f0             	mov    rsi,rax
   14008bc9f:	48 85 c0             	test   rax,rax
   14008bca2:	74 2d                	je     0x14008bcd1
   14008bca4:	8b d7                	mov    edx,edi
   14008bca6:	48 8b ce             	mov    rcx,rsi
   14008bca9:	c1 ea 02             	shr    edx,0x2
   14008bcac:	e8 0f 17 2a 00       	call   0x14032d3c0
   14008bcb1:	48 8b d6             	mov    rdx,rsi
   14008bcb4:	48 8d 4d 30          	lea    rcx,[rbp+0x30]
   14008bcb8:	e8 73 40 29 00       	call   0x14031fd30
   14008bcbd:	8b d7                	mov    edx,edi
   14008bcbf:	48 8d 4d 30          	lea    rcx,[rbp+0x30]
   14008bcc3:	e8 f8 43 29 00       	call   0x1403200c0
   14008bcc8:	b0 01                	mov    al,0x1
   14008bcca:	c6 85 b0 00 00 00 01 	mov    BYTE PTR [rbp+0xb0],0x1
   14008bcd1:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14008bcd6:	48 8b 6c 24 38       	mov    rbp,QWORD PTR [rsp+0x38]
   14008bcdb:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   14008bce0:	48 83 c4 20          	add    rsp,0x20
   14008bce4:	5f                   	pop    rdi
   14008bce5:	c3                   	ret
