
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031ee00 <.text+0x31de00>:
   14031ee00:	40 53                	rex push rbx
   14031ee02:	41 54                	push   r12
   14031ee04:	41 56                	push   r14
   14031ee06:	48 83 ec 40          	sub    rsp,0x40
   14031ee0a:	4c 8b 81 f8 00 00 00 	mov    r8,QWORD PTR [rcx+0xf8]
   14031ee11:	44 0f b7 e2          	movzx  r12d,dx
   14031ee15:	48 8b d9             	mov    rbx,rcx
   14031ee18:	4d 85 c0             	test   r8,r8
   14031ee1b:	75 0d                	jne    0x14031ee2a
   14031ee1d:	83 c8 ff             	or     eax,0xffffffff
   14031ee20:	48 83 c4 40          	add    rsp,0x40
   14031ee24:	41 5e                	pop    r14
   14031ee26:	41 5c                	pop    r12
   14031ee28:	5b                   	pop    rbx
   14031ee29:	c3                   	ret
   14031ee2a:	66 44 39 21          	cmp    WORD PTR [rcx],r12w
   14031ee2e:	7f 0f                	jg     0x14031ee3f
   14031ee30:	b8 fe ff ff ff       	mov    eax,0xfffffffe
   14031ee35:	48 83 c4 40          	add    rsp,0x40
   14031ee39:	41 5e                	pop    r14
   14031ee3b:	41 5c                	pop    r12
   14031ee3d:	5b                   	pop    rbx
   14031ee3e:	c3                   	ret
   14031ee3f:	48 89 6c 24 68       	mov    QWORD PTR [rsp+0x68],rbp
   14031ee44:	41 83 ce ff          	or     r14d,0xffffffff
   14031ee48:	48 89 7c 24 78       	mov    QWORD PTR [rsp+0x78],rdi
   14031ee4d:	4c 89 6c 24 38       	mov    QWORD PTR [rsp+0x38],r13
   14031ee52:	4c 89 7c 24 30       	mov    QWORD PTR [rsp+0x30],r15
   14031ee57:	45 0f bf fc          	movsx  r15d,r12w
   14031ee5b:	41 8d 47 01          	lea    eax,[r15+0x1]
   14031ee5f:	41 39 40 04          	cmp    DWORD PTR [r8+0x4],eax
   14031ee63:	72 1d                	jb     0x14031ee82
   14031ee65:	41 8d 47 02          	lea    eax,[r15+0x2]
   14031ee69:	41 8b 04 80          	mov    eax,DWORD PTR [r8+rax*4]
   14031ee6d:	85 c0                	test   eax,eax
   14031ee6f:	74 11                	je     0x14031ee82
   14031ee71:	49 03 c0             	add    rax,r8
   14031ee74:	74 0c                	je     0x14031ee82
   14031ee76:	44 0f b7 68 04       	movzx  r13d,WORD PTR [rax+0x4]
   14031ee7b:	44 89 6c 24 60       	mov    DWORD PTR [rsp+0x60],r13d
   14031ee80:	eb 08                	jmp    0x14031ee8a
   14031ee82:	45 8b ee             	mov    r13d,r14d
   14031ee85:	44 89 74 24 60       	mov    DWORD PTR [rsp+0x60],r14d
   14031ee8a:	33 ed                	xor    ebp,ebp
   14031ee8c:	48 89 74 24 70       	mov    QWORD PTR [rsp+0x70],rsi
   14031ee91:	8b fd                	mov    edi,ebp
   14031ee93:	66 3b 69 54          	cmp    bp,WORD PTR [rcx+0x54]
   14031ee97:	7d 33                	jge    0x14031eecc
   14031ee99:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
   14031eea0:	48 63 c7             	movsxd rax,edi
   14031eea3:	66 39 6c 43 2c       	cmp    WORD PTR [rbx+rax*2+0x2c],bp
   14031eea8:	48 8d 34 43          	lea    rsi,[rbx+rax*2]
   14031eeac:	7c 14                	jl     0x14031eec2
   14031eeae:	0f b6 56 2c          	movzx  edx,BYTE PTR [rsi+0x2c]
   14031eeb2:	45 33 c0             	xor    r8d,r8d
   14031eeb5:	48 8b cb             	mov    rcx,rbx
   14031eeb8:	e8 53 fe ff ff       	call   0x14031ed10
   14031eebd:	66 44 89 76 2c       	mov    WORD PTR [rsi+0x2c],r14w
   14031eec2:	0f bf 43 54          	movsx  eax,WORD PTR [rbx+0x54]
   14031eec6:	ff c7                	inc    edi
   14031eec8:	3b f8                	cmp    edi,eax
   14031eeca:	7c d4                	jl     0x14031eea0
   14031eecc:	45 0f bf ed          	movsx  r13d,r13w
   14031eed0:	45 85 ed             	test   r13d,r13d
   14031eed3:	0f 8e 34 01 00 00    	jle    0x14031f00d
   14031eed9:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
   14031eee0:	44 0f b7 03          	movzx  r8d,WORD PTR [rbx]
   14031eee4:	66 45 3b c4          	cmp    r8w,r12w
   14031eee8:	7e 66                	jle    0x14031ef50
   14031eeea:	48 8b 93 f8 00 00 00 	mov    rdx,QWORD PTR [rbx+0xf8]
   14031eef1:	48 85 d2             	test   rdx,rdx
   14031eef4:	74 21                	je     0x14031ef17
   14031eef6:	41 8d 47 01          	lea    eax,[r15+0x1]
   14031eefa:	39 42 04             	cmp    DWORD PTR [rdx+0x4],eax
   14031eefd:	72 18                	jb     0x14031ef17
   14031eeff:	41 8d 47 02          	lea    eax,[r15+0x2]
   14031ef03:	8b c8                	mov    ecx,eax
   14031ef05:	8b 04 82             	mov    eax,DWORD PTR [rdx+rax*4]
   14031ef08:	85 c0                	test   eax,eax
   14031ef0a:	74 0b                	je     0x14031ef17
   14031ef0c:	48 03 c2             	add    rax,rdx
   14031ef0f:	74 06                	je     0x14031ef17
   14031ef11:	0f b7 48 04          	movzx  ecx,WORD PTR [rax+0x4]
   14031ef15:	eb 03                	jmp    0x14031ef1a
   14031ef17:	41 8b ce             	mov    ecx,r14d
   14031ef1a:	66 3b cd             	cmp    cx,bp
   14031ef1d:	7f 07                	jg     0x14031ef26
   14031ef1f:	bf fe ff ff ff       	mov    edi,0xfffffffe
   14031ef24:	eb 2d                	jmp    0x14031ef53
   14031ef26:	48 85 d2             	test   rdx,rdx
   14031ef29:	74 25                	je     0x14031ef50
   14031ef2b:	41 8d 47 01          	lea    eax,[r15+0x1]
   14031ef2f:	39 42 04             	cmp    DWORD PTR [rdx+0x4],eax
   14031ef32:	72 1c                	jb     0x14031ef50
   14031ef34:	41 8d 47 02          	lea    eax,[r15+0x2]
   14031ef38:	8b 04 82             	mov    eax,DWORD PTR [rdx+rax*4]
   14031ef3b:	85 c0                	test   eax,eax
   14031ef3d:	74 11                	je     0x14031ef50
   14031ef3f:	8b c8                	mov    ecx,eax
   14031ef41:	48 03 ca             	add    rcx,rdx
   14031ef44:	74 0a                	je     0x14031ef50
   14031ef46:	48 0f bf c5          	movsx  rax,bp
   14031ef4a:	8b 7c c1 08          	mov    edi,DWORD PTR [rcx+rax*8+0x8]
   14031ef4e:	eb 03                	jmp    0x14031ef53
   14031ef50:	41 8b fe             	mov    edi,r14d
   14031ef53:	66 45 3b c4          	cmp    r8w,r12w
   14031ef57:	0f 8e e2 00 00 00    	jle    0x14031f03f
   14031ef5d:	48 8b 93 f8 00 00 00 	mov    rdx,QWORD PTR [rbx+0xf8]
   14031ef64:	48 85 d2             	test   rdx,rdx
   14031ef67:	74 21                	je     0x14031ef8a
   14031ef69:	41 8d 47 01          	lea    eax,[r15+0x1]
   14031ef6d:	39 42 04             	cmp    DWORD PTR [rdx+0x4],eax
   14031ef70:	72 18                	jb     0x14031ef8a
   14031ef72:	41 8d 47 02          	lea    eax,[r15+0x2]
   14031ef76:	8b c8                	mov    ecx,eax
   14031ef78:	8b 04 82             	mov    eax,DWORD PTR [rdx+rax*4]
   14031ef7b:	85 c0                	test   eax,eax
   14031ef7d:	74 0b                	je     0x14031ef8a
   14031ef7f:	48 03 c2             	add    rax,rdx
   14031ef82:	74 06                	je     0x14031ef8a
   14031ef84:	0f b7 48 04          	movzx  ecx,WORD PTR [rax+0x4]
   14031ef88:	eb 03                	jmp    0x14031ef8d
   14031ef8a:	41 8b ce             	mov    ecx,r14d
   14031ef8d:	66 3b cd             	cmp    cx,bp
   14031ef90:	0f 8e a9 00 00 00    	jle    0x14031f03f
   14031ef96:	48 85 d2             	test   rdx,rdx
   14031ef99:	0f 84 a0 00 00 00    	je     0x14031f03f
   14031ef9f:	41 8d 47 01          	lea    eax,[r15+0x1]
   14031efa3:	39 42 04             	cmp    DWORD PTR [rdx+0x4],eax
   14031efa6:	0f 82 93 00 00 00    	jb     0x14031f03f
   14031efac:	41 8d 47 02          	lea    eax,[r15+0x2]
   14031efb0:	8b 04 82             	mov    eax,DWORD PTR [rdx+rax*4]
   14031efb3:	85 c0                	test   eax,eax
   14031efb5:	0f 84 84 00 00 00    	je     0x14031f03f
   14031efbb:	8b c8                	mov    ecx,eax
   14031efbd:	48 03 ca             	add    rcx,rdx
   14031efc0:	74 7d                	je     0x14031f03f
   14031efc2:	48 0f bf c5          	movsx  rax,bp
   14031efc6:	8b 74 c1 0c          	mov    esi,DWORD PTR [rcx+rax*8+0xc]
   14031efca:	48 03 f1             	add    rsi,rcx
   14031efcd:	74 70                	je     0x14031f03f
   14031efcf:	45 33 c0             	xor    r8d,r8d
   14031efd2:	40 0f b6 d7          	movzx  edx,dil
   14031efd6:	48 8b cb             	mov    rcx,rbx
   14031efd9:	e8 72 fa ff ff       	call   0x14031ea50
   14031efde:	85 c0                	test   eax,eax
   14031efe0:	78 07                	js     0x14031efe9
   14031efe2:	48 98                	cdqe
   14031efe4:	66 89 7c 43 2c       	mov    WORD PTR [rbx+rax*2+0x2c],di
   14031efe9:	41 b1 01             	mov    r9b,0x1
   14031efec:	c6 44 24 20 00       	mov    BYTE PTR [rsp+0x20],0x0
   14031eff1:	4c 8b c6             	mov    r8,rsi
   14031eff4:	8b d7                	mov    edx,edi
   14031eff6:	48 8b cb             	mov    rcx,rbx
   14031eff9:	e8 52 00 00 00       	call   0x14031f050
   14031effe:	85 c0                	test   eax,eax
   14031f000:	75 3d                	jne    0x14031f03f
   14031f002:	ff c5                	inc    ebp
   14031f004:	41 3b ed             	cmp    ebp,r13d
   14031f007:	0f 8c d3 fe ff ff    	jl     0x14031eee0
   14031f00d:	8b 44 24 60          	mov    eax,DWORD PTR [rsp+0x60]
   14031f011:	66 89 43 54          	mov    WORD PTR [rbx+0x54],ax
   14031f015:	33 c0                	xor    eax,eax
   14031f017:	66 44 89 63 02       	mov    WORD PTR [rbx+0x2],r12w
   14031f01c:	48 8b 74 24 70       	mov    rsi,QWORD PTR [rsp+0x70]
   14031f021:	48 8b 7c 24 78       	mov    rdi,QWORD PTR [rsp+0x78]
   14031f026:	4c 8b 6c 24 38       	mov    r13,QWORD PTR [rsp+0x38]
   14031f02b:	48 8b 6c 24 68       	mov    rbp,QWORD PTR [rsp+0x68]
   14031f030:	4c 8b 7c 24 30       	mov    r15,QWORD PTR [rsp+0x30]
   14031f035:	48 83 c4 40          	add    rsp,0x40
   14031f039:	41 5e                	pop    r14
   14031f03b:	41 5c                	pop    r12
   14031f03d:	5b                   	pop    rbx
   14031f03e:	c3                   	ret
   14031f03f:	41 8b c6             	mov    eax,r14d
   14031f042:	eb d8                	jmp    0x14031f01c
