; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14023b98a, 0x14023b9c8); code slice, not necessarily one unwind entry
0x14023b98a: 498b5618                 mov      rdx, qword ptr [r14 + 0x18]
0x14023b98e: 488b4218                 mov      rax, qword ptr [rdx + 0x18]
0x14023b992: 663930                   cmp      word ptr [rax], si
0x14023b995: 7521                     jne      0x14023b9b8
0x14023b997: 488b4220                 mov      rax, qword ptr [rdx + 0x20]
0x14023b99b: 83780403                 cmp      dword ptr [rax + 4], 3
0x14023b99f: 7305                     jae      0x14023b9a6
0x14023b9a1: 488bd6                   mov      rdx, rsi
0x14023b9a4: eb16                     jmp      0x14023b9bc
0x14023b9a6: 8b4810                   mov      ecx, dword ptr [rax + 0x10]
0x14023b9a9: 85c9                     test     ecx, ecx
0x14023b9ab: 7505                     jne      0x14023b9b2
0x14023b9ad: 488bd6                   mov      rdx, rsi
0x14023b9b0: eb0a                     jmp      0x14023b9bc
0x14023b9b2: 488d1408                 lea      rdx, [rax + rcx]
0x14023b9b6: eb04                     jmp      0x14023b9bc
0x14023b9b8: 488b5220                 mov      rdx, qword ptr [rdx + 0x20]
0x14023b9bc: 488d8fa0b00100           lea      rcx, [rdi + 0x1b0a0]
0x14023b9c3: e8e8330e00               call     0x14031edb0
