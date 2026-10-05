; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1403376f0, 0x140337705); contiguous code slice, may span split unwind fragments
0x1403376f0: 33c0                     xor      eax, eax
0x1403376f2: c74118ffffffff           mov      dword ptr [rcx + 0x18], 0xffffffff
0x1403376f9: 48894108                 mov      qword ptr [rcx + 8], rax
0x1403376fd: 48894110                 mov      qword ptr [rcx + 0x10], rax
0x140337701: 89411c                   mov      dword ptr [rcx + 0x1c], eax
0x140337704: c3                       ret      
