; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140315bd0, 0x140315c3a); contiguous code slice, may span split unwind fragments
0x140315bd0: 4883ec28                 sub      rsp, 0x28
0x140315bd4: 4533c9                   xor      r9d, r9d
0x140315bd7: 48895c2420               mov      qword ptr [rsp + 0x20], rbx
0x140315bdc: 458bc1                   mov      r8d, r9d
0x140315bdf: 488bc1                   mov      rax, rcx
0x140315be2: 803800                   cmp      byte ptr [rax], 0
0x140315be5: 741e                     je       0x140315c05
0x140315be7: 41ffc1                   inc      r9d
0x140315bea: 49ffc0                   inc      r8
0x140315bed: 480580020000             add      rax, 0x280
0x140315bf3: 4983f810                 cmp      r8, 0x10
0x140315bf7: 7ce9                     jl       0x140315be2
0x140315bf9: 33c0                     xor      eax, eax
0x140315bfb: 488b5c2420               mov      rbx, qword ptr [rsp + 0x20]
0x140315c00: 4883c428                 add      rsp, 0x28
0x140315c04: c3                       ret      
0x140315c05: 4963c1                   movsxd   rax, r9d
0x140315c08: 488d1c80                 lea      rbx, [rax + rax*4]
0x140315c0c: 48c1e307                 shl      rbx, 7
0x140315c10: 4803d9                   add      rbx, rcx
0x140315c13: 750c                     jne      0x140315c21
0x140315c15: 33c0                     xor      eax, eax
0x140315c17: 488b5c2420               mov      rbx, qword ptr [rsp + 0x20]
0x140315c1c: 4883c428                 add      rsp, 0x28
0x140315c20: c3                       ret      
0x140315c21: 440fb6c2                 movzx    r8d, dl
0x140315c25: 488bd3                   mov      rdx, rbx
0x140315c28: e8e3110000               call     0x140316e10
0x140315c2d: 488bc3                   mov      rax, rbx
0x140315c30: 488b5c2420               mov      rbx, qword ptr [rsp + 0x20]
0x140315c35: 4883c428                 add      rsp, 0x28
0x140315c39: c3                       ret      
