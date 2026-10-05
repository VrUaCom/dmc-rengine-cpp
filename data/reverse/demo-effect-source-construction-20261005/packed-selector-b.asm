; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14023cb8e, 0x14023cbda); code slice, not necessarily one unwind entry
0x14023cb8e: 498b5518                 mov      rdx, qword ptr [r13 + 0x18]
0x14023cb92: 488b4218                 mov      rax, qword ptr [rdx + 0x18]
0x14023cb96: 663930                   cmp      word ptr [rax], si
0x14023cb99: 7521                     jne      0x14023cbbc
0x14023cb9b: 488b4220                 mov      rax, qword ptr [rdx + 0x20]
0x14023cb9f: 83780403                 cmp      dword ptr [rax + 4], 3
0x14023cba3: 7305                     jae      0x14023cbaa
0x14023cba5: 488bd6                   mov      rdx, rsi
0x14023cba8: eb16                     jmp      0x14023cbc0
0x14023cbaa: 8b4810                   mov      ecx, dword ptr [rax + 0x10]
0x14023cbad: 85c9                     test     ecx, ecx
0x14023cbaf: 7505                     jne      0x14023cbb6
0x14023cbb1: 488bd6                   mov      rdx, rsi
0x14023cbb4: eb0a                     jmp      0x14023cbc0
0x14023cbb6: 488d1408                 lea      rdx, [rax + rcx]
0x14023cbba: eb04                     jmp      0x14023cbc0
0x14023cbbc: 488b5220                 mov      rdx, qword ptr [rdx + 0x20]
0x14023cbc0: 488d8fa0b00100           lea      rcx, [rdi + 0x1b0a0]
0x14023cbc7: e8e4210e00               call     0x14031edb0
0x14023cbcc: 33d2                     xor      edx, edx
0x14023cbce: 488d8fa0b00100           lea      rcx, [rdi + 0x1b0a0]
0x14023cbd5: e826220e00               call     0x14031ee00
