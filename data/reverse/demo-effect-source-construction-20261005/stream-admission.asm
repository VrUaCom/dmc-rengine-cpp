; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031ee00, 0x14031eeb0); code slice, not necessarily one unwind entry
0x14031ee00: 4053                     push     rbx
0x14031ee02: 4154                     push     r12
0x14031ee04: 4156                     push     r14
0x14031ee06: 4883ec40                 sub      rsp, 0x40
0x14031ee0a: 4c8b81f8000000           mov      r8, qword ptr [rcx + 0xf8]
0x14031ee11: 440fb7e2                 movzx    r12d, dx
0x14031ee15: 488bd9                   mov      rbx, rcx
0x14031ee18: 4d85c0                   test     r8, r8
0x14031ee1b: 750d                     jne      0x14031ee2a
0x14031ee1d: 83c8ff                   or       eax, 0xffffffff
0x14031ee20: 4883c440                 add      rsp, 0x40
0x14031ee24: 415e                     pop      r14
0x14031ee26: 415c                     pop      r12
0x14031ee28: 5b                       pop      rbx
0x14031ee29: c3                       ret      
0x14031ee2a: 66443921                 cmp      word ptr [rcx], r12w
0x14031ee2e: 7f0f                     jg       0x14031ee3f
0x14031ee30: b8feffffff               mov      eax, 0xfffffffe
0x14031ee35: 4883c440                 add      rsp, 0x40
0x14031ee39: 415e                     pop      r14
0x14031ee3b: 415c                     pop      r12
0x14031ee3d: 5b                       pop      rbx
0x14031ee3e: c3                       ret      
0x14031ee3f: 48896c2468               mov      qword ptr [rsp + 0x68], rbp
0x14031ee44: 4183ceff                 or       r14d, 0xffffffff
0x14031ee48: 48897c2478               mov      qword ptr [rsp + 0x78], rdi
0x14031ee4d: 4c896c2438               mov      qword ptr [rsp + 0x38], r13
0x14031ee52: 4c897c2430               mov      qword ptr [rsp + 0x30], r15
0x14031ee57: 450fbffc                 movsx    r15d, r12w
0x14031ee5b: 418d4701                 lea      eax, [r15 + 1]
0x14031ee5f: 41394004                 cmp      dword ptr [r8 + 4], eax
0x14031ee63: 721d                     jb       0x14031ee82
0x14031ee65: 418d4702                 lea      eax, [r15 + 2]
0x14031ee69: 418b0480                 mov      eax, dword ptr [r8 + rax*4]
0x14031ee6d: 85c0                     test     eax, eax
0x14031ee6f: 7411                     je       0x14031ee82
0x14031ee71: 4903c0                   add      rax, r8
0x14031ee74: 740c                     je       0x14031ee82
0x14031ee76: 440fb76804               movzx    r13d, word ptr [rax + 4]
0x14031ee7b: 44896c2460               mov      dword ptr [rsp + 0x60], r13d
0x14031ee80: eb08                     jmp      0x14031ee8a
0x14031ee82: 458bee                   mov      r13d, r14d
0x14031ee85: 4489742460               mov      dword ptr [rsp + 0x60], r14d
0x14031ee8a: 33ed                     xor      ebp, ebp
0x14031ee8c: 4889742470               mov      qword ptr [rsp + 0x70], rsi
0x14031ee91: 8bfd                     mov      edi, ebp
0x14031ee93: 663b6954                 cmp      bp, word ptr [rcx + 0x54]
0x14031ee97: 7d33                     jge      0x14031eecc
0x14031ee99: 0f1f8000000000           nop      dword ptr [rax]
0x14031eea0: 4863c7                   movsxd   rax, edi
0x14031eea3: 66396c432c               cmp      word ptr [rbx + rax*2 + 0x2c], bp
0x14031eea8: 488d3443                 lea      rsi, [rbx + rax*2]
0x14031eeac: 7c14                     jl       0x14031eec2
