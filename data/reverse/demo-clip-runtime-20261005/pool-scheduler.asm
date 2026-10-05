; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140323cf0, 0x140323ebc); contiguous code slice, may span split unwind fragments
0x140323cf0: 48896c2418               mov      qword ptr [rsp + 0x18], rbp
0x140323cf5: 48897c2420               mov      qword ptr [rsp + 0x20], rdi
0x140323cfa: 4156                     push     r14
0x140323cfc: 4883ec50                 sub      rsp, 0x50
0x140323d00: 488b01                   mov      rax, qword ptr [rcx]
0x140323d03: 33ed                     xor      ebp, ebp
0x140323d05: 440f29442420             movaps   xmmword ptr [rsp + 0x20], xmm8
0x140323d0b: 4532f6                   xor      r14b, r14b
0x140323d0e: 488bf9                   mov      rdi, rcx
0x140323d11: f3440f10401c             movss    xmm8, dword ptr [rax + 0x1c]
0x140323d17: f3440f584018             addss    xmm8, dword ptr [rax + 0x18]
0x140323d1d: 89691c                   mov      dword ptr [rcx + 0x1c], ebp
0x140323d20: 396918                   cmp      dword ptr [rcx + 0x18], ebp
0x140323d23: 0f8e78010000             jle      0x140323ea1
0x140323d29: 48895c2460               mov      qword ptr [rsp + 0x60], rbx
0x140323d2e: 4889742468               mov      qword ptr [rsp + 0x68], rsi
0x140323d33: 8bf5                     mov      esi, ebp
0x140323d35: 0f29742440               movaps   xmmword ptr [rsp + 0x40], xmm6
0x140323d3a: 0f297c2430               movaps   xmmword ptr [rsp + 0x30], xmm7
0x140323d3f: 90                       nop      
0x140323d40: 488b5f40                 mov      rbx, qword ptr [rdi + 0x40]
0x140323d44: 48837c1e0800             cmp      qword ptr [rsi + rbx + 8], 0
0x140323d4a: 0f842e010000             je       0x140323e7e
0x140323d50: 488b5c1e10               mov      rbx, qword ptr [rsi + rbx + 0x10]
0x140323d55: 8b4b08                   mov      ecx, dword ptr [rbx + 8]
0x140323d58: 83e901                   sub      ecx, 1
0x140323d5b: 0f84a0000000             je       0x140323e01
0x140323d61: 83e901                   sub      ecx, 1
0x140323d64: 7443                     je       0x140323da9
0x140323d66: 83f902                   cmp      ecx, 2
0x140323d69: 0f850f010000             jne      0x140323e7e
0x140323d6f: 488b03                   mov      rax, qword ptr [rbx]
0x140323d72: 488bcb                   mov      rcx, rbx
0x140323d75: f30f107b10               movss    xmm7, dword ptr [rbx + 0x10]
0x140323d7a: f30f10731c               movss    xmm6, dword ptr [rbx + 0x1c]
0x140323d7f: ff5018                   call     qword ptr [rax + 0x18]
0x140323d82: f30f59c6                 mulss    xmm0, xmm6
0x140323d86: f30f597318               mulss    xmm6, dword ptr [rbx + 0x18]
0x140323d8b: f30f5cc6                 subss    xmm0, xmm6
0x140323d8f: f30f58c7                 addss    xmm0, xmm7
0x140323d93: 440f2fc0                 comiss   xmm8, xmm0
0x140323d97: 0f86e1000000             jbe      0x140323e7e
0x140323d9d: c7430803000000           mov      dword ptr [rbx + 8], 3
0x140323da4: e9d5000000               jmp      0x140323e7e
0x140323da9: 488b03                   mov      rax, qword ptr [rbx]
0x140323dac: 488bcb                   mov      rcx, rbx
0x140323daf: f30f107b10               movss    xmm7, dword ptr [rbx + 0x10]
0x140323db4: f30f10731c               movss    xmm6, dword ptr [rbx + 0x1c]
0x140323db9: ff5018                   call     qword ptr [rax + 0x18]
0x140323dbc: 488b03                   mov      rax, qword ptr [rbx]
0x140323dbf: 488bcb                   mov      rcx, rbx
0x140323dc2: f30f59c6                 mulss    xmm0, xmm6
0x140323dc6: f30f597318               mulss    xmm6, dword ptr [rbx + 0x18]
0x140323dcb: f30f5cc6                 subss    xmm0, xmm6
0x140323dcf: f30f58c7                 addss    xmm0, xmm7
0x140323dd3: 440f2fc0                 comiss   xmm8, xmm0
0x140323dd7: 720f                     jb       0x140323de8
0x140323dd9: ff5010                   call     qword ptr [rax + 0x10]
0x140323ddc: c7430803000000           mov      dword ptr [rbx + 8], 3
0x140323de3: e996000000               jmp      0x140323e7e
0x140323de8: ff5008                   call     qword ptr [rax + 8]
0x140323deb: f30f104310               movss    xmm0, dword ptr [rbx + 0x10]
0x140323df0: 0f2f471c                 comiss   xmm0, dword ptr [rdi + 0x1c]
0x140323df4: 0f8684000000             jbe      0x140323e7e
0x140323dfa: f30f11471c               movss    dword ptr [rdi + 0x1c], xmm0
0x140323dff: eb7d                     jmp      0x140323e7e
0x140323e01: f30f104314               movss    xmm0, dword ptr [rbx + 0x14]
0x140323e06: f30f59431c               mulss    xmm0, dword ptr [rbx + 0x1c]
0x140323e0b: f30f107b10               movss    xmm7, dword ptr [rbx + 0x10]
0x140323e10: f30f58c7                 addss    xmm0, xmm7
0x140323e14: 440f2fc0                 comiss   xmm8, xmm0
0x140323e18: 7264                     jb       0x140323e7e
0x140323e1a: 488b03                   mov      rax, qword ptr [rbx]
0x140323e1d: 488bcb                   mov      rcx, rbx
0x140323e20: f30f10731c               movss    xmm6, dword ptr [rbx + 0x1c]
0x140323e25: ff5018                   call     qword ptr [rax + 0x18]
0x140323e28: f30f59c6                 mulss    xmm0, xmm6
0x140323e2c: f30f597318               mulss    xmm6, dword ptr [rbx + 0x18]
0x140323e31: f30f5cc6                 subss    xmm0, xmm6
0x140323e35: f30f58c7                 addss    xmm0, xmm7
0x140323e39: 410f2fc0                 comiss   xmm0, xmm8
0x140323e3d: 763f                     jbe      0x140323e7e
0x140323e3f: 488b07                   mov      rax, qword ptr [rdi]
0x140323e42: 83780400                 cmp      dword ptr [rax + 4], 0
0x140323e46: 7506                     jne      0x140323e4e
0x140323e48: 837f1007                 cmp      dword ptr [rdi + 0x10], 7
0x140323e4c: 7430                     je       0x140323e7e
0x140323e4e: 488b03                   mov      rax, qword ptr [rbx]
0x140323e51: 488bcb                   mov      rcx, rbx
0x140323e54: ff10                     call     qword ptr [rax]
0x140323e56: 3c01                     cmp      al, 1
0x140323e58: 751d                     jne      0x140323e77
0x140323e5a: f30f104310               movss    xmm0, dword ptr [rbx + 0x10]
0x140323e5f: 440fb6f0                 movzx    r14d, al
0x140323e63: c7430802000000           mov      dword ptr [rbx + 8], 2
0x140323e6a: 0f2f471c                 comiss   xmm0, dword ptr [rdi + 0x1c]
0x140323e6e: 760e                     jbe      0x140323e7e
0x140323e70: f30f11471c               movss    dword ptr [rdi + 0x1c], xmm0
0x140323e75: eb07                     jmp      0x140323e7e
0x140323e77: c7430804000000           mov      dword ptr [rbx + 8], 4
0x140323e7e: ffc5                     inc      ebp
0x140323e80: 4883c620                 add      rsi, 0x20
0x140323e84: 3b6f18                   cmp      ebp, dword ptr [rdi + 0x18]
0x140323e87: 0f8cb3feffff             jl       0x140323d40
0x140323e8d: 0f287c2430               movaps   xmm7, xmmword ptr [rsp + 0x30]
0x140323e92: 0f28742440               movaps   xmm6, xmmword ptr [rsp + 0x40]
0x140323e97: 488b742468               mov      rsi, qword ptr [rsp + 0x68]
0x140323e9c: 488b5c2460               mov      rbx, qword ptr [rsp + 0x60]
0x140323ea1: 488b6c2470               mov      rbp, qword ptr [rsp + 0x70]
0x140323ea6: 410fb6c6                 movzx    eax, r14b
0x140323eaa: 488b7c2478               mov      rdi, qword ptr [rsp + 0x78]
0x140323eaf: 440f28442420             movaps   xmm8, xmmword ptr [rsp + 0x20]
0x140323eb5: 4883c450                 add      rsp, 0x50
0x140323eb9: 415e                     pop      r14
0x140323ebb: c3                       ret      
