; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031ed00, 0x14031eda8); contiguous code slice, may span split unwind fragments
0x14031ed00: 4533c0                   xor      r8d, r8d
0x14031ed03: e908000000               jmp      0x14031ed10
0x14031ed08: cc                       int3     
0x14031ed09: cc                       int3     
0x14031ed0a: cc                       int3     
0x14031ed0b: cc                       int3     
0x14031ed0c: cc                       int3     
0x14031ed0d: cc                       int3     
0x14031ed0e: cc                       int3     
0x14031ed0f: cc                       int3     
0x14031ed10: 48895c2410               mov      qword ptr [rsp + 0x10], rbx
0x14031ed15: 57                       push     rdi
0x14031ed16: 4883ec20                 sub      rsp, 0x20
0x14031ed1a: 488bd9                   mov      rbx, rcx
0x14031ed1d: 4533c9                   xor      r9d, r9d
0x14031ed20: 0fb6ca                   movzx    ecx, dl
0x14031ed23: 0f1f4000                 nop      dword ptr [rax]
0x14031ed27: 660f1f840000000000       nop      word ptr [rax + rax]
0x14031ed30: 4963d1                   movsxd   rdx, r9d
0x14031ed33: 4183caff                 or       r10d, 0xffffffff
0x14031ed37: 0fbf445304               movsx    eax, word ptr [rbx + rdx*2 + 4]
0x14031ed3c: 3bc1                     cmp      eax, ecx
0x14031ed3e: 750a                     jne      0x14031ed4a
0x14031ed40: 4438841a00010000         cmp      byte ptr [rdx + rbx + 0x100], r8b
0x14031ed48: 740d                     je       0x14031ed57
0x14031ed4a: 41ffc1                   inc      r9d
0x14031ed4d: 4183f914                 cmp      r9d, 0x14
0x14031ed51: 7cdd                     jl       0x14031ed30
0x14031ed53: 450fb7ca                 movzx    r9d, r10w
0x14031ed57: 490fbff9                 movsx    rdi, r9w
0x14031ed5b: 85ff                     test     edi, edi
0x14031ed5d: 790e                     jns      0x14031ed6d
0x14031ed5f: 418bc2                   mov      eax, r10d
0x14031ed62: 488b5c2438               mov      rbx, qword ptr [rsp + 0x38]
0x14031ed67: 4883c420                 add      rsp, 0x20
0x14031ed6b: 5f                       pop      rdi
0x14031ed6c: c3                       ret      
0x14031ed6d: 664489547b04             mov      word ptr [rbx + rdi*2 + 4], r10w
0x14031ed73: 488b54fb58               mov      rdx, qword ptr [rbx + rdi*8 + 0x58]
0x14031ed78: 4889742430               mov      qword ptr [rsp + 0x30], rsi
0x14031ed7d: 4885d2                   test     rdx, rdx
0x14031ed80: 740c                     je       0x14031ed8e
0x14031ed82: 488d0df7ad9b00           lea      rcx, [rip + 0x9badf7]
0x14031ed89: e8927effff               call     0x140316c20
0x14031ed8e: 488b742430               mov      rsi, qword ptr [rsp + 0x30]
0x14031ed93: 8bc7                     mov      eax, edi
0x14031ed95: c6841f0001000000         mov      byte ptr [rdi + rbx + 0x100], 0
0x14031ed9d: 488b5c2438               mov      rbx, qword ptr [rsp + 0x38]
0x14031eda2: 4883c420                 add      rsp, 0x20
0x14031eda6: 5f                       pop      rdi
0x14031eda7: c3                       ret      
