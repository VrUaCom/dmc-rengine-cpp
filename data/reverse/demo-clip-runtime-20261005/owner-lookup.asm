; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402c8430, 0x1402c8438); contiguous code slice, may span split unwind fragments
0x1402c8430: 488b4120                 mov      rax, qword ptr [rcx + 0x20]
0x1402c8434: 488b00                   mov      rax, qword ptr [rax]
0x1402c8437: c3                       ret      
