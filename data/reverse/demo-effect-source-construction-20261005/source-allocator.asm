; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031e0b0, 0x14031e154); code slice, not necessarily one unwind entry
0x14031e0b0: 48895c2408               mov      qword ptr [rsp + 8], rbx
0x14031e0b5: 4889742410               mov      qword ptr [rsp + 0x10], rsi
0x14031e0ba: 57                       push     rdi
0x14031e0bb: 4883ec20                 sub      rsp, 0x20
0x14031e0bf: 488bf1                   mov      rsi, rcx
0x14031e0c2: 8d5aff                   lea      ebx, [rdx - 1]
0x14031e0c5: 4883c138                 add      rcx, 0x38
0x14031e0c9: e842960100               call     0x140337710
0x14031e0ce: 4533c0                   xor      r8d, r8d
0x14031e0d1: 83fb0e                   cmp      ebx, 0xe
0x14031e0d4: 776a                     ja       0x14031e140
0x14031e0d6: 488d0d231fceff           lea      rcx, [rip - 0x31e0dd]
0x14031e0dd: 4863c3                   movsxd   rax, ebx
0x14031e0e0: 8b948154e13100           mov      edx, dword ptr [rcx + rax*4 + 0x31e154]
0x14031e0e7: 4803d1                   add      rdx, rcx
0x14031e0ea: ffe2                     jmp      rdx
0x14031e0ec: ba30000000               mov      edx, 0x30
0x14031e0f1: eb3d                     jmp      0x14031e130
0x14031e0f3: ba98000000               mov      edx, 0x98
0x14031e0f8: eb36                     jmp      0x14031e130
0x14031e0fa: baa8000000               mov      edx, 0xa8
0x14031e0ff: eb2f                     jmp      0x14031e130
0x14031e101: ba28000000               mov      edx, 0x28
0x14031e106: eb28                     jmp      0x14031e130
0x14031e108: ba1c000000               mov      edx, 0x1c
0x14031e10d: eb21                     jmp      0x14031e130
0x14031e10f: ba20000000               mov      edx, 0x20
0x14031e114: eb1a                     jmp      0x14031e130
0x14031e116: ba2c000000               mov      edx, 0x2c
0x14031e11b: eb13                     jmp      0x14031e130
0x14031e11d: ba40000000               mov      edx, 0x40
0x14031e122: eb0c                     jmp      0x14031e130
0x14031e124: ba3c000000               mov      edx, 0x3c
0x14031e129: eb05                     jmp      0x14031e130
0x14031e12b: ba18000000               mov      edx, 0x18
0x14031e130: 4183c8ff                 or       r8d, 0xffffffff
0x14031e134: 488d4e38                 lea      rcx, [rsi + 0x38]
0x14031e138: e81380faff               call     0x1402c6150
0x14031e13d: 4c8bc0                   mov      r8, rax
0x14031e140: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031e145: 4c894658                 mov      qword ptr [rsi + 0x58], r8
0x14031e149: 488b742438               mov      rsi, qword ptr [rsp + 0x38]
0x14031e14e: 4883c420                 add      rsp, 0x20
0x14031e152: 5f                       pop      rdi
0x14031e153: c3                       ret      
