; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140316c20, 0x140316c45); contiguous code slice, may span split unwind fragments
0x140316c20: 0fb74204                 movzx    eax, word ptr [rdx + 4]
0x140316c24: a804                     test     al, 4
0x140316c26: 751c                     jne      0x140316c44
0x140316c28: 803a00                   cmp      byte ptr [rdx], 0
0x140316c2b: 7417                     je       0x140316c44
0x140316c2d: 807a0300                 cmp      byte ptr [rdx + 3], 0
0x140316c31: 7411                     je       0x140316c44
0x140316c33: 6683c804                 or       ax, 4
0x140316c37: 66894204                 mov      word ptr [rdx + 4], ax
0x140316c3b: b803000000               mov      eax, 3
0x140316c40: 6689420c                 mov      word ptr [rdx + 0xc], ax
0x140316c44: c3                       ret      
