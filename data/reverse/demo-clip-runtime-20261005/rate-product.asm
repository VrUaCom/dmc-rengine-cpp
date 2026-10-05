; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140326960, 0x1403269eb); contiguous code slice, may span split unwind fragments
0x140326960: f30f1005046c0300         movss    xmm0, dword ptr [rip + 0x36c04]
0x140326968: 4c8d1d9196cdff           lea      r11, [rip - 0x32696f]
0x14032696f: 41b801000000             mov      r8d, 1
0x140326975: 4c8bc9                   mov      r9, rcx
0x140326978: 41c1c000                 rol      r8d, 0
0x14032697c: 41ba08000000             mov      r10d, 8
0x140326982: 418bc0                   mov      eax, r8d
0x140326985: 23c2                     and      eax, edx
0x140326987: ffc8                     dec      eax
0x140326989: 83f87f                   cmp      eax, 0x7f
0x14032698c: 7753                     ja       0x1403269e1
0x14032698e: 410fb68403106a3200       movzx    eax, byte ptr [r11 + rax + 0x326a10]
0x140326997: 418b8c83ec693200         mov      ecx, dword ptr [r11 + rax*4 + 0x3269ec]
0x14032699f: 4903cb                   add      rcx, r11
0x1403269a2: ffe1                     jmp      rcx
0x1403269a4: f3410f5901               mulss    xmm0, dword ptr [r9]
0x1403269a9: eb36                     jmp      0x1403269e1
0x1403269ab: f3410f594104             mulss    xmm0, dword ptr [r9 + 4]
0x1403269b1: eb2e                     jmp      0x1403269e1
0x1403269b3: f3410f594108             mulss    xmm0, dword ptr [r9 + 8]
0x1403269b9: eb26                     jmp      0x1403269e1
0x1403269bb: f3410f59410c             mulss    xmm0, dword ptr [r9 + 0xc]
0x1403269c1: eb1e                     jmp      0x1403269e1
0x1403269c3: f3410f594110             mulss    xmm0, dword ptr [r9 + 0x10]
0x1403269c9: eb16                     jmp      0x1403269e1
0x1403269cb: f3410f594114             mulss    xmm0, dword ptr [r9 + 0x14]
0x1403269d1: eb0e                     jmp      0x1403269e1
0x1403269d3: f3410f594118             mulss    xmm0, dword ptr [r9 + 0x18]
0x1403269d9: eb06                     jmp      0x1403269e1
0x1403269db: f3410f59411c             mulss    xmm0, dword ptr [r9 + 0x1c]
0x1403269e1: 41d1c0                   rol      r8d, 1
0x1403269e4: 4983ea01                 sub      r10, 1
0x1403269e8: 7598                     jne      0x140326982
0x1403269ea: c3                       ret      
