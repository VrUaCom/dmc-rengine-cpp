; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031e190, 0x14031e53c); code slice, not necessarily one unwind entry
0x14031e190: 4883ec08                 sub      rsp, 8
0x14031e194: ffca                     dec      edx
0x14031e196: 83fa0e                   cmp      edx, 0xe
0x14031e199: 0f8795030000             ja       0x14031e534
0x14031e19f: 4863c2                   movsxd   rax, edx
0x14031e1a2: 48893c24                 mov      qword ptr [rsp], rdi
0x14031e1a6: 488d3d531eceff           lea      rdi, [rip - 0x31e1ad]
0x14031e1ad: 8b94873ce53100           mov      edx, dword ptr [rdi + rax*4 + 0x31e53c]
0x14031e1b4: 4803d7                   add      rdx, rdi
0x14031e1b7: ffe2                     jmp      rdx
0x14031e1b9: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e1bd: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e1c1: c70000007a44             mov      dword ptr [rax], 0x447a0000
0x14031e1c7: c7400400808944           mov      dword ptr [rax + 4], 0x44898000
0x14031e1ce: c7400880808080           mov      dword ptr [rax + 8], 0x80808080
0x14031e1d5: c7400c03000000           mov      dword ptr [rax + 0xc], 3
0x14031e1dc: c6401000                 mov      byte ptr [rax + 0x10], 0
0x14031e1e0: 4883c408                 add      rsp, 8
0x14031e1e4: c3                       ret      
0x14031e1e5: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e1e9: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e1ed: c70000007a44             mov      dword ptr [rax], 0x447a0000
0x14031e1f3: c7400400808944           mov      dword ptr [rax + 4], 0x44898000
0x14031e1fa: c7400880808040           mov      dword ptr [rax + 8], 0x40808080
0x14031e201: c7400c10000000           mov      dword ptr [rax + 0xc], 0x10
0x14031e208: c7401010000000           mov      dword ptr [rax + 0x10], 0x10
0x14031e20f: c7401408000000           mov      dword ptr [rax + 0x14], 8
0x14031e216: c7401808000000           mov      dword ptr [rax + 0x18], 8
0x14031e21d: c7401c04000000           mov      dword ptr [rax + 0x1c], 4
0x14031e224: 48c7402003000000         mov      qword ptr [rax + 0x20], 3
0x14031e22c: c6402800                 mov      byte ptr [rax + 0x28], 0
0x14031e230: 4883c408                 add      rsp, 8
0x14031e234: c3                       ret      
0x14031e235: 488b5158                 mov      rdx, qword ptr [rcx + 0x58]
0x14031e239: c70240404080             mov      dword ptr [rdx], 0x80404040
0x14031e23f: c6420440                 mov      byte ptr [rdx + 4], 0x40
0x14031e243: 48c7420803000000         mov      qword ptr [rdx + 8], 3
0x14031e24b: e9a7020000               jmp      0x14031e4f7
0x14031e250: 488b5158                 mov      rdx, qword ptr [rcx + 0x58]
0x14031e254: c70280808080             mov      dword ptr [rdx], 0x80808080
0x14031e25a: c6420480                 mov      byte ptr [rdx + 4], 0x80
0x14031e25e: 48c7420806000000         mov      qword ptr [rdx + 8], 6
0x14031e266: e98c020000               jmp      0x14031e4f7
0x14031e26b: 488b5158                 mov      rdx, qword ptr [rcx + 0x58]
0x14031e26f: 4533c0                   xor      r8d, r8d
0x14031e272: 33c0                     xor      eax, eax
0x14031e274: b980000000               mov      ecx, 0x80
0x14031e279: 448902                   mov      dword ptr [rdx], r8d
0x14031e27c: 488d7a04                 lea      rdi, [rdx + 4]
0x14031e280: f3aa                     rep stosb byte ptr [rdi], al
0x14031e282: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e286: 44898284000000           mov      dword ptr [rdx + 0x84], r8d
0x14031e28d: c78288000000ffffff80     mov      dword ptr [rdx + 0x88], 0x80ffffff
0x14031e297: 66c7828c0000004040       mov      word ptr [rdx + 0x8c], 0x4040
0x14031e2a0: 88828e000000             mov      byte ptr [rdx + 0x8e], al
0x14031e2a6: 4883c408                 add      rsp, 8
0x14031e2aa: c3                       ret      
0x14031e2ab: 488b5158                 mov      rdx, qword ptr [rcx + 0x58]
0x14031e2af: c70280808080             mov      dword ptr [rdx], 0x80808080
0x14031e2b5: c6420440                 mov      byte ptr [rdx + 4], 0x40
0x14031e2b9: c7420810000000           mov      dword ptr [rdx + 8], 0x10
0x14031e2c0: 48c7420c04000000         mov      qword ptr [rdx + 0xc], 4
0x14031e2c8: c7421405000000           mov      dword ptr [rdx + 0x14], 5
0x14031e2cf: 48c742186666663f         mov      qword ptr [rdx + 0x18], 0x3f666666
0x14031e2d7: e9db000000               jmp      0x14031e3b7
0x14031e2dc: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e2e0: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e2e4: c70064000000             mov      dword ptr [rax], 0x64
0x14031e2ea: c7400464000000           mov      dword ptr [rax + 4], 0x64
0x14031e2f1: c7400880808010           mov      dword ptr [rax + 8], 0x10808080
0x14031e2f8: c7400c01000000           mov      dword ptr [rax + 0xc], 1
0x14031e2ff: 48c7401004000000         mov      qword ptr [rax + 0x10], 4
0x14031e307: c7401800000080           mov      dword ptr [rax + 0x18], 0x80000000
0x14031e30e: c7401cffffff80           mov      dword ptr [rax + 0x1c], 0x80ffffff
0x14031e315: c6402000                 mov      byte ptr [rax + 0x20], 0
0x14031e319: 4883c408                 add      rsp, 8
0x14031e31d: c3                       ret      
0x14031e31e: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e322: 4533c0                   xor      r8d, r8d
0x14031e325: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e329: 4c8900                   mov      qword ptr [rax], r8
0x14031e32c: 4c894008                 mov      qword ptr [rax + 8], r8
0x14031e330: c7401080808040           mov      dword ptr [rax + 0x10], 0x40808080
0x14031e337: 44884014                 mov      byte ptr [rax + 0x14], r8b
0x14031e33b: 4883c408                 add      rsp, 8
0x14031e33f: c3                       ret      
0x14031e340: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e344: 4533c0                   xor      r8d, r8d
0x14031e347: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e34b: 448900                   mov      dword ptr [rax], r8d
0x14031e34e: c740040000fa43           mov      dword ptr [rax + 4], 0x43fa0000
0x14031e355: c7400800007a44           mov      dword ptr [rax + 8], 0x447a0000
0x14031e35c: c7400c00000080           mov      dword ptr [rax + 0xc], 0x80000000
0x14031e363: 66c740140040             mov      word ptr [rax + 0x14], 0x4000
0x14031e369: c740104040ff80           mov      dword ptr [rax + 0x10], 0x80ff4040
0x14031e370: 44884016                 mov      byte ptr [rax + 0x16], r8b
0x14031e374: 4883c408                 add      rsp, 8
0x14031e378: c3                       ret      
0x14031e379: 488b5158                 mov      rdx, qword ptr [rcx + 0x58]
0x14031e37d: c70264000000             mov      dword ptr [rdx], 0x64
0x14031e383: c7420464000000           mov      dword ptr [rdx + 4], 0x64
0x14031e38a: c7420880808060           mov      dword ptr [rdx + 8], 0x60808080
0x14031e391: 66c7420c8080             mov      word ptr [rdx + 0xc], 0x8080
0x14031e397: c6420e20                 mov      byte ptr [rdx + 0xe], 0x20
0x14031e39b: c7421008000000           mov      dword ptr [rdx + 0x10], 8
0x14031e3a2: c7421401000000           mov      dword ptr [rdx + 0x14], 1
0x14031e3a9: c742185a000000           mov      dword ptr [rdx + 0x18], 0x5a
0x14031e3b0: c7421c2d000000           mov      dword ptr [rdx + 0x1c], 0x2d
0x14031e3b7: 33c0                     xor      eax, eax
0x14031e3b9: 488d7a20                 lea      rdi, [rdx + 0x20]
0x14031e3bd: b980000000               mov      ecx, 0x80
0x14031e3c2: f3aa                     rep stosb byte ptr [rdi], al
0x14031e3c4: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e3c8: 8882a0000000             mov      byte ptr [rdx + 0xa0], al
0x14031e3ce: 4883c408                 add      rsp, 8
0x14031e3d2: c3                       ret      
0x14031e3d3: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e3d7: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e3db: c70000007a44             mov      dword ptr [rax], 0x447a0000
0x14031e3e1: c740040000fa43           mov      dword ptr [rax + 4], 0x43fa0000
0x14031e3e8: c7400800007a44           mov      dword ptr [rax + 8], 0x447a0000
0x14031e3ef: c7400c0000fa43           mov      dword ptr [rax + 0xc], 0x43fa0000
0x14031e3f6: c7401000007a44           mov      dword ptr [rax + 0x10], 0x447a0000
0x14031e3fd: c7401480808040           mov      dword ptr [rax + 0x14], 0x40808080
0x14031e404: c7401801000000           mov      dword ptr [rax + 0x18], 1
0x14031e40b: 48c7401c0000003f         mov      qword ptr [rax + 0x1c], 0x3f000000
0x14031e413: c6402400                 mov      byte ptr [rax + 0x24], 0
0x14031e417: 4883c408                 add      rsp, 8
0x14031e41b: c3                       ret      
0x14031e41c: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e420: 4533c0                   xor      r8d, r8d
0x14031e423: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e427: 4c8900                   mov      qword ptr [rax], r8
0x14031e42a: 48c7400800009643         mov      qword ptr [rax + 8], 0x43960000
0x14031e432: c740100000c842           mov      dword ptr [rax + 0x10], 0x42c80000
0x14031e439: c7401410000000           mov      dword ptr [rax + 0x14], 0x10
0x14031e440: c74018cdcccc3d           mov      dword ptr [rax + 0x18], 0x3dcccccd
0x14031e447: c7401ccdcc4c3e           mov      dword ptr [rax + 0x1c], 0x3e4ccccd
0x14031e44e: c740200000003f           mov      dword ptr [rax + 0x20], 0x3f000000
0x14031e455: c740240000003f           mov      dword ptr [rax + 0x24], 0x3f000000
0x14031e45c: c7402830000000           mov      dword ptr [rax + 0x28], 0x30
0x14031e463: 66c7402c0080             mov      word ptr [rax + 0x2c], 0x8000
0x14031e469: c740304020f080           mov      dword ptr [rax + 0x30], 0x80f02040
0x14031e470: 44894034                 mov      dword ptr [rax + 0x34], r8d
0x14031e474: 44884038                 mov      byte ptr [rax + 0x38], r8b
0x14031e478: 4883c408                 add      rsp, 8
0x14031e47c: c3                       ret      
0x14031e47d: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e481: 4533c0                   xor      r8d, r8d
0x14031e484: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e488: 448900                   mov      dword ptr [rax], r8d
0x14031e48b: c7400404000000           mov      dword ptr [rax + 4], 4
0x14031e492: c740080000803e           mov      dword ptr [rax + 8], 0x3e800000
0x14031e499: c7400c0000003f           mov      dword ptr [rax + 0xc], 0x3f000000
0x14031e4a0: c740100000803f           mov      dword ptr [rax + 0x10], 0x3f800000
0x14031e4a7: 48c740140000803f         mov      qword ptr [rax + 0x14], 0x3f800000
0x14031e4af: 48c7401ccdcc4c3e         mov      qword ptr [rax + 0x1c], 0x3e4ccccd
0x14031e4b7: 44894024                 mov      dword ptr [rax + 0x24], r8d
0x14031e4bb: c7402800000080           mov      dword ptr [rax + 0x28], 0x80000000
0x14031e4c2: c7402cffffff80           mov      dword ptr [rax + 0x2c], 0x80ffffff
0x14031e4c9: 66c740300080             mov      word ptr [rax + 0x30], 0x8000
0x14031e4cf: 44884032                 mov      byte ptr [rax + 0x32], r8b
0x14031e4d3: 4883c408                 add      rsp, 8
0x14031e4d7: c3                       ret      
0x14031e4d8: 488b5158                 mov      rdx, qword ptr [rcx + 0x58]
0x14031e4dc: 4533c0                   xor      r8d, r8d
0x14031e4df: 448902                   mov      dword ptr [rdx], r8d
0x14031e4e2: c7420400000080           mov      dword ptr [rdx + 4], 0x80000000
0x14031e4e9: c7420801000000           mov      dword ptr [rdx + 8], 1
0x14031e4f0: c7420c01000000           mov      dword ptr [rdx + 0xc], 1
0x14031e4f7: 33c0                     xor      eax, eax
0x14031e4f9: 488d7a10                 lea      rdi, [rdx + 0x10]
0x14031e4fd: b980000000               mov      ecx, 0x80
0x14031e502: f3aa                     rep stosb byte ptr [rdi], al
0x14031e504: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e508: 888290000000             mov      byte ptr [rdx + 0x90], al
0x14031e50e: 4883c408                 add      rsp, 8
0x14031e512: c3                       ret      
0x14031e513: 488b4158                 mov      rax, qword ptr [rcx + 0x58]
0x14031e517: 4533c0                   xor      r8d, r8d
0x14031e51a: 448900                   mov      dword ptr [rax], r8d
0x14031e51d: c64004ff                 mov      byte ptr [rax + 4], 0xff
0x14031e521: 44894008                 mov      dword ptr [rax + 8], r8d
0x14031e525: c7400c000000a0           mov      dword ptr [rax + 0xc], 0xa0000000
0x14031e52c: 44884010                 mov      byte ptr [rax + 0x10], r8b
0x14031e530: 488b3c24                 mov      rdi, qword ptr [rsp]
0x14031e534: 4883c408                 add      rsp, 8
0x14031e538: c3                       ret      
0x14031e539: 0f1f00                   nop      dword ptr [rax]
