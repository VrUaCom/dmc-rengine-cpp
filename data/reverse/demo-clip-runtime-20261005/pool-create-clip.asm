; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140323fb0, 0x140324295); contiguous code slice, may span split unwind fragments
0x140323fb0: 4055                     push     rbp
0x140323fb2: 56                       push     rsi
0x140323fb3: 57                       push     rdi
0x140323fb4: 4883ec30                 sub      rsp, 0x30
0x140323fb8: 48c7442420feffffff       mov      qword ptr [rsp + 0x20], 0xfffffffffffffffe
0x140323fc1: 48895c2460               mov      qword ptr [rsp + 0x60], rbx
0x140323fc6: 488bf9                   mov      rdi, rcx
0x140323fc9: 33f6                     xor      esi, esi
0x140323fcb: 4889742450               mov      qword ptr [rsp + 0x50], rsi
0x140323fd0: 83caff                   or       edx, 0xffffffff
0x140323fd3: 448bc6                   mov      r8d, esi
0x140323fd6: 4c635910                 movsxd   r11, dword ptr [rcx + 0x10]
0x140323fda: 49c1e305                 shl      r11, 5
0x140323fde: 488d2d1bc0cdff           lea      rbp, [rip - 0x323fe5]
0x140323fe5: 450fb7942bf00e5d00       movzx    r10d, word ptr [r11 + rbp + 0x5d0ef0]
0x140323fee: 66413bf2                 cmp      si, r10w
0x140323ff2: 0f8d8e020000             jge      0x140324286
0x140323ff8: 488b5940                 mov      rbx, qword ptr [rcx + 0x40]
0x140323ffc: 488d4b10                 lea      rcx, [rbx + 0x10]
0x140324000: 483971f8                 cmp      qword ptr [rcx - 8], rsi
0x140324004: 7424                     je       0x14032402a
0x140324006: 488b01                   mov      rax, qword ptr [rcx]
0x140324009: 4889442450               mov      qword ptr [rsp + 0x50], rax
0x14032400e: 448b4808                 mov      r9d, dword ptr [rax + 8]
0x140324012: 4585c9                   test     r9d, r9d
0x140324015: 0f846d020000             je       0x140324288
0x14032401b: 418d41ff                 lea      eax, [r9 - 1]
0x14032401f: 83f803                   cmp      eax, 3
0x140324022: 0f875e020000             ja       0x140324286
0x140324028: eb07                     jmp      0x140324031
0x14032402a: 83faff                   cmp      edx, -1
0x14032402d: 410f44d0                 cmove    edx, r8d
0x140324031: 41ffc0                   inc      r8d
0x140324034: 4883c120                 add      rcx, 0x20
0x140324038: 410fbfc2                 movsx    eax, r10w
0x14032403c: 443bc0                   cmp      r8d, eax
0x14032403f: 7cbf                     jl       0x140324000
0x140324041: 83faff                   cmp      edx, -1
0x140324044: 0f843c020000             je       0x140324286
0x14032404a: 4863ca                   movsxd   rcx, edx
0x14032404d: 48c1e105                 shl      rcx, 5
0x140324051: 4803cb                   add      rcx, rbx
0x140324054: 458b842bf80e5d00         mov      r8d, dword ptr [r11 + rbp + 0x5d0ef8]
0x14032405c: 488d151dd19c00           lea      rdx, [rip + 0x9cd11d]
0x140324063: e898350100               call     0x140337600
0x140324068: 488bd8                   mov      rbx, rax
0x14032406b: 4889442450               mov      qword ptr [rsp + 0x50], rax
0x140324070: 4885c0                   test     rax, rax
0x140324073: 0f840d020000             je       0x140324286
0x140324079: 48634710                 movsxd   rax, dword ptr [rdi + 0x10]
0x14032407d: 83f80f                   cmp      eax, 0xf
0x140324080: 0f87f0010000             ja       0x140324276
0x140324086: 8b8c8598423200           mov      ecx, dword ptr [rbp + rax*4 + 0x324298]
0x14032408d: 4803cd                   add      rcx, rbp
0x140324090: ffe1                     jmp      rcx
0x140324092: 897308                   mov      dword ptr [rbx + 8], esi
0x140324095: 897310                   mov      dword ptr [rbx + 0x10], esi
0x140324098: c74314000080bf           mov      dword ptr [rbx + 0x14], 0xbf800000
0x14032409f: c74318000080bf           mov      dword ptr [rbx + 0x18], 0xbf800000
0x1403240a6: c7431c0000803f           mov      dword ptr [rbx + 0x1c], 0x3f800000
0x1403240ad: 48897320                 mov      qword ptr [rbx + 0x20], rsi
0x1403240b1: 48897328                 mov      qword ptr [rbx + 0x28], rsi
0x1403240b5: 488d05dc411e00           lea      rax, [rip + 0x1e41dc]
0x1403240bc: 488903                   mov      qword ptr [rbx], rax
0x1403240bf: e9b2010000               jmp      0x140324276
0x1403240c4: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x1403240c9: 488bcb                   mov      rcx, rbx
0x1403240cc: e8afefffff               call     0x140323080
0x1403240d1: 90                       nop      
0x1403240d2: e99f010000               jmp      0x140324276
0x1403240d7: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x1403240dc: 488bcb                   mov      rcx, rbx
0x1403240df: e8ccf1ffff               call     0x1403232b0
0x1403240e4: 90                       nop      
0x1403240e5: e98c010000               jmp      0x140324276
0x1403240ea: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x1403240ef: 488bcb                   mov      rcx, rbx
0x1403240f2: e889f2ffff               call     0x140323380
0x1403240f7: 90                       nop      
0x1403240f8: e979010000               jmp      0x140324276
0x1403240fd: 488bcb                   mov      rcx, rbx
0x140324100: e86bf0ffff               call     0x140323170
0x140324105: e96c010000               jmp      0x140324276
0x14032410a: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x14032410f: 488bcb                   mov      rcx, rbx
0x140324112: e8f9efffff               call     0x140323110
0x140324117: 90                       nop      
0x140324118: e959010000               jmp      0x140324276
0x14032411d: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x140324122: 488bcb                   mov      rcx, rbx
0x140324125: e806f1ffff               call     0x140323230
0x14032412a: 90                       nop      
0x14032412b: e946010000               jmp      0x140324276
0x140324130: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x140324135: 488bcb                   mov      rcx, rbx
0x140324138: e8b3f2ffff               call     0x1403233f0
0x14032413d: 90                       nop      
0x14032413e: e933010000               jmp      0x140324276
0x140324143: 897308                   mov      dword ptr [rbx + 8], esi
0x140324146: 897310                   mov      dword ptr [rbx + 0x10], esi
0x140324149: c74314000080bf           mov      dword ptr [rbx + 0x14], 0xbf800000
0x140324150: c74318000080bf           mov      dword ptr [rbx + 0x18], 0xbf800000
0x140324157: c7431c0000803f           mov      dword ptr [rbx + 0x1c], 0x3f800000
0x14032415e: 48897320                 mov      qword ptr [rbx + 0x20], rsi
0x140324162: 48897328                 mov      qword ptr [rbx + 0x28], rsi
0x140324166: 488d059b441e00           lea      rax, [rip + 0x1e449b]
0x14032416d: 488903                   mov      qword ptr [rbx], rax
0x140324170: 897330                   mov      dword ptr [rbx + 0x30], esi
0x140324173: c7433400000080           mov      dword ptr [rbx + 0x34], 0x80000000
0x14032417a: c7433800007042           mov      dword ptr [rbx + 0x38], 0x42700000
0x140324181: e9f0000000               jmp      0x140324276
0x140324186: 897308                   mov      dword ptr [rbx + 8], esi
0x140324189: 897310                   mov      dword ptr [rbx + 0x10], esi
0x14032418c: c74314000080bf           mov      dword ptr [rbx + 0x14], 0xbf800000
0x140324193: c74318000080bf           mov      dword ptr [rbx + 0x18], 0xbf800000
0x14032419a: c7431c0000803f           mov      dword ptr [rbx + 0x1c], 0x3f800000
0x1403241a1: 48897320                 mov      qword ptr [rbx + 0x20], rsi
0x1403241a5: 48897328                 mov      qword ptr [rbx + 0x28], rsi
0x1403241a9: 488d0528421e00           lea      rax, [rip + 0x1e4228]
0x1403241b0: 488903                   mov      qword ptr [rbx], rax
0x1403241b3: 48897330                 mov      qword ptr [rbx + 0x30], rsi
0x1403241b7: 4889733c                 mov      qword ptr [rbx + 0x3c], rsi
0x1403241bb: 897338                   mov      dword ptr [rbx + 0x38], esi
0x1403241be: e9b3000000               jmp      0x140324276
0x1403241c3: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x1403241c8: 488bcb                   mov      rcx, rbx
0x1403241cb: e8a0f2ffff               call     0x140323470
0x1403241d0: 90                       nop      
0x1403241d1: e9a0000000               jmp      0x140324276
0x1403241d6: 897308                   mov      dword ptr [rbx + 8], esi
0x1403241d9: 897310                   mov      dword ptr [rbx + 0x10], esi
0x1403241dc: c74314000080bf           mov      dword ptr [rbx + 0x14], 0xbf800000
0x1403241e3: c74318000080bf           mov      dword ptr [rbx + 0x18], 0xbf800000
0x1403241ea: c7431c0000803f           mov      dword ptr [rbx + 0x1c], 0x3f800000
0x1403241f1: 48897320                 mov      qword ptr [rbx + 0x20], rsi
0x1403241f5: 48897328                 mov      qword ptr [rbx + 0x28], rsi
0x1403241f9: 488d0578421e00           lea      rax, [rip + 0x1e4278]
0x140324200: 488903                   mov      qword ptr [rbx], rax
0x140324203: c74330000080bf           mov      dword ptr [rbx + 0x30], 0xbf800000
0x14032420a: c74334000080bf           mov      dword ptr [rbx + 0x34], 0xbf800000
0x140324211: c74338000080bf           mov      dword ptr [rbx + 0x38], 0xbf800000
0x140324218: c7433c000080bf           mov      dword ptr [rbx + 0x3c], 0xbf800000
0x14032421f: c7434000007042           mov      dword ptr [rbx + 0x40], 0x42700000
0x140324226: eb4e                     jmp      0x140324276
0x140324228: 897308                   mov      dword ptr [rbx + 8], esi
0x14032422b: 897310                   mov      dword ptr [rbx + 0x10], esi
0x14032422e: c74314000080bf           mov      dword ptr [rbx + 0x14], 0xbf800000
0x140324235: c74318000080bf           mov      dword ptr [rbx + 0x18], 0xbf800000
0x14032423c: c7431c0000803f           mov      dword ptr [rbx + 0x1c], 0x3f800000
0x140324243: 48897320                 mov      qword ptr [rbx + 0x20], rsi
0x140324247: 48897328                 mov      qword ptr [rbx + 0x28], rsi
0x14032424b: 488d05d6411e00           lea      rax, [rip + 0x1e41d6]
0x140324252: 488903                   mov      qword ptr [rbx], rax
0x140324255: c74330ffffffff           mov      dword ptr [rbx + 0x30], 0xffffffff
0x14032425c: eb18                     jmp      0x140324276
0x14032425e: 48895c2458               mov      qword ptr [rsp + 0x58], rbx
0x140324263: 488bcb                   mov      rcx, rbx
0x140324266: e8a5eeffff               call     0x140323110
0x14032426b: 90                       nop      
0x14032426c: 488bcb                   mov      rcx, rbx
0x14032426f: e86c000000               call     0x1403242e0
0x140324274: eb10                     jmp      0x140324286
0x140324276: c7430801000000           mov      dword ptr [rbx + 8], 1
0x14032427d: 48897b20                 mov      qword ptr [rbx + 0x20], rdi
0x140324281: 488bc3                   mov      rax, rbx
0x140324284: eb02                     jmp      0x140324288
0x140324286: 33c0                     xor      eax, eax
0x140324288: 488b5c2460               mov      rbx, qword ptr [rsp + 0x60]
0x14032428d: 4883c430                 add      rsp, 0x30
0x140324291: 5f                       pop      rdi
0x140324292: 5e                       pop      rsi
0x140324293: 5d                       pop      rbp
0x140324294: c3                       ret      
