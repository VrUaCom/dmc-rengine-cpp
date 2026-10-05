; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031c2fa, 0x14031c5f6); code slice, not necessarily one unwind entry
0x14031c2fa: 498b0f                   mov      rcx, qword ptr [r15]
0x14031c2fd: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c301: 488b7358                 mov      rsi, qword ptr [rbx + 0x58]
0x14031c305: e896a90000               call     0x140326ca0
0x14031c30a: 807db023                 cmp      byte ptr [rbp - 0x50], 0x23
0x14031c30e: 0f84311d0000             je       0x14031e045
0x14031c314: 488d1d15ba1e00           lea      rbx, [rip + 0x1eba15]
0x14031c31b: 488d3d1eba1e00           lea      rdi, [rip + 0x1eba1e]
0x14031c322: 4c8d35cbba1e00           lea      r14, [rip + 0x1ebacb]
0x14031c329: 4c8d25ccba1e00           lea      r12, [rip + 0x1ebacc]
0x14031c330: 4c8d2dddba1e00           lea      r13, [rip + 0x1ebadd]
0x14031c337: 660f1f840000000000       nop      word ptr [rax + rax]
0x14031c340: 4c8d050e000500           lea      r8, [rip + 0x5000e]
0x14031c347: 498bd7                   mov      rdx, r15
0x14031c34a: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c34e: e8cda0fbff               call     0x1402d6420
0x14031c353: 84c0                     test     al, al
0x14031c355: 0f84c8f1ffff             je       0x14031b523
0x14031c35b: 33c9                     xor      ecx, ecx
0x14031c35d: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c361: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c365: 48ffc1                   inc      rcx
0x14031c368: 3a440bff                 cmp      al, byte ptr [rbx + rcx - 1]
0x14031c36c: 7559                     jne      0x14031c3c7
0x14031c36e: 4883f906                 cmp      rcx, 6
0x14031c372: 75ed                     jne      0x14031c361
0x14031c374: 33db                     xor      ebx, ebx
0x14031c376: 488d7e18                 lea      rdi, [rsi + 0x18]
0x14031c37a: 660f1f440000             nop      word ptr [rax + rax]
0x14031c380: 4c8d05ceff0400           lea      r8, [rip + 0x4ffce]
0x14031c387: 498bd7                   mov      rdx, r15
0x14031c38a: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c38e: e88da0fbff               call     0x1402d6420
0x14031c393: 84c0                     test     al, al
0x14031c395: 0f8488f1ffff             je       0x14031b523
0x14031c39b: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c39f: ff157b300300             call     qword ptr [rip + 0x3307b]
0x14031c3a5: 8907                     mov      dword ptr [rdi], eax
0x14031c3a7: 48ffc3                   inc      rbx
0x14031c3aa: 4883c704                 add      rdi, 4
0x14031c3ae: 4883fb02                 cmp      rbx, 2
0x14031c3b2: 7ccc                     jl       0x14031c380
0x14031c3b4: 488d1d75b91e00           lea      rbx, [rip + 0x1eb975]
0x14031c3bb: 488d3d7eb91e00           lea      rdi, [rip + 0x1eb97e]
0x14031c3c2: e914020000               jmp      0x14031c5db
0x14031c3c7: 33c9                     xor      ecx, ecx
0x14031c3c9: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c3cd: 0f1f00                   nop      dword ptr [rax]
0x14031c3d0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c3d4: 48ffc1                   inc      rcx
0x14031c3d7: 413a440eff               cmp      al, byte ptr [r14 + rcx - 1]
0x14031c3dc: 7532                     jne      0x14031c410
0x14031c3de: 4883f907                 cmp      rcx, 7
0x14031c3e2: 75ec                     jne      0x14031c3d0
0x14031c3e4: 4c8d056aff0400           lea      r8, [rip + 0x4ff6a]
0x14031c3eb: 498bd7                   mov      rdx, r15
0x14031c3ee: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c3f2: e829a0fbff               call     0x1402d6420
0x14031c3f7: 84c0                     test     al, al
0x14031c3f9: 0f8424f1ffff             je       0x14031b523
0x14031c3ff: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c403: ff1517300300             call     qword ptr [rip + 0x33017]
0x14031c409: 8906                     mov      dword ptr [rsi], eax
0x14031c40b: e9cb010000               jmp      0x14031c5db
0x14031c410: 33c9                     xor      ecx, ecx
0x14031c412: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c416: 66660f1f840000000000     nop      word ptr [rax + rax]
0x14031c420: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c424: 48ffc1                   inc      rcx
0x14031c427: 413a440cff               cmp      al, byte ptr [r12 + rcx - 1]
0x14031c42c: 7533                     jne      0x14031c461
0x14031c42e: 4883f907                 cmp      rcx, 7
0x14031c432: 75ec                     jne      0x14031c420
0x14031c434: 4c8d051aff0400           lea      r8, [rip + 0x4ff1a]
0x14031c43b: 498bd7                   mov      rdx, r15
0x14031c43e: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c442: e8d99ffbff               call     0x1402d6420
0x14031c447: 84c0                     test     al, al
0x14031c449: 0f84d4f0ffff             je       0x14031b523
0x14031c44f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c453: ff15c72f0300             call     qword ptr [rip + 0x32fc7]
0x14031c459: 894604                   mov      dword ptr [rsi + 4], eax
0x14031c45c: e97a010000               jmp      0x14031c5db
0x14031c461: 488d15a0b91e00           lea      rdx, [rip + 0x1eb9a0]
0x14031c468: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c46c: e8c7a70200               call     0x140346c38
0x14031c471: 85c0                     test     eax, eax
0x14031c473: 752d                     jne      0x14031c4a2
0x14031c475: 4c8d05d9fe0400           lea      r8, [rip + 0x4fed9]
0x14031c47c: 498bd7                   mov      rdx, r15
0x14031c47f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c483: e8989ffbff               call     0x1402d6420
0x14031c488: 84c0                     test     al, al
0x14031c48a: 0f8493f0ffff             je       0x14031b523
0x14031c490: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c494: ff15862f0300             call     qword ptr [rip + 0x32f86]
0x14031c49a: 894608                   mov      dword ptr [rsi + 8], eax
0x14031c49d: e939010000               jmp      0x14031c5db
0x14031c4a2: 33c9                     xor      ecx, ecx
0x14031c4a4: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c4a8: 0f1f840000000000         nop      dword ptr [rax + rax]
0x14031c4b0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c4b4: 48ffc1                   inc      rcx
0x14031c4b7: 423a4429ff               cmp      al, byte ptr [rcx + r13 - 1]
0x14031c4bc: 7536                     jne      0x14031c4f4
0x14031c4be: 4883f905                 cmp      rcx, 5
0x14031c4c2: 75ec                     jne      0x14031c4b0
0x14031c4c4: 4c8d058afe0400           lea      r8, [rip + 0x4fe8a]
0x14031c4cb: 498bd7                   mov      rdx, r15
0x14031c4ce: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c4d2: e8499ffbff               call     0x1402d6420
0x14031c4d7: 84c0                     test     al, al
0x14031c4d9: 0f8444f0ffff             je       0x14031b523
0x14031c4df: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c4e3: ff15372f0300             call     qword ptr [rip + 0x32f37]
0x14031c4e9: 0fb6c8                   movzx    ecx, al
0x14031c4ec: 894e0c                   mov      dword ptr [rsi + 0xc], ecx
0x14031c4ef: e9e7000000               jmp      0x14031c5db
0x14031c4f4: 33c9                     xor      ecx, ecx
0x14031c4f6: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c4fa: 4c8d051fb91e00           lea      r8, [rip + 0x1eb91f]
0x14031c501: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c505: 48ffc1                   inc      rcx
0x14031c508: 413a4408ff               cmp      al, byte ptr [r8 + rcx - 1]
0x14031c50d: 7533                     jne      0x14031c542
0x14031c50f: 4883f908                 cmp      rcx, 8
0x14031c513: 75ec                     jne      0x14031c501
0x14031c515: 4c8d0539fe0400           lea      r8, [rip + 0x4fe39]
0x14031c51c: 498bd7                   mov      rdx, r15
0x14031c51f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c523: e8f89efbff               call     0x1402d6420
0x14031c528: 84c0                     test     al, al
0x14031c52a: 0f84f3efffff             je       0x14031b523
0x14031c530: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c534: ff15e62e0300             call     qword ptr [rip + 0x32ee6]
0x14031c53a: 894610                   mov      dword ptr [rsi + 0x10], eax
0x14031c53d: e999000000               jmp      0x14031c5db
0x14031c542: 33c9                     xor      ecx, ecx
0x14031c544: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c548: 4c8d05d9b81e00           lea      r8, [rip + 0x1eb8d9]
0x14031c54f: 90                       nop      
0x14031c550: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c554: 48ffc1                   inc      rcx
0x14031c557: 413a4408ff               cmp      al, byte ptr [r8 + rcx - 1]
0x14031c55c: 7530                     jne      0x14031c58e
0x14031c55e: 4883f906                 cmp      rcx, 6
0x14031c562: 75ec                     jne      0x14031c550
0x14031c564: 4c8d05eafd0400           lea      r8, [rip + 0x4fdea]
0x14031c56b: 498bd7                   mov      rdx, r15
0x14031c56e: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c572: e8a99efbff               call     0x1402d6420
0x14031c577: 84c0                     test     al, al
0x14031c579: 0f84a4efffff             je       0x14031b523
0x14031c57f: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c583: ff15972e0300             call     qword ptr [rip + 0x32e97]
0x14031c589: 894614                   mov      dword ptr [rsi + 0x14], eax
0x14031c58c: eb4d                     jmp      0x14031c5db
0x14031c58e: 33c9                     xor      ecx, ecx
0x14031c590: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c594: 0f1f4000                 nop      dword ptr [rax]
0x14031c598: 0f1f840000000000         nop      dword ptr [rax + rax]
0x14031c5a0: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x14031c5a4: 48ffc1                   inc      rcx
0x14031c5a7: 3a440fff                 cmp      al, byte ptr [rdi + rcx - 1]
0x14031c5ab: 752e                     jne      0x14031c5db
0x14031c5ad: 4883f907                 cmp      rcx, 7
0x14031c5b1: 75ed                     jne      0x14031c5a0
0x14031c5b3: 4c8d059bfd0400           lea      r8, [rip + 0x4fd9b]
0x14031c5ba: 498bd7                   mov      rdx, r15
0x14031c5bd: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c5c1: e85a9efbff               call     0x1402d6420
0x14031c5c6: 84c0                     test     al, al
0x14031c5c8: 0f8455efffff             je       0x14031b523
0x14031c5ce: 488d4db0                 lea      rcx, [rbp - 0x50]
0x14031c5d2: ff15482e0300             call     qword ptr [rip + 0x32e48]
0x14031c5d8: 884620                   mov      byte ptr [rsi + 0x20], al
0x14031c5db: 498b0f                   mov      rcx, qword ptr [r15]
0x14031c5de: 488d55b0                 lea      rdx, [rbp - 0x50]
0x14031c5e2: e8b9a60000               call     0x140326ca0
0x14031c5e7: 807db023                 cmp      byte ptr [rbp - 0x50], 0x23
0x14031c5eb: 0f854ffdffff             jne      0x14031c340
0x14031c5f1: e94f1a0000               jmp      0x14031e045
