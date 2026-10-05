; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031f050, 0x14031f778); contiguous code slice, may span split unwind fragments
0x14031f050: 48895c2408               mov      qword ptr [rsp + 8], rbx
0x14031f055: 57                       push     rdi
0x14031f056: 4883ec20                 sub      rsp, 0x20
0x14031f05a: 8d42ff                   lea      eax, [rdx - 1]
0x14031f05d: 410fb6f9                 movzx    edi, r9b
0x14031f061: 498bd8                   mov      rbx, r8
0x14031f064: 83f80e                   cmp      eax, 0xe
0x14031f067: 0f87fd060000             ja       0x14031f76a
0x14031f06d: 4c8d058c0fceff           lea      r8, [rip - 0x31f074]
0x14031f074: 4898                     cdqe     
0x14031f076: 458b948078f73100         mov      r10d, dword ptr [r8 + rax*4 + 0x31f778]
0x14031f07e: 4d03d0                   add      r10, r8
0x14031f081: 41ffe2                   jmp      r10
0x14031f084: e8d7fbffff               call     0x14031ec60
0x14031f089: 488bc8                   mov      rcx, rax
0x14031f08c: 4084ff                   test     dil, dil
0x14031f08f: 7513                     jne      0x14031f0a4
0x14031f091: 4885c0                   test     rax, rax
0x14031f094: 750e                     jne      0x14031f0a4
0x14031f096: 83c8ff                   or       eax, 0xffffffff
0x14031f099: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f09e: 4883c420                 add      rsp, 0x20
0x14031f0a2: 5f                       pop      rdi
0x14031f0a3: c3                       ret      
0x14031f0a4: 8b03                     mov      eax, dword ptr [rbx]
0x14031f0a6: 894118                   mov      dword ptr [rcx + 0x18], eax
0x14031f0a9: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f0ac: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x14031f0af: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f0b2: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f0b5: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f0b8: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f0bb: 85c0                     test     eax, eax
0x14031f0bd: 0f85a0060000             jne      0x14031f763
0x14031f0c3: c7412001000000           mov      dword ptr [rcx + 0x20], 1
0x14031f0ca: 0fb64310                 movzx    eax, byte ptr [rbx + 0x10]
0x14031f0ce: c7430c01000000           mov      dword ptr [rbx + 0xc], 1
0x14031f0d5: 884101                   mov      byte ptr [rcx + 1], al
0x14031f0d8: 33c0                     xor      eax, eax
0x14031f0da: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f0df: 4883c420                 add      rsp, 0x20
0x14031f0e3: 5f                       pop      rdi
0x14031f0e4: c3                       ret      
0x14031f0e5: b20b                     mov      dl, 0xb
0x14031f0e7: e874fbffff               call     0x14031ec60
0x14031f0ec: 488bc8                   mov      rcx, rax
0x14031f0ef: 4084ff                   test     dil, dil
0x14031f0f2: 7505                     jne      0x14031f0f9
0x14031f0f4: 4885c0                   test     rax, rax
0x14031f0f7: 749d                     je       0x14031f096
0x14031f0f9: 8b03                     mov      eax, dword ptr [rbx]
0x14031f0fb: 89412c                   mov      dword ptr [rcx + 0x2c], eax
0x14031f0fe: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f101: 894130                   mov      dword ptr [rcx + 0x30], eax
0x14031f104: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f107: 894128                   mov      dword ptr [rcx + 0x28], eax
0x14031f10a: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f10d: 894118                   mov      dword ptr [rcx + 0x18], eax
0x14031f110: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f113: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x14031f116: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f119: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f11c: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f11f: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f122: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f125: 894158                   mov      dword ptr [rcx + 0x58], eax
0x14031f128: 8b4320                   mov      eax, dword ptr [rbx + 0x20]
0x14031f12b: 89415c                   mov      dword ptr [rcx + 0x5c], eax
0x14031f12e: 8b4324                   mov      eax, dword ptr [rbx + 0x24]
0x14031f131: 894160                   mov      dword ptr [rcx + 0x60], eax
0x14031f134: 0fb64328                 movzx    eax, byte ptr [rbx + 0x28]
0x14031f138: 884101                   mov      byte ptr [rcx + 1], al
0x14031f13b: 33c0                     xor      eax, eax
0x14031f13d: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f142: 4883c420                 add      rsp, 0x20
0x14031f146: 5f                       pop      rdi
0x14031f147: c3                       ret      
0x14031f148: b208                     mov      dl, 8
0x14031f14a: e811fbffff               call     0x14031ec60
0x14031f14f: 4c8bc8                   mov      r9, rax
0x14031f152: 4084ff                   test     dil, dil
0x14031f155: 7509                     jne      0x14031f160
0x14031f157: 4885c0                   test     rax, rax
0x14031f15a: 0f8436ffffff             je       0x14031f096
0x14031f160: 8b03                     mov      eax, dword ptr [rbx]
0x14031f162: 498d5148                 lea      rdx, [r9 + 0x48]
0x14031f166: 41894118                 mov      dword ptr [r9 + 0x18], eax
0x14031f16a: 4c8bc3                   mov      r8, rbx
0x14031f16d: 0fb64304                 movzx    eax, byte ptr [rbx + 4]
0x14031f171: 4d2bc1                   sub      r8, r9
0x14031f174: 4188411d                 mov      byte ptr [r9 + 0x1d], al
0x14031f178: b940000000               mov      ecx, 0x40
0x14031f17d: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f180: 41894140                 mov      dword ptr [r9 + 0x40], eax
0x14031f184: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f187: 41894144                 mov      dword ptr [r9 + 0x44], eax
0x14031f18b: 0f1f440000               nop      dword ptr [rax + rax]
0x14031f190: 410fb64410c8             movzx    eax, byte ptr [r8 + rdx - 0x38]
0x14031f196: 8802                     mov      byte ptr [rdx], al
0x14031f198: 410fb64410c9             movzx    eax, byte ptr [r8 + rdx - 0x37]
0x14031f19e: 884201                   mov      byte ptr [rdx + 1], al
0x14031f1a1: 488d5202                 lea      rdx, [rdx + 2]
0x14031f1a5: 4883e901                 sub      rcx, 1
0x14031f1a9: 75e5                     jne      0x14031f190
0x14031f1ab: 0fb68390000000           movzx    eax, byte ptr [rbx + 0x90]
0x14031f1b2: 41884101                 mov      byte ptr [r9 + 1], al
0x14031f1b6: 33c0                     xor      eax, eax
0x14031f1b8: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f1bd: 4883c420                 add      rsp, 0x20
0x14031f1c1: 5f                       pop      rdi
0x14031f1c2: c3                       ret      
0x14031f1c3: b209                     mov      dl, 9
0x14031f1c5: e896faffff               call     0x14031ec60
0x14031f1ca: 4c8bc8                   mov      r9, rax
0x14031f1cd: 4084ff                   test     dil, dil
0x14031f1d0: 7509                     jne      0x14031f1db
0x14031f1d2: 4885c0                   test     rax, rax
0x14031f1d5: 0f84bbfeffff             je       0x14031f096
0x14031f1db: 8b03                     mov      eax, dword ptr [rbx]
0x14031f1dd: 498d5148                 lea      rdx, [r9 + 0x48]
0x14031f1e1: 41894118                 mov      dword ptr [r9 + 0x18], eax
0x14031f1e5: 4c8bc3                   mov      r8, rbx
0x14031f1e8: 0fb64304                 movzx    eax, byte ptr [rbx + 4]
0x14031f1ec: 4d2bc1                   sub      r8, r9
0x14031f1ef: 4188411d                 mov      byte ptr [r9 + 0x1d], al
0x14031f1f3: b940000000               mov      ecx, 0x40
0x14031f1f8: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f1fb: 41894140                 mov      dword ptr [r9 + 0x40], eax
0x14031f1ff: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f202: 41894144                 mov      dword ptr [r9 + 0x44], eax
0x14031f206: 66660f1f840000000000     nop      word ptr [rax + rax]
0x14031f210: 410fb64410c8             movzx    eax, byte ptr [r8 + rdx - 0x38]
0x14031f216: 8802                     mov      byte ptr [rdx], al
0x14031f218: 410fb64410c9             movzx    eax, byte ptr [r8 + rdx - 0x37]
0x14031f21e: 884201                   mov      byte ptr [rdx + 1], al
0x14031f221: 488d5202                 lea      rdx, [rdx + 2]
0x14031f225: 4883e901                 sub      rcx, 1
0x14031f229: 75e5                     jne      0x14031f210
0x14031f22b: 0fb68390000000           movzx    eax, byte ptr [rbx + 0x90]
0x14031f232: 41884101                 mov      byte ptr [r9 + 1], al
0x14031f236: 33c0                     xor      eax, eax
0x14031f238: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f23d: 4883c420                 add      rsp, 0x20
0x14031f241: 5f                       pop      rdi
0x14031f242: c3                       ret      
0x14031f243: b205                     mov      dl, 5
0x14031f245: e816faffff               call     0x14031ec60
0x14031f24a: 4c8bc8                   mov      r9, rax
0x14031f24d: 4084ff                   test     dil, dil
0x14031f250: 7509                     jne      0x14031f25b
0x14031f252: 4885c0                   test     rax, rax
0x14031f255: 0f843bfeffff             je       0x14031f096
0x14031f25b: 8b03                     mov      eax, dword ptr [rbx]
0x14031f25d: 498d9194000000           lea      rdx, [r9 + 0x94]
0x14031f264: 4c8bc3                   mov      r8, rbx
0x14031f267: 41894118                 mov      dword ptr [r9 + 0x18], eax
0x14031f26b: 4d2bc1                   sub      r8, r9
0x14031f26e: b940000000               mov      ecx, 0x40
0x14031f273: 410fb6841070ffffff       movzx    eax, byte ptr [r8 + rdx - 0x90]
0x14031f27c: 8802                     mov      byte ptr [rdx], al
0x14031f27e: 410fb6841071ffffff       movzx    eax, byte ptr [r8 + rdx - 0x8f]
0x14031f287: 884201                   mov      byte ptr [rdx + 1], al
0x14031f28a: 488d5202                 lea      rdx, [rdx + 2]
0x14031f28e: 4883e901                 sub      rcx, 1
0x14031f292: 75df                     jne      0x14031f273
0x14031f294: 8b8384000000             mov      eax, dword ptr [rbx + 0x84]
0x14031f29a: 4189411c                 mov      dword ptr [r9 + 0x1c], eax
0x14031f29e: 8b8388000000             mov      eax, dword ptr [rbx + 0x88]
0x14031f2a4: 41894120                 mov      dword ptr [r9 + 0x20], eax
0x14031f2a8: 0fb6838c000000           movzx    eax, byte ptr [rbx + 0x8c]
0x14031f2af: 41884170                 mov      byte ptr [r9 + 0x70], al
0x14031f2b3: 0fb6838d000000           movzx    eax, byte ptr [rbx + 0x8d]
0x14031f2ba: 41884171                 mov      byte ptr [r9 + 0x71], al
0x14031f2be: 0fb6838e000000           movzx    eax, byte ptr [rbx + 0x8e]
0x14031f2c5: 41884101                 mov      byte ptr [r9 + 1], al
0x14031f2c9: 33c0                     xor      eax, eax
0x14031f2cb: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f2d0: 4883c420                 add      rsp, 0x20
0x14031f2d4: 5f                       pop      rdi
0x14031f2d5: c3                       ret      
0x14031f2d6: b207                     mov      dl, 7
0x14031f2d8: e883f9ffff               call     0x14031ec60
0x14031f2dd: 4c8bc8                   mov      r9, rax
0x14031f2e0: 4084ff                   test     dil, dil
0x14031f2e3: 7509                     jne      0x14031f2ee
0x14031f2e5: 4885c0                   test     rax, rax
0x14031f2e8: 0f84a8fdffff             je       0x14031f096
0x14031f2ee: 8b03                     mov      eax, dword ptr [rbx]
0x14031f2f0: 498d515c                 lea      rdx, [r9 + 0x5c]
0x14031f2f4: 41894118                 mov      dword ptr [r9 + 0x18], eax
0x14031f2f8: 4c8bc3                   mov      r8, rbx
0x14031f2fb: 0fb64304                 movzx    eax, byte ptr [rbx + 4]
0x14031f2ff: 4d2bc1                   sub      r8, r9
0x14031f302: 4188411d                 mov      byte ptr [r9 + 0x1d], al
0x14031f306: b940000000               mov      ecx, 0x40
0x14031f30b: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f30e: 41894148                 mov      dword ptr [r9 + 0x48], eax
0x14031f312: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f315: 41894140                 mov      dword ptr [r9 + 0x40], eax
0x14031f319: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f31c: 41894144                 mov      dword ptr [r9 + 0x44], eax
0x14031f320: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f323: 4189414c                 mov      dword ptr [r9 + 0x4c], eax
0x14031f327: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f32a: 41894154                 mov      dword ptr [r9 + 0x54], eax
0x14031f32e: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f331: 41894150                 mov      dword ptr [r9 + 0x50], eax
0x14031f335: 6666660f1f840000000000   nop      word ptr [rax + rax]
0x14031f340: 410fb64410c4             movzx    eax, byte ptr [r8 + rdx - 0x3c]
0x14031f346: 8802                     mov      byte ptr [rdx], al
0x14031f348: 410fb64410c5             movzx    eax, byte ptr [r8 + rdx - 0x3b]
0x14031f34e: 884201                   mov      byte ptr [rdx + 1], al
0x14031f351: 488d5202                 lea      rdx, [rdx + 2]
0x14031f355: 4883e901                 sub      rcx, 1
0x14031f359: 75e5                     jne      0x14031f340
0x14031f35b: 0fb683a0000000           movzx    eax, byte ptr [rbx + 0xa0]
0x14031f362: 41884101                 mov      byte ptr [r9 + 1], al
0x14031f366: 33c0                     xor      eax, eax
0x14031f368: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f36d: 4883c420                 add      rsp, 0x20
0x14031f371: 5f                       pop      rdi
0x14031f372: c3                       ret      
0x14031f373: b20c                     mov      dl, 0xc
0x14031f375: e8e6f8ffff               call     0x14031ec60
0x14031f37a: 488bc8                   mov      rcx, rax
0x14031f37d: 4084ff                   test     dil, dil
0x14031f380: 7509                     jne      0x14031f38b
0x14031f382: 4885c0                   test     rax, rax
0x14031f385: 0f840bfdffff             je       0x14031f096
0x14031f38b: 8b03                     mov      eax, dword ptr [rbx]
0x14031f38d: 89412c                   mov      dword ptr [rcx + 0x2c], eax
0x14031f390: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f393: 894130                   mov      dword ptr [rcx + 0x30], eax
0x14031f396: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f399: 894128                   mov      dword ptr [rcx + 0x28], eax
0x14031f39c: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f39f: 894118                   mov      dword ptr [rcx + 0x18], eax
0x14031f3a2: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f3a5: 898188000000             mov      dword ptr [rcx + 0x88], eax
0x14031f3ab: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f3ae: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f3b1: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f3b4: 894134                   mov      dword ptr [rcx + 0x34], eax
0x14031f3b7: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f3ba: 894138                   mov      dword ptr [rcx + 0x38], eax
0x14031f3bd: 0fb64320                 movzx    eax, byte ptr [rbx + 0x20]
0x14031f3c1: 884101                   mov      byte ptr [rcx + 1], al
0x14031f3c4: 33c0                     xor      eax, eax
0x14031f3c6: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f3cb: 4883c420                 add      rsp, 0x20
0x14031f3cf: 5f                       pop      rdi
0x14031f3d0: c3                       ret      
0x14031f3d1: b201                     mov      dl, 1
0x14031f3d3: e888f8ffff               call     0x14031ec60
0x14031f3d8: 488bc8                   mov      rcx, rax
0x14031f3db: 4084ff                   test     dil, dil
0x14031f3de: 7509                     jne      0x14031f3e9
0x14031f3e0: 4885c0                   test     rax, rax
0x14031f3e3: 0f84adfcffff             je       0x14031f096
0x14031f3e9: 8b03                     mov      eax, dword ptr [rbx]
0x14031f3eb: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x14031f3ee: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f3f1: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f3f4: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f3f7: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f3fa: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f3fd: 894128                   mov      dword ptr [rcx + 0x28], eax
0x14031f400: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f403: 894118                   mov      dword ptr [rcx + 0x18], eax
0x14031f406: 0fb64314                 movzx    eax, byte ptr [rbx + 0x14]
0x14031f40a: 884101                   mov      byte ptr [rcx + 1], al
0x14031f40d: 33c0                     xor      eax, eax
0x14031f40f: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f414: 4883c420                 add      rsp, 0x20
0x14031f418: 5f                       pop      rdi
0x14031f419: c3                       ret      
0x14031f41a: b204                     mov      dl, 4
0x14031f41c: e83ff8ffff               call     0x14031ec60
0x14031f421: 488bc8                   mov      rcx, rax
0x14031f424: 4084ff                   test     dil, dil
0x14031f427: 7509                     jne      0x14031f432
0x14031f429: 4885c0                   test     rax, rax
0x14031f42c: 0f8464fcffff             je       0x14031f096
0x14031f432: 8b03                     mov      eax, dword ptr [rbx]
0x14031f434: 898198000000             mov      dword ptr [rcx + 0x98], eax
0x14031f43a: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f43d: 894118                   mov      dword ptr [rcx + 0x18], eax
0x14031f440: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f443: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x14031f446: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f449: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f44c: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f44f: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f452: 0fb64314                 movzx    eax, byte ptr [rbx + 0x14]
0x14031f456: 884174                   mov      byte ptr [rcx + 0x74], al
0x14031f459: 0fb64315                 movzx    eax, byte ptr [rbx + 0x15]
0x14031f45d: 884175                   mov      byte ptr [rcx + 0x75], al
0x14031f460: 0fb64316                 movzx    eax, byte ptr [rbx + 0x16]
0x14031f464: 884101                   mov      byte ptr [rcx + 1], al
0x14031f467: 33c0                     xor      eax, eax
0x14031f469: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f46e: 4883c420                 add      rsp, 0x20
0x14031f472: 5f                       pop      rdi
0x14031f473: c3                       ret      
0x14031f474: b20a                     mov      dl, 0xa
0x14031f476: e8e5f7ffff               call     0x14031ec60
0x14031f47b: 4c8bc8                   mov      r9, rax
0x14031f47e: 4084ff                   test     dil, dil
0x14031f481: 7509                     jne      0x14031f48c
0x14031f483: 4885c0                   test     rax, rax
0x14031f486: 0f840afcffff             je       0x14031f096
0x14031f48c: 8b03                     mov      eax, dword ptr [rbx]
0x14031f48e: 498d5164                 lea      rdx, [r9 + 0x64]
0x14031f492: 41894134                 mov      dword ptr [r9 + 0x34], eax
0x14031f496: 4c8bc3                   mov      r8, rbx
0x14031f499: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f49c: 4d2bc1                   sub      r8, r9
0x14031f49f: 41894138                 mov      dword ptr [r9 + 0x38], eax
0x14031f4a3: b940000000               mov      ecx, 0x40
0x14031f4a8: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f4ab: 41894128                 mov      dword ptr [r9 + 0x28], eax
0x14031f4af: 0fb6430c                 movzx    eax, byte ptr [rbx + 0xc]
0x14031f4b3: 41884141                 mov      byte ptr [r9 + 0x41], al
0x14031f4b7: 0fb6430d                 movzx    eax, byte ptr [rbx + 0xd]
0x14031f4bb: 4188413c                 mov      byte ptr [r9 + 0x3c], al
0x14031f4bf: 0fb6430e                 movzx    eax, byte ptr [rbx + 0xe]
0x14031f4c3: 4188413d                 mov      byte ptr [r9 + 0x3d], al
0x14031f4c7: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f4ca: 4189412c                 mov      dword ptr [r9 + 0x2c], eax
0x14031f4ce: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f4d1: 41894130                 mov      dword ptr [r9 + 0x30], eax
0x14031f4d5: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f4d8: 41894118                 mov      dword ptr [r9 + 0x18], eax
0x14031f4dc: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f4df: 4189411c                 mov      dword ptr [r9 + 0x1c], eax
0x14031f4e3: 0f1f4000                 nop      dword ptr [rax]
0x14031f4e7: 660f1f840000000000       nop      word ptr [rax + rax]
0x14031f4f0: 410fb64410bc             movzx    eax, byte ptr [r8 + rdx - 0x44]
0x14031f4f6: 8802                     mov      byte ptr [rdx], al
0x14031f4f8: 410fb64410bd             movzx    eax, byte ptr [r8 + rdx - 0x43]
0x14031f4fe: 884201                   mov      byte ptr [rdx + 1], al
0x14031f501: 488d5202                 lea      rdx, [rdx + 2]
0x14031f505: 4883e901                 sub      rcx, 1
0x14031f509: 75e5                     jne      0x14031f4f0
0x14031f50b: 0fb683a0000000           movzx    eax, byte ptr [rbx + 0xa0]
0x14031f512: 41884101                 mov      byte ptr [r9 + 1], al
0x14031f516: 33c0                     xor      eax, eax
0x14031f518: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f51d: 4883c420                 add      rsp, 0x20
0x14031f521: 5f                       pop      rdi
0x14031f522: c3                       ret      
0x14031f523: b203                     mov      dl, 3
0x14031f525: e836f7ffff               call     0x14031ec60
0x14031f52a: 488bc8                   mov      rcx, rax
0x14031f52d: 4084ff                   test     dil, dil
0x14031f530: 7509                     jne      0x14031f53b
0x14031f532: 4885c0                   test     rax, rax
0x14031f535: 0f845bfbffff             je       0x14031f096
0x14031f53b: 8b03                     mov      eax, dword ptr [rbx]
0x14031f53d: 89413c                   mov      dword ptr [rcx + 0x3c], eax
0x14031f540: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f543: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x14031f546: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f549: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f54c: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f54f: 894134                   mov      dword ptr [rcx + 0x34], eax
0x14031f552: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f555: 894138                   mov      dword ptr [rcx + 0x38], eax
0x14031f558: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f55b: 894130                   mov      dword ptr [rcx + 0x30], eax
0x14031f55e: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f561: 89412c                   mov      dword ptr [rcx + 0x2c], eax
0x14031f564: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f567: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f56a: 837b2001                 cmp      dword ptr [rbx + 0x20], 1
0x14031f56e: 7607                     jbe      0x14031f577
0x14031f570: c7432000000000           mov      dword ptr [rbx + 0x20], 0
0x14031f577: 8b4320                   mov      eax, dword ptr [rbx + 0x20]
0x14031f57a: 894144                   mov      dword ptr [rcx + 0x44], eax
0x14031f57d: 0fb64324                 movzx    eax, byte ptr [rbx + 0x24]
0x14031f581: 884101                   mov      byte ptr [rcx + 1], al
0x14031f584: 33c0                     xor      eax, eax
0x14031f586: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f58b: 4883c420                 add      rsp, 0x20
0x14031f58f: 5f                       pop      rdi
0x14031f590: c3                       ret      
0x14031f591: b206                     mov      dl, 6
0x14031f593: e8c8f6ffff               call     0x14031ec60
0x14031f598: 488bc8                   mov      rcx, rax
0x14031f59b: 4084ff                   test     dil, dil
0x14031f59e: 7509                     jne      0x14031f5a9
0x14031f5a0: 4885c0                   test     rax, rax
0x14031f5a3: 0f84edfaffff             je       0x14031f096
0x14031f5a9: 8b03                     mov      eax, dword ptr [rbx]
0x14031f5ab: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f5ae: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f5b1: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f5b4: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f5b7: 894128                   mov      dword ptr [rcx + 0x28], eax
0x14031f5ba: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f5bd: 898188000000             mov      dword ptr [rcx + 0x88], eax
0x14031f5c3: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f5c6: 894138                   mov      dword ptr [rcx + 0x38], eax
0x14031f5c9: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f5cc: 89416c                   mov      dword ptr [rcx + 0x6c], eax
0x14031f5cf: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f5d2: 89417c                   mov      dword ptr [rcx + 0x7c], eax
0x14031f5d5: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f5d8: 898180000000             mov      dword ptr [rcx + 0x80], eax
0x14031f5de: 8b4320                   mov      eax, dword ptr [rbx + 0x20]
0x14031f5e1: 894174                   mov      dword ptr [rcx + 0x74], eax
0x14031f5e4: 8b4324                   mov      eax, dword ptr [rbx + 0x24]
0x14031f5e7: 894178                   mov      dword ptr [rcx + 0x78], eax
0x14031f5ea: 8b4328                   mov      eax, dword ptr [rbx + 0x28]
0x14031f5ed: 894170                   mov      dword ptr [rcx + 0x70], eax
0x14031f5f0: 0fb6432c                 movzx    eax, byte ptr [rbx + 0x2c]
0x14031f5f4: 884144                   mov      byte ptr [rcx + 0x44], al
0x14031f5f7: 0fb6432d                 movzx    eax, byte ptr [rbx + 0x2d]
0x14031f5fb: 884145                   mov      byte ptr [rcx + 0x45], al
0x14031f5fe: 8b4330                   mov      eax, dword ptr [rbx + 0x30]
0x14031f601: 894130                   mov      dword ptr [rcx + 0x30], eax
0x14031f604: 8b4334                   mov      eax, dword ptr [rbx + 0x34]
0x14031f607: 894134                   mov      dword ptr [rcx + 0x34], eax
0x14031f60a: 0fb64338                 movzx    eax, byte ptr [rbx + 0x38]
0x14031f60e: 884101                   mov      byte ptr [rcx + 1], al
0x14031f611: 33c0                     xor      eax, eax
0x14031f613: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f618: 4883c420                 add      rsp, 0x20
0x14031f61c: 5f                       pop      rdi
0x14031f61d: c3                       ret      
0x14031f61e: b20d                     mov      dl, 0xd
0x14031f620: e83bf6ffff               call     0x14031ec60
0x14031f625: 488bc8                   mov      rcx, rax
0x14031f628: 4084ff                   test     dil, dil
0x14031f62b: 7509                     jne      0x14031f636
0x14031f62d: 4885c0                   test     rax, rax
0x14031f630: 0f8460faffff             je       0x14031f096
0x14031f636: 8b03                     mov      eax, dword ptr [rbx]
0x14031f638: 894118                   mov      dword ptr [rcx + 0x18], eax
0x14031f63b: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f63e: 898114010000             mov      dword ptr [rcx + 0x114], eax
0x14031f644: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f647: 898118010000             mov      dword ptr [rcx + 0x118], eax
0x14031f64d: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f650: 89811c010000             mov      dword ptr [rcx + 0x11c], eax
0x14031f656: 8b4310                   mov      eax, dword ptr [rbx + 0x10]
0x14031f659: 898120010000             mov      dword ptr [rcx + 0x120], eax
0x14031f65f: 8b4314                   mov      eax, dword ptr [rbx + 0x14]
0x14031f662: 898128010000             mov      dword ptr [rcx + 0x128], eax
0x14031f668: 8b4318                   mov      eax, dword ptr [rbx + 0x18]
0x14031f66b: 898130010000             mov      dword ptr [rcx + 0x130], eax
0x14031f671: 8b431c                   mov      eax, dword ptr [rbx + 0x1c]
0x14031f674: 898134010000             mov      dword ptr [rcx + 0x134], eax
0x14031f67a: 8b4320                   mov      eax, dword ptr [rbx + 0x20]
0x14031f67d: 89812c010000             mov      dword ptr [rcx + 0x12c], eax
0x14031f683: 8b4324                   mov      eax, dword ptr [rbx + 0x24]
0x14031f686: 898124010000             mov      dword ptr [rcx + 0x124], eax
0x14031f68c: 8b4328                   mov      eax, dword ptr [rbx + 0x28]
0x14031f68f: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x14031f692: 8b432c                   mov      eax, dword ptr [rbx + 0x2c]
0x14031f695: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f698: 0fb64330                 movzx    eax, byte ptr [rbx + 0x30]
0x14031f69c: 884170                   mov      byte ptr [rcx + 0x70], al
0x14031f69f: 0fb64331                 movzx    eax, byte ptr [rbx + 0x31]
0x14031f6a3: 884171                   mov      byte ptr [rcx + 0x71], al
0x14031f6a6: 0fb64332                 movzx    eax, byte ptr [rbx + 0x32]
0x14031f6aa: 884101                   mov      byte ptr [rcx + 1], al
0x14031f6ad: 33c0                     xor      eax, eax
0x14031f6af: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f6b4: 4883c420                 add      rsp, 0x20
0x14031f6b8: 5f                       pop      rdi
0x14031f6b9: c3                       ret      
0x14031f6ba: b20e                     mov      dl, 0xe
0x14031f6bc: e89ff5ffff               call     0x14031ec60
0x14031f6c1: 4c8bc8                   mov      r9, rax
0x14031f6c4: 4084ff                   test     dil, dil
0x14031f6c7: 7509                     jne      0x14031f6d2
0x14031f6c9: 4885c0                   test     rax, rax
0x14031f6cc: 0f84c4f9ffff             je       0x14031f096
0x14031f6d2: 8b03                     mov      eax, dword ptr [rbx]
0x14031f6d4: 498d5128                 lea      rdx, [r9 + 0x28]
0x14031f6d8: 41894124                 mov      dword ptr [r9 + 0x24], eax
0x14031f6dc: 4c8bc3                   mov      r8, rbx
0x14031f6df: 8b4304                   mov      eax, dword ptr [rbx + 4]
0x14031f6e2: 4d2bc1                   sub      r8, r9
0x14031f6e5: 41894120                 mov      dword ptr [r9 + 0x20], eax
0x14031f6e9: b940000000               mov      ecx, 0x40
0x14031f6ee: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f6f1: 41894118                 mov      dword ptr [r9 + 0x18], eax
0x14031f6f5: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f6f8: 4189411c                 mov      dword ptr [r9 + 0x1c], eax
0x14031f6fc: 0f1f4000                 nop      dword ptr [rax]
0x14031f700: 410fb64410e8             movzx    eax, byte ptr [r8 + rdx - 0x18]
0x14031f706: 8802                     mov      byte ptr [rdx], al
0x14031f708: 410fb64410e9             movzx    eax, byte ptr [r8 + rdx - 0x17]
0x14031f70e: 884201                   mov      byte ptr [rdx + 1], al
0x14031f711: 488d5202                 lea      rdx, [rdx + 2]
0x14031f715: 4883e901                 sub      rcx, 1
0x14031f719: 75e5                     jne      0x14031f700
0x14031f71b: 0fb68390000000           movzx    eax, byte ptr [rbx + 0x90]
0x14031f722: 41884101                 mov      byte ptr [r9 + 1], al
0x14031f726: 33c0                     xor      eax, eax
0x14031f728: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f72d: 4883c420                 add      rsp, 0x20
0x14031f731: 5f                       pop      rdi
0x14031f732: c3                       ret      
0x14031f733: b20f                     mov      dl, 0xf
0x14031f735: e826f5ffff               call     0x14031ec60
0x14031f73a: 488bc8                   mov      rcx, rax
0x14031f73d: 4084ff                   test     dil, dil
0x14031f740: 7509                     jne      0x14031f74b
0x14031f742: 4885c0                   test     rax, rax
0x14031f745: 0f844bf9ffff             je       0x14031f096
0x14031f74b: 8b03                     mov      eax, dword ptr [rbx]
0x14031f74d: 89414c                   mov      dword ptr [rcx + 0x4c], eax
0x14031f750: 0fb64304                 movzx    eax, byte ptr [rbx + 4]
0x14031f754: 884129                   mov      byte ptr [rcx + 0x29], al
0x14031f757: 8b4308                   mov      eax, dword ptr [rbx + 8]
0x14031f75a: 894124                   mov      dword ptr [rcx + 0x24], eax
0x14031f75d: 8b430c                   mov      eax, dword ptr [rbx + 0xc]
0x14031f760: 894120                   mov      dword ptr [rcx + 0x20], eax
0x14031f763: 0fb64310                 movzx    eax, byte ptr [rbx + 0x10]
0x14031f767: 884101                   mov      byte ptr [rcx + 1], al
0x14031f76a: 33c0                     xor      eax, eax
0x14031f76c: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031f771: 4883c420                 add      rsp, 0x20
0x14031f775: 5f                       pop      rdi
0x14031f776: c3                       ret      
0x14031f777: 90                       nop      
