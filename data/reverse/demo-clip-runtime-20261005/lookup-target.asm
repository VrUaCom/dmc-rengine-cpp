; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031ec60, 0x14031eca4); contiguous code slice, may span split unwind fragments
0x14031ec60: 33c0                     xor      eax, eax
0x14031ec62: 440fb6ca                 movzx    r9d, dl
0x14031ec66: 66660f1f840000000000     nop      word ptr [rax + rax]
0x14031ec70: 4c63c0                   movsxd   r8, eax
0x14031ec73: 420fbf544104             movsx    edx, word ptr [rcx + r8*2 + 4]
0x14031ec79: 413bd1                   cmp      edx, r9d
0x14031ec7c: 750b                     jne      0x14031ec89
0x14031ec7e: 4180bc080001000000       cmp      byte ptr [r8 + rcx + 0x100], 0
0x14031ec87: 740a                     je       0x14031ec93
0x14031ec89: ffc0                     inc      eax
0x14031ec8b: 83f814                   cmp      eax, 0x14
0x14031ec8e: 7ce0                     jl       0x14031ec70
0x14031ec90: 83c8ff                   or       eax, 0xffffffff
0x14031ec93: 480fbfd0                 movsx    rdx, ax
0x14031ec97: 85d2                     test     edx, edx
0x14031ec99: 7903                     jns      0x14031ec9e
0x14031ec9b: 33c0                     xor      eax, eax
0x14031ec9d: c3                       ret      
0x14031ec9e: 488b44d158               mov      rax, qword ptr [rcx + rdx*8 + 0x58]
0x14031eca3: c3                       ret      
