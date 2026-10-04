
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140315bd0 <.text+0x314bd0>:
   140315bd0:	48 83 ec 28          	sub    rsp,0x28
   140315bd4:	45 33 c9             	xor    r9d,r9d
   140315bd7:	48 89 5c 24 20       	mov    QWORD PTR [rsp+0x20],rbx
   140315bdc:	45 8b c1             	mov    r8d,r9d
   140315bdf:	48 8b c1             	mov    rax,rcx
   140315be2:	80 38 00             	cmp    BYTE PTR [rax],0x0
   140315be5:	74 1e                	je     0x140315c05
   140315be7:	41 ff c1             	inc    r9d
   140315bea:	49 ff c0             	inc    r8
   140315bed:	48 05 80 02 00 00    	add    rax,0x280
   140315bf3:	49 83 f8 10          	cmp    r8,0x10
   140315bf7:	7c e9                	jl     0x140315be2
   140315bf9:	33 c0                	xor    eax,eax
   140315bfb:	48 8b 5c 24 20       	mov    rbx,QWORD PTR [rsp+0x20]
   140315c00:	48 83 c4 28          	add    rsp,0x28
   140315c04:	c3                   	ret
   140315c05:	49 63 c1             	movsxd rax,r9d
   140315c08:	48 8d 1c 80          	lea    rbx,[rax+rax*4]
   140315c0c:	48 c1 e3 07          	shl    rbx,0x7
   140315c10:	48 03 d9             	add    rbx,rcx
   140315c13:	75 0c                	jne    0x140315c21
   140315c15:	33 c0                	xor    eax,eax
   140315c17:	48 8b 5c 24 20       	mov    rbx,QWORD PTR [rsp+0x20]
   140315c1c:	48 83 c4 28          	add    rsp,0x28
   140315c20:	c3                   	ret
   140315c21:	44 0f b6 c2          	movzx  r8d,dl
   140315c25:	48 8b d3             	mov    rdx,rbx
   140315c28:	e8 e3 11 00 00       	call   0x140316e10
   140315c2d:	48 8b c3             	mov    rax,rbx
   140315c30:	48 8b 5c 24 20       	mov    rbx,QWORD PTR [rsp+0x20]
   140315c35:	48 83 c4 28          	add    rsp,0x28
   140315c39:	c3                   	ret
