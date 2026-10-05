; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x14031ea40, 0x14031eb34); contiguous code slice, may span split unwind fragments
0x14031ea40: 4533c0                   xor      r8d, r8d
0x14031ea43: e908000000               jmp      0x14031ea50
0x14031ea48: cc                       int3     
0x14031ea49: cc                       int3     
0x14031ea4a: cc                       int3     
0x14031ea4b: cc                       int3     
0x14031ea4c: cc                       int3     
0x14031ea4d: cc                       int3     
0x14031ea4e: cc                       int3     
0x14031ea4f: cc                       int3     
0x14031ea50: 48896c2410               mov      qword ptr [rsp + 0x10], rbp
0x14031ea55: 4889742418               mov      qword ptr [rsp + 0x18], rsi
0x14031ea5a: 57                       push     rdi
0x14031ea5b: 4883ec20                 sub      rsp, 0x20
0x14031ea5f: 33ff                     xor      edi, edi
0x14031ea61: 488bf1                   mov      rsi, rcx
0x14031ea64: 448bcf                   mov      r9d, edi
0x14031ea67: 0fb6ca                   movzx    ecx, dl
0x14031ea6a: 410fb6e8                 movzx    ebp, r8b
0x14031ea6e: 6690                     nop      
0x14031ea70: 4d63d1                   movsxd   r10, r9d
0x14031ea73: 420fbf445604             movsx    eax, word ptr [rsi + r10*2 + 4]
0x14031ea79: 3bc1                     cmp      eax, ecx
0x14031ea7b: 750a                     jne      0x14031ea87
0x14031ea7d: 4138ac3200010000         cmp      byte ptr [r10 + rsi + 0x100], bpl
0x14031ea85: 7438                     je       0x14031eabf
0x14031ea87: 41ffc1                   inc      r9d
0x14031ea8a: 4183f914                 cmp      r9d, 0x14
0x14031ea8e: 7ce0                     jl       0x14031ea70
0x14031ea90: 488bcf                   mov      rcx, rdi
0x14031ea93: 488d4604                 lea      rax, [rsi + 4]
0x14031ea97: 668338ff                 cmp      word ptr [rax], -1
0x14031ea9b: 743d                     je       0x14031eada
0x14031ea9d: ffc7                     inc      edi
0x14031ea9f: 48ffc1                   inc      rcx
0x14031eaa2: 4883c002                 add      rax, 2
0x14031eaa6: 4883f914                 cmp      rcx, 0x14
0x14031eaaa: 7ceb                     jl       0x14031ea97
0x14031eaac: 83c8ff                   or       eax, 0xffffffff
0x14031eaaf: 488b6c2438               mov      rbp, qword ptr [rsp + 0x38]
0x14031eab4: 488b742440               mov      rsi, qword ptr [rsp + 0x40]
0x14031eab9: 4883c420                 add      rsp, 0x20
0x14031eabd: 5f                       pop      rdi
0x14031eabe: c3                       ret      
0x14031eabf: 664585c9                 test     r9w, r9w
0x14031eac3: 78cb                     js       0x14031ea90
0x14031eac5: b8feffffff               mov      eax, 0xfffffffe
0x14031eaca: 488b6c2438               mov      rbp, qword ptr [rsp + 0x38]
0x14031eacf: 488b742440               mov      rsi, qword ptr [rsp + 0x40]
0x14031ead4: 4883c420                 add      rsp, 0x20
0x14031ead8: 5f                       pop      rdi
0x14031ead9: c3                       ret      
0x14031eada: 85ff                     test     edi, edi
0x14031eadc: 7913                     jns      0x14031eaf1
0x14031eade: 83c8ff                   or       eax, 0xffffffff
0x14031eae1: 488b6c2438               mov      rbp, qword ptr [rsp + 0x38]
0x14031eae6: 488b742440               mov      rsi, qword ptr [rsp + 0x40]
0x14031eaeb: 4883c420                 add      rsp, 0x20
0x14031eaef: 5f                       pop      rdi
0x14031eaf0: c3                       ret      
0x14031eaf1: 0fb6ca                   movzx    ecx, dl
0x14031eaf4: 4533c0                   xor      r8d, r8d
0x14031eaf7: 48895c2430               mov      qword ptr [rsp + 0x30], rbx
0x14031eafc: 4863df                   movsxd   rbx, edi
0x14031eaff: 66894c5e04               mov      word ptr [rsi + rbx*2 + 4], cx
0x14031eb04: 488d0d75b09b00           lea      rcx, [rip + 0x9bb075]
0x14031eb0b: e8c070ffff               call     0x140315bd0
0x14031eb10: 488944de58               mov      qword ptr [rsi + rbx*8 + 0x58], rax
0x14031eb15: 8bc7                     mov      eax, edi
0x14031eb17: 4088ac3300010000         mov      byte ptr [rbx + rsi + 0x100], bpl
0x14031eb1f: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x14031eb24: 488b6c2438               mov      rbp, qword ptr [rsp + 0x38]
0x14031eb29: 488b742440               mov      rsi, qword ptr [rsp + 0x40]
0x14031eb2e: 4883c420                 add      rsp, 0x20
0x14031eb32: 5f                       pop      rdi
0x14031eb33: c3                       ret      
