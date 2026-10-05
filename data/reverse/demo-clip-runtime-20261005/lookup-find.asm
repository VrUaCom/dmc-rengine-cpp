; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031ea00, 0x14031ea34); contiguous code slice, may span split unwind fragments
0x14031ea00: 33c0                     xor      eax, eax
0x14031ea02: 440fb6ca                 movzx    r9d, dl
0x14031ea06: 66660f1f840000000000     nop      word ptr [rax + rax]
0x14031ea10: 4c63c0                   movsxd   r8, eax
0x14031ea13: 420fbf544104             movsx    edx, word ptr [rcx + r8*2 + 4]
0x14031ea19: 413bd1                   cmp      edx, r9d
0x14031ea1c: 750b                     jne      0x14031ea29
0x14031ea1e: 4180bc080001000000       cmp      byte ptr [r8 + rcx + 0x100], 0
0x14031ea27: 740a                     je       0x14031ea33
0x14031ea29: ffc0                     inc      eax
0x14031ea2b: 83f814                   cmp      eax, 0x14
0x14031ea2e: 7ce0                     jl       0x14031ea10
0x14031ea30: 83c8ff                   or       eax, 0xffffffff
0x14031ea33: c3                       ret      
