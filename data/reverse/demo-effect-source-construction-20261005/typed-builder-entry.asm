; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031b2d0, 0x14031b331); code slice, not necessarily one unwind entry
0x14031b2d0: 48895c2418               mov      qword ptr [rsp + 0x18], rbx
0x14031b2d5: 55                       push     rbp
0x14031b2d6: 56                       push     rsi
0x14031b2d7: 57                       push     rdi
0x14031b2d8: 4154                     push     r12
0x14031b2da: 4155                     push     r13
0x14031b2dc: 4156                     push     r14
0x14031b2de: 4157                     push     r15
0x14031b2e0: 488bec                   mov      rbp, rsp
0x14031b2e3: 4883ec70                 sub      rsp, 0x70
0x14031b2e7: 488b05c29d2b00           mov      rax, qword ptr [rip + 0x2b9dc2]
0x14031b2ee: 4833c4                   xor      rax, rsp
0x14031b2f1: 488945f0                 mov      qword ptr [rbp - 0x10], rax
0x14031b2f5: 4c8bfa                   mov      r15, rdx
0x14031b2f8: 488bd9                   mov      rbx, rcx
0x14031b2fb: 8b5164                   mov      edx, dword ptr [rcx + 0x64]
0x14031b2fe: e8ad2d0000               call     0x14031e0b0
0x14031b303: 8b5364                   mov      edx, dword ptr [rbx + 0x64]
0x14031b306: 488bcb                   mov      rcx, rbx
0x14031b309: e8822e0000               call     0x14031e190
0x14031b30e: 8b4364                   mov      eax, dword ptr [rbx + 0x64]
0x14031b311: ffc0                     inc      eax
0x14031b313: 83f810                   cmp      eax, 0x10
0x14031b316: 0f87292d0000             ja       0x14031e045
0x14031b31c: 488d15dd4cceff           lea      rdx, [rip - 0x31b323]
0x14031b323: 4898                     cdqe     
0x14031b325: 8b8c826ce03100           mov      ecx, dword ptr [rdx + rax*4 + 0x31e06c]
0x14031b32c: 4803ca                   add      rcx, rdx
0x14031b32f: ffe1                     jmp      rcx
