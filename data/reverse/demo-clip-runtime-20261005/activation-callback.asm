; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031e690, 0x14031e72f); contiguous code slice, may span split unwind fragments
0x14031e690: 4053                     push     rbx
0x14031e692: 4883ec30                 sub      rsp, 0x30
0x14031e696: f30f10491c               movss    xmm1, dword ptr [rcx + 0x1c]
0x14031e69b: 0f57c0                   xorps    xmm0, xmm0
0x14031e69e: 0f2fc1                   comiss   xmm0, xmm1
0x14031e6a1: 488bd9                   mov      rbx, rcx
0x14031e6a4: 7607                     jbe      0x14031e6ad
0x14031e6a6: 0f570d23030500           xorps    xmm1, xmmword ptr [rip + 0x50323]
0x14031e6ad: f30f1005331b1c00         movss    xmm0, dword ptr [rip + 0x1c1b33]
0x14031e6b5: 0f2fc1                   comiss   xmm0, xmm1
0x14031e6b8: 7608                     jbe      0x14031e6c2
0x14031e6ba: 32c0                     xor      al, al
0x14031e6bc: 4883c430                 add      rsp, 0x30
0x14031e6c0: 5b                       pop      rbx
0x14031e6c1: c3                       ret      
0x14031e6c2: 8b4134                   mov      eax, dword ptr [rcx + 0x34]
0x14031e6c5: 894130                   mov      dword ptr [rcx + 0x30], eax
0x14031e6c8: e8639dfaff               call     0x1402c8430
0x14031e6cd: 0fb65364                 movzx    edx, byte ptr [rbx + 0x64]
0x14031e6d1: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e6d8: e823030000               call     0x14031ea00
0x14031e6dd: 6685c0                   test     ax, ax
0x14031e6e0: 791c                     jns      0x14031e6fe
0x14031e6e2: 488bcb                   mov      rcx, rbx
0x14031e6e5: e8469dfaff               call     0x1402c8430
0x14031e6ea: 0fb65364                 movzx    edx, byte ptr [rbx + 0x64]
0x14031e6ee: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e6f5: e846030000               call     0x14031ea40
0x14031e6fa: 85c0                     test     eax, eax
0x14031e6fc: 78bc                     js       0x14031e6ba
0x14031e6fe: 837b6400                 cmp      dword ptr [rbx + 0x64], 0
0x14031e702: 7c23                     jl       0x14031e727
0x14031e704: 488bcb                   mov      rcx, rbx
0x14031e707: e8249dfaff               call     0x1402c8430
0x14031e70c: 4c8b4358                 mov      r8, qword ptr [rbx + 0x58]
0x14031e710: 4533c9                   xor      r9d, r9d
0x14031e713: 8b5364                   mov      edx, dword ptr [rbx + 0x64]
0x14031e716: c644242000               mov      byte ptr [rsp + 0x20], 0
0x14031e71b: 488d88603f0000           lea      rcx, [rax + 0x3f60]
0x14031e722: e829090000               call     0x14031f050
0x14031e727: b001                     mov      al, 1
0x14031e729: 4883c430                 add      rsp, 0x30
0x14031e72d: 5b                       pop      rbx
0x14031e72e: c3                       ret      
