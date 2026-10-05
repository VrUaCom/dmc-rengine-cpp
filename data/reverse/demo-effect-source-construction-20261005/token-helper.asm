; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1402d6420, 0x1402d6443); code slice, not necessarily one unwind entry
0x1402d6420: 4053                     push     rbx
0x1402d6422: 4883ec20                 sub      rsp, 0x20
0x1402d6426: 488bda                   mov      rbx, rdx
0x1402d6429: 488bd1                   mov      rdx, rcx
0x1402d642c: 488b0b                   mov      rcx, qword ptr [rbx]
0x1402d642f: e86c080500               call     0x140326ca0
0x1402d6434: 4885c0                   test     rax, rax
0x1402d6437: 488903                   mov      qword ptr [rbx], rax
0x1402d643a: 0f95c0                   setne    al
0x1402d643d: 4883c420                 add      rsp, 0x20
0x1402d6441: 5b                       pop      rbx
0x1402d6442: c3                       ret      
