
canonical/dmc3.exe:     file format pei-x86-64


Disassembly of section .text:

00000001403162e0 <.text+0x3152e0>:
   1403162e0:	48 89 5c 24 10       	mov    QWORD PTR [rsp+0x10],rbx
   1403162e5:	56                   	push   rsi
   1403162e6:	44 0f b7 5a 06       	movzx  r11d,WORD PTR [rdx+0x6]
   1403162eb:	48 8d 72 18          	lea    rsi,[rdx+0x18]
   1403162ef:	45 33 c9             	xor    r9d,r9d
   1403162f2:	41 c1 eb 02          	shr    r11d,0x2
   1403162f6:	45 8b d1             	mov    r10d,r9d
   1403162f9:	41 8b d9             	mov    ebx,r9d
   1403162fc:	41 8b c1             	mov    eax,r9d
   1403162ff:	41 83 fb 02          	cmp    r11d,0x2
   140316303:	72 38                	jb     0x14031633d
   140316305:	48 89 7c 24 10       	mov    QWORD PTR [rsp+0x10],rdi
   14031630a:	4c 8b c6             	mov    r8,rsi
   14031630d:	41 8d 7b ff          	lea    edi,[r11-0x1]
   140316311:	41 8b 10             	mov    edx,DWORD PTR [r8]
   140316314:	4d 8d 40 08          	lea    r8,[r8+0x8]
   140316318:	8b c8                	mov    ecx,eax
   14031631a:	83 e1 1f             	and    ecx,0x1f
   14031631d:	d3 ca                	ror    edx,cl
   14031631f:	8d 48 01             	lea    ecx,[rax+0x1]
   140316322:	44 03 ca             	add    r9d,edx
   140316325:	83 e1 1f             	and    ecx,0x1f
   140316328:	41 8b 50 fc          	mov    edx,DWORD PTR [r8-0x4]
   14031632c:	83 c0 02             	add    eax,0x2
   14031632f:	d3 ca                	ror    edx,cl
   140316331:	44 03 d2             	add    r10d,edx
   140316334:	3b c7                	cmp    eax,edi
   140316336:	72 d9                	jb     0x140316311
   140316338:	48 8b 7c 24 10       	mov    rdi,QWORD PTR [rsp+0x10]
   14031633d:	41 3b c3             	cmp    eax,r11d
   140316340:	73 0d                	jae    0x14031634f
   140316342:	8b d0                	mov    edx,eax
   140316344:	83 e0 1f             	and    eax,0x1f
   140316347:	0f b6 c8             	movzx  ecx,al
   14031634a:	8b 1c 96             	mov    ebx,DWORD PTR [rsi+rdx*4]
   14031634d:	d3 cb                	ror    ebx,cl
   14031634f:	43 8d 04 0a          	lea    eax,[r10+r9*1]
   140316353:	03 c3                	add    eax,ebx
   140316355:	48 8b 5c 24 18       	mov    rbx,QWORD PTR [rsp+0x18]
   14031635a:	5e                   	pop    rsi
   14031635b:	c3                   	ret
