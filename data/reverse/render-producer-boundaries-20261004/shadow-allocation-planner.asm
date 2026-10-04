
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031fc40 <.text+0x31ec40>:
   14031fc40:	48 89 5c 24 08       	mov    QWORD PTR [rsp+0x8],rbx
   14031fc45:	48 89 6c 24 10       	mov    QWORD PTR [rsp+0x10],rbp
   14031fc4a:	48 89 74 24 18       	mov    QWORD PTR [rsp+0x18],rsi
   14031fc4f:	48 89 7c 24 20       	mov    QWORD PTR [rsp+0x20],rdi
   14031fc54:	41 56                	push   r14
   14031fc56:	4c 8b 71 28          	mov    r14,QWORD PTR [rcx+0x28]
   14031fc5a:	bd 02 00 00 00       	mov    ebp,0x2
   14031fc5f:	41 0f b6 76 10       	movzx  esi,BYTE PTR [r14+0x10]
   14031fc64:	44 8d 14 b6          	lea    r10d,[rsi+rsi*4]
   14031fc68:	41 c1 e2 05          	shl    r10d,0x5
   14031fc6c:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   14031fc70:	45 33 db             	xor    r11d,r11d
   14031fc73:	33 db                	xor    ebx,ebx
   14031fc75:	41 81 c2 40 01 00 00 	add    r10d,0x140
   14031fc7c:	33 ff                	xor    edi,edi
   14031fc7e:	48 83 fe 02          	cmp    rsi,0x2
   14031fc82:	72 51                	jb     0x14031fcd5
   14031fc84:	4c 8d 4e fe          	lea    r9,[rsi-0x2]
   14031fc88:	49 d1 e9             	shr    r9,1
   14031fc8b:	49 8d 46 62          	lea    rax,[r14+0x62]
   14031fc8f:	49 ff c1             	inc    r9
   14031fc92:	4b 8d 3c 09          	lea    rdi,[r9+r9*1]
   14031fc96:	48 0f bf 50 be       	movsx  rdx,WORD PTR [rax-0x42]
   14031fc9b:	4c 0f bf 40 c0       	movsx  r8,WORD PTR [rax-0x40]
   14031fca0:	48 8d 80 80 00 00 00 	lea    rax,[rax+0x80]
   14031fca7:	83 c2 06             	add    edx,0x6
   14031fcaa:	44 03 c2             	add    r8d,edx
   14031fcad:	48 0f bf 90 7e ff ff 	movsx  rdx,WORD PTR [rax-0x82]
   14031fcb4:	ff 
   14031fcb5:	41 c1 e0 04          	shl    r8d,0x4
   14031fcb9:	45 03 d8             	add    r11d,r8d
   14031fcbc:	4c 0f bf 40 80       	movsx  r8,WORD PTR [rax-0x80]
   14031fcc1:	41 83 c0 06          	add    r8d,0x6
   14031fcc5:	44 03 c2             	add    r8d,edx
   14031fcc8:	41 c1 e0 04          	shl    r8d,0x4
   14031fccc:	41 03 d8             	add    ebx,r8d
   14031fccf:	49 83 e9 01          	sub    r9,0x1
   14031fcd3:	75 c1                	jne    0x14031fc96
   14031fcd5:	48 3b fe             	cmp    rdi,rsi
   14031fcd8:	7d 1e                	jge    0x14031fcf8
   14031fcda:	48 c1 e7 06          	shl    rdi,0x6
   14031fcde:	4e 0f bf 44 37 20    	movsx  r8,WORD PTR [rdi+r14*1+0x20]
   14031fce4:	4a 0f bf 54 37 22    	movsx  rdx,WORD PTR [rdi+r14*1+0x22]
   14031fcea:	41 83 c0 06          	add    r8d,0x6
   14031fcee:	44 03 c2             	add    r8d,edx
   14031fcf1:	41 c1 e0 04          	shl    r8d,0x4
   14031fcf5:	45 03 d0             	add    r10d,r8d
   14031fcf8:	42 8d 0c 1b          	lea    ecx,[rbx+r11*1]
   14031fcfc:	44 03 d1             	add    r10d,ecx
   14031fcff:	48 83 ed 01          	sub    rbp,0x1
   14031fd03:	0f 85 67 ff ff ff    	jne    0x14031fc70
   14031fd09:	48 8b 5c 24 10       	mov    rbx,QWORD PTR [rsp+0x10]
   14031fd0e:	41 8b c2             	mov    eax,r10d
   14031fd11:	48 8b 6c 24 18       	mov    rbp,QWORD PTR [rsp+0x18]
   14031fd16:	48 8b 74 24 20       	mov    rsi,QWORD PTR [rsp+0x20]
   14031fd1b:	48 8b 7c 24 28       	mov    rdi,QWORD PTR [rsp+0x28]
   14031fd20:	41 5e                	pop    r14
   14031fd22:	c3                   	ret
