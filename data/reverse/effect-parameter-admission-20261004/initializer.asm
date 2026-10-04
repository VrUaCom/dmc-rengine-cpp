
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

0000000140316e10 <.text+0x315e10>:
   140316e10:	48 89 5c 24 08       	mov    QWORD PTR [rsp+0x8],rbx
   140316e15:	48 89 6c 24 10       	mov    QWORD PTR [rsp+0x10],rbp
   140316e1a:	48 89 74 24 18       	mov    QWORD PTR [rsp+0x18],rsi
   140316e1f:	57                   	push   rdi
   140316e20:	48 83 ec 20          	sub    rsp,0x20
   140316e24:	48 8b fa             	mov    rdi,rdx
   140316e27:	41 0f b6 d8          	movzx  ebx,r8b
   140316e2b:	48 8b f1             	mov    rsi,rcx
   140316e2e:	33 d2                	xor    edx,edx
   140316e30:	48 8b cf             	mov    rcx,rdi
   140316e33:	41 b8 80 02 00 00    	mov    r8d,0x280
   140316e39:	e8 ac fd 02 00       	call   0x140346bea
   140316e3e:	33 ed                	xor    ebp,ebp
   140316e40:	88 1f                	mov    BYTE PTR [rdi],bl
   140316e42:	8d 43 ff             	lea    eax,[rbx-0x1]
   140316e45:	66 c7 47 01 00 00    	mov    WORD PTR [rdi+0x1],0x0
   140316e4b:	89 6f 08             	mov    DWORD PTR [rdi+0x8],ebp
   140316e4e:	c6 47 03 01          	mov    BYTE PTR [rdi+0x3],0x1
   140316e52:	66 89 6f 04          	mov    WORD PTR [rdi+0x4],bp
   140316e56:	48 89 6f 10          	mov    QWORD PTR [rdi+0x10],rbp
   140316e5a:	66 89 6f 0c          	mov    WORD PTR [rdi+0xc],bp
   140316e5e:	83 f8 0e             	cmp    eax,0xe
   140316e61:	0f 87 19 05 00 00    	ja     0x140317380
   140316e67:	48 8d 15 92 91 ce ff 	lea    rdx,[rip+0xffffffffffce9192]        # 0x140000000
   140316e6e:	48 98                	cdqe
   140316e70:	8b 8c 82 98 73 31 00 	mov    ecx,DWORD PTR [rdx+rax*4+0x317398]
   140316e77:	48 03 ca             	add    rcx,rdx
   140316e7a:	ff e1                	jmp    rcx
   140316e7c:	b8 18 00 00 00       	mov    eax,0x18
   140316e81:	c6 47 02 ff          	mov    BYTE PTR [rdi+0x2],0xff
   140316e85:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140316e89:	48 c7 47 18 80 80 80 	mov    QWORD PTR [rdi+0x18],0x40808080
   140316e90:	40 
   140316e91:	48 89 6f 24          	mov    QWORD PTR [rdi+0x24],rbp
   140316e95:	89 6f 20             	mov    DWORD PTR [rdi+0x20],ebp
   140316e98:	e9 e3 04 00 00       	jmp    0x140317380
   140316e9d:	b9 10 00 00 00       	mov    ecx,0x10
   140316ea2:	c6 47 02 28          	mov    BYTE PTR [rdi+0x2],0x28
   140316ea6:	66 89 4f 06          	mov    WORD PTR [rdi+0x6],cx
   140316eaa:	c7 47 18 00 00 7a 44 	mov    DWORD PTR [rdi+0x18],0x447a0000
   140316eb1:	c7 47 1c 00 80 89 44 	mov    DWORD PTR [rdi+0x1c],0x44898000
   140316eb8:	c7 47 20 03 00 00 00 	mov    DWORD PTR [rdi+0x20],0x3
   140316ebf:	c7 47 24 80 80 80 80 	mov    DWORD PTR [rdi+0x24],0x80808080
   140316ec6:	e9 b5 04 00 00       	jmp    0x140317380
   140316ecb:	b9 10 00 00 00       	mov    ecx,0x10
   140316ed0:	c7 47 04 02 00 48 00 	mov    DWORD PTR [rdi+0x4],0x480002
   140316ed7:	89 4f 28             	mov    DWORD PTR [rdi+0x28],ecx
   140316eda:	c6 47 02 2d          	mov    BYTE PTR [rdi+0x2],0x2d
   140316ede:	c7 47 2c 06 00 00 00 	mov    DWORD PTR [rdi+0x2c],0x6
   140316ee5:	c7 47 30 80 80 80 40 	mov    DWORD PTR [rdi+0x30],0x40808080
   140316eec:	c7 47 38 00 00 7a 44 	mov    DWORD PTR [rdi+0x38],0x447a0000
   140316ef3:	c7 47 34 00 00 fa 43 	mov    DWORD PTR [rdi+0x34],0x43fa0000
   140316efa:	48 c7 47 3c 00 00 7a 	mov    QWORD PTR [rdi+0x3c],0x447a0000
   140316f01:	44 
   140316f02:	c7 47 1c 00 00 fa 43 	mov    DWORD PTR [rdi+0x1c],0x43fa0000
   140316f09:	c7 47 20 00 00 7a 44 	mov    DWORD PTR [rdi+0x20],0x447a0000
   140316f10:	c7 47 24 00 00 00 3f 	mov    DWORD PTR [rdi+0x24],0x3f000000
   140316f17:	89 6f 44             	mov    DWORD PTR [rdi+0x44],ebp
   140316f1a:	e9 61 04 00 00       	jmp    0x140317380
   140316f1f:	c7 47 04 02 00 88 00 	mov    DWORD PTR [rdi+0x4],0x880002
   140316f26:	b8 02 00 00 00       	mov    eax,0x2
   140316f2b:	89 47 70             	mov    DWORD PTR [rdi+0x70],eax
   140316f2e:	c6 47 02 20          	mov    BYTE PTR [rdi+0x2],0x20
   140316f32:	c7 47 18 00 00 7a 44 	mov    DWORD PTR [rdi+0x18],0x447a0000
   140316f39:	c7 47 1c 00 80 bb 44 	mov    DWORD PTR [rdi+0x1c],0x44bb8000
   140316f40:	c7 47 20 40 40 ff 80 	mov    DWORD PTR [rdi+0x20],0x80ff4040
   140316f47:	c7 47 24 ff ff ff 80 	mov    DWORD PTR [rdi+0x24],0x80ffffff
   140316f4e:	66 c7 47 60 00 ff    	mov    WORD PTR [rdi+0x60],0xff00
   140316f54:	89 87 94 00 00 00    	mov    DWORD PTR [rdi+0x94],eax
   140316f5a:	66 c7 47 74 00 40    	mov    WORD PTR [rdi+0x74],0x4000
   140316f60:	66 c7 87 84 00 00 00 	mov    WORD PTR [rdi+0x84],0xff00
   140316f67:	00 ff 
   140316f69:	e9 12 04 00 00       	jmp    0x140317380
   140316f6e:	c6 47 02 50          	mov    BYTE PTR [rdi+0x2],0x50
   140316f72:	b8 10 01 00 00       	mov    eax,0x110
   140316f77:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140316f7b:	b8 02 00 00 00       	mov    eax,0x2
   140316f80:	89 47 6c             	mov    DWORD PTR [rdi+0x6c],eax
   140316f83:	89 6f 18             	mov    DWORD PTR [rdi+0x18],ebp
   140316f86:	c7 47 1c 00 00 00 80 	mov    DWORD PTR [rdi+0x1c],0x80000000
   140316f8d:	c7 47 20 ff ff ff 80 	mov    DWORD PTR [rdi+0x20],0x80ffffff
   140316f94:	66 c7 47 5c 00 ff    	mov    WORD PTR [rdi+0x5c],0xff00
   140316f9a:	89 87 90 00 00 00    	mov    DWORD PTR [rdi+0x90],eax
   140316fa0:	66 c7 47 70 40 40    	mov    WORD PTR [rdi+0x70],0x4040
   140316fa6:	66 c7 87 80 00 00 00 	mov    WORD PTR [rdi+0x80],0xff00
   140316fad:	00 ff 
   140316faf:	ba 01 00 00 00       	mov    edx,0x1
   140316fb4:	48 8d 8f 94 00 00 00 	lea    rcx,[rdi+0x94]
   140316fbb:	44 8d 42 7f          	lea    r8d,[rdx+0x7f]
   140316fbf:	e8 26 fc 02 00       	call   0x140346bea
   140316fc4:	e9 b7 03 00 00       	jmp    0x140317380
   140316fc9:	c7 47 30 40 20 f0 80 	mov    DWORD PTR [rdi+0x30],0x80f02040
   140316fd0:	48 8d 57 20          	lea    rdx,[rdi+0x20]
   140316fd4:	89 6f 34             	mov    DWORD PTR [rdi+0x34],ebp
   140316fd7:	b8 78 00 00 00       	mov    eax,0x78
   140316fdc:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140316fe0:	4c 8b c2             	mov    r8,rdx
   140316fe3:	48 8b ce             	mov    rcx,rsi
   140316fe6:	89 2a                	mov    DWORD PTR [rdx],ebp
   140316fe8:	89 6f 24             	mov    DWORD PTR [rdi+0x24],ebp
   140316feb:	c7 47 28 00 00 fa 43 	mov    DWORD PTR [rdi+0x28],0x43fa0000
   140316ff2:	c7 47 2c 00 00 80 3f 	mov    DWORD PTR [rdi+0x2c],0x3f800000
   140316ff9:	e8 e2 fa ff ff       	call   0x140316ae0
   140316ffe:	c7 47 38 00 00 c8 42 	mov    DWORD PTR [rdi+0x38],0x42c80000
   140317005:	b8 02 00 00 00       	mov    eax,0x2
   14031700a:	89 47 64             	mov    DWORD PTR [rdi+0x64],eax
   14031700d:	b9 10 00 00 00       	mov    ecx,0x10
   140317012:	66 c7 47 44 00 80    	mov    WORD PTR [rdi+0x44],0x8000
   140317018:	66 c7 47 54 00 ff    	mov    WORD PTR [rdi+0x54],0xff00
   14031701e:	89 4f 6c             	mov    DWORD PTR [rdi+0x6c],ecx
   140317021:	c7 47 70 30 00 00 00 	mov    DWORD PTR [rdi+0x70],0x30
   140317028:	c7 47 74 00 00 00 3f 	mov    DWORD PTR [rdi+0x74],0x3f000000
   14031702f:	c7 47 78 00 00 00 3f 	mov    DWORD PTR [rdi+0x78],0x3f000000
   140317036:	c7 47 7c cd cc cc 3d 	mov    DWORD PTR [rdi+0x7c],0x3dcccccd
   14031703d:	c7 87 80 00 00 00 cd 	mov    DWORD PTR [rdi+0x80],0x3e4ccccd
   140317044:	cc 4c 3e 
   140317047:	48 c7 87 84 00 00 00 	mov    QWORD PTR [rdi+0x84],0x3f800000
   14031704e:	00 00 80 3f 
   140317052:	89 af 98 00 00 00    	mov    DWORD PTR [rdi+0x98],ebp
   140317058:	e9 23 03 00 00       	jmp    0x140317380
   14031705d:	b8 c8 00 00 00       	mov    eax,0xc8
   140317062:	b9 10 00 00 00       	mov    ecx,0x10
   140317067:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   14031706b:	33 d2                	xor    edx,edx
   14031706d:	b8 02 00 00 00       	mov    eax,0x2
   140317072:	89 47 3c             	mov    DWORD PTR [rdi+0x3c],eax
   140317075:	66 c7 47 1c 00 40    	mov    WORD PTR [rdi+0x1c],0x4000
   14031707b:	66 c7 47 2c 00 ff    	mov    WORD PTR [rdi+0x2c],0xff00
   140317081:	89 4f 48             	mov    DWORD PTR [rdi+0x48],ecx
   140317084:	44 8d 40 7e          	lea    r8d,[rax+0x7e]
   140317088:	48 8d 4f 5c          	lea    rcx,[rdi+0x5c]
   14031708c:	c7 47 18 80 80 80 80 	mov    DWORD PTR [rdi+0x18],0x80808080
   140317093:	48 c7 47 40 04 00 00 	mov    QWORD PTR [rdi+0x40],0x4
   14031709a:	00 
   14031709b:	48 c7 47 4c 05 00 00 	mov    QWORD PTR [rdi+0x4c],0x5
   1403170a2:	00 
   1403170a3:	c7 47 54 66 66 66 3f 	mov    DWORD PTR [rdi+0x54],0x3f666666
   1403170aa:	c6 47 58 ff          	mov    BYTE PTR [rdi+0x58],0xff
   1403170ae:	e8 37 fb 02 00       	call   0x140346bea
   1403170b3:	c6 87 db 00 00 00 80 	mov    BYTE PTR [rdi+0xdb],0x80
   1403170ba:	e9 c1 02 00 00       	jmp    0x140317380
   1403170bf:	b8 b0 00 00 00       	mov    eax,0xb0
   1403170c4:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   1403170c8:	b8 02 00 00 00       	mov    eax,0x2
   1403170cd:	89 47 3c             	mov    DWORD PTR [rdi+0x3c],eax
   1403170d0:	66 c7 47 1c 00 40    	mov    WORD PTR [rdi+0x1c],0x4000
   1403170d6:	66 c7 47 2c 00 ff    	mov    WORD PTR [rdi+0x2c],0xff00
   1403170dc:	c7 47 18 40 40 40 80 	mov    DWORD PTR [rdi+0x18],0x80404040
   1403170e3:	48 c7 47 40 03 00 00 	mov    QWORD PTR [rdi+0x40],0x3
   1403170ea:	00 
   1403170eb:	48 8d 4f 48          	lea    rcx,[rdi+0x48]
   1403170ef:	33 d2                	xor    edx,edx
   1403170f1:	41 b8 80 00 00 00    	mov    r8d,0x80
   1403170f7:	e8 ee fa 02 00       	call   0x140346bea
   1403170fc:	c6 87 c7 00 00 00 80 	mov    BYTE PTR [rdi+0xc7],0x80
   140317103:	e9 78 02 00 00       	jmp    0x140317380
   140317108:	b8 b0 00 00 00       	mov    eax,0xb0
   14031710d:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140317111:	b8 02 00 00 00       	mov    eax,0x2
   140317116:	89 47 3c             	mov    DWORD PTR [rdi+0x3c],eax
   140317119:	66 c7 47 1c 00 80    	mov    WORD PTR [rdi+0x1c],0x8000
   14031711f:	66 c7 47 2c 00 ff    	mov    WORD PTR [rdi+0x2c],0xff00
   140317125:	c7 47 18 80 80 80 80 	mov    DWORD PTR [rdi+0x18],0x80808080
   14031712c:	48 c7 47 40 06 00 00 	mov    QWORD PTR [rdi+0x40],0x6
   140317133:	00 
   140317134:	eb b5                	jmp    0x1403170eb
   140317136:	b8 e8 00 00 00       	mov    eax,0xe8
   14031713b:	c6 47 02 1e          	mov    BYTE PTR [rdi+0x2],0x1e
   14031713f:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140317143:	48 8d 4f 64          	lea    rcx,[rdi+0x64]
   140317147:	b8 02 00 00 00       	mov    eax,0x2
   14031714c:	c7 47 20 a0 a0 a0 00 	mov    DWORD PTR [rdi+0x20],0xa0a0a0
   140317153:	c7 47 24 80 80 80 80 	mov    DWORD PTR [rdi+0x24],0x80808080
   14031715a:	33 d2                	xor    edx,edx
   14031715c:	c7 47 28 80 80 80 60 	mov    DWORD PTR [rdi+0x28],0x60808080
   140317163:	c7 47 2c 08 00 00 00 	mov    DWORD PTR [rdi+0x2c],0x8
   14031716a:	c7 47 34 64 00 00 00 	mov    DWORD PTR [rdi+0x34],0x64
   140317171:	44 8d 40 7e          	lea    r8d,[rax+0x7e]
   140317175:	c7 47 38 64 00 00 00 	mov    DWORD PTR [rdi+0x38],0x64
   14031717c:	66 c7 47 3c 80 20    	mov    WORD PTR [rdi+0x3c],0x2080
   140317182:	c7 47 30 01 00 00 00 	mov    DWORD PTR [rdi+0x30],0x1
   140317189:	c7 47 18 5a 00 00 00 	mov    DWORD PTR [rdi+0x18],0x5a
   140317190:	c7 47 1c 2d 00 00 00 	mov    DWORD PTR [rdi+0x1c],0x2d
   140317197:	89 47 60             	mov    DWORD PTR [rdi+0x60],eax
   14031719a:	66 c7 47 40 00 80    	mov    WORD PTR [rdi+0x40],0x8000
   1403171a0:	66 c7 47 50 00 ff    	mov    WORD PTR [rdi+0x50],0xff00
   1403171a6:	e8 3f fa 02 00       	call   0x140346bea
   1403171ab:	c6 87 e3 00 00 00 80 	mov    BYTE PTR [rdi+0xe3],0x80
   1403171b2:	e9 c9 01 00 00       	jmp    0x140317380
   1403171b7:	b9 10 00 00 00       	mov    ecx,0x10
   1403171bc:	c7 47 04 02 00 50 00 	mov    DWORD PTR [rdi+0x4],0x500002
   1403171c3:	89 4f 1c             	mov    DWORD PTR [rdi+0x1c],ecx
   1403171c6:	b8 02 00 00 00       	mov    eax,0x2
   1403171cb:	89 4f 20             	mov    DWORD PTR [rdi+0x20],ecx
   1403171ce:	89 47 54             	mov    DWORD PTR [rdi+0x54],eax
   1403171d1:	c6 47 02 3c          	mov    BYTE PTR [rdi+0x2],0x3c
   1403171d5:	c7 47 2c 00 00 7a 44 	mov    DWORD PTR [rdi+0x2c],0x447a0000
   1403171dc:	c7 47 30 00 00 96 44 	mov    DWORD PTR [rdi+0x30],0x44960000
   1403171e3:	c7 47 28 80 80 80 40 	mov    DWORD PTR [rdi+0x28],0x40808080
   1403171ea:	c7 47 18 20 00 00 00 	mov    DWORD PTR [rdi+0x18],0x20
   1403171f1:	c7 47 24 08 00 00 00 	mov    DWORD PTR [rdi+0x24],0x8
   1403171f8:	66 c7 47 34 00 80    	mov    WORD PTR [rdi+0x34],0x8000
   1403171fe:	66 c7 47 44 00 ff    	mov    WORD PTR [rdi+0x44],0xff00
   140317204:	c7 47 58 04 00 00 00 	mov    DWORD PTR [rdi+0x58],0x4
   14031720b:	48 c7 47 5c 03 00 00 	mov    QWORD PTR [rdi+0x5c],0x3
   140317212:	00 
   140317213:	e9 68 01 00 00       	jmp    0x140317380
   140317218:	c6 47 02 ff          	mov    BYTE PTR [rdi+0x2],0xff
   14031721c:	b8 78 00 00 00       	mov    eax,0x78
   140317221:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140317225:	b8 02 00 00 00       	mov    eax,0x2
   14031722a:	89 87 84 00 00 00    	mov    DWORD PTR [rdi+0x84],eax
   140317230:	c7 47 28 80 80 80 10 	mov    DWORD PTR [rdi+0x28],0x10808080
   140317237:	c7 47 18 01 00 00 00 	mov    DWORD PTR [rdi+0x18],0x1
   14031723e:	89 6f 24             	mov    DWORD PTR [rdi+0x24],ebp
   140317241:	c7 47 2c 64 00 00 00 	mov    DWORD PTR [rdi+0x2c],0x64
   140317248:	c7 47 30 64 00 00 00 	mov    DWORD PTR [rdi+0x30],0x64
   14031724f:	c7 47 34 00 00 00 80 	mov    DWORD PTR [rdi+0x34],0x80000000
   140317256:	c7 47 38 ff ff ff 80 	mov    DWORD PTR [rdi+0x38],0x80ffffff
   14031725d:	66 c7 47 74 00 ff    	mov    WORD PTR [rdi+0x74],0xff00
   140317263:	c7 87 88 00 00 00 04 	mov    DWORD PTR [rdi+0x88],0x4
   14031726a:	00 00 00 
   14031726d:	e9 0e 01 00 00       	jmp    0x140317380
   140317272:	c6 47 02 3d          	mov    BYTE PTR [rdi+0x2],0x3d
   140317276:	b8 20 01 00 00       	mov    eax,0x120
   14031727b:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   14031727f:	b8 02 00 00 00       	mov    eax,0x2
   140317284:	89 47 6c             	mov    DWORD PTR [rdi+0x6c],eax
   140317287:	89 6f 18             	mov    DWORD PTR [rdi+0x18],ebp
   14031728a:	c7 47 1c 00 00 00 80 	mov    DWORD PTR [rdi+0x1c],0x80000000
   140317291:	c7 47 20 ff ff ff 80 	mov    DWORD PTR [rdi+0x20],0x80ffffff
   140317298:	66 c7 47 5c 00 ff    	mov    WORD PTR [rdi+0x5c],0xff00
   14031729e:	89 87 90 00 00 00    	mov    DWORD PTR [rdi+0x90],eax
   1403172a4:	66 c7 47 70 00 80    	mov    WORD PTR [rdi+0x70],0x8000
   1403172aa:	66 c7 87 80 00 00 00 	mov    WORD PTR [rdi+0x80],0xff00
   1403172b1:	00 ff 
   1403172b3:	c7 87 14 01 00 00 04 	mov    DWORD PTR [rdi+0x114],0x4
   1403172ba:	00 00 00 
   1403172bd:	48 c7 87 20 01 00 00 	mov    QWORD PTR [rdi+0x120],0x3f800000
   1403172c4:	00 00 80 3f 
   1403172c8:	48 c7 87 28 01 00 00 	mov    QWORD PTR [rdi+0x128],0x3f800000
   1403172cf:	00 00 80 3f 
   1403172d3:	c7 87 18 01 00 00 00 	mov    DWORD PTR [rdi+0x118],0x3e800000
   1403172da:	00 80 3e 
   1403172dd:	c7 87 1c 01 00 00 00 	mov    DWORD PTR [rdi+0x11c],0x3f000000
   1403172e4:	00 00 3f 
   1403172e7:	48 89 af 40 01 00 00 	mov    QWORD PTR [rdi+0x140],rbp
   1403172ee:	89 af 48 01 00 00    	mov    DWORD PTR [rdi+0x148],ebp
   1403172f4:	c7 87 4c 01 00 00 00 	mov    DWORD PTR [rdi+0x14c],0x3f000000
   1403172fb:	00 00 3f 
   1403172fe:	48 89 af 60 01 00 00 	mov    QWORD PTR [rdi+0x160],rbp
   140317305:	89 af 68 01 00 00    	mov    DWORD PTR [rdi+0x168],ebp
   14031730b:	89 af 30 01 00 00    	mov    DWORD PTR [rdi+0x130],ebp
   140317311:	c7 87 34 01 00 00 cd 	mov    DWORD PTR [rdi+0x134],0x3e4ccccd
   140317318:	cc 4c 3e 
   14031731b:	e9 8f fc ff ff       	jmp    0x140316faf
   140317320:	b8 90 00 00 00       	mov    eax,0x90
   140317325:	c7 47 20 00 00 00 80 	mov    DWORD PTR [rdi+0x20],0x80000000
   14031732c:	48 8d 4f 28          	lea    rcx,[rdi+0x28]
   140317330:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140317334:	33 d2                	xor    edx,edx
   140317336:	c7 47 18 01 00 00 00 	mov    DWORD PTR [rdi+0x18],0x1
   14031733d:	c7 47 1c 01 00 00 00 	mov    DWORD PTR [rdi+0x1c],0x1
   140317344:	44 8d 40 f0          	lea    r8d,[rax-0x10]
   140317348:	89 6f 24             	mov    DWORD PTR [rdi+0x24],ebp
   14031734b:	e8 9a f8 02 00       	call   0x140346bea
   140317350:	c6 87 a7 00 00 00 80 	mov    BYTE PTR [rdi+0xa7],0x80
   140317357:	eb 27                	jmp    0x140317380
   140317359:	c7 47 20 00 00 00 a0 	mov    DWORD PTR [rdi+0x20],0xa0000000
   140317360:	b8 38 00 00 00       	mov    eax,0x38
   140317365:	66 89 47 06          	mov    WORD PTR [rdi+0x6],ax
   140317369:	48 c7 47 48 02 00 00 	mov    QWORD PTR [rdi+0x48],0x2
   140317370:	00 
   140317371:	66 c7 47 28 00 ff    	mov    WORD PTR [rdi+0x28],0xff00
   140317377:	66 c7 47 38 00 ff    	mov    WORD PTR [rdi+0x38],0xff00
   14031737d:	89 6f 24             	mov    DWORD PTR [rdi+0x24],ebp
   140317380:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   140317385:	48 8b 6c 24 38       	mov    rbp,QWORD PTR [rsp+0x38]
   14031738a:	48 8b 74 24 40       	mov    rsi,QWORD PTR [rsp+0x40]
   14031738f:	48 83 c4 20          	add    rsp,0x20
   140317393:	5f                   	pop    rdi
   140317394:	c3                   	ret
   140317395:	0f 1f 00             	nop    DWORD PTR [rax]
