; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402dad60, 0x1402db10e); code slice, not necessarily one unwind entry
0x1402dad60: 48895c2408               mov      qword ptr [rsp + 8], rbx
0x1402dad65: 4889742420               mov      qword ptr [rsp + 0x20], rsi
0x1402dad6a: 55                       push     rbp
0x1402dad6b: 57                       push     rdi
0x1402dad6c: 4154                     push     r12
0x1402dad6e: 4156                     push     r14
0x1402dad70: 4157                     push     r15
0x1402dad72: 488bec                   mov      rbp, rsp
0x1402dad75: 4881ec80000000           sub      rsp, 0x80
0x1402dad7c: 0f29742470               movaps   xmmword ptr [rsp + 0x70], xmm6
0x1402dad81: 488b0528a32f00           mov      rax, qword ptr [rip + 0x2fa328]
0x1402dad88: 4833c4                   xor      rax, rsp
0x1402dad8b: 488945e0                 mov      qword ptr [rbp - 0x20], rax
0x1402dad8f: 33ff                     xor      edi, edi
0x1402dad91: 4c8d3d78bc2200           lea      r15, [rip + 0x22bc78]
0x1402dad98: 8bf7                     mov      esi, edi
0x1402dad9a: 4c8d25c3bc2200           lea      r12, [rip + 0x22bcc3]
0x1402dada1: 4d8bf0                   mov      r14, r8
0x1402dada4: 488bda                   mov      rbx, rdx
0x1402dada7: 0f57f6                   xorps    xmm6, xmm6
0x1402dadaa: 660f1f440000             nop      word ptr [rax + rax]
0x1402dadb0: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402dadb4: 488bcb                   mov      rcx, rbx
0x1402dadb7: e8e4be0400               call     0x140326ca0
0x1402dadbc: 488bd8                   mov      rbx, rax
0x1402dadbf: 4885c0                   test     rax, rax
0x1402dadc2: 7428                     je       0x1402dadec
0x1402dadc4: 0fb645a0                 movzx    eax, byte ptr [rbp - 0x60]
0x1402dadc8: 3c3b                     cmp      al, 0x3b
0x1402dadca: 754f                     jne      0x1402dae1b
0x1402dadcc: 488d1539bc2200           lea      rdx, [rip + 0x22bc39]
0x1402dadd3: 488bcb                   mov      rcx, rbx
0x1402dadd6: ff15f4450700             call     qword ptr [rip + 0x745f4]
0x1402daddc: 488bd8                   mov      rbx, rax
0x1402daddf: 4885c0                   test     rax, rax
0x1402dade2: 7408                     je       0x1402dadec
0x1402dade4: 48ffc3                   inc      rbx
0x1402dade7: 40383b                   cmp      byte ptr [rbx], dil
0x1402dadea: 75c4                     jne      0x1402dadb0
0x1402dadec: 32c0                     xor      al, al
0x1402dadee: 488b4de0                 mov      rcx, qword ptr [rbp - 0x20]
0x1402dadf2: 4833cc                   xor      rcx, rsp
0x1402dadf5: e8f6a70600               call     0x1403455f0
0x1402dadfa: 4c8d9c2480000000         lea      r11, [rsp + 0x80]
0x1402dae02: 498b5b30                 mov      rbx, qword ptr [r11 + 0x30]
0x1402dae06: 498b7348                 mov      rsi, qword ptr [r11 + 0x48]
0x1402dae0a: 0f28742470               movaps   xmm6, xmmword ptr [rsp + 0x70]
0x1402dae0f: 498be3                   mov      rsp, r11
0x1402dae12: 415f                     pop      r15
0x1402dae14: 415e                     pop      r14
0x1402dae16: 415c                     pop      r12
0x1402dae18: 5f                       pop      rdi
0x1402dae19: 5d                       pop      rbp
0x1402dae1a: c3                       ret      
0x1402dae1b: 3c23                     cmp      al, 0x23
0x1402dae1d: 7549                     jne      0x1402dae68
0x1402dae1f: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402dae23: 488bcb                   mov      rcx, rbx
0x1402dae26: e875be0400               call     0x140326ca0
0x1402dae2b: 488bd8                   mov      rbx, rax
0x1402dae2e: 4885c0                   test     rax, rax
0x1402dae31: 74b9                     je       0x1402dadec
0x1402dae33: 488bcf                   mov      rcx, rdi
0x1402dae36: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402dae3a: 660f1f440000             nop      word ptr [rax + rax]
0x1402dae40: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402dae44: 48ffc1                   inc      rcx
0x1402dae47: 413a440fff               cmp      al, byte ptr [r15 + rcx - 1]
0x1402dae4c: 0f8589020000             jne      0x1402db0db
0x1402dae52: 4883f905                 cmp      rcx, 5
0x1402dae56: 75e8                     jne      0x1402dae40
0x1402dae58: 498bce                   mov      rcx, r14
0x1402dae5b: e850910400               call     0x140323fb0
0x1402dae60: 488bf0                   mov      rsi, rax
0x1402dae63: 4885c0                   test     rax, rax
0x1402dae66: eb82                     jmp      0x1402dadea
0x1402dae68: 488d15a9bb2200           lea      rdx, [rip + 0x22bba9]
0x1402dae6f: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402dae73: e8c0bd0600               call     0x140346c38
0x1402dae78: 85c0                     test     eax, eax
0x1402dae7a: 7542                     jne      0x1402daebe
0x1402dae7c: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402dae80: 488bcb                   mov      rcx, rbx
0x1402dae83: e818be0400               call     0x140326ca0
0x1402dae88: 488bd8                   mov      rbx, rax
0x1402dae8b: 4885c0                   test     rax, rax
0x1402dae8e: 0f8458ffffff             je       0x1402dadec
0x1402dae94: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402dae98: ff158a450700             call     qword ptr [rip + 0x7458a]
0x1402dae9e: 0f57c9                   xorps    xmm1, xmm1
0x1402daea1: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402daea5: f30f2cc1                 cvttss2si eax, xmm1
0x1402daea9: 660f6ed0                 movd     xmm2, eax
0x1402daead: 0f5bd2                   cvtdq2ps xmm2, xmm2
0x1402daeb0: f30f5fd6                 maxss    xmm2, xmm6
0x1402daeb4: f30f115610               movss    dword ptr [rsi + 0x10], xmm2
0x1402daeb9: e9f2feffff               jmp      0x1402dadb0
0x1402daebe: 488d1563bb2200           lea      rdx, [rip + 0x22bb63]
0x1402daec5: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daec9: e86abd0600               call     0x140346c38
0x1402daece: 85c0                     test     eax, eax
0x1402daed0: 7533                     jne      0x1402daf05
0x1402daed2: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402daed6: 488bcb                   mov      rcx, rbx
0x1402daed9: e8c2bd0400               call     0x140326ca0
0x1402daede: 488bd8                   mov      rbx, rax
0x1402daee1: 4885c0                   test     rax, rax
0x1402daee4: 0f8402ffffff             je       0x1402dadec
0x1402daeea: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daeee: ff1534450700             call     qword ptr [rip + 0x74534]
0x1402daef4: 0f57c9                   xorps    xmm1, xmm1
0x1402daef7: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402daefb: f30f114e14               movss    dword ptr [rsi + 0x14], xmm1
0x1402daf00: e9abfeffff               jmp      0x1402dadb0
0x1402daf05: 488d152cbb2200           lea      rdx, [rip + 0x22bb2c]
0x1402daf0c: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daf10: e823bd0600               call     0x140346c38
0x1402daf15: 85c0                     test     eax, eax
0x1402daf17: 7533                     jne      0x1402daf4c
0x1402daf19: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402daf1d: 488bcb                   mov      rcx, rbx
0x1402daf20: e87bbd0400               call     0x140326ca0
0x1402daf25: 488bd8                   mov      rbx, rax
0x1402daf28: 4885c0                   test     rax, rax
0x1402daf2b: 0f84bbfeffff             je       0x1402dadec
0x1402daf31: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daf35: ff15ed440700             call     qword ptr [rip + 0x744ed]
0x1402daf3b: 0f57c9                   xorps    xmm1, xmm1
0x1402daf3e: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402daf42: f30f114e18               movss    dword ptr [rsi + 0x18], xmm1
0x1402daf47: e964feffff               jmp      0x1402dadb0
0x1402daf4c: 488d15f5ba2200           lea      rdx, [rip + 0x22baf5]
0x1402daf53: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daf57: e8dcbc0600               call     0x140346c38
0x1402daf5c: 85c0                     test     eax, eax
0x1402daf5e: 7533                     jne      0x1402daf93
0x1402daf60: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402daf64: 488bcb                   mov      rcx, rbx
0x1402daf67: e834bd0400               call     0x140326ca0
0x1402daf6c: 488bd8                   mov      rbx, rax
0x1402daf6f: 4885c0                   test     rax, rax
0x1402daf72: 0f8474feffff             je       0x1402dadec
0x1402daf78: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daf7c: ff15a6440700             call     qword ptr [rip + 0x744a6]
0x1402daf82: 0f57c9                   xorps    xmm1, xmm1
0x1402daf85: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402daf89: f30f114e1c               movss    dword ptr [rsi + 0x1c], xmm1
0x1402daf8e: e91dfeffff               jmp      0x1402dadb0
0x1402daf93: 488d15beba2200           lea      rdx, [rip + 0x22babe]
0x1402daf9a: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402daf9e: e895bc0600               call     0x140346c38
0x1402dafa3: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402dafa7: 85c0                     test     eax, eax
0x1402dafa9: 7538                     jne      0x1402dafe3
0x1402dafab: 488bcb                   mov      rcx, rbx
0x1402dafae: e8edbc0400               call     0x140326ca0
0x1402dafb3: 488bd8                   mov      rbx, rax
0x1402dafb6: 4885c0                   test     rax, rax
0x1402dafb9: 0f842dfeffff             je       0x1402dadec
0x1402dafbf: 0fb645a0                 movzx    eax, byte ptr [rbp - 0x60]
0x1402dafc3: 3c44                     cmp      al, 0x44
0x1402dafc5: 7414                     je       0x1402dafdb
0x1402dafc7: 3c4c                     cmp      al, 0x4c
0x1402dafc9: 0f85e1fdffff             jne      0x1402dadb0
0x1402dafcf: c7463001000000           mov      dword ptr [rsi + 0x30], 1
0x1402dafd6: e9d5fdffff               jmp      0x1402dadb0
0x1402dafdb: 897e30                   mov      dword ptr [rsi + 0x30], edi
0x1402dafde: e9cdfdffff               jmp      0x1402dadb0
0x1402dafe3: 488bcf                   mov      rcx, rdi
0x1402dafe6: 66660f1f840000000000     nop      word ptr [rax + rax]
0x1402daff0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402daff4: 48ffc1                   inc      rcx
0x1402daff7: 413a440cff               cmp      al, byte ptr [r12 + rcx - 1]
0x1402daffc: 7539                     jne      0x1402db037
0x1402daffe: 4883f905                 cmp      rcx, 5
0x1402db002: 75ec                     jne      0x1402daff0
0x1402db004: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402db008: 488bcb                   mov      rcx, rbx
0x1402db00b: e890bc0400               call     0x140326ca0
0x1402db010: 488bd8                   mov      rbx, rax
0x1402db013: 4885c0                   test     rax, rax
0x1402db016: 0f84d0fdffff             je       0x1402dadec
0x1402db01c: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402db020: ff1502440700             call     qword ptr [rip + 0x74402]
0x1402db026: 0f57c9                   xorps    xmm1, xmm1
0x1402db029: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402db02d: f30f114e34               movss    dword ptr [rsi + 0x34], xmm1
0x1402db032: e979fdffff               jmp      0x1402dadb0
0x1402db037: 488d1532ba2200           lea      rdx, [rip + 0x22ba32]
0x1402db03e: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402db042: e8f1bb0600               call     0x140346c38
0x1402db047: 85c0                     test     eax, eax
0x1402db049: 753c                     jne      0x1402db087
0x1402db04b: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402db04f: 488bcb                   mov      rcx, rbx
0x1402db052: e849bc0400               call     0x140326ca0
0x1402db057: 488bd8                   mov      rbx, rax
0x1402db05a: 4885c0                   test     rax, rax
0x1402db05d: 0f8489fdffff             je       0x1402dadec
0x1402db063: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402db067: ff15bb430700             call     qword ptr [rip + 0x743bb]
0x1402db06d: 0f57c9                   xorps    xmm1, xmm1
0x1402db070: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402db074: 0f2ff1                   comiss   xmm6, xmm1
0x1402db077: 0f836ffdffff             jae      0x1402dadec
0x1402db07d: f30f114e3c               movss    dword ptr [rsi + 0x3c], xmm1
0x1402db082: e929fdffff               jmp      0x1402dadb0
0x1402db087: 488d15f2b92200           lea      rdx, [rip + 0x22b9f2]
0x1402db08e: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402db092: e8a1bb0600               call     0x140346c38
0x1402db097: 85c0                     test     eax, eax
0x1402db099: 0f8511fdffff             jne      0x1402dadb0
0x1402db09f: 488d55a0                 lea      rdx, [rbp - 0x60]
0x1402db0a3: 488bcb                   mov      rcx, rbx
0x1402db0a6: e8f5bb0400               call     0x140326ca0
0x1402db0ab: 488bd8                   mov      rbx, rax
0x1402db0ae: 4885c0                   test     rax, rax
0x1402db0b1: 0f8435fdffff             je       0x1402dadec
0x1402db0b7: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402db0bb: ff1567430700             call     qword ptr [rip + 0x74367]
0x1402db0c1: 0f57c9                   xorps    xmm1, xmm1
0x1402db0c4: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402db0c8: 0f2ff1                   comiss   xmm6, xmm1
0x1402db0cb: 0f831bfdffff             jae      0x1402dadec
0x1402db0d1: f30f114e40               movss    dword ptr [rsi + 0x40], xmm1
0x1402db0d6: e9d5fcffff               jmp      0x1402dadb0
0x1402db0db: 488d4da0                 lea      rcx, [rbp - 0x60]
0x1402db0df: 488d1562b72200           lea      rdx, [rip + 0x22b762]
0x1402db0e6: 66660f1f840000000000     nop      word ptr [rax + rax]
0x1402db0f0: 0fb60439                 movzx    eax, byte ptr [rcx + rdi]
0x1402db0f4: 48ffc7                   inc      rdi
0x1402db0f7: 3a443aff                 cmp      al, byte ptr [rdx + rdi - 1]
0x1402db0fb: 0f85ebfcffff             jne      0x1402dadec
0x1402db101: 4883ff04                 cmp      rdi, 4
0x1402db105: 75e9                     jne      0x1402db0f0
0x1402db107: b001                     mov      al, 1
0x1402db109: e9e0fcffff               jmp      0x1402dadee
