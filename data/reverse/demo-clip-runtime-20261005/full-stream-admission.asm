; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031ee00, 0x14031f044); contiguous code slice, may span split unwind fragments
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
0x14031eeae: 0fb6562c                 movzx    edx, byte ptr [rsi + 0x2c]
0x14031eeb2: 4533c0                   xor      r8d, r8d
0x14031eeb5: 488bcb                   mov      rcx, rbx
0x14031eeb8: e853feffff               call     0x14031ed10
0x14031eebd: 664489762c               mov      word ptr [rsi + 0x2c], r14w
0x14031eec2: 0fbf4354                 movsx    eax, word ptr [rbx + 0x54]
0x14031eec6: ffc7                     inc      edi
0x14031eec8: 3bf8                     cmp      edi, eax
0x14031eeca: 7cd4                     jl       0x14031eea0
0x14031eecc: 450fbfed                 movsx    r13d, r13w
0x14031eed0: 4585ed                   test     r13d, r13d
0x14031eed3: 0f8e34010000             jle      0x14031f00d
0x14031eed9: 0f1f8000000000           nop      dword ptr [rax]
0x14031eee0: 440fb703                 movzx    r8d, word ptr [rbx]
0x14031eee4: 66453bc4                 cmp      r8w, r12w
0x14031eee8: 7e66                     jle      0x14031ef50
0x14031eeea: 488b93f8000000           mov      rdx, qword ptr [rbx + 0xf8]
0x14031eef1: 4885d2                   test     rdx, rdx
0x14031eef4: 7421                     je       0x14031ef17
0x14031eef6: 418d4701                 lea      eax, [r15 + 1]
0x14031eefa: 394204                   cmp      dword ptr [rdx + 4], eax
0x14031eefd: 7218                     jb       0x14031ef17
0x14031eeff: 418d4702                 lea      eax, [r15 + 2]
0x14031ef03: 8bc8                     mov      ecx, eax
0x14031ef05: 8b0482                   mov      eax, dword ptr [rdx + rax*4]
0x14031ef08: 85c0                     test     eax, eax
0x14031ef0a: 740b                     je       0x14031ef17
0x14031ef0c: 4803c2                   add      rax, rdx
0x14031ef0f: 7406                     je       0x14031ef17
0x14031ef11: 0fb74804                 movzx    ecx, word ptr [rax + 4]
0x14031ef15: eb03                     jmp      0x14031ef1a
0x14031ef17: 418bce                   mov      ecx, r14d
0x14031ef1a: 663bcd                   cmp      cx, bp
0x14031ef1d: 7f07                     jg       0x14031ef26
0x14031ef1f: bffeffffff               mov      edi, 0xfffffffe
0x14031ef24: eb2d                     jmp      0x14031ef53
0x14031ef26: 4885d2                   test     rdx, rdx
0x14031ef29: 7425                     je       0x14031ef50
0x14031ef2b: 418d4701                 lea      eax, [r15 + 1]
0x14031ef2f: 394204                   cmp      dword ptr [rdx + 4], eax
0x14031ef32: 721c                     jb       0x14031ef50
0x14031ef34: 418d4702                 lea      eax, [r15 + 2]
0x14031ef38: 8b0482                   mov      eax, dword ptr [rdx + rax*4]
0x14031ef3b: 85c0                     test     eax, eax
0x14031ef3d: 7411                     je       0x14031ef50
0x14031ef3f: 8bc8                     mov      ecx, eax
0x14031ef41: 4803ca                   add      rcx, rdx
0x14031ef44: 740a                     je       0x14031ef50
0x14031ef46: 480fbfc5                 movsx    rax, bp
0x14031ef4a: 8b7cc108                 mov      edi, dword ptr [rcx + rax*8 + 8]
0x14031ef4e: eb03                     jmp      0x14031ef53
0x14031ef50: 418bfe                   mov      edi, r14d
0x14031ef53: 66453bc4                 cmp      r8w, r12w
0x14031ef57: 0f8ee2000000             jle      0x14031f03f
0x14031ef5d: 488b93f8000000           mov      rdx, qword ptr [rbx + 0xf8]
0x14031ef64: 4885d2                   test     rdx, rdx
0x14031ef67: 7421                     je       0x14031ef8a
0x14031ef69: 418d4701                 lea      eax, [r15 + 1]
0x14031ef6d: 394204                   cmp      dword ptr [rdx + 4], eax
0x14031ef70: 7218                     jb       0x14031ef8a
0x14031ef72: 418d4702                 lea      eax, [r15 + 2]
0x14031ef76: 8bc8                     mov      ecx, eax
0x14031ef78: 8b0482                   mov      eax, dword ptr [rdx + rax*4]
0x14031ef7b: 85c0                     test     eax, eax
0x14031ef7d: 740b                     je       0x14031ef8a
0x14031ef7f: 4803c2                   add      rax, rdx
0x14031ef82: 7406                     je       0x14031ef8a
0x14031ef84: 0fb74804                 movzx    ecx, word ptr [rax + 4]
0x14031ef88: eb03                     jmp      0x14031ef8d
0x14031ef8a: 418bce                   mov      ecx, r14d
0x14031ef8d: 663bcd                   cmp      cx, bp
0x14031ef90: 0f8ea9000000             jle      0x14031f03f
0x14031ef96: 4885d2                   test     rdx, rdx
0x14031ef99: 0f84a0000000             je       0x14031f03f
0x14031ef9f: 418d4701                 lea      eax, [r15 + 1]
0x14031efa3: 394204                   cmp      dword ptr [rdx + 4], eax
0x14031efa6: 0f8293000000             jb       0x14031f03f
0x14031efac: 418d4702                 lea      eax, [r15 + 2]
0x14031efb0: 8b0482                   mov      eax, dword ptr [rdx + rax*4]
0x14031efb3: 85c0                     test     eax, eax
0x14031efb5: 0f8484000000             je       0x14031f03f
0x14031efbb: 8bc8                     mov      ecx, eax
0x14031efbd: 4803ca                   add      rcx, rdx
0x14031efc0: 747d                     je       0x14031f03f
0x14031efc2: 480fbfc5                 movsx    rax, bp
0x14031efc6: 8b74c10c                 mov      esi, dword ptr [rcx + rax*8 + 0xc]
0x14031efca: 4803f1                   add      rsi, rcx
0x14031efcd: 7470                     je       0x14031f03f
0x14031efcf: 4533c0                   xor      r8d, r8d
0x14031efd2: 400fb6d7                 movzx    edx, dil
0x14031efd6: 488bcb                   mov      rcx, rbx
0x14031efd9: e872faffff               call     0x14031ea50
0x14031efde: 85c0                     test     eax, eax
0x14031efe0: 7807                     js       0x14031efe9
0x14031efe2: 4898                     cdqe     
0x14031efe4: 66897c432c               mov      word ptr [rbx + rax*2 + 0x2c], di
0x14031efe9: 41b101                   mov      r9b, 1
0x14031efec: c644242000               mov      byte ptr [rsp + 0x20], 0
0x14031eff1: 4c8bc6                   mov      r8, rsi
0x14031eff4: 8bd7                     mov      edx, edi
0x14031eff6: 488bcb                   mov      rcx, rbx
0x14031eff9: e852000000               call     0x14031f050
0x14031effe: 85c0                     test     eax, eax
0x14031f000: 753d                     jne      0x14031f03f
0x14031f002: ffc5                     inc      ebp
0x14031f004: 413bed                   cmp      ebp, r13d
0x14031f007: 0f8cd3feffff             jl       0x14031eee0
0x14031f00d: 8b442460                 mov      eax, dword ptr [rsp + 0x60]
0x14031f011: 66894354                 mov      word ptr [rbx + 0x54], ax
0x14031f015: 33c0                     xor      eax, eax
0x14031f017: 6644896302               mov      word ptr [rbx + 2], r12w
0x14031f01c: 488b742470               mov      rsi, qword ptr [rsp + 0x70]
0x14031f021: 488b7c2478               mov      rdi, qword ptr [rsp + 0x78]
0x14031f026: 4c8b6c2438               mov      r13, qword ptr [rsp + 0x38]
0x14031f02b: 488b6c2468               mov      rbp, qword ptr [rsp + 0x68]
0x14031f030: 4c8b7c2430               mov      r15, qword ptr [rsp + 0x30]
0x14031f035: 4883c440                 add      rsp, 0x40
0x14031f039: 415e                     pop      r14
0x14031f03b: 415c                     pop      r12
0x14031f03d: 5b                       pop      rbx
0x14031f03e: c3                       ret      
0x14031f03f: 418bc6                   mov      eax, r14d
0x14031f042: ebd8                     jmp      0x14031f01c
