
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140337cd0 <.text+0x336cd0>:
   140337cd0:	48 83 ec 28          	sub    rsp,0x28
   140337cd4:	48 8b 05 25 56 a3 00 	mov    rax,QWORD PTR [rip+0xa35625]        # 0x140d6d300
   140337cdb:	c6 00 00             	mov    BYTE PTR [rax],0x0
   140337cde:	48 8b 05 1b 56 a3 00 	mov    rax,QWORD PTR [rip+0xa3561b]        # 0x140d6d300
   140337ce5:	c6 40 01 01          	mov    BYTE PTR [rax+0x1],0x1
   140337ce9:	48 8b 05 10 56 a3 00 	mov    rax,QWORD PTR [rip+0xa35610]        # 0x140d6d300
   140337cf0:	85 d2                	test   edx,edx
   140337cf2:	74 18                	je     0x140337d0c
   140337cf4:	c6 40 02 03          	mov    BYTE PTR [rax+0x2],0x3
   140337cf8:	48 63 c1             	movsxd rax,ecx
   140337cfb:	48 8d 0d fe 82 cc ff 	lea    rcx,[rip+0xffffffffffcc82fe]        # 0x140000000
   140337d02:	48 8b 94 c1 50 1b 5d 	mov    rdx,QWORD PTR [rcx+rax*8+0x5d1b50]
   140337d09:	00 
   140337d0a:	eb 16                	jmp    0x140337d22
   140337d0c:	c6 40 02 02          	mov    BYTE PTR [rax+0x2],0x2
   140337d10:	48 63 c1             	movsxd rax,ecx
   140337d13:	48 8d 0d e6 82 cc ff 	lea    rcx,[rip+0xffffffffffcc82e6]        # 0x140000000
   140337d1a:	48 8b 94 c1 08 1b 5d 	mov    rdx,QWORD PTR [rcx+rax*8+0x5d1b08]
   140337d21:	00 
   140337d22:	0f b6 0a             	movzx  ecx,BYTE PTR [rdx]
   140337d25:	48 8b 05 d4 55 a3 00 	mov    rax,QWORD PTR [rip+0xa355d4]        # 0x140d6d300
   140337d2c:	88 48 03             	mov    BYTE PTR [rax+0x3],cl
   140337d2f:	0f b6 4a 02          	movzx  ecx,BYTE PTR [rdx+0x2]
   140337d33:	48 8b 05 c6 55 a3 00 	mov    rax,QWORD PTR [rip+0xa355c6]        # 0x140d6d300
   140337d3a:	88 48 04             	mov    BYTE PTR [rax+0x4],cl
   140337d3d:	0f b6 4a 04          	movzx  ecx,BYTE PTR [rdx+0x4]
   140337d41:	48 8b 05 b8 55 a3 00 	mov    rax,QWORD PTR [rip+0xa355b8]        # 0x140d6d300
   140337d48:	88 48 05             	mov    BYTE PTR [rax+0x5],cl
   140337d4b:	0f b6 4a 06          	movzx  ecx,BYTE PTR [rdx+0x6]
   140337d4f:	48 8b 05 aa 55 a3 00 	mov    rax,QWORD PTR [rip+0xa355aa]        # 0x140d6d300
   140337d56:	88 48 08             	mov    BYTE PTR [rax+0x8],cl
   140337d59:	0f b7 4a 08          	movzx  ecx,WORD PTR [rdx+0x8]
   140337d5d:	48 8b 05 9c 55 a3 00 	mov    rax,QWORD PTR [rip+0xa3559c]        # 0x140d6d300
   140337d64:	66 89 48 20          	mov    WORD PTR [rax+0x20],cx
   140337d68:	0f b7 4a 0a          	movzx  ecx,WORD PTR [rdx+0xa]
   140337d6c:	48 8b 05 8d 55 a3 00 	mov    rax,QWORD PTR [rip+0xa3558d]        # 0x140d6d300
   140337d73:	66 89 48 22          	mov    WORD PTR [rax+0x22],cx
   140337d77:	0f b7 4a 0c          	movzx  ecx,WORD PTR [rdx+0xc]
   140337d7b:	48 8b 05 7e 55 a3 00 	mov    rax,QWORD PTR [rip+0xa3557e]        # 0x140d6d300
   140337d82:	66 89 48 24          	mov    WORD PTR [rax+0x24],cx
   140337d86:	0f b6 4a 0e          	movzx  ecx,BYTE PTR [rdx+0xe]
   140337d8a:	48 8b 05 6f 55 a3 00 	mov    rax,QWORD PTR [rip+0xa3556f]        # 0x140d6d300
   140337d91:	88 48 06             	mov    BYTE PTR [rax+0x6],cl
   140337d94:	48 8b 05 65 55 a3 00 	mov    rax,QWORD PTR [rip+0xa35565]        # 0x140d6d300
   140337d9b:	c6 40 07 00          	mov    BYTE PTR [rax+0x7],0x0
   140337d9f:	48 8b 05 5a 55 a3 00 	mov    rax,QWORD PTR [rip+0xa3555a]        # 0x140d6d300
   140337da6:	c6 40 09 01          	mov    BYTE PTR [rax+0x9],0x1
   140337daa:	48 8b 05 4f 55 a3 00 	mov    rax,QWORD PTR [rip+0xa3554f]        # 0x140d6d300
   140337db1:	c6 40 0a 80          	mov    BYTE PTR [rax+0xa],0x80
   140337db5:	48 8b 05 44 55 a3 00 	mov    rax,QWORD PTR [rip+0xa35544]        # 0x140d6d300
   140337dbc:	0f bf 48 24          	movsx  ecx,WORD PTR [rax+0x24]
   140337dc0:	0f bf 40 22          	movsx  eax,WORD PTR [rax+0x22]
   140337dc4:	2b c1                	sub    eax,ecx
   140337dc6:	33 c9                	xor    ecx,ecx
   140337dc8:	99                   	cdq
   140337dc9:	2b c2                	sub    eax,edx
   140337dcb:	33 d2                	xor    edx,edx
   140337dcd:	d1 f8                	sar    eax,1
   140337dcf:	44 8b c0             	mov    r8d,eax
   140337dd2:	e8 59 b0 ff ff       	call   0x140332e30
   140337dd7:	33 d2                	xor    edx,edx
   140337dd9:	33 c9                	xor    ecx,ecx
   140337ddb:	48 83 c4 28          	add    rsp,0x28
   140337ddf:	e9 dc a8 ff ff       	jmp    0x1403326c0
