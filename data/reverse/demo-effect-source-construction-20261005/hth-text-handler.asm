; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031b52a, 0x14031b8c8); code slice, not necessarily one unwind entry
0x14031b52a: 498b0f                   mov      rcx, qword ptr [r15]
0x14031b52d: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b531: 488b5b58                 mov      rbx, qword ptr [rbx + 0x58]
0x14031b535: e866b70000               call     0x140326ca0
0x14031b53a: 807db023                 cmp      byte ptr [rbp - 0x50], 0x23
0x14031b53e: 0f84012b0000             je       0x14031e045
0x14031b544: 488d35d5c71e00           lea      rsi, [rip + 0x1ec7d5]
0x14031b54b: 4c8d35d6c71e00           lea      r14, [rip + 0x1ec7d6]
0x14031b552: 4c8d25d7c71e00           lea      r12, [rip + 0x1ec7d7]
0x14031b559: 488d3de0c71e00           lea      rdi, [rip + 0x1ec7e0]
0x14031b560: 4c8d2dfdc71e00           lea      r13, [rip + 0x1ec7fd]
0x14031b567: 660f1f840000000000       nop      word ptr [rax + rax]
0x14031b570: 4c8d05de0d0500           lea      r8, [rip + 0x50dde]
0x14031b577: 498bd7                   mov      rdx, r15
0x14031b57a: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b57e: e89daefbff               call     0x1402d6420
0x14031b583: 84c0                     test     al, al
0x14031b585: 749c                     je       0x14031b523
0x14031b587: 33c9                     xor      ecx, ecx
0x14031b589: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b58d: 0f1f00                   nop      dword ptr [rax]
0x14031b590: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b594: 48ffc1                   inc      rcx
0x14031b597: 3a440eff                 cmp      al, byte ptr [rsi + rcx - 1]
0x14031b59b: 753b                     jne      0x14031b5d8
0x14031b59d: 4883f907                 cmp      rcx, 7
0x14031b5a1: 75ed                     jne      0x14031b590
0x14031b5a3: 4c8d05ab0d0500           lea      r8, [rip + 0x50dab]
0x14031b5aa: 498bd7                   mov      rdx, r15
0x14031b5ad: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b5b1: e86aaefbff               call     0x1402d6420
0x14031b5b6: 84c0                     test     al, al
0x14031b5b8: 0f8465ffffff             je       0x14031b523
0x14031b5be: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b5c2: ff15603e0300             call     qword ptr [rip + 0x33e60]
0x14031b5c8: 0f57c9                   xorps    xmm1, xmm1
0x14031b5cb: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x14031b5cf: f30f110b                 movss    dword ptr [rbx], xmm1
0x14031b5d3: e9d5020000               jmp      0x14031b8ad
0x14031b5d8: 33c9                     xor      ecx, ecx
0x14031b5da: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b5de: 6690                     nop      
0x14031b5e0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b5e4: 48ffc1                   inc      rcx
0x14031b5e7: 413a440eff               cmp      al, byte ptr [r14 + rcx - 1]
0x14031b5ec: 753c                     jne      0x14031b62a
0x14031b5ee: 4883f905                 cmp      rcx, 5
0x14031b5f2: 75ec                     jne      0x14031b5e0
0x14031b5f4: 4c8d055a0d0500           lea      r8, [rip + 0x50d5a]
0x14031b5fb: 498bd7                   mov      rdx, r15
0x14031b5fe: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b602: e819aefbff               call     0x1402d6420
0x14031b607: 84c0                     test     al, al
0x14031b609: 0f8414ffffff             je       0x14031b523
0x14031b60f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b613: ff150f3e0300             call     qword ptr [rip + 0x33e0f]
0x14031b619: 0f57c9                   xorps    xmm1, xmm1
0x14031b61c: f20f5ac8                 cvtsd2ss xmm1, xmm0
0x14031b620: f30f114b04               movss    dword ptr [rbx + 4], xmm1
0x14031b625: e983020000               jmp      0x14031b8ad
0x14031b62a: 33c9                     xor      ecx, ecx
0x14031b62c: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b630: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b634: 48ffc1                   inc      rcx
0x14031b637: 413a440cff               cmp      al, byte ptr [r12 + rcx - 1]
0x14031b63c: 7533                     jne      0x14031b671
0x14031b63e: 4883f906                 cmp      rcx, 6
0x14031b642: 75ec                     jne      0x14031b630
0x14031b644: 4c8d050a0d0500           lea      r8, [rip + 0x50d0a]
0x14031b64b: 498bd7                   mov      rdx, r15
0x14031b64e: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b652: e8c9adfbff               call     0x1402d6420
0x14031b657: 84c0                     test     al, al
0x14031b659: 0f84c4feffff             je       0x14031b523
0x14031b65f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b663: ff15b73d0300             call     qword ptr [rip + 0x33db7]
0x14031b669: 894308                   mov      dword ptr [rbx + 8], eax
0x14031b66c: e93c020000               jmp      0x14031b8ad
0x14031b671: 488d15d0c61e00           lea      rdx, [rip + 0x1ec6d0]
0x14031b678: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b67c: e8b7b50200               call     0x140346c38
0x14031b681: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b685: 85c0                     test     eax, eax
0x14031b687: 7529                     jne      0x14031b6b2
0x14031b689: 4c8d05c50c0500           lea      r8, [rip + 0x50cc5]
0x14031b690: 498bd7                   mov      rdx, r15
0x14031b693: e888adfbff               call     0x1402d6420
0x14031b698: 84c0                     test     al, al
0x14031b69a: 0f8483feffff             je       0x14031b523
0x14031b6a0: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b6a4: ff15763d0300             call     qword ptr [rip + 0x33d76]
0x14031b6aa: 89430c                   mov      dword ptr [rbx + 0xc], eax
0x14031b6ad: e9fb010000               jmp      0x14031b8ad
0x14031b6b2: 488d159fc61e00           lea      rdx, [rip + 0x1ec69f]
0x14031b6b9: e87ab50200               call     0x140346c38
0x14031b6be: 85c0                     test     eax, eax
0x14031b6c0: 752d                     jne      0x14031b6ef
0x14031b6c2: 4c8d058c0c0500           lea      r8, [rip + 0x50c8c]
0x14031b6c9: 498bd7                   mov      rdx, r15
0x14031b6cc: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b6d0: e84badfbff               call     0x1402d6420
0x14031b6d5: 84c0                     test     al, al
0x14031b6d7: 0f8446feffff             je       0x14031b523
0x14031b6dd: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b6e1: ff15393d0300             call     qword ptr [rip + 0x33d39]
0x14031b6e7: 894310                   mov      dword ptr [rbx + 0x10], eax
0x14031b6ea: e9be010000               jmp      0x14031b8ad
0x14031b6ef: 33c9                     xor      ecx, ecx
0x14031b6f1: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b6f5: 6666660f1f840000000000   nop      word ptr [rax + rax]
0x14031b700: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b704: 48ffc1                   inc      rcx
0x14031b707: 423a4429ff               cmp      al, byte ptr [rcx + r13 - 1]
0x14031b70c: 7533                     jne      0x14031b741
0x14031b70e: 4883f907                 cmp      rcx, 7
0x14031b712: 75ec                     jne      0x14031b700
0x14031b714: 4c8d053a0c0500           lea      r8, [rip + 0x50c3a]
0x14031b71b: 498bd7                   mov      rdx, r15
0x14031b71e: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b722: e8f9acfbff               call     0x1402d6420
0x14031b727: 84c0                     test     al, al
0x14031b729: 0f84f4fdffff             je       0x14031b523
0x14031b72f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b733: ff15e73c0300             call     qword ptr [rip + 0x33ce7]
0x14031b739: 894314                   mov      dword ptr [rbx + 0x14], eax
0x14031b73c: e96c010000               jmp      0x14031b8ad
0x14031b741: 33c9                     xor      ecx, ecx
0x14031b743: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b747: 4c8d051ec61e00           lea      r8, [rip + 0x1ec61e]
0x14031b74e: 6690                     nop      
0x14031b750: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b754: 48ffc1                   inc      rcx
0x14031b757: 413a4408ff               cmp      al, byte ptr [r8 + rcx - 1]
0x14031b75c: 7533                     jne      0x14031b791
0x14031b75e: 4883f907                 cmp      rcx, 7
0x14031b762: 75ec                     jne      0x14031b750
0x14031b764: 4c8d05ea0b0500           lea      r8, [rip + 0x50bea]
0x14031b76b: 498bd7                   mov      rdx, r15
0x14031b76e: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b772: e8a9acfbff               call     0x1402d6420
0x14031b777: 84c0                     test     al, al
0x14031b779: 0f84a4fdffff             je       0x14031b523
0x14031b77f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b783: ff15973c0300             call     qword ptr [rip + 0x33c97]
0x14031b789: 894318                   mov      dword ptr [rbx + 0x18], eax
0x14031b78c: e91c010000               jmp      0x14031b8ad
0x14031b791: 33c9                     xor      ecx, ecx
0x14031b793: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b797: 4c8d05dac51e00           lea      r8, [rip + 0x1ec5da]
0x14031b79e: 6690                     nop      
0x14031b7a0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b7a4: 48ffc1                   inc      rcx
0x14031b7a7: 413a4408ff               cmp      al, byte ptr [r8 + rcx - 1]
0x14031b7ac: 7533                     jne      0x14031b7e1
0x14031b7ae: 4883f908                 cmp      rcx, 8
0x14031b7b2: 75ec                     jne      0x14031b7a0
0x14031b7b4: 4c8d059a0b0500           lea      r8, [rip + 0x50b9a]
0x14031b7bb: 498bd7                   mov      rdx, r15
0x14031b7be: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b7c2: e859acfbff               call     0x1402d6420
0x14031b7c7: 84c0                     test     al, al
0x14031b7c9: 0f8454fdffff             je       0x14031b523
0x14031b7cf: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b7d3: ff15473c0300             call     qword ptr [rip + 0x33c47]
0x14031b7d9: 89431c                   mov      dword ptr [rbx + 0x1c], eax
0x14031b7dc: e9cc000000               jmp      0x14031b8ad
0x14031b7e1: 33c9                     xor      ecx, ecx
0x14031b7e3: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b7e7: 4c8d0592c51e00           lea      r8, [rip + 0x1ec592]
0x14031b7ee: 6690                     nop      
0x14031b7f0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b7f4: 48ffc1                   inc      rcx
0x14031b7f7: 413a4408ff               cmp      al, byte ptr [r8 + rcx - 1]
0x14031b7fc: 7530                     jne      0x14031b82e
0x14031b7fe: 4883f908                 cmp      rcx, 8
0x14031b802: 75ec                     jne      0x14031b7f0
0x14031b804: 4c8d054a0b0500           lea      r8, [rip + 0x50b4a]
0x14031b80b: 498bd7                   mov      rdx, r15
0x14031b80e: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b812: e809acfbff               call     0x1402d6420
0x14031b817: 84c0                     test     al, al
0x14031b819: 0f8404fdffff             je       0x14031b523
0x14031b81f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b823: ff15f73b0300             call     qword ptr [rip + 0x33bf7]
0x14031b829: 894320                   mov      dword ptr [rbx + 0x20], eax
0x14031b82c: eb7f                     jmp      0x14031b8ad
0x14031b82e: 488d1553c51e00           lea      rdx, [rip + 0x1ec553]
0x14031b835: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b839: e8fab30200               call     0x140346c38
0x14031b83e: 85c0                     test     eax, eax
0x14031b840: 752a                     jne      0x14031b86c
0x14031b842: 4c8d050c0b0500           lea      r8, [rip + 0x50b0c]
0x14031b849: 498bd7                   mov      rdx, r15
0x14031b84c: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b850: e8cbabfbff               call     0x1402d6420
0x14031b855: 84c0                     test     al, al
0x14031b857: 0f84c6fcffff             je       0x14031b523
0x14031b85d: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b861: ff15b93b0300             call     qword ptr [rip + 0x33bb9]
0x14031b867: 894324                   mov      dword ptr [rbx + 0x24], eax
0x14031b86a: eb41                     jmp      0x14031b8ad
0x14031b86c: 33c9                     xor      ecx, ecx
0x14031b86e: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b872: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031b876: 48ffc1                   inc      rcx
0x14031b879: 3a440fff                 cmp      al, byte ptr [rdi + rcx - 1]
0x14031b87d: 752e                     jne      0x14031b8ad
0x14031b87f: 4883f907                 cmp      rcx, 7
0x14031b883: 75ed                     jne      0x14031b872
0x14031b885: 4c8d05c90a0500           lea      r8, [rip + 0x50ac9]
0x14031b88c: 498bd7                   mov      rdx, r15
0x14031b88f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b893: e888abfbff               call     0x1402d6420
0x14031b898: 84c0                     test     al, al
0x14031b89a: 0f8483fcffff             je       0x14031b523
0x14031b8a0: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031b8a4: ff15763b0300             call     qword ptr [rip + 0x33b76]
0x14031b8aa: 884328                   mov      byte ptr [rbx + 0x28], al
0x14031b8ad: 498b0f                   mov      rcx, qword ptr [r15]
0x14031b8b0: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031b8b4: e8e7b30000               call     0x140326ca0
0x14031b8b9: 807db023                 cmp      byte ptr [rbp - 0x50], 0x23
0x14031b8bd: 0f85adfcffff             jne      0x14031b570
0x14031b8c3: e97d270000               jmp      0x14031e045
