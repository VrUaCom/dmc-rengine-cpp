; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031e5d0, 0x14031e688); contiguous code slice, may span split unwind fragments
0x14031e5d0: 4053                     push     rbx
0x14031e5d2: 4883ec30                 sub      rsp, 0x30
0x14031e5d6: 488bd9                   mov      rbx, rcx
0x14031e5d9: ba87000000               mov      edx, 0x87
0x14031e5de: 488d0dab479d00           lea      rcx, [rip + 0x9d47ab]
0x14031e5e5: e876830000               call     0x140326960
0x14031e5ea: f30f59431c               mulss    xmm0, dword ptr [rbx + 0x1c]
0x14031e5ef: 488bcb                   mov      rcx, rbx
0x14031e5f2: f30f104b30               movss    xmm1, dword ptr [rbx + 0x30]
0x14031e5f7: f30f5cc8                 subss    xmm1, xmm0
0x14031e5fb: 0f57c0                   xorps    xmm0, xmm0
0x14031e5fe: 0f2fc1                   comiss   xmm0, xmm1
0x14031e601: f30f114b30               movss    dword ptr [rbx + 0x30], xmm1
0x14031e606: 760c                     jbe      0x14031e614
0x14031e608: 488b03                   mov      rax, qword ptr [rbx]
0x14031e60b: 4883c430                 add      rsp, 0x30
0x14031e60f: 5b                       pop      rbx
0x14031e610: 48ff6010                 jmp      qword ptr [rax + 0x10]
0x14031e614: e8179efaff               call     0x1402c8430
0x14031e619: 0fb65364                 movzx    edx, byte ptr [rbx + 0x64]
0x14031e61d: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e624: e8d7030000               call     0x14031ea00
0x14031e629: 6685c0                   test     ax, ax
0x14031e62c: 7954                     jns      0x14031e682
0x14031e62e: 488bcb                   mov      rcx, rbx
0x14031e631: e8fa9dfaff               call     0x1402c8430
0x14031e636: 0fb65364                 movzx    edx, byte ptr [rbx + 0x64]
0x14031e63a: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e641: e8fa030000               call     0x14031ea40
0x14031e646: 85c0                     test     eax, eax
0x14031e648: 790f                     jns      0x14031e659
0x14031e64a: 488b03                   mov      rax, qword ptr [rbx]
0x14031e64d: 488bcb                   mov      rcx, rbx
0x14031e650: 4883c430                 add      rsp, 0x30
0x14031e654: 5b                       pop      rbx
0x14031e655: 48ff6010                 jmp      qword ptr [rax + 0x10]
0x14031e659: 837b6400                 cmp      dword ptr [rbx + 0x64], 0
0x14031e65d: 7c23                     jl       0x14031e682
0x14031e65f: 488bcb                   mov      rcx, rbx
0x14031e662: e8c99dfaff               call     0x1402c8430
0x14031e667: 4c8b4358                 mov      r8, qword ptr [rbx + 0x58]
0x14031e66b: 4533c9                   xor      r9d, r9d
0x14031e66e: 8b5364                   mov      edx, dword ptr [rbx + 0x64]
0x14031e671: c644242000               mov      byte ptr [rsp + 0x20], 0
0x14031e676: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e67d: e8ce090000               call     0x14031f050
0x14031e682: 4883c430                 add      rsp, 0x30
0x14031e686: 5b                       pop      rbx
0x14031e687: c3                       ret      
