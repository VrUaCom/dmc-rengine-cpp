
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031ec60 <.text+0x31dc60>:
   14031ec60:	33 c0                	xor    eax,eax
   14031ec62:	44 0f b6 ca          	movzx  r9d,dl
   14031ec66:	66 66 0f 1f 84 00 00 	data16 nop WORD PTR [rax+rax*1+0x0]
   14031ec6d:	00 00 00 
   14031ec70:	4c 63 c0             	movsxd r8,eax
   14031ec73:	42 0f bf 54 41 04    	movsx  edx,WORD PTR [rcx+r8*2+0x4]
   14031ec79:	41 3b d1             	cmp    edx,r9d
   14031ec7c:	75 0b                	jne    0x14031ec89
   14031ec7e:	41 80 bc 08 00 01 00 	cmp    BYTE PTR [r8+rcx*1+0x100],0x0
   14031ec85:	00 00 
   14031ec87:	74 0a                	je     0x14031ec93
   14031ec89:	ff c0                	inc    eax
   14031ec8b:	83 f8 14             	cmp    eax,0x14
   14031ec8e:	7c e0                	jl     0x14031ec70
   14031ec90:	83 c8 ff             	or     eax,0xffffffff
   14031ec93:	48 0f bf d0          	movsx  rdx,ax
   14031ec97:	85 d2                	test   edx,edx
   14031ec99:	79 03                	jns    0x14031ec9e
   14031ec9b:	33 c0                	xor    eax,eax
   14031ec9d:	c3                   	ret
   14031ec9e:	48 8b 44 d1 58       	mov    rax,QWORD PTR [rcx+rdx*8+0x58]
   14031eca3:	c3                   	ret
