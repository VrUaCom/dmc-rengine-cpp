; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402d602b, 0x1402d604c); contiguous code slice, may span split unwind fragments
0x1402d602b: 488b83503f0000           mov      rax, qword ptr [rbx + 0x3f50]
0x1402d6032: 4c8d05478d2f00           lea      r8, [rip + 0x2f8d47]
0x1402d6039: 488bcb                   mov      rcx, rbx
0x1402d603c: 4088740500               mov      byte ptr [rbp + rax], sil
0x1402d6041: 488b93503f0000           mov      rdx, qword ptr [rbx + 0x3f50]
0x1402d6048: 41ff14f8                 call     qword ptr [r8 + rdi*8]
