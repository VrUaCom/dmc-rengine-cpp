; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140326ca0, 0x140326d16); code slice, not necessarily one unwind entry
0x140326ca0: 4c8bd2                   mov      r10, rdx
0x140326ca3: 4533c9                   xor      r9d, r9d
0x140326ca6: 48ba0026000001000000     movabs   rdx, 0x100002600
0x140326cb0: 440fb601                 movzx    r8d, byte ptr [rcx]
0x140326cb4: 4180f820                 cmp      r8b, 0x20
0x140326cb8: 7714                     ja       0x140326cce
0x140326cba: 490fbec0                 movsx    rax, r8b
0x140326cbe: 480fa3c2                 bt       rdx, rax
0x140326cc2: 730a                     jae      0x140326cce
0x140326cc4: 4584c0                   test     r8b, r8b
0x140326cc7: 7447                     je       0x140326d10
0x140326cc9: 48ffc1                   inc      rcx
0x140326ccc: ebe2                     jmp      0x140326cb0
0x140326cce: 0fb611                   movzx    edx, byte ptr [rcx]
0x140326cd1: 84d2                     test     dl, dl
0x140326cd3: 743b                     je       0x140326d10
0x140326cd5: 80fa20                   cmp      dl, 0x20
0x140326cd8: 742a                     je       0x140326d04
0x140326cda: 4d8bc2                   mov      r8, r10
0x140326cdd: 41bb01260000             mov      r11d, 0x2601
0x140326ce3: 4c2bc1                   sub      r8, rcx
0x140326ce6: 80fa0d                   cmp      dl, 0xd
0x140326ce9: 7706                     ja       0x140326cf1
0x140326ceb: 410fa3d3                 bt       r11d, edx
0x140326cef: 7213                     jb       0x140326d04
0x140326cf1: 41881408                 mov      byte ptr [r8 + rcx], dl
0x140326cf5: 41ffc1                   inc      r9d
0x140326cf8: 0fb65101                 movzx    edx, byte ptr [rcx + 1]
0x140326cfc: 48ffc1                   inc      rcx
0x140326cff: 80fa20                   cmp      dl, 0x20
0x140326d02: 75e2                     jne      0x140326ce6
0x140326d04: 4963c1                   movsxd   rax, r9d
0x140326d07: 42c6041000               mov      byte ptr [rax + r10], 0
0x140326d0c: 488bc1                   mov      rax, rcx
0x140326d0f: c3                       ret      
0x140326d10: 45880a                   mov      byte ptr [r10], r9b
0x140326d13: 33c0                     xor      eax, eax
0x140326d15: c3                       ret      
