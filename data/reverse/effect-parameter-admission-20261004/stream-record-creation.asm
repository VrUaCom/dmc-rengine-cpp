
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031ea50 <.text+0x31da50>:
   14031ea50:	48 89 6c 24 10       	mov    QWORD PTR [rsp+0x10],rbp
   14031ea55:	48 89 74 24 18       	mov    QWORD PTR [rsp+0x18],rsi
   14031ea5a:	57                   	push   rdi
   14031ea5b:	48 83 ec 20          	sub    rsp,0x20
   14031ea5f:	33 ff                	xor    edi,edi
   14031ea61:	48 8b f1             	mov    rsi,rcx
   14031ea64:	44 8b cf             	mov    r9d,edi
   14031ea67:	0f b6 ca             	movzx  ecx,dl
   14031ea6a:	41 0f b6 e8          	movzx  ebp,r8b
   14031ea6e:	66 90                	xchg   ax,ax
   14031ea70:	4d 63 d1             	movsxd r10,r9d
   14031ea73:	42 0f bf 44 56 04    	movsx  eax,WORD PTR [rsi+r10*2+0x4]
   14031ea79:	3b c1                	cmp    eax,ecx
   14031ea7b:	75 0a                	jne    0x14031ea87
   14031ea7d:	41 38 ac 32 00 01 00 	cmp    BYTE PTR [r10+rsi*1+0x100],bpl
   14031ea84:	00 
   14031ea85:	74 38                	je     0x14031eabf
   14031ea87:	41 ff c1             	inc    r9d
   14031ea8a:	41 83 f9 14          	cmp    r9d,0x14
   14031ea8e:	7c e0                	jl     0x14031ea70
   14031ea90:	48 8b cf             	mov    rcx,rdi
   14031ea93:	48 8d 46 04          	lea    rax,[rsi+0x4]
   14031ea97:	66 83 38 ff          	cmp    WORD PTR [rax],0xffff
   14031ea9b:	74 3d                	je     0x14031eada
   14031ea9d:	ff c7                	inc    edi
   14031ea9f:	48 ff c1             	inc    rcx
   14031eaa2:	48 83 c0 02          	add    rax,0x2
   14031eaa6:	48 83 f9 14          	cmp    rcx,0x14
   14031eaaa:	7c eb                	jl     0x14031ea97
   14031eaac:	83 c8 ff             	or     eax,0xffffffff
   14031eaaf:	48 8b 6c 24 38       	mov    rbp,QWORD PTR [rsp+0x38]
   14031eab4:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   14031eab9:	48 83 c4 20          	add    rsp,0x20
   14031eabd:	5f                   	pop    rdi
   14031eabe:	c3                   	ret
   14031eabf:	66 45 85 c9          	test   r9w,r9w
   14031eac3:	78 cb                	js     0x14031ea90
   14031eac5:	b8 fe ff ff ff       	mov    eax,0xfffffffe
   14031eaca:	48 8b 6c 24 38       	mov    rbp,QWORD PTR [rsp+0x38]
   14031eacf:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   14031ead4:	48 83 c4 20          	add    rsp,0x20
   14031ead8:	5f                   	pop    rdi
   14031ead9:	c3                   	ret
   14031eada:	85 ff                	test   edi,edi
   14031eadc:	79 13                	jns    0x14031eaf1
   14031eade:	83 c8 ff             	or     eax,0xffffffff
   14031eae1:	48 8b 6c 24 38       	mov    rbp,QWORD PTR [rsp+0x38]
   14031eae6:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   14031eaeb:	48 83 c4 20          	add    rsp,0x20
   14031eaef:	5f                   	pop    rdi
   14031eaf0:	c3                   	ret
   14031eaf1:	0f b6 ca             	movzx  ecx,dl
   14031eaf4:	45 33 c0             	xor    r8d,r8d
   14031eaf7:	48 89 5c 24 30       	mov    QWORD PTR [rsp+0x30],rbx
   14031eafc:	48 63 df             	movsxd rbx,edi
   14031eaff:	66 89 4c 5e 04       	mov    WORD PTR [rsi+rbx*2+0x4],cx
   14031eb04:	48 8d 0d 75 b0 9b 00 	lea    rcx,[rip+0x9bb075]        # 0x140cd9b80
   14031eb0b:	e8 c0 70 ff ff       	call   0x140315bd0
   14031eb10:	48 89 44 de 58       	mov    QWORD PTR [rsi+rbx*8+0x58],rax
   14031eb15:	8b c7                	mov    eax,edi
   14031eb17:	40 88 ac 33 00 01 00 	mov    BYTE PTR [rbx+rsi*1+0x100],bpl
   14031eb1e:	00 
   14031eb1f:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031eb24:	48 8b 6c 24 38       	mov    rbp,QWORD PTR [rsp+0x38]
   14031eb29:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   14031eb2e:	48 83 c4 20          	add    rsp,0x20
   14031eb32:	5f                   	pop    rdi
   14031eb33:	c3                   	ret
