; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031e580, 0x14031e5c7); contiguous code slice, may span split unwind fragments
0x14031e580: 4053                     push     rbx
0x14031e582: 4883ec20                 sub      rsp, 0x20
0x14031e586: 488bd9                   mov      rbx, rcx
0x14031e589: e8a29efaff               call     0x1402c8430
0x14031e58e: 0fb65364                 movzx    edx, byte ptr [rbx + 0x64]
0x14031e592: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e599: e862040000               call     0x14031ea00
0x14031e59e: 6683f8ff                 cmp      ax, -1
0x14031e5a2: 741d                     je       0x14031e5c1
0x14031e5a4: 488bcb                   mov      rcx, rbx
0x14031e5a7: e8849efaff               call     0x1402c8430
0x14031e5ac: 0fb65364                 movzx    edx, byte ptr [rbx + 0x64]
0x14031e5b0: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e5b7: 4883c420                 add      rsp, 0x20
0x14031e5bb: 5b                       pop      rbx
0x14031e5bc: e93f070000               jmp      0x14031ed00
0x14031e5c1: 4883c420                 add      rsp, 0x20
0x14031e5c5: 5b                       pop      rbx
0x14031e5c6: c3                       ret      
