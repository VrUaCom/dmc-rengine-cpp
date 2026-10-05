; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402da9d0, 0x1402dad4e); code slice, not necessarily one unwind entry
0x1402da9d0: 4c8bdc                   mov      r11, rsp
0x1402da9d3: 55                       push     rbp
0x1402da9d4: 53                       push     rbx
0x1402da9d5: 56                       push     rsi
0x1402da9d6: 4154                     push     r12
0x1402da9d8: 4155                     push     r13
0x1402da9da: 4156                     push     r14
0x1402da9dc: 4157                     push     r15
0x1402da9de: 498d6ba1                 lea      rbp, [r11 - 0x5f]
0x1402da9e2: 4881ec90000000           sub      rsp, 0x90
0x1402da9e9: 410f2973b8               movaps   xmmword ptr [r11 - 0x48], xmm6
0x1402da9ee: 488b05bba62f00           mov      rax, qword ptr [rip + 0x2fa6bb]
0x1402da9f5: 4833c4                   xor      rax, rsp
0x1402da9f8: 48894507                 mov      qword ptr [rbp + 7], rax
0x1402da9fc: 33f6                     xor      esi, esi
0x1402da9fe: 488955b7                 mov      qword ptr [rbp - 0x49], rdx
0x1402daa02: 448bf6                   mov      r14d, esi
0x1402daa05: 49897b08                 mov      qword ptr [r11 + 8], rdi
0x1402daa09: 4d8bf8                   mov      r15, r8
0x1402daa0c: 4c8d25fdbf2200           lea      r12, [rip + 0x22bffd]
0x1402daa13: 488bda                   mov      rbx, rdx
0x1402daa16: 4c8d2d47c02200           lea      r13, [rip + 0x22c047]
0x1402daa1d: 0f57f6                   xorps    xmm6, xmm6
0x1402daa20: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402daa24: 488bcb                   mov      rcx, rbx
0x1402daa27: e874c20400               call     0x140326ca0
0x1402daa2c: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402daa30: 488bd8                   mov      rbx, rax
0x1402daa33: 4885c0                   test     rax, rax
0x1402daa36: 742e                     je       0x1402daa66
0x1402daa38: 0fb67dc7                 movzx    edi, byte ptr [rbp - 0x39]
0x1402daa3c: 4080ff3b                 cmp      dil, 0x3b
0x1402daa40: 7555                     jne      0x1402daa97
0x1402daa42: 488d15c3bf2200           lea      rdx, [rip + 0x22bfc3]
0x1402daa49: 488bc8                   mov      rcx, rax
0x1402daa4c: ff157e490700             call     qword ptr [rip + 0x7497e]
0x1402daa52: 488bd8                   mov      rbx, rax
0x1402daa55: 4885c0                   test     rax, rax
0x1402daa58: 740c                     je       0x1402daa66
0x1402daa5a: 48ffc3                   inc      rbx
0x1402daa5d: 48895db7                 mov      qword ptr [rbp - 0x49], rbx
0x1402daa61: 403833                   cmp      byte ptr [rbx], sil
0x1402daa64: 75ba                     jne      0x1402daa20
0x1402daa66: 32c0                     xor      al, al
0x1402daa68: 488bbc24d0000000         mov      rdi, qword ptr [rsp + 0xd0]
0x1402daa70: 488b4d07                 mov      rcx, qword ptr [rbp + 7]
0x1402daa74: 4833cc                   xor      rcx, rsp
0x1402daa77: e874ab0600               call     0x1403455f0
0x1402daa7c: 0f28b42480000000         movaps   xmm6, xmmword ptr [rsp + 0x80]
0x1402daa84: 4881c490000000           add      rsp, 0x90
0x1402daa8b: 415f                     pop      r15
0x1402daa8d: 415e                     pop      r14
0x1402daa8f: 415d                     pop      r13
0x1402daa91: 415c                     pop      r12
0x1402daa93: 5e                       pop      rsi
0x1402daa94: 5b                       pop      rbx
0x1402daa95: 5d                       pop      rbp
0x1402daa96: c3                       ret      
0x1402daa97: 4080ff23                 cmp      dil, 0x23
0x1402daa9b: 7558                     jne      0x1402daaf5
0x1402daa9d: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402daaa1: 488bcb                   mov      rcx, rbx
0x1402daaa4: e8f7c10400               call     0x140326ca0
0x1402daaa9: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402daaad: 4885c0                   test     rax, rax
0x1402daab0: 74b4                     je       0x1402daa66
0x1402daab2: 488bce                   mov      rcx, rsi
0x1402daab5: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402daab9: 0f1f8000000000           nop      dword ptr [rax]
0x1402daac0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402daac4: 48ffc1                   inc      rcx
0x1402daac7: 413a440cff               cmp      al, byte ptr [r12 + rcx - 1]
0x1402daacc: 0f8553020000             jne      0x1402dad25
0x1402daad2: 4883f905                 cmp      rcx, 5
0x1402daad6: 75e8                     jne      0x1402daac0
0x1402daad8: 498bcf                   mov      rcx, r15
0x1402daadb: e8d0940400               call     0x140323fb0
0x1402daae0: 4c8bf0                   mov      r14, rax
0x1402daae3: 4885c0                   test     rax, rax
0x1402daae6: 0f847affffff             je       0x1402daa66
0x1402daaec: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402daaf0: e92bffffff               jmp      0x1402daa20
0x1402daaf5: 488d151cbf2200           lea      rdx, [rip + 0x22bf1c]
0x1402daafc: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dab00: e833c10600               call     0x140346c38
0x1402dab05: 85c0                     test     eax, eax
0x1402dab07: 7548                     jne      0x1402dab51
0x1402dab09: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dab0d: 488bcb                   mov      rcx, rbx
0x1402dab10: e88bc10400               call     0x140326ca0
0x1402dab15: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402dab19: 4885c0                   test     rax, rax
0x1402dab1c: 0f8444ffffff             je       0x1402daa66
0x1402dab22: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dab26: ff15fc480700             call     qword ptr [rip + 0x748fc]
0x1402dab2c: 0f57c9                   xorps    xmm1, xmm1
0x1402dab2f: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402dab33: f30f2cc1                 cvttss2si eax, xmm1
0x1402dab37: 660f6ed0                 movd     xmm2, eax
0x1402dab3b: 0f5bd2                   cvtdq2ps xmm2, xmm2
0x1402dab3e: f30f5fd6                 maxss    xmm2, xmm6
0x1402dab42: f3410f115610             movss    dword ptr [r14 + 0x10], xmm2
0x1402dab48: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dab4c: e9cffeffff               jmp      0x1402daa20
0x1402dab51: 488d15d0be2200           lea      rdx, [rip + 0x22bed0]
0x1402dab58: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dab5c: e8d7c00600               call     0x140346c38
0x1402dab61: 85c0                     test     eax, eax
0x1402dab63: 7539                     jne      0x1402dab9e
0x1402dab65: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dab69: 488bcb                   mov      rcx, rbx
0x1402dab6c: e82fc10400               call     0x140326ca0
0x1402dab71: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402dab75: 4885c0                   test     rax, rax
0x1402dab78: 0f84e8feffff             je       0x1402daa66
0x1402dab7e: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dab82: ff15a0480700             call     qword ptr [rip + 0x748a0]
0x1402dab88: 0f57c9                   xorps    xmm1, xmm1
0x1402dab8b: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402dab8f: f3410f114e14             movss    dword ptr [r14 + 0x14], xmm1
0x1402dab95: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dab99: e982feffff               jmp      0x1402daa20
0x1402dab9e: 488d1593be2200           lea      rdx, [rip + 0x22be93]
0x1402daba5: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402daba9: e88ac00600               call     0x140346c38
0x1402dabae: 85c0                     test     eax, eax
0x1402dabb0: 7539                     jne      0x1402dabeb
0x1402dabb2: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dabb6: 488bcb                   mov      rcx, rbx
0x1402dabb9: e8e2c00400               call     0x140326ca0
0x1402dabbe: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402dabc2: 4885c0                   test     rax, rax
0x1402dabc5: 0f849bfeffff             je       0x1402daa66
0x1402dabcb: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dabcf: ff1553480700             call     qword ptr [rip + 0x74853]
0x1402dabd5: 0f57c9                   xorps    xmm1, xmm1
0x1402dabd8: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402dabdc: f3410f114e18             movss    dword ptr [r14 + 0x18], xmm1
0x1402dabe2: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dabe6: e935feffff               jmp      0x1402daa20
0x1402dabeb: 488d1556be2200           lea      rdx, [rip + 0x22be56]
0x1402dabf2: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dabf6: e83dc00600               call     0x140346c38
0x1402dabfb: 85c0                     test     eax, eax
0x1402dabfd: 7539                     jne      0x1402dac38
0x1402dabff: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dac03: 488bcb                   mov      rcx, rbx
0x1402dac06: e895c00400               call     0x140326ca0
0x1402dac0b: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402dac0f: 4885c0                   test     rax, rax
0x1402dac12: 0f844efeffff             je       0x1402daa66
0x1402dac18: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dac1c: ff1506480700             call     qword ptr [rip + 0x74806]
0x1402dac22: 0f57c9                   xorps    xmm1, xmm1
0x1402dac25: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402dac29: f3410f114e1c             movss    dword ptr [r14 + 0x1c], xmm1
0x1402dac2f: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dac33: e9e8fdffff               jmp      0x1402daa20
0x1402dac38: 403a3da5be2200           cmp      dil, byte ptr [rip + 0x22bea5]
0x1402dac3f: 7546                     jne      0x1402dac87
0x1402dac41: 0fb645c8                 movzx    eax, byte ptr [rbp - 0x38]
0x1402dac45: 3a059abe2200             cmp      al, byte ptr [rip + 0x22be9a]
0x1402dac4b: 753a                     jne      0x1402dac87
0x1402dac4d: 0fb645c9                 movzx    eax, byte ptr [rbp - 0x37]
0x1402dac51: 3a058fbe2200             cmp      al, byte ptr [rip + 0x22be8f]
0x1402dac57: 752e                     jne      0x1402dac87
0x1402dac59: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dac5d: 488bcb                   mov      rcx, rbx
0x1402dac60: e83bc00400               call     0x140326ca0
0x1402dac65: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402dac69: 4885c0                   test     rax, rax
0x1402dac6c: 0f84f4fdffff             je       0x1402daa66
0x1402dac72: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dac76: 498bce                   mov      rcx, r14
0x1402dac79: e8b23a0400               call     0x14031e730
0x1402dac7e: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dac82: e999fdffff               jmp      0x1402daa20
0x1402dac87: 488bce                   mov      rcx, rsi
0x1402dac8a: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dac8e: 6690                     nop      
0x1402dac90: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402dac94: 48ffc1                   inc      rcx
0x1402dac97: 423a4429ff               cmp      al, byte ptr [rcx + r13 - 1]
0x1402dac9c: 753f                     jne      0x1402dacdd
0x1402dac9e: 4883f905                 cmp      rcx, 5
0x1402daca2: 75ec                     jne      0x1402dac90
0x1402daca4: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402daca8: 488bcb                   mov      rcx, rbx
0x1402dacab: e8f0bf0400               call     0x140326ca0
0x1402dacb0: 488945b7                 mov      qword ptr [rbp - 0x49], rax
0x1402dacb4: 4885c0                   test     rax, rax
0x1402dacb7: 0f84a9fdffff             je       0x1402daa66
0x1402dacbd: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dacc1: ff1561470700             call     qword ptr [rip + 0x74761]
0x1402dacc7: 0f57c9                   xorps    xmm1, xmm1
0x1402dacca: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x1402dacce: f3410f114e34             movss    dword ptr [r14 + 0x34], xmm1
0x1402dacd4: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dacd8: e943fdffff               jmp      0x1402daa20
0x1402dacdd: 488bce                   mov      rcx, rsi
0x1402dace0: 488d55c7                 lea      rdx, [rbp - 0x39]
0x1402dace4: 4c8d05d1be2200           lea      r8, [rip + 0x22bed1]
0x1402daceb: 0f1f440000               nop      dword ptr [rax + rax]
0x1402dacf0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402dacf4: 48ffc1                   inc      rcx
0x1402dacf7: 413a4408ff               cmp      al, byte ptr [r8 + rcx - 1]
0x1402dacfc: 0f851efdffff             jne      0x1402daa20
0x1402dad02: 4883f906                 cmp      rcx, 6
0x1402dad06: 75e8                     jne      0x1402dacf0
0x1402dad08: 488d55b7                 lea      rdx, [rbp - 0x49]
0x1402dad0c: 498bce                   mov      rcx, r14
0x1402dad0f: e8bc050400               call     0x14031b2d0
0x1402dad14: 84c0                     test     al, al
0x1402dad16: 0f844afdffff             je       0x1402daa66
0x1402dad1c: 488b5db7                 mov      rbx, qword ptr [rbp - 0x49]
0x1402dad20: e9fbfcffff               jmp      0x1402daa20
0x1402dad25: 488d4dc7                 lea      rcx, [rbp - 0x39]
0x1402dad29: 488d1518bb2200           lea      rdx, [rip + 0x22bb18]
0x1402dad30: 0fb60431                 movzx    eax, byte ptr [rcx + rsi]
0x1402dad34: 48ffc6                   inc      rsi
0x1402dad37: 3a4432ff                 cmp      al, byte ptr [rdx + rsi - 1]
0x1402dad3b: 0f8525fdffff             jne      0x1402daa66
0x1402dad41: 4883fe04                 cmp      rsi, 4
0x1402dad45: 75e9                     jne      0x1402dad30
0x1402dad47: b001                     mov      al, 1
0x1402dad49: e91afdffff               jmp      0x1402daa68
