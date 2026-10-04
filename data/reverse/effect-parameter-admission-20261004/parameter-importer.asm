
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

000000014031f050 <.text+0x31e050>:
   14031f050:	48 89 5c 24 08       	mov    QWORD PTR [rsp+0x8],rbx
   14031f055:	57                   	push   rdi
   14031f056:	48 83 ec 20          	sub    rsp,0x20
   14031f05a:	8d 42 ff             	lea    eax,[rdx-0x1]
   14031f05d:	41 0f b6 f9          	movzx  edi,r9b
   14031f061:	49 8b d8             	mov    rbx,r8
   14031f064:	83 f8 0e             	cmp    eax,0xe
   14031f067:	0f 87 fd 06 00 00    	ja     0x14031f76a
   14031f06d:	4c 8d 05 8c 0f ce ff 	lea    r8,[rip+0xffffffffffce0f8c]        # 0x140000000
   14031f074:	48 98                	cdqe
   14031f076:	45 8b 94 80 78 f7 31 	mov    r10d,DWORD PTR [r8+rax*4+0x31f778]
   14031f07d:	00 
   14031f07e:	4d 03 d0             	add    r10,r8
   14031f081:	41 ff e2             	jmp    r10
   14031f084:	e8 d7 fb ff ff       	call   0x14031ec60
   14031f089:	48 8b c8             	mov    rcx,rax
   14031f08c:	40 84 ff             	test   dil,dil
   14031f08f:	75 13                	jne    0x14031f0a4
   14031f091:	48 85 c0             	test   rax,rax
   14031f094:	75 0e                	jne    0x14031f0a4
   14031f096:	83 c8 ff             	or     eax,0xffffffff
   14031f099:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f09e:	48 83 c4 20          	add    rsp,0x20
   14031f0a2:	5f                   	pop    rdi
   14031f0a3:	c3                   	ret
   14031f0a4:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f0a6:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   14031f0a9:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f0ac:	89 41 1c             	mov    DWORD PTR [rcx+0x1c],eax
   14031f0af:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f0b2:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f0b5:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f0b8:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f0bb:	85 c0                	test   eax,eax
   14031f0bd:	0f 85 a0 06 00 00    	jne    0x14031f763
   14031f0c3:	c7 41 20 01 00 00 00 	mov    DWORD PTR [rcx+0x20],0x1
   14031f0ca:	0f b6 43 10          	movzx  eax,BYTE PTR [rbx+0x10]
   14031f0ce:	c7 43 0c 01 00 00 00 	mov    DWORD PTR [rbx+0xc],0x1
   14031f0d5:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f0d8:	33 c0                	xor    eax,eax
   14031f0da:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f0df:	48 83 c4 20          	add    rsp,0x20
   14031f0e3:	5f                   	pop    rdi
   14031f0e4:	c3                   	ret
   14031f0e5:	b2 0b                	mov    dl,0xb
   14031f0e7:	e8 74 fb ff ff       	call   0x14031ec60
   14031f0ec:	48 8b c8             	mov    rcx,rax
   14031f0ef:	40 84 ff             	test   dil,dil
   14031f0f2:	75 05                	jne    0x14031f0f9
   14031f0f4:	48 85 c0             	test   rax,rax
   14031f0f7:	74 9d                	je     0x14031f096
   14031f0f9:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f0fb:	89 41 2c             	mov    DWORD PTR [rcx+0x2c],eax
   14031f0fe:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f101:	89 41 30             	mov    DWORD PTR [rcx+0x30],eax
   14031f104:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f107:	89 41 28             	mov    DWORD PTR [rcx+0x28],eax
   14031f10a:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f10d:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   14031f110:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f113:	89 41 1c             	mov    DWORD PTR [rcx+0x1c],eax
   14031f116:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f119:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f11c:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f11f:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f122:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f125:	89 41 58             	mov    DWORD PTR [rcx+0x58],eax
   14031f128:	8b 43 20             	mov    eax,DWORD PTR [rbx+0x20]
   14031f12b:	89 41 5c             	mov    DWORD PTR [rcx+0x5c],eax
   14031f12e:	8b 43 24             	mov    eax,DWORD PTR [rbx+0x24]
   14031f131:	89 41 60             	mov    DWORD PTR [rcx+0x60],eax
   14031f134:	0f b6 43 28          	movzx  eax,BYTE PTR [rbx+0x28]
   14031f138:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f13b:	33 c0                	xor    eax,eax
   14031f13d:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f142:	48 83 c4 20          	add    rsp,0x20
   14031f146:	5f                   	pop    rdi
   14031f147:	c3                   	ret
   14031f148:	b2 08                	mov    dl,0x8
   14031f14a:	e8 11 fb ff ff       	call   0x14031ec60
   14031f14f:	4c 8b c8             	mov    r9,rax
   14031f152:	40 84 ff             	test   dil,dil
   14031f155:	75 09                	jne    0x14031f160
   14031f157:	48 85 c0             	test   rax,rax
   14031f15a:	0f 84 36 ff ff ff    	je     0x14031f096
   14031f160:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f162:	49 8d 51 48          	lea    rdx,[r9+0x48]
   14031f166:	41 89 41 18          	mov    DWORD PTR [r9+0x18],eax
   14031f16a:	4c 8b c3             	mov    r8,rbx
   14031f16d:	0f b6 43 04          	movzx  eax,BYTE PTR [rbx+0x4]
   14031f171:	4d 2b c1             	sub    r8,r9
   14031f174:	41 88 41 1d          	mov    BYTE PTR [r9+0x1d],al
   14031f178:	b9 40 00 00 00       	mov    ecx,0x40
   14031f17d:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f180:	41 89 41 40          	mov    DWORD PTR [r9+0x40],eax
   14031f184:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f187:	41 89 41 44          	mov    DWORD PTR [r9+0x44],eax
   14031f18b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]
   14031f190:	41 0f b6 44 10 c8    	movzx  eax,BYTE PTR [r8+rdx*1-0x38]
   14031f196:	88 02                	mov    BYTE PTR [rdx],al
   14031f198:	41 0f b6 44 10 c9    	movzx  eax,BYTE PTR [r8+rdx*1-0x37]
   14031f19e:	88 42 01             	mov    BYTE PTR [rdx+0x1],al
   14031f1a1:	48 8d 52 02          	lea    rdx,[rdx+0x2]
   14031f1a5:	48 83 e9 01          	sub    rcx,0x1
   14031f1a9:	75 e5                	jne    0x14031f190
   14031f1ab:	0f b6 83 90 00 00 00 	movzx  eax,BYTE PTR [rbx+0x90]
   14031f1b2:	41 88 41 01          	mov    BYTE PTR [r9+0x1],al
   14031f1b6:	33 c0                	xor    eax,eax
   14031f1b8:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f1bd:	48 83 c4 20          	add    rsp,0x20
   14031f1c1:	5f                   	pop    rdi
   14031f1c2:	c3                   	ret
   14031f1c3:	b2 09                	mov    dl,0x9
   14031f1c5:	e8 96 fa ff ff       	call   0x14031ec60
   14031f1ca:	4c 8b c8             	mov    r9,rax
   14031f1cd:	40 84 ff             	test   dil,dil
   14031f1d0:	75 09                	jne    0x14031f1db
   14031f1d2:	48 85 c0             	test   rax,rax
   14031f1d5:	0f 84 bb fe ff ff    	je     0x14031f096
   14031f1db:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f1dd:	49 8d 51 48          	lea    rdx,[r9+0x48]
   14031f1e1:	41 89 41 18          	mov    DWORD PTR [r9+0x18],eax
   14031f1e5:	4c 8b c3             	mov    r8,rbx
   14031f1e8:	0f b6 43 04          	movzx  eax,BYTE PTR [rbx+0x4]
   14031f1ec:	4d 2b c1             	sub    r8,r9
   14031f1ef:	41 88 41 1d          	mov    BYTE PTR [r9+0x1d],al
   14031f1f3:	b9 40 00 00 00       	mov    ecx,0x40
   14031f1f8:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f1fb:	41 89 41 40          	mov    DWORD PTR [r9+0x40],eax
   14031f1ff:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f202:	41 89 41 44          	mov    DWORD PTR [r9+0x44],eax
   14031f206:	66 66 0f 1f 84 00 00 	data16 nop WORD PTR [rax+rax*1+0x0]
   14031f20d:	00 00 00 
   14031f210:	41 0f b6 44 10 c8    	movzx  eax,BYTE PTR [r8+rdx*1-0x38]
   14031f216:	88 02                	mov    BYTE PTR [rdx],al
   14031f218:	41 0f b6 44 10 c9    	movzx  eax,BYTE PTR [r8+rdx*1-0x37]
   14031f21e:	88 42 01             	mov    BYTE PTR [rdx+0x1],al
   14031f221:	48 8d 52 02          	lea    rdx,[rdx+0x2]
   14031f225:	48 83 e9 01          	sub    rcx,0x1
   14031f229:	75 e5                	jne    0x14031f210
   14031f22b:	0f b6 83 90 00 00 00 	movzx  eax,BYTE PTR [rbx+0x90]
   14031f232:	41 88 41 01          	mov    BYTE PTR [r9+0x1],al
   14031f236:	33 c0                	xor    eax,eax
   14031f238:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f23d:	48 83 c4 20          	add    rsp,0x20
   14031f241:	5f                   	pop    rdi
   14031f242:	c3                   	ret
   14031f243:	b2 05                	mov    dl,0x5
   14031f245:	e8 16 fa ff ff       	call   0x14031ec60
   14031f24a:	4c 8b c8             	mov    r9,rax
   14031f24d:	40 84 ff             	test   dil,dil
   14031f250:	75 09                	jne    0x14031f25b
   14031f252:	48 85 c0             	test   rax,rax
   14031f255:	0f 84 3b fe ff ff    	je     0x14031f096
   14031f25b:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f25d:	49 8d 91 94 00 00 00 	lea    rdx,[r9+0x94]
   14031f264:	4c 8b c3             	mov    r8,rbx
   14031f267:	41 89 41 18          	mov    DWORD PTR [r9+0x18],eax
   14031f26b:	4d 2b c1             	sub    r8,r9
   14031f26e:	b9 40 00 00 00       	mov    ecx,0x40
   14031f273:	41 0f b6 84 10 70 ff 	movzx  eax,BYTE PTR [r8+rdx*1-0x90]
   14031f27a:	ff ff 
   14031f27c:	88 02                	mov    BYTE PTR [rdx],al
   14031f27e:	41 0f b6 84 10 71 ff 	movzx  eax,BYTE PTR [r8+rdx*1-0x8f]
   14031f285:	ff ff 
   14031f287:	88 42 01             	mov    BYTE PTR [rdx+0x1],al
   14031f28a:	48 8d 52 02          	lea    rdx,[rdx+0x2]
   14031f28e:	48 83 e9 01          	sub    rcx,0x1
   14031f292:	75 df                	jne    0x14031f273
   14031f294:	8b 83 84 00 00 00    	mov    eax,DWORD PTR [rbx+0x84]
   14031f29a:	41 89 41 1c          	mov    DWORD PTR [r9+0x1c],eax
   14031f29e:	8b 83 88 00 00 00    	mov    eax,DWORD PTR [rbx+0x88]
   14031f2a4:	41 89 41 20          	mov    DWORD PTR [r9+0x20],eax
   14031f2a8:	0f b6 83 8c 00 00 00 	movzx  eax,BYTE PTR [rbx+0x8c]
   14031f2af:	41 88 41 70          	mov    BYTE PTR [r9+0x70],al
   14031f2b3:	0f b6 83 8d 00 00 00 	movzx  eax,BYTE PTR [rbx+0x8d]
   14031f2ba:	41 88 41 71          	mov    BYTE PTR [r9+0x71],al
   14031f2be:	0f b6 83 8e 00 00 00 	movzx  eax,BYTE PTR [rbx+0x8e]
   14031f2c5:	41 88 41 01          	mov    BYTE PTR [r9+0x1],al
   14031f2c9:	33 c0                	xor    eax,eax
   14031f2cb:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f2d0:	48 83 c4 20          	add    rsp,0x20
   14031f2d4:	5f                   	pop    rdi
   14031f2d5:	c3                   	ret
   14031f2d6:	b2 07                	mov    dl,0x7
   14031f2d8:	e8 83 f9 ff ff       	call   0x14031ec60
   14031f2dd:	4c 8b c8             	mov    r9,rax
   14031f2e0:	40 84 ff             	test   dil,dil
   14031f2e3:	75 09                	jne    0x14031f2ee
   14031f2e5:	48 85 c0             	test   rax,rax
   14031f2e8:	0f 84 a8 fd ff ff    	je     0x14031f096
   14031f2ee:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f2f0:	49 8d 51 5c          	lea    rdx,[r9+0x5c]
   14031f2f4:	41 89 41 18          	mov    DWORD PTR [r9+0x18],eax
   14031f2f8:	4c 8b c3             	mov    r8,rbx
   14031f2fb:	0f b6 43 04          	movzx  eax,BYTE PTR [rbx+0x4]
   14031f2ff:	4d 2b c1             	sub    r8,r9
   14031f302:	41 88 41 1d          	mov    BYTE PTR [r9+0x1d],al
   14031f306:	b9 40 00 00 00       	mov    ecx,0x40
   14031f30b:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f30e:	41 89 41 48          	mov    DWORD PTR [r9+0x48],eax
   14031f312:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f315:	41 89 41 40          	mov    DWORD PTR [r9+0x40],eax
   14031f319:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f31c:	41 89 41 44          	mov    DWORD PTR [r9+0x44],eax
   14031f320:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f323:	41 89 41 4c          	mov    DWORD PTR [r9+0x4c],eax
   14031f327:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f32a:	41 89 41 54          	mov    DWORD PTR [r9+0x54],eax
   14031f32e:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f331:	41 89 41 50          	mov    DWORD PTR [r9+0x50],eax
   14031f335:	66 66 66 0f 1f 84 00 	data16 data16 nop WORD PTR [rax+rax*1+0x0]
   14031f33c:	00 00 00 00 
   14031f340:	41 0f b6 44 10 c4    	movzx  eax,BYTE PTR [r8+rdx*1-0x3c]
   14031f346:	88 02                	mov    BYTE PTR [rdx],al
   14031f348:	41 0f b6 44 10 c5    	movzx  eax,BYTE PTR [r8+rdx*1-0x3b]
   14031f34e:	88 42 01             	mov    BYTE PTR [rdx+0x1],al
   14031f351:	48 8d 52 02          	lea    rdx,[rdx+0x2]
   14031f355:	48 83 e9 01          	sub    rcx,0x1
   14031f359:	75 e5                	jne    0x14031f340
   14031f35b:	0f b6 83 a0 00 00 00 	movzx  eax,BYTE PTR [rbx+0xa0]
   14031f362:	41 88 41 01          	mov    BYTE PTR [r9+0x1],al
   14031f366:	33 c0                	xor    eax,eax
   14031f368:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f36d:	48 83 c4 20          	add    rsp,0x20
   14031f371:	5f                   	pop    rdi
   14031f372:	c3                   	ret
   14031f373:	b2 0c                	mov    dl,0xc
   14031f375:	e8 e6 f8 ff ff       	call   0x14031ec60
   14031f37a:	48 8b c8             	mov    rcx,rax
   14031f37d:	40 84 ff             	test   dil,dil
   14031f380:	75 09                	jne    0x14031f38b
   14031f382:	48 85 c0             	test   rax,rax
   14031f385:	0f 84 0b fd ff ff    	je     0x14031f096
   14031f38b:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f38d:	89 41 2c             	mov    DWORD PTR [rcx+0x2c],eax
   14031f390:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f393:	89 41 30             	mov    DWORD PTR [rcx+0x30],eax
   14031f396:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f399:	89 41 28             	mov    DWORD PTR [rcx+0x28],eax
   14031f39c:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f39f:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   14031f3a2:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f3a5:	89 81 88 00 00 00    	mov    DWORD PTR [rcx+0x88],eax
   14031f3ab:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f3ae:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f3b1:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f3b4:	89 41 34             	mov    DWORD PTR [rcx+0x34],eax
   14031f3b7:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f3ba:	89 41 38             	mov    DWORD PTR [rcx+0x38],eax
   14031f3bd:	0f b6 43 20          	movzx  eax,BYTE PTR [rbx+0x20]
   14031f3c1:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f3c4:	33 c0                	xor    eax,eax
   14031f3c6:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f3cb:	48 83 c4 20          	add    rsp,0x20
   14031f3cf:	5f                   	pop    rdi
   14031f3d0:	c3                   	ret
   14031f3d1:	b2 01                	mov    dl,0x1
   14031f3d3:	e8 88 f8 ff ff       	call   0x14031ec60
   14031f3d8:	48 8b c8             	mov    rcx,rax
   14031f3db:	40 84 ff             	test   dil,dil
   14031f3de:	75 09                	jne    0x14031f3e9
   14031f3e0:	48 85 c0             	test   rax,rax
   14031f3e3:	0f 84 ad fc ff ff    	je     0x14031f096
   14031f3e9:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f3eb:	89 41 1c             	mov    DWORD PTR [rcx+0x1c],eax
   14031f3ee:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f3f1:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f3f4:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f3f7:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f3fa:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f3fd:	89 41 28             	mov    DWORD PTR [rcx+0x28],eax
   14031f400:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f403:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   14031f406:	0f b6 43 14          	movzx  eax,BYTE PTR [rbx+0x14]
   14031f40a:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f40d:	33 c0                	xor    eax,eax
   14031f40f:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f414:	48 83 c4 20          	add    rsp,0x20
   14031f418:	5f                   	pop    rdi
   14031f419:	c3                   	ret
   14031f41a:	b2 04                	mov    dl,0x4
   14031f41c:	e8 3f f8 ff ff       	call   0x14031ec60
   14031f421:	48 8b c8             	mov    rcx,rax
   14031f424:	40 84 ff             	test   dil,dil
   14031f427:	75 09                	jne    0x14031f432
   14031f429:	48 85 c0             	test   rax,rax
   14031f42c:	0f 84 64 fc ff ff    	je     0x14031f096
   14031f432:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f434:	89 81 98 00 00 00    	mov    DWORD PTR [rcx+0x98],eax
   14031f43a:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f43d:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   14031f440:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f443:	89 41 1c             	mov    DWORD PTR [rcx+0x1c],eax
   14031f446:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f449:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f44c:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f44f:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f452:	0f b6 43 14          	movzx  eax,BYTE PTR [rbx+0x14]
   14031f456:	88 41 74             	mov    BYTE PTR [rcx+0x74],al
   14031f459:	0f b6 43 15          	movzx  eax,BYTE PTR [rbx+0x15]
   14031f45d:	88 41 75             	mov    BYTE PTR [rcx+0x75],al
   14031f460:	0f b6 43 16          	movzx  eax,BYTE PTR [rbx+0x16]
   14031f464:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f467:	33 c0                	xor    eax,eax
   14031f469:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f46e:	48 83 c4 20          	add    rsp,0x20
   14031f472:	5f                   	pop    rdi
   14031f473:	c3                   	ret
   14031f474:	b2 0a                	mov    dl,0xa
   14031f476:	e8 e5 f7 ff ff       	call   0x14031ec60
   14031f47b:	4c 8b c8             	mov    r9,rax
   14031f47e:	40 84 ff             	test   dil,dil
   14031f481:	75 09                	jne    0x14031f48c
   14031f483:	48 85 c0             	test   rax,rax
   14031f486:	0f 84 0a fc ff ff    	je     0x14031f096
   14031f48c:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f48e:	49 8d 51 64          	lea    rdx,[r9+0x64]
   14031f492:	41 89 41 34          	mov    DWORD PTR [r9+0x34],eax
   14031f496:	4c 8b c3             	mov    r8,rbx
   14031f499:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f49c:	4d 2b c1             	sub    r8,r9
   14031f49f:	41 89 41 38          	mov    DWORD PTR [r9+0x38],eax
   14031f4a3:	b9 40 00 00 00       	mov    ecx,0x40
   14031f4a8:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f4ab:	41 89 41 28          	mov    DWORD PTR [r9+0x28],eax
   14031f4af:	0f b6 43 0c          	movzx  eax,BYTE PTR [rbx+0xc]
   14031f4b3:	41 88 41 41          	mov    BYTE PTR [r9+0x41],al
   14031f4b7:	0f b6 43 0d          	movzx  eax,BYTE PTR [rbx+0xd]
   14031f4bb:	41 88 41 3c          	mov    BYTE PTR [r9+0x3c],al
   14031f4bf:	0f b6 43 0e          	movzx  eax,BYTE PTR [rbx+0xe]
   14031f4c3:	41 88 41 3d          	mov    BYTE PTR [r9+0x3d],al
   14031f4c7:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f4ca:	41 89 41 2c          	mov    DWORD PTR [r9+0x2c],eax
   14031f4ce:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f4d1:	41 89 41 30          	mov    DWORD PTR [r9+0x30],eax
   14031f4d5:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f4d8:	41 89 41 18          	mov    DWORD PTR [r9+0x18],eax
   14031f4dc:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f4df:	41 89 41 1c          	mov    DWORD PTR [r9+0x1c],eax
   14031f4e3:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   14031f4e7:	66 0f 1f 84 00 00 00 	nop    WORD PTR [rax+rax*1+0x0]
   14031f4ee:	00 00 
   14031f4f0:	41 0f b6 44 10 bc    	movzx  eax,BYTE PTR [r8+rdx*1-0x44]
   14031f4f6:	88 02                	mov    BYTE PTR [rdx],al
   14031f4f8:	41 0f b6 44 10 bd    	movzx  eax,BYTE PTR [r8+rdx*1-0x43]
   14031f4fe:	88 42 01             	mov    BYTE PTR [rdx+0x1],al
   14031f501:	48 8d 52 02          	lea    rdx,[rdx+0x2]
   14031f505:	48 83 e9 01          	sub    rcx,0x1
   14031f509:	75 e5                	jne    0x14031f4f0
   14031f50b:	0f b6 83 a0 00 00 00 	movzx  eax,BYTE PTR [rbx+0xa0]
   14031f512:	41 88 41 01          	mov    BYTE PTR [r9+0x1],al
   14031f516:	33 c0                	xor    eax,eax
   14031f518:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f51d:	48 83 c4 20          	add    rsp,0x20
   14031f521:	5f                   	pop    rdi
   14031f522:	c3                   	ret
   14031f523:	b2 03                	mov    dl,0x3
   14031f525:	e8 36 f7 ff ff       	call   0x14031ec60
   14031f52a:	48 8b c8             	mov    rcx,rax
   14031f52d:	40 84 ff             	test   dil,dil
   14031f530:	75 09                	jne    0x14031f53b
   14031f532:	48 85 c0             	test   rax,rax
   14031f535:	0f 84 5b fb ff ff    	je     0x14031f096
   14031f53b:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f53d:	89 41 3c             	mov    DWORD PTR [rcx+0x3c],eax
   14031f540:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f543:	89 41 1c             	mov    DWORD PTR [rcx+0x1c],eax
   14031f546:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f549:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f54c:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f54f:	89 41 34             	mov    DWORD PTR [rcx+0x34],eax
   14031f552:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f555:	89 41 38             	mov    DWORD PTR [rcx+0x38],eax
   14031f558:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f55b:	89 41 30             	mov    DWORD PTR [rcx+0x30],eax
   14031f55e:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f561:	89 41 2c             	mov    DWORD PTR [rcx+0x2c],eax
   14031f564:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f567:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f56a:	83 7b 20 01          	cmp    DWORD PTR [rbx+0x20],0x1
   14031f56e:	76 07                	jbe    0x14031f577
   14031f570:	c7 43 20 00 00 00 00 	mov    DWORD PTR [rbx+0x20],0x0
   14031f577:	8b 43 20             	mov    eax,DWORD PTR [rbx+0x20]
   14031f57a:	89 41 44             	mov    DWORD PTR [rcx+0x44],eax
   14031f57d:	0f b6 43 24          	movzx  eax,BYTE PTR [rbx+0x24]
   14031f581:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f584:	33 c0                	xor    eax,eax
   14031f586:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f58b:	48 83 c4 20          	add    rsp,0x20
   14031f58f:	5f                   	pop    rdi
   14031f590:	c3                   	ret
   14031f591:	b2 06                	mov    dl,0x6
   14031f593:	e8 c8 f6 ff ff       	call   0x14031ec60
   14031f598:	48 8b c8             	mov    rcx,rax
   14031f59b:	40 84 ff             	test   dil,dil
   14031f59e:	75 09                	jne    0x14031f5a9
   14031f5a0:	48 85 c0             	test   rax,rax
   14031f5a3:	0f 84 ed fa ff ff    	je     0x14031f096
   14031f5a9:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f5ab:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f5ae:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f5b1:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f5b4:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f5b7:	89 41 28             	mov    DWORD PTR [rcx+0x28],eax
   14031f5ba:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f5bd:	89 81 88 00 00 00    	mov    DWORD PTR [rcx+0x88],eax
   14031f5c3:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f5c6:	89 41 38             	mov    DWORD PTR [rcx+0x38],eax
   14031f5c9:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f5cc:	89 41 6c             	mov    DWORD PTR [rcx+0x6c],eax
   14031f5cf:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f5d2:	89 41 7c             	mov    DWORD PTR [rcx+0x7c],eax
   14031f5d5:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f5d8:	89 81 80 00 00 00    	mov    DWORD PTR [rcx+0x80],eax
   14031f5de:	8b 43 20             	mov    eax,DWORD PTR [rbx+0x20]
   14031f5e1:	89 41 74             	mov    DWORD PTR [rcx+0x74],eax
   14031f5e4:	8b 43 24             	mov    eax,DWORD PTR [rbx+0x24]
   14031f5e7:	89 41 78             	mov    DWORD PTR [rcx+0x78],eax
   14031f5ea:	8b 43 28             	mov    eax,DWORD PTR [rbx+0x28]
   14031f5ed:	89 41 70             	mov    DWORD PTR [rcx+0x70],eax
   14031f5f0:	0f b6 43 2c          	movzx  eax,BYTE PTR [rbx+0x2c]
   14031f5f4:	88 41 44             	mov    BYTE PTR [rcx+0x44],al
   14031f5f7:	0f b6 43 2d          	movzx  eax,BYTE PTR [rbx+0x2d]
   14031f5fb:	88 41 45             	mov    BYTE PTR [rcx+0x45],al
   14031f5fe:	8b 43 30             	mov    eax,DWORD PTR [rbx+0x30]
   14031f601:	89 41 30             	mov    DWORD PTR [rcx+0x30],eax
   14031f604:	8b 43 34             	mov    eax,DWORD PTR [rbx+0x34]
   14031f607:	89 41 34             	mov    DWORD PTR [rcx+0x34],eax
   14031f60a:	0f b6 43 38          	movzx  eax,BYTE PTR [rbx+0x38]
   14031f60e:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f611:	33 c0                	xor    eax,eax
   14031f613:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f618:	48 83 c4 20          	add    rsp,0x20
   14031f61c:	5f                   	pop    rdi
   14031f61d:	c3                   	ret
   14031f61e:	b2 0d                	mov    dl,0xd
   14031f620:	e8 3b f6 ff ff       	call   0x14031ec60
   14031f625:	48 8b c8             	mov    rcx,rax
   14031f628:	40 84 ff             	test   dil,dil
   14031f62b:	75 09                	jne    0x14031f636
   14031f62d:	48 85 c0             	test   rax,rax
   14031f630:	0f 84 60 fa ff ff    	je     0x14031f096
   14031f636:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f638:	89 41 18             	mov    DWORD PTR [rcx+0x18],eax
   14031f63b:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f63e:	89 81 14 01 00 00    	mov    DWORD PTR [rcx+0x114],eax
   14031f644:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f647:	89 81 18 01 00 00    	mov    DWORD PTR [rcx+0x118],eax
   14031f64d:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f650:	89 81 1c 01 00 00    	mov    DWORD PTR [rcx+0x11c],eax
   14031f656:	8b 43 10             	mov    eax,DWORD PTR [rbx+0x10]
   14031f659:	89 81 20 01 00 00    	mov    DWORD PTR [rcx+0x120],eax
   14031f65f:	8b 43 14             	mov    eax,DWORD PTR [rbx+0x14]
   14031f662:	89 81 28 01 00 00    	mov    DWORD PTR [rcx+0x128],eax
   14031f668:	8b 43 18             	mov    eax,DWORD PTR [rbx+0x18]
   14031f66b:	89 81 30 01 00 00    	mov    DWORD PTR [rcx+0x130],eax
   14031f671:	8b 43 1c             	mov    eax,DWORD PTR [rbx+0x1c]
   14031f674:	89 81 34 01 00 00    	mov    DWORD PTR [rcx+0x134],eax
   14031f67a:	8b 43 20             	mov    eax,DWORD PTR [rbx+0x20]
   14031f67d:	89 81 2c 01 00 00    	mov    DWORD PTR [rcx+0x12c],eax
   14031f683:	8b 43 24             	mov    eax,DWORD PTR [rbx+0x24]
   14031f686:	89 81 24 01 00 00    	mov    DWORD PTR [rcx+0x124],eax
   14031f68c:	8b 43 28             	mov    eax,DWORD PTR [rbx+0x28]
   14031f68f:	89 41 1c             	mov    DWORD PTR [rcx+0x1c],eax
   14031f692:	8b 43 2c             	mov    eax,DWORD PTR [rbx+0x2c]
   14031f695:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f698:	0f b6 43 30          	movzx  eax,BYTE PTR [rbx+0x30]
   14031f69c:	88 41 70             	mov    BYTE PTR [rcx+0x70],al
   14031f69f:	0f b6 43 31          	movzx  eax,BYTE PTR [rbx+0x31]
   14031f6a3:	88 41 71             	mov    BYTE PTR [rcx+0x71],al
   14031f6a6:	0f b6 43 32          	movzx  eax,BYTE PTR [rbx+0x32]
   14031f6aa:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f6ad:	33 c0                	xor    eax,eax
   14031f6af:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f6b4:	48 83 c4 20          	add    rsp,0x20
   14031f6b8:	5f                   	pop    rdi
   14031f6b9:	c3                   	ret
   14031f6ba:	b2 0e                	mov    dl,0xe
   14031f6bc:	e8 9f f5 ff ff       	call   0x14031ec60
   14031f6c1:	4c 8b c8             	mov    r9,rax
   14031f6c4:	40 84 ff             	test   dil,dil
   14031f6c7:	75 09                	jne    0x14031f6d2
   14031f6c9:	48 85 c0             	test   rax,rax
   14031f6cc:	0f 84 c4 f9 ff ff    	je     0x14031f096
   14031f6d2:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f6d4:	49 8d 51 28          	lea    rdx,[r9+0x28]
   14031f6d8:	41 89 41 24          	mov    DWORD PTR [r9+0x24],eax
   14031f6dc:	4c 8b c3             	mov    r8,rbx
   14031f6df:	8b 43 04             	mov    eax,DWORD PTR [rbx+0x4]
   14031f6e2:	4d 2b c1             	sub    r8,r9
   14031f6e5:	41 89 41 20          	mov    DWORD PTR [r9+0x20],eax
   14031f6e9:	b9 40 00 00 00       	mov    ecx,0x40
   14031f6ee:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f6f1:	41 89 41 18          	mov    DWORD PTR [r9+0x18],eax
   14031f6f5:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f6f8:	41 89 41 1c          	mov    DWORD PTR [r9+0x1c],eax
   14031f6fc:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
   14031f700:	41 0f b6 44 10 e8    	movzx  eax,BYTE PTR [r8+rdx*1-0x18]
   14031f706:	88 02                	mov    BYTE PTR [rdx],al
   14031f708:	41 0f b6 44 10 e9    	movzx  eax,BYTE PTR [r8+rdx*1-0x17]
   14031f70e:	88 42 01             	mov    BYTE PTR [rdx+0x1],al
   14031f711:	48 8d 52 02          	lea    rdx,[rdx+0x2]
   14031f715:	48 83 e9 01          	sub    rcx,0x1
   14031f719:	75 e5                	jne    0x14031f700
   14031f71b:	0f b6 83 90 00 00 00 	movzx  eax,BYTE PTR [rbx+0x90]
   14031f722:	41 88 41 01          	mov    BYTE PTR [r9+0x1],al
   14031f726:	33 c0                	xor    eax,eax
   14031f728:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f72d:	48 83 c4 20          	add    rsp,0x20
   14031f731:	5f                   	pop    rdi
   14031f732:	c3                   	ret
   14031f733:	b2 0f                	mov    dl,0xf
   14031f735:	e8 26 f5 ff ff       	call   0x14031ec60
   14031f73a:	48 8b c8             	mov    rcx,rax
   14031f73d:	40 84 ff             	test   dil,dil
   14031f740:	75 09                	jne    0x14031f74b
   14031f742:	48 85 c0             	test   rax,rax
   14031f745:	0f 84 4b f9 ff ff    	je     0x14031f096
   14031f74b:	8b 03                	mov    eax,DWORD PTR [rbx]
   14031f74d:	89 41 4c             	mov    DWORD PTR [rcx+0x4c],eax
   14031f750:	0f b6 43 04          	movzx  eax,BYTE PTR [rbx+0x4]
   14031f754:	88 41 29             	mov    BYTE PTR [rcx+0x29],al
   14031f757:	8b 43 08             	mov    eax,DWORD PTR [rbx+0x8]
   14031f75a:	89 41 24             	mov    DWORD PTR [rcx+0x24],eax
   14031f75d:	8b 43 0c             	mov    eax,DWORD PTR [rbx+0xc]
   14031f760:	89 41 20             	mov    DWORD PTR [rcx+0x20],eax
   14031f763:	0f b6 43 10          	movzx  eax,BYTE PTR [rbx+0x10]
   14031f767:	88 41 01             	mov    BYTE PTR [rcx+0x1],al
   14031f76a:	33 c0                	xor    eax,eax
   14031f76c:	48 8b 5c 24 30       	mov    rbx,QWORD PTR [rsp+0x30]
   14031f771:	48 83 c4 20          	add    rsp,0x20
   14031f775:	5f                   	pop    rdi
   14031f776:	c3                   	ret
