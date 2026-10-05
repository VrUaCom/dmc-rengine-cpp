; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402d5eb0, 0x1402d6086); code slice, not necessarily one unwind entry
0x1402d5eb0: 4053                     push     rbx
0x1402d5eb2: 55                       push     rbp
0x1402d5eb3: 56                       push     rsi
0x1402d5eb4: 57                       push     rdi
0x1402d5eb5: 4156                     push     r14
0x1402d5eb7: 4883ec70                 sub      rsp, 0x70
0x1402d5ebb: 488b05eef12f00           mov      rax, qword ptr [rip + 0x2ff1ee]
0x1402d5ec2: 4833c4                   xor      rax, rsp
0x1402d5ec5: 4889442460               mov      qword ptr [rsp + 0x60], rax
0x1402d5eca: 488bd9                   mov      rbx, rcx
0x1402d5ecd: 4963f8                   movsxd   rdi, r8d
0x1402d5ed0: 33f6                     xor      esi, esi
0x1402d5ed2: 4c8bf2                   mov      r14, rdx
0x1402d5ed5: 410fb7e9                 movzx    ebp, r9w
0x1402d5ed9: 8bd6                     mov      edx, esi
0x1402d5edb: 8bce                     mov      ecx, esi
0x1402d5edd: 488d4358                 lea      rax, [rbx + 0x58]
0x1402d5ee1: 3970f8                   cmp      dword ptr [rax - 8], esi
0x1402d5ee4: 7410                     je       0x1402d5ef6
0x1402d5ee6: 6683fdff                 cmp      bp, -1
0x1402d5eea: 7406                     je       0x1402d5ef2
0x1402d5eec: 66396804                 cmp      word ptr [rax + 4], bp
0x1402d5ef0: 7504                     jne      0x1402d5ef6
0x1402d5ef2: 3938                     cmp      dword ptr [rax], edi
0x1402d5ef4: 7414                     je       0x1402d5f0a
0x1402d5ef6: ffc2                     inc      edx
0x1402d5ef8: 48ffc1                   inc      rcx
0x1402d5efb: 4883c050                 add      rax, 0x50
0x1402d5eff: 4881f9c8000000           cmp      rcx, 0xc8
0x1402d5f06: 7cd9                     jl       0x1402d5ee1
0x1402d5f08: eb1b                     jmp      0x1402d5f25
0x1402d5f0a: 4863c2                   movsxd   rax, edx
0x1402d5f0d: 488d0c80                 lea      rcx, [rax + rax*4]
0x1402d5f11: 48c1e104                 shl      rcx, 4
0x1402d5f15: 488d4348                 lea      rax, [rbx + 0x48]
0x1402d5f19: 4803c1                   add      rax, rcx
0x1402d5f1c: 4885c0                   test     rax, rax
0x1402d5f1f: 0f8537010000             jne      0x1402d605c
0x1402d5f25: 833b02                   cmp      dword ptr [rbx], 2
0x1402d5f28: 750e                     jne      0x1402d5f38
0x1402d5f2a: 8d47f7                   lea      eax, [rdi - 9]
0x1402d5f2d: a9fbffffff               test     eax, 0xfffffffb
0x1402d5f32: 0f8534010000             jne      0x1402d606c
0x1402d5f38: 8bd7                     mov      edx, edi
0x1402d5f3a: 488bcb                   mov      rcx, rbx
0x1402d5f3d: e86e040000               call     0x1402d63b0
0x1402d5f42: 4885c0                   test     rax, rax
0x1402d5f45: 0f8411010000             je       0x1402d605c
0x1402d5f4b: 66896814                 mov      word ptr [rax + 0x14], bp
0x1402d5f4f: 66ff83c83e0000           inc      word ptr [rbx + 0x3ec8]
0x1402d5f56: 8b03                     mov      eax, dword ptr [rbx]
0x1402d5f58: 85c0                     test     eax, eax
0x1402d5f5a: 7553                     jne      0x1402d5faf
0x1402d5f5c: 488b83d83e0000           mov      rax, qword ptr [rbx + 0x3ed8]
0x1402d5f63: 488b4820                 mov      rcx, qword ptr [rax + 0x20]
0x1402d5f67: 83ff07                   cmp      edi, 7
0x1402d5f6a: 0f85fc000000             jne      0x1402d606c
0x1402d5f70: 8b4104                   mov      eax, dword ptr [rcx + 4]
0x1402d5f73: 83f80d                   cmp      eax, 0xd
0x1402d5f76: 7305                     jae      0x1402d5f7d
0x1402d5f78: 488bd6                   mov      rdx, rsi
0x1402d5f7b: eb0f                     jmp      0x1402d5f8c
0x1402d5f7d: 8b5138                   mov      edx, dword ptr [rcx + 0x38]
0x1402d5f80: 85d2                     test     edx, edx
0x1402d5f82: 7505                     jne      0x1402d5f89
0x1402d5f84: 488bd6                   mov      rdx, rsi
0x1402d5f87: eb03                     jmp      0x1402d5f8c
0x1402d5f89: 4803d1                   add      rdx, rcx
0x1402d5f8c: 83f80c                   cmp      eax, 0xc
0x1402d5f8f: 720b                     jb       0x1402d5f9c
0x1402d5f91: 8b4134                   mov      eax, dword ptr [rcx + 0x34]
0x1402d5f94: 85c0                     test     eax, eax
0x1402d5f96: 7404                     je       0x1402d5f9c
0x1402d5f98: 488d3401                 lea      rsi, [rcx + rax]
0x1402d5f9c: 41b804000000             mov      r8d, 4
0x1402d5fa2: 488bce                   mov      rcx, rsi
0x1402d5fa5: e856a5feff               call     0x1402c0500
0x1402d5faa: e9bd000000               jmp      0x1402d606c
0x1402d5faf: 83f802                   cmp      eax, 2
0x1402d5fb2: 0f84b4000000             je       0x1402d606c
0x1402d5fb8: 4c8d4328                 lea      r8, [rbx + 0x28]
0x1402d5fbc: 4d8bce                   mov      r9, r14
0x1402d5fbf: 488d153a0a2300           lea      rdx, [rip + 0x230a3a]
0x1402d5fc6: 488d4c2420               lea      rcx, [rsp + 0x20]
0x1402d5fcb: e8b09fd5ff               call     0x14002ff80
0x1402d5fd0: 488d542420               lea      rdx, [rsp + 0x20]
0x1402d5fd5: 488d0d3476a900           lea      rcx, [rip + 0xa97634]
0x1402d5fdc: e84f210600               call     0x140338130
0x1402d5fe1: 488d93083f0000           lea      rdx, [rbx + 0x3f08]
0x1402d5fe8: 4863e8                   movsxd   rbp, eax
0x1402d5feb: 488d8b483f0000           lea      rcx, [rbx + 0x3f48]
0x1402d5ff2: e8b9190600               call     0x1403379b0
0x1402d5ff7: 84c0                     test     al, al
0x1402d5ff9: 7461                     je       0x1402d605c
0x1402d5ffb: 8d5501                   lea      edx, [rbp + 1]
0x1402d5ffe: 488d8b483f0000           lea      rcx, [rbx + 0x3f48]
0x1402d6005: e816190600               call     0x140337920
0x1402d600a: 4885c0                   test     rax, rax
0x1402d600d: 744d                     je       0x1402d605c
0x1402d600f: 4c8b83503f0000           mov      r8, qword ptr [rbx + 0x3f50]
0x1402d6016: 488d542420               lea      rdx, [rsp + 0x20]
0x1402d601b: 488d0dee75a900           lea      rcx, [rip + 0xa975ee]
0x1402d6022: e829220600               call     0x140338250
0x1402d6027: 84c0                     test     al, al
0x1402d6029: 7435                     je       0x1402d6060
0x1402d602b: 488b83503f0000           mov      rax, qword ptr [rbx + 0x3f50]
0x1402d6032: 4c8d05478d2f00           lea      r8, [rip + 0x2f8d47]
0x1402d6039: 488bcb                   mov      rcx, rbx
0x1402d603c: 4088740500               mov      byte ptr [rbp + rax], sil
0x1402d6041: 488b93503f0000           mov      rdx, qword ptr [rbx + 0x3f50]
0x1402d6048: 41ff14f8                 call     qword ptr [r8 + rdi*8]
0x1402d604c: 84c0                     test     al, al
0x1402d604e: 7510                     jne      0x1402d6060
0x1402d6050: 488d8b483f0000           lea      rcx, [rbx + 0x3f48]
0x1402d6057: e884190600               call     0x1403379e0
0x1402d605c: 32c0                     xor      al, al
0x1402d605e: eb0e                     jmp      0x1402d606e
0x1402d6060: 488d8b483f0000           lea      rcx, [rbx + 0x3f48]
0x1402d6067: e874190600               call     0x1403379e0
0x1402d606c: b001                     mov      al, 1
0x1402d606e: 488b4c2460               mov      rcx, qword ptr [rsp + 0x60]
0x1402d6073: 4833cc                   xor      rcx, rsp
0x1402d6076: e875f50600               call     0x1403455f0
0x1402d607b: 4883c470                 add      rsp, 0x70
0x1402d607f: 415e                     pop      r14
0x1402d6081: 5f                       pop      rdi
0x1402d6082: 5e                       pop      rsi
0x1402d6083: 5d                       pop      rbp
0x1402d6084: 5b                       pop      rbx
0x1402d6085: c3                       ret      
