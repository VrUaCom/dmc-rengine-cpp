
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140316a90 <.text+0x315a90>:
   140316a90:	48 83 ec 28          	sub    rsp,0x28
   140316a94:	48 85 d2             	test   rdx,rdx
   140316a97:	75 07                	jne    0x140316aa0
   140316a99:	32 c0                	xor    al,al
   140316a9b:	48 83 c4 28          	add    rsp,0x28
   140316a9f:	c3                   	ret
   140316aa0:	41 8b c8             	mov    ecx,r8d
   140316aa3:	48 83 7c ca 40 00    	cmp    QWORD PTR [rdx+rcx*8+0x40],0x0
   140316aa9:	48 89 5c 24 20       	mov    QWORD PTR [rsp+0x20],rbx
   140316aae:	48 8d 1c ca          	lea    rbx,[rdx+rcx*8]
   140316ab2:	75 0c                	jne    0x140316ac0
   140316ab4:	32 c0                	xor    al,al
   140316ab6:	48 8b 5c 24 20       	mov    rbx,QWORD PTR [rsp+0x20]
   140316abb:	48 83 c4 28          	add    rsp,0x28
   140316abf:	c3                   	ret
   140316ac0:	48 c1 e1 05          	shl    rcx,0x5
   140316ac4:	48 03 ca             	add    rcx,rdx
   140316ac7:	e8 94 f7 fa ff       	call   0x1402c6260
   140316acc:	48 c7 43 40 00 00 00 	mov    QWORD PTR [rbx+0x40],0x0
   140316ad3:	00 
   140316ad4:	b0 01                	mov    al,0x1
   140316ad6:	48 8b 5c 24 20       	mov    rbx,QWORD PTR [rsp+0x20]
   140316adb:	48 83 c4 28          	add    rsp,0x28
   140316adf:	c3                   	ret
