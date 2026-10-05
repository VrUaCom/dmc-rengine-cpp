; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402da750, 0x1402da9d0); code slice, not necessarily one unwind entry
0x1402da750: 48895c2408               mov      qword ptr [rsp + 8], rbx
0x1402da755: 55                       push     rbp
0x1402da756: 56                       push     rsi
0x1402da757: 57                       push     rdi
0x1402da758: 4154                     push     r12
0x1402da75a: 4155                     push     r13
0x1402da75c: 4156                     push     r14
0x1402da75e: 4157                     push     r15
0x1402da760: 488d6c2490               lea      rbp, [rsp - 0x70]
0x1402da765: 4881ec70010000           sub      rsp, 0x170
0x1402da76c: 0f29b42460010000         movaps   xmmword ptr [rsp + 0x160], xmm6
0x1402da774: 488b0535a92f00           mov      rax, qword ptr [rip + 0x2fa935]
0x1402da77b: 4833c4                   xor      rax, rsp
0x1402da77e: 48894550                 mov      qword ptr [rbp + 0x50], rax
0x1402da782: 33c0                     xor      eax, eax
0x1402da784: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da789: 4889442430               mov      qword ptr [rsp + 0x30], rax
0x1402da78e: 4d8be0                   mov      r12, r8
0x1402da791: 4889442438               mov      qword ptr [rsp + 0x38], rax
0x1402da796: 488bda                   mov      rbx, rdx
0x1402da799: 4889442440               mov      qword ptr [rsp + 0x40], rax
0x1402da79e: e80d850400               call     0x140322cb0
0x1402da7a3: 488bd3                   mov      rdx, rbx
0x1402da7a6: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da7ab: e8f0840400               call     0x140322ca0
0x1402da7b0: 4533f6                   xor      r14d, r14d
0x1402da7b3: 4c8d2d56c22200           lea      r13, [rip + 0x22c256]
0x1402da7ba: 458bfe                   mov      r15d, r14d
0x1402da7bd: 0f57f6                   xorps    xmm6, xmm6
0x1402da7c0: 488d3dd9c22200           lea      rdi, [rip + 0x22c2d9]
0x1402da7c7: 660f1f840000000000       nop      word ptr [rax + rax]
0x1402da7d0: 488d542450               lea      rdx, [rsp + 0x50]
0x1402da7d5: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da7da: e8d1820400               call     0x140322ab0
0x1402da7df: 0fb65c2450               movzx    ebx, byte ptr [rsp + 0x50]
0x1402da7e4: 80fb23                   cmp      bl, 0x23
0x1402da7e7: 7570                     jne      0x1402da859
0x1402da7e9: 488d542450               lea      rdx, [rsp + 0x50]
0x1402da7ee: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da7f3: e8b8820400               call     0x140322ab0
0x1402da7f8: 498bce                   mov      rcx, r14
0x1402da7fb: 488d542450               lea      rdx, [rsp + 0x50]
0x1402da800: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402da804: 48ffc1                   inc      rcx
0x1402da807: 423a4429ff               cmp      al, byte ptr [rcx + r13 - 1]
0x1402da80c: 0f858b010000             jne      0x1402da99d
0x1402da812: 4883f905                 cmp      rcx, 5
0x1402da816: 75e8                     jne      0x1402da800
0x1402da818: 498bcc                   mov      rcx, r12
0x1402da81b: e890970400               call     0x140323fb0
0x1402da820: 4c8bf8                   mov      r15, rax
0x1402da823: 4885c0                   test     rax, rax
0x1402da826: 75a8                     jne      0x1402da7d0
0x1402da828: 32c0                     xor      al, al
0x1402da82a: 488b4d50                 mov      rcx, qword ptr [rbp + 0x50]
0x1402da82e: 4833cc                   xor      rcx, rsp
0x1402da831: e8baad0600               call     0x1403455f0
0x1402da836: 488b9c24b0010000         mov      rbx, qword ptr [rsp + 0x1b0]
0x1402da83e: 0f28b42460010000         movaps   xmm6, xmmword ptr [rsp + 0x160]
0x1402da846: 4881c470010000           add      rsp, 0x170
0x1402da84d: 415f                     pop      r15
0x1402da84f: 415e                     pop      r14
0x1402da851: 415d                     pop      r13
0x1402da853: 415c                     pop      r12
0x1402da855: 5f                       pop      rdi
0x1402da856: 5e                       pop      rsi
0x1402da857: 5d                       pop      rbp
0x1402da858: c3                       ret      
0x1402da859: 488d15b8c12200           lea      rdx, [rip + 0x22c1b8]
0x1402da860: 488d4c2450               lea      rcx, [rsp + 0x50]
0x1402da865: e8cec30600               call     0x140346c38
0x1402da86a: 85c0                     test     eax, eax
0x1402da86c: 7524                     jne      0x1402da892
0x1402da86e: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da873: e898810400               call     0x140322a10
0x1402da878: f30f2cc0                 cvttss2si eax, xmm0
0x1402da87c: 660f6ec8                 movd     xmm1, eax
0x1402da880: 0f5bc9                   cvtdq2ps xmm1, xmm1
0x1402da883: f30f5fce                 maxss    xmm1, xmm6
0x1402da887: f3410f114f10             movss    dword ptr [r15 + 0x10], xmm1
0x1402da88d: e93effffff               jmp      0x1402da7d0
0x1402da892: 488d158fc12200           lea      rdx, [rip + 0x22c18f]
0x1402da899: 488d4c2450               lea      rcx, [rsp + 0x50]
0x1402da89e: e895c30600               call     0x140346c38
0x1402da8a3: 85c0                     test     eax, eax
0x1402da8a5: 7515                     jne      0x1402da8bc
0x1402da8a7: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da8ac: e85f810400               call     0x140322a10
0x1402da8b1: f3410f114714             movss    dword ptr [r15 + 0x14], xmm0
0x1402da8b7: e914ffffff               jmp      0x1402da7d0
0x1402da8bc: 488d1575c12200           lea      rdx, [rip + 0x22c175]
0x1402da8c3: 488d4c2450               lea      rcx, [rsp + 0x50]
0x1402da8c8: e86bc30600               call     0x140346c38
0x1402da8cd: 85c0                     test     eax, eax
0x1402da8cf: 7515                     jne      0x1402da8e6
0x1402da8d1: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da8d6: e835810400               call     0x140322a10
0x1402da8db: f3410f114718             movss    dword ptr [r15 + 0x18], xmm0
0x1402da8e1: e9eafeffff               jmp      0x1402da7d0
0x1402da8e6: 488d155bc12200           lea      rdx, [rip + 0x22c15b]
0x1402da8ed: 488d4c2450               lea      rcx, [rsp + 0x50]
0x1402da8f2: e841c30600               call     0x140346c38
0x1402da8f7: 85c0                     test     eax, eax
0x1402da8f9: 7515                     jne      0x1402da910
0x1402da8fb: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da900: e80b810400               call     0x140322a10
0x1402da905: f3410f11471c             movss    dword ptr [r15 + 0x1c], xmm0
0x1402da90b: e9c0feffff               jmp      0x1402da7d0
0x1402da910: 3a1d86c12200             cmp      bl, byte ptr [rip + 0x22c186]
0x1402da916: 752d                     jne      0x1402da945
0x1402da918: 0fb6442451               movzx    eax, byte ptr [rsp + 0x51]
0x1402da91d: 3a057ac12200             cmp      al, byte ptr [rip + 0x22c17a]
0x1402da923: 7520                     jne      0x1402da945
0x1402da925: 0fb6442452               movzx    eax, byte ptr [rsp + 0x52]
0x1402da92a: 3a056ec12200             cmp      al, byte ptr [rip + 0x22c16e]
0x1402da930: 7513                     jne      0x1402da945
0x1402da932: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da937: e824810400               call     0x140322a60
0x1402da93c: 41894730                 mov      dword ptr [r15 + 0x30], eax
0x1402da940: e98bfeffff               jmp      0x1402da7d0
0x1402da945: 498bce                   mov      rcx, r14
0x1402da948: 488d542450               lea      rdx, [rsp + 0x50]
0x1402da94d: 0f1f00                   nop      dword ptr [rax]
0x1402da950: 0fb6040a                 movzx    eax, byte ptr [rdx + rcx]
0x1402da954: 48ffc1                   inc      rcx
0x1402da957: 3a440fff                 cmp      al, byte ptr [rdi + rcx - 1]
0x1402da95b: 0f856ffeffff             jne      0x1402da7d0
0x1402da961: 4883f904                 cmp      rcx, 4
0x1402da965: 75e9                     jne      0x1402da950
0x1402da967: 488d442420               lea      rax, [rsp + 0x20]
0x1402da96c: 498bf7                   mov      rsi, r15
0x1402da96f: 482bf0                   sub      rsi, rax
0x1402da972: 488d5c2420               lea      rbx, [rsp + 0x20]
0x1402da977: 488bf9                   mov      rdi, rcx
0x1402da97a: 660f1f440000             nop      word ptr [rax + rax]
0x1402da980: 488d4c2430               lea      rcx, [rsp + 0x30]
0x1402da985: e8d6800400               call     0x140322a60
0x1402da98a: 89441e34                 mov      dword ptr [rsi + rbx + 0x34], eax
0x1402da98e: 4883c304                 add      rbx, 4
0x1402da992: 4883ef01                 sub      rdi, 1
0x1402da996: 75e8                     jne      0x1402da980
0x1402da998: e923feffff               jmp      0x1402da7c0
0x1402da99d: 488d4c2450               lea      rcx, [rsp + 0x50]
0x1402da9a2: 488d159fbe2200           lea      rdx, [rip + 0x22be9f]
0x1402da9a9: 0f1f8000000000           nop      dword ptr [rax]
0x1402da9b0: 420fb60431               movzx    eax, byte ptr [rcx + r14]
0x1402da9b5: 49ffc6                   inc      r14
0x1402da9b8: 423a4432ff               cmp      al, byte ptr [rdx + r14 - 1]
0x1402da9bd: 0f8565feffff             jne      0x1402da828
0x1402da9c3: 4983fe04                 cmp      r14, 4
0x1402da9c7: 75e7                     jne      0x1402da9b0
0x1402da9c9: b001                     mov      al, 1
0x1402da9cb: e95afeffff               jmp      0x1402da82a
