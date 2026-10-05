; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x1403233f0, 0x140323463); contiguous code slice, may span split unwind fragments
0x1403233f0: 48894c2408               mov      qword ptr [rsp + 8], rcx
0x1403233f5: 57                       push     rdi
0x1403233f6: 4883ec30                 sub      rsp, 0x30
0x1403233fa: 48c7442420feffffff       mov      qword ptr [rsp + 0x20], 0xfffffffffffffffe
0x140323403: 48895c2450               mov      qword ptr [rsp + 0x50], rbx
0x140323408: 488bf9                   mov      rdi, rcx
0x14032340b: 33c0                     xor      eax, eax
0x14032340d: 894108                   mov      dword ptr [rcx + 8], eax
0x140323410: 894110                   mov      dword ptr [rcx + 0x10], eax
0x140323413: c74114000080bf           mov      dword ptr [rcx + 0x14], 0xbf800000
0x14032341a: c74118000080bf           mov      dword ptr [rcx + 0x18], 0xbf800000
0x140323421: c7411c0000803f           mov      dword ptr [rcx + 0x1c], 0x3f800000
0x140323428: 48894120                 mov      qword ptr [rcx + 0x20], rax
0x14032342c: 48894128                 mov      qword ptr [rcx + 0x28], rax
0x140323430: 488d0581511e00           lea      rax, [rip + 0x1e5181]
0x140323437: 488901                   mov      qword ptr [rcx], rax
0x14032343a: 488d5938                 lea      rbx, [rcx + 0x38]
0x14032343e: 48895c2448               mov      qword ptr [rsp + 0x48], rbx
0x140323443: 488bcb                   mov      rcx, rbx
0x140323446: e8b55d0000               call     0x140329200
0x14032344b: 90                       nop      
0x14032344c: 488bcb                   mov      rcx, rbx
0x14032344f: e89c420100               call     0x1403376f0
0x140323454: 90                       nop      
0x140323455: 488bc7                   mov      rax, rdi
0x140323458: 488b5c2450               mov      rbx, qword ptr [rsp + 0x50]
0x14032345d: 4883c430                 add      rsp, 0x30
0x140323461: 5f                       pop      rdi
0x140323462: c3                       ret      
