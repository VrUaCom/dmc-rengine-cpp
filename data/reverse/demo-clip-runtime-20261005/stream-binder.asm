; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031edb0, 0x14031edf1); contiguous code slice, may span split unwind fragments
0x14031edb0: 4885d2                   test     rdx, rdx
0x14031edb3: 7439                     je       0x14031edee
0x14031edb5: 488991f8000000           mov      qword ptr [rcx + 0xf8], rdx
0x14031edbc: 0fb74204                 movzx    eax, word ptr [rdx + 4]
0x14031edc0: 668901                   mov      word ptr [rcx], ax
0x14031edc3: 6685c0                   test     ax, ax
0x14031edc6: 7e1b                     jle      0x14031ede3
0x14031edc8: 837a0401                 cmp      dword ptr [rdx + 4], 1
0x14031edcc: 7215                     jb       0x14031ede3
0x14031edce: 8b4208                   mov      eax, dword ptr [rdx + 8]
0x14031edd1: 85c0                     test     eax, eax
0x14031edd3: 740e                     je       0x14031ede3
0x14031edd5: 4803c2                   add      rax, rdx
0x14031edd8: 7409                     je       0x14031ede3
0x14031edda: 80780601                 cmp      byte ptr [rax + 6], 1
0x14031edde: 7503                     jne      0x14031ede3
0x14031ede0: b001                     mov      al, 1
0x14031ede2: c3                       ret      
0x14031ede3: 48c781f800000000000000   mov      qword ptr [rcx + 0xf8], 0
0x14031edee: 32c0                     xor      al, al
0x14031edf0: c3                       ret      
