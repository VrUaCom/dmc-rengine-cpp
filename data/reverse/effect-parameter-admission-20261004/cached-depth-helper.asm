
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140316d70 <.text+0x315d70>:
   140316d70:	48 83 ec 58          	sub    rsp,0x58
   140316d74:	0f 29 74 24 40       	movaps XMMWORD PTR [rsp+0x40],xmm6
   140316d79:	48 8b 05 30 e3 2b 00 	mov    rax,QWORD PTR [rip+0x2be330]        # 0x1405d50b0
   140316d80:	48 33 c4             	xor    rax,rsp
   140316d83:	48 89 44 24 30       	mov    QWORD PTR [rsp+0x30],rax
   140316d88:	48 8b 15 a1 b5 9d 00 	mov    rdx,QWORD PTR [rip+0x9db5a1]        # 0x140cf2330
   140316d8f:	0f 57 f6             	xorps  xmm6,xmm6
   140316d92:	f3 0f 11 4c 24 28    	movss  DWORD PTR [rsp+0x28],xmm1
   140316d98:	f3 0f 10 0d cc 67 04 	movss  xmm1,DWORD PTR [rip+0x467cc]        # 0x14035d56c
   140316d9f:	00 
   140316da0:	f3 0f 11 4c 24 2c    	movss  DWORD PTR [rsp+0x2c],xmm1
   140316da6:	48 c7 44 24 20 00 00 	mov    QWORD PTR [rsp+0x20],0x0
   140316dad:	00 00 
   140316daf:	48 85 d2             	test   rdx,rdx
   140316db2:	75 05                	jne    0x140316db9
   140316db4:	0f 28 c6             	movaps xmm0,xmm6
   140316db7:	eb 3b                	jmp    0x140316df4
   140316db9:	48 83 c2 40          	add    rdx,0x40
   140316dbd:	4c 8d 44 24 20       	lea    r8,[rsp+0x20]
   140316dc2:	48 8d 4c 24 20       	lea    rcx,[rsp+0x20]
   140316dc7:	e8 a4 9c d1 ff       	call   0x140030a70
   140316dcc:	f3 0f 10 44 24 2c    	movss  xmm0,DWORD PTR [rsp+0x2c]
   140316dd2:	f3 0f 58 05 c6 0e 1f 	addss  xmm0,DWORD PTR [rip+0x1f0ec6]        # 0x140507ca0
   140316dd9:	00 
   140316dda:	f3 0f 10 4c 24 28    	movss  xmm1,DWORD PTR [rsp+0x28]
   140316de0:	f3 0f 5e c8          	divss  xmm1,xmm0
   140316de4:	f3 0f 10 05 78 f3 1a 	movss  xmm0,DWORD PTR [rip+0x1af378]        # 0x1404c6164
   140316deb:	00 
   140316dec:	f3 0f 5f f1          	maxss  xmm6,xmm1
   140316df0:	f3 0f 5d c6          	minss  xmm0,xmm6
   140316df4:	48 8b 4c 24 30       	mov    rcx,QWORD PTR [rsp+0x30]
   140316df9:	48 33 cc             	xor    rcx,rsp
   140316dfc:	e8 ef e7 02 00       	call   0x1403455f0
   140316e01:	0f 28 74 24 40       	movaps xmm6,XMMWORD PTR [rsp+0x40]
   140316e06:	48 83 c4 58          	add    rsp,0x58
   140316e0a:	c3                   	ret
