; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140323930, 0x1403239da); contiguous code slice, may span split unwind fragments
0x140323930: 48894c2408               mov      qword ptr [rsp + 8], rcx
0x140323935: 53                       push     rbx
0x140323936: 55                       push     rbp
0x140323937: 56                       push     rsi
0x140323938: 57                       push     rdi
0x140323939: 4156                     push     r14
0x14032393b: 4883ec30                 sub      rsp, 0x30
0x14032393f: 48c7442420feffffff       mov      qword ptr [rsp + 0x20], 0xfffffffffffffffe
0x140323948: 4c63f2                   movsxd   r14, edx
0x14032394b: 488bf1                   mov      rsi, rcx
0x14032394e: 4c8901                   mov      qword ptr [rcx], r8
0x140323951: 498bee                   mov      rbp, r14
0x140323954: 48c1e505                 shl      rbp, 5
0x140323958: 488d0d91d52a00           lea      rcx, [rip + 0x2ad591]
0x14032395f: 4803e9                   add      rbp, rcx
0x140323962: 4c0fbf4500               movsx    r8, word ptr [rbp]
0x140323967: 41c1e005                 shl      r8d, 5
0x14032396b: 488d4e20                 lea      rcx, [rsi + 0x20]
0x14032396f: 488d150ad89c00           lea      rdx, [rip + 0x9cd80a]
0x140323976: e8853c0100               call     0x140337600
0x14032397b: 48894640                 mov      qword ptr [rsi + 0x40], rax
0x14032397f: 4885c0                   test     rax, rax
0x140323982: 744b                     je       0x1403239cf
0x140323984: 33ff                     xor      edi, edi
0x140323986: 897c2468                 mov      dword ptr [rsp + 0x68], edi
0x14032398a: 663b7d00                 cmp      di, word ptr [rbp]
0x14032398e: 7d32                     jge      0x1403239c2
0x140323990: 4863df                   movsxd   rbx, edi
0x140323993: 48c1e305                 shl      rbx, 5
0x140323997: 48035e40                 add      rbx, qword ptr [rsi + 0x40]
0x14032399b: 48895c2470               mov      qword ptr [rsp + 0x70], rbx
0x1403239a0: 7412                     je       0x1403239b4
0x1403239a2: 488bcb                   mov      rcx, rbx
0x1403239a5: e856580000               call     0x140329200
0x1403239aa: 90                       nop      
0x1403239ab: 488bcb                   mov      rcx, rbx
0x1403239ae: e83d3d0100               call     0x1403376f0
0x1403239b3: 90                       nop      
0x1403239b4: ffc7                     inc      edi
0x1403239b6: 897c2468                 mov      dword ptr [rsp + 0x68], edi
0x1403239ba: 0fbf4500                 movsx    eax, word ptr [rbp]
0x1403239be: 3bf8                     cmp      edi, eax
0x1403239c0: 7cce                     jl       0x140323990
0x1403239c2: c7460801000000           mov      dword ptr [rsi + 8], 1
0x1403239c9: 44897610                 mov      dword ptr [rsi + 0x10], r14d
0x1403239cd: b001                     mov      al, 1
0x1403239cf: 4883c430                 add      rsp, 0x30
0x1403239d3: 415e                     pop      r14
0x1403239d5: 5f                       pop      rdi
0x1403239d6: 5e                       pop      rsi
0x1403239d7: 5d                       pop      rbp
0x1403239d8: 5b                       pop      rbx
0x1403239d9: c3                       ret      
