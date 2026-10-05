; Canonical dmc3.exe SHA256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
; VA range [0x140316e10, 0x140317398); contiguous code slice, may span split unwind fragments
0x140316e10: 48895c2408               mov      qword ptr [rsp + 8], rbx
0x140316e15: 48896c2410               mov      qword ptr [rsp + 0x10], rbp
0x140316e1a: 4889742418               mov      qword ptr [rsp + 0x18], rsi
0x140316e1f: 57                       push     rdi
0x140316e20: 4883ec20                 sub      rsp, 0x20
0x140316e24: 488bfa                   mov      rdi, rdx
0x140316e27: 410fb6d8                 movzx    ebx, r8b
0x140316e2b: 488bf1                   mov      rsi, rcx
0x140316e2e: 33d2                     xor      edx, edx
0x140316e30: 488bcf                   mov      rcx, rdi
0x140316e33: 41b880020000             mov      r8d, 0x280
0x140316e39: e8acfd0200               call     0x140346bea
0x140316e3e: 33ed                     xor      ebp, ebp
0x140316e40: 881f                     mov      byte ptr [rdi], bl
0x140316e42: 8d43ff                   lea      eax, [rbx - 1]
0x140316e45: 66c747010000             mov      word ptr [rdi + 1], 0
0x140316e4b: 896f08                   mov      dword ptr [rdi + 8], ebp
0x140316e4e: c6470301                 mov      byte ptr [rdi + 3], 1
0x140316e52: 66896f04                 mov      word ptr [rdi + 4], bp
0x140316e56: 48896f10                 mov      qword ptr [rdi + 0x10], rbp
0x140316e5a: 66896f0c                 mov      word ptr [rdi + 0xc], bp
0x140316e5e: 83f80e                   cmp      eax, 0xe
0x140316e61: 0f8719050000             ja       0x140317380
0x140316e67: 488d159291ceff           lea      rdx, [rip - 0x316e6e]
0x140316e6e: 4898                     cdqe     
0x140316e70: 8b8c8298733100           mov      ecx, dword ptr [rdx + rax*4 + 0x317398]
0x140316e77: 4803ca                   add      rcx, rdx
0x140316e7a: ffe1                     jmp      rcx
0x140316e7c: b818000000               mov      eax, 0x18
0x140316e81: c64702ff                 mov      byte ptr [rdi + 2], 0xff
0x140316e85: 66894706                 mov      word ptr [rdi + 6], ax
0x140316e89: 48c7471880808040         mov      qword ptr [rdi + 0x18], 0x40808080
0x140316e91: 48896f24                 mov      qword ptr [rdi + 0x24], rbp
0x140316e95: 896f20                   mov      dword ptr [rdi + 0x20], ebp
0x140316e98: e9e3040000               jmp      0x140317380
0x140316e9d: b910000000               mov      ecx, 0x10
0x140316ea2: c6470228                 mov      byte ptr [rdi + 2], 0x28
0x140316ea6: 66894f06                 mov      word ptr [rdi + 6], cx
0x140316eaa: c7471800007a44           mov      dword ptr [rdi + 0x18], 0x447a0000
0x140316eb1: c7471c00808944           mov      dword ptr [rdi + 0x1c], 0x44898000
0x140316eb8: c7472003000000           mov      dword ptr [rdi + 0x20], 3
0x140316ebf: c7472480808080           mov      dword ptr [rdi + 0x24], 0x80808080
0x140316ec6: e9b5040000               jmp      0x140317380
0x140316ecb: b910000000               mov      ecx, 0x10
0x140316ed0: c7470402004800           mov      dword ptr [rdi + 4], 0x480002
0x140316ed7: 894f28                   mov      dword ptr [rdi + 0x28], ecx
0x140316eda: c647022d                 mov      byte ptr [rdi + 2], 0x2d
0x140316ede: c7472c06000000           mov      dword ptr [rdi + 0x2c], 6
0x140316ee5: c7473080808040           mov      dword ptr [rdi + 0x30], 0x40808080
0x140316eec: c7473800007a44           mov      dword ptr [rdi + 0x38], 0x447a0000
0x140316ef3: c747340000fa43           mov      dword ptr [rdi + 0x34], 0x43fa0000
0x140316efa: 48c7473c00007a44         mov      qword ptr [rdi + 0x3c], 0x447a0000
0x140316f02: c7471c0000fa43           mov      dword ptr [rdi + 0x1c], 0x43fa0000
0x140316f09: c7472000007a44           mov      dword ptr [rdi + 0x20], 0x447a0000
0x140316f10: c747240000003f           mov      dword ptr [rdi + 0x24], 0x3f000000
0x140316f17: 896f44                   mov      dword ptr [rdi + 0x44], ebp
0x140316f1a: e961040000               jmp      0x140317380
0x140316f1f: c7470402008800           mov      dword ptr [rdi + 4], 0x880002
0x140316f26: b802000000               mov      eax, 2
0x140316f2b: 894770                   mov      dword ptr [rdi + 0x70], eax
0x140316f2e: c6470220                 mov      byte ptr [rdi + 2], 0x20
0x140316f32: c7471800007a44           mov      dword ptr [rdi + 0x18], 0x447a0000
0x140316f39: c7471c0080bb44           mov      dword ptr [rdi + 0x1c], 0x44bb8000
0x140316f40: c747204040ff80           mov      dword ptr [rdi + 0x20], 0x80ff4040
0x140316f47: c74724ffffff80           mov      dword ptr [rdi + 0x24], 0x80ffffff
0x140316f4e: 66c7476000ff             mov      word ptr [rdi + 0x60], 0xff00
0x140316f54: 898794000000             mov      dword ptr [rdi + 0x94], eax
0x140316f5a: 66c747740040             mov      word ptr [rdi + 0x74], 0x4000
0x140316f60: 66c7878400000000ff       mov      word ptr [rdi + 0x84], 0xff00
0x140316f69: e912040000               jmp      0x140317380
0x140316f6e: c6470250                 mov      byte ptr [rdi + 2], 0x50
0x140316f72: b810010000               mov      eax, 0x110
0x140316f77: 66894706                 mov      word ptr [rdi + 6], ax
0x140316f7b: b802000000               mov      eax, 2
0x140316f80: 89476c                   mov      dword ptr [rdi + 0x6c], eax
0x140316f83: 896f18                   mov      dword ptr [rdi + 0x18], ebp
0x140316f86: c7471c00000080           mov      dword ptr [rdi + 0x1c], 0x80000000
0x140316f8d: c74720ffffff80           mov      dword ptr [rdi + 0x20], 0x80ffffff
0x140316f94: 66c7475c00ff             mov      word ptr [rdi + 0x5c], 0xff00
0x140316f9a: 898790000000             mov      dword ptr [rdi + 0x90], eax
0x140316fa0: 66c747704040             mov      word ptr [rdi + 0x70], 0x4040
0x140316fa6: 66c7878000000000ff       mov      word ptr [rdi + 0x80], 0xff00
0x140316faf: ba01000000               mov      edx, 1
0x140316fb4: 488d8f94000000           lea      rcx, [rdi + 0x94]
0x140316fbb: 448d427f                 lea      r8d, [rdx + 0x7f]
0x140316fbf: e826fc0200               call     0x140346bea
0x140316fc4: e9b7030000               jmp      0x140317380
0x140316fc9: c747304020f080           mov      dword ptr [rdi + 0x30], 0x80f02040
0x140316fd0: 488d5720                 lea      rdx, [rdi + 0x20]
0x140316fd4: 896f34                   mov      dword ptr [rdi + 0x34], ebp
0x140316fd7: b878000000               mov      eax, 0x78
0x140316fdc: 66894706                 mov      word ptr [rdi + 6], ax
0x140316fe0: 4c8bc2                   mov      r8, rdx
0x140316fe3: 488bce                   mov      rcx, rsi
0x140316fe6: 892a                     mov      dword ptr [rdx], ebp
0x140316fe8: 896f24                   mov      dword ptr [rdi + 0x24], ebp
0x140316feb: c747280000fa43           mov      dword ptr [rdi + 0x28], 0x43fa0000
0x140316ff2: c7472c0000803f           mov      dword ptr [rdi + 0x2c], 0x3f800000
0x140316ff9: e8e2faffff               call     0x140316ae0
0x140316ffe: c747380000c842           mov      dword ptr [rdi + 0x38], 0x42c80000
0x140317005: b802000000               mov      eax, 2
0x14031700a: 894764                   mov      dword ptr [rdi + 0x64], eax
0x14031700d: b910000000               mov      ecx, 0x10
0x140317012: 66c747440080             mov      word ptr [rdi + 0x44], 0x8000
0x140317018: 66c7475400ff             mov      word ptr [rdi + 0x54], 0xff00
0x14031701e: 894f6c                   mov      dword ptr [rdi + 0x6c], ecx
0x140317021: c7477030000000           mov      dword ptr [rdi + 0x70], 0x30
0x140317028: c747740000003f           mov      dword ptr [rdi + 0x74], 0x3f000000
0x14031702f: c747780000003f           mov      dword ptr [rdi + 0x78], 0x3f000000
0x140317036: c7477ccdcccc3d           mov      dword ptr [rdi + 0x7c], 0x3dcccccd
0x14031703d: c78780000000cdcc4c3e     mov      dword ptr [rdi + 0x80], 0x3e4ccccd
0x140317047: 48c787840000000000803f   mov      qword ptr [rdi + 0x84], 0x3f800000
0x140317052: 89af98000000             mov      dword ptr [rdi + 0x98], ebp
0x140317058: e923030000               jmp      0x140317380
0x14031705d: b8c8000000               mov      eax, 0xc8
0x140317062: b910000000               mov      ecx, 0x10
0x140317067: 66894706                 mov      word ptr [rdi + 6], ax
0x14031706b: 33d2                     xor      edx, edx
0x14031706d: b802000000               mov      eax, 2
0x140317072: 89473c                   mov      dword ptr [rdi + 0x3c], eax
0x140317075: 66c7471c0040             mov      word ptr [rdi + 0x1c], 0x4000
0x14031707b: 66c7472c00ff             mov      word ptr [rdi + 0x2c], 0xff00
0x140317081: 894f48                   mov      dword ptr [rdi + 0x48], ecx
0x140317084: 448d407e                 lea      r8d, [rax + 0x7e]
0x140317088: 488d4f5c                 lea      rcx, [rdi + 0x5c]
0x14031708c: c7471880808080           mov      dword ptr [rdi + 0x18], 0x80808080
0x140317093: 48c7474004000000         mov      qword ptr [rdi + 0x40], 4
0x14031709b: 48c7474c05000000         mov      qword ptr [rdi + 0x4c], 5
0x1403170a3: c747546666663f           mov      dword ptr [rdi + 0x54], 0x3f666666
0x1403170aa: c64758ff                 mov      byte ptr [rdi + 0x58], 0xff
0x1403170ae: e837fb0200               call     0x140346bea
0x1403170b3: c687db00000080           mov      byte ptr [rdi + 0xdb], 0x80
0x1403170ba: e9c1020000               jmp      0x140317380
0x1403170bf: b8b0000000               mov      eax, 0xb0
0x1403170c4: 66894706                 mov      word ptr [rdi + 6], ax
0x1403170c8: b802000000               mov      eax, 2
0x1403170cd: 89473c                   mov      dword ptr [rdi + 0x3c], eax
0x1403170d0: 66c7471c0040             mov      word ptr [rdi + 0x1c], 0x4000
0x1403170d6: 66c7472c00ff             mov      word ptr [rdi + 0x2c], 0xff00
0x1403170dc: c7471840404080           mov      dword ptr [rdi + 0x18], 0x80404040
0x1403170e3: 48c7474003000000         mov      qword ptr [rdi + 0x40], 3
0x1403170eb: 488d4f48                 lea      rcx, [rdi + 0x48]
0x1403170ef: 33d2                     xor      edx, edx
0x1403170f1: 41b880000000             mov      r8d, 0x80
0x1403170f7: e8eefa0200               call     0x140346bea
0x1403170fc: c687c700000080           mov      byte ptr [rdi + 0xc7], 0x80
0x140317103: e978020000               jmp      0x140317380
0x140317108: b8b0000000               mov      eax, 0xb0
0x14031710d: 66894706                 mov      word ptr [rdi + 6], ax
0x140317111: b802000000               mov      eax, 2
0x140317116: 89473c                   mov      dword ptr [rdi + 0x3c], eax
0x140317119: 66c7471c0080             mov      word ptr [rdi + 0x1c], 0x8000
0x14031711f: 66c7472c00ff             mov      word ptr [rdi + 0x2c], 0xff00
0x140317125: c7471880808080           mov      dword ptr [rdi + 0x18], 0x80808080
0x14031712c: 48c7474006000000         mov      qword ptr [rdi + 0x40], 6
0x140317134: ebb5                     jmp      0x1403170eb
0x140317136: b8e8000000               mov      eax, 0xe8
0x14031713b: c647021e                 mov      byte ptr [rdi + 2], 0x1e
0x14031713f: 66894706                 mov      word ptr [rdi + 6], ax
0x140317143: 488d4f64                 lea      rcx, [rdi + 0x64]
0x140317147: b802000000               mov      eax, 2
0x14031714c: c74720a0a0a000           mov      dword ptr [rdi + 0x20], 0xa0a0a0
0x140317153: c7472480808080           mov      dword ptr [rdi + 0x24], 0x80808080
0x14031715a: 33d2                     xor      edx, edx
0x14031715c: c7472880808060           mov      dword ptr [rdi + 0x28], 0x60808080
0x140317163: c7472c08000000           mov      dword ptr [rdi + 0x2c], 8
0x14031716a: c7473464000000           mov      dword ptr [rdi + 0x34], 0x64
0x140317171: 448d407e                 lea      r8d, [rax + 0x7e]
0x140317175: c7473864000000           mov      dword ptr [rdi + 0x38], 0x64
0x14031717c: 66c7473c8020             mov      word ptr [rdi + 0x3c], 0x2080
0x140317182: c7473001000000           mov      dword ptr [rdi + 0x30], 1
0x140317189: c747185a000000           mov      dword ptr [rdi + 0x18], 0x5a
0x140317190: c7471c2d000000           mov      dword ptr [rdi + 0x1c], 0x2d
0x140317197: 894760                   mov      dword ptr [rdi + 0x60], eax
0x14031719a: 66c747400080             mov      word ptr [rdi + 0x40], 0x8000
0x1403171a0: 66c7475000ff             mov      word ptr [rdi + 0x50], 0xff00
0x1403171a6: e83ffa0200               call     0x140346bea
0x1403171ab: c687e300000080           mov      byte ptr [rdi + 0xe3], 0x80
0x1403171b2: e9c9010000               jmp      0x140317380
0x1403171b7: b910000000               mov      ecx, 0x10
0x1403171bc: c7470402005000           mov      dword ptr [rdi + 4], 0x500002
0x1403171c3: 894f1c                   mov      dword ptr [rdi + 0x1c], ecx
0x1403171c6: b802000000               mov      eax, 2
0x1403171cb: 894f20                   mov      dword ptr [rdi + 0x20], ecx
0x1403171ce: 894754                   mov      dword ptr [rdi + 0x54], eax
0x1403171d1: c647023c                 mov      byte ptr [rdi + 2], 0x3c
0x1403171d5: c7472c00007a44           mov      dword ptr [rdi + 0x2c], 0x447a0000
0x1403171dc: c7473000009644           mov      dword ptr [rdi + 0x30], 0x44960000
0x1403171e3: c7472880808040           mov      dword ptr [rdi + 0x28], 0x40808080
0x1403171ea: c7471820000000           mov      dword ptr [rdi + 0x18], 0x20
0x1403171f1: c7472408000000           mov      dword ptr [rdi + 0x24], 8
0x1403171f8: 66c747340080             mov      word ptr [rdi + 0x34], 0x8000
0x1403171fe: 66c7474400ff             mov      word ptr [rdi + 0x44], 0xff00
0x140317204: c7475804000000           mov      dword ptr [rdi + 0x58], 4
0x14031720b: 48c7475c03000000         mov      qword ptr [rdi + 0x5c], 3
0x140317213: e968010000               jmp      0x140317380
0x140317218: c64702ff                 mov      byte ptr [rdi + 2], 0xff
0x14031721c: b878000000               mov      eax, 0x78
0x140317221: 66894706                 mov      word ptr [rdi + 6], ax
0x140317225: b802000000               mov      eax, 2
0x14031722a: 898784000000             mov      dword ptr [rdi + 0x84], eax
0x140317230: c7472880808010           mov      dword ptr [rdi + 0x28], 0x10808080
0x140317237: c7471801000000           mov      dword ptr [rdi + 0x18], 1
0x14031723e: 896f24                   mov      dword ptr [rdi + 0x24], ebp
0x140317241: c7472c64000000           mov      dword ptr [rdi + 0x2c], 0x64
0x140317248: c7473064000000           mov      dword ptr [rdi + 0x30], 0x64
0x14031724f: c7473400000080           mov      dword ptr [rdi + 0x34], 0x80000000
0x140317256: c74738ffffff80           mov      dword ptr [rdi + 0x38], 0x80ffffff
0x14031725d: 66c7477400ff             mov      word ptr [rdi + 0x74], 0xff00
0x140317263: c7878800000004000000     mov      dword ptr [rdi + 0x88], 4
0x14031726d: e90e010000               jmp      0x140317380
0x140317272: c647023d                 mov      byte ptr [rdi + 2], 0x3d
0x140317276: b820010000               mov      eax, 0x120
0x14031727b: 66894706                 mov      word ptr [rdi + 6], ax
0x14031727f: b802000000               mov      eax, 2
0x140317284: 89476c                   mov      dword ptr [rdi + 0x6c], eax
0x140317287: 896f18                   mov      dword ptr [rdi + 0x18], ebp
0x14031728a: c7471c00000080           mov      dword ptr [rdi + 0x1c], 0x80000000
0x140317291: c74720ffffff80           mov      dword ptr [rdi + 0x20], 0x80ffffff
0x140317298: 66c7475c00ff             mov      word ptr [rdi + 0x5c], 0xff00
0x14031729e: 898790000000             mov      dword ptr [rdi + 0x90], eax
0x1403172a4: 66c747700080             mov      word ptr [rdi + 0x70], 0x8000
0x1403172aa: 66c7878000000000ff       mov      word ptr [rdi + 0x80], 0xff00
0x1403172b3: c7871401000004000000     mov      dword ptr [rdi + 0x114], 4
0x1403172bd: 48c787200100000000803f   mov      qword ptr [rdi + 0x120], 0x3f800000
0x1403172c8: 48c787280100000000803f   mov      qword ptr [rdi + 0x128], 0x3f800000
0x1403172d3: c787180100000000803e     mov      dword ptr [rdi + 0x118], 0x3e800000
0x1403172dd: c7871c0100000000003f     mov      dword ptr [rdi + 0x11c], 0x3f000000
0x1403172e7: 4889af40010000           mov      qword ptr [rdi + 0x140], rbp
0x1403172ee: 89af48010000             mov      dword ptr [rdi + 0x148], ebp
0x1403172f4: c7874c0100000000003f     mov      dword ptr [rdi + 0x14c], 0x3f000000
0x1403172fe: 4889af60010000           mov      qword ptr [rdi + 0x160], rbp
0x140317305: 89af68010000             mov      dword ptr [rdi + 0x168], ebp
0x14031730b: 89af30010000             mov      dword ptr [rdi + 0x130], ebp
0x140317311: c78734010000cdcc4c3e     mov      dword ptr [rdi + 0x134], 0x3e4ccccd
0x14031731b: e98ffcffff               jmp      0x140316faf
0x140317320: b890000000               mov      eax, 0x90
0x140317325: c7472000000080           mov      dword ptr [rdi + 0x20], 0x80000000
0x14031732c: 488d4f28                 lea      rcx, [rdi + 0x28]
0x140317330: 66894706                 mov      word ptr [rdi + 6], ax
0x140317334: 33d2                     xor      edx, edx
0x140317336: c7471801000000           mov      dword ptr [rdi + 0x18], 1
0x14031733d: c7471c01000000           mov      dword ptr [rdi + 0x1c], 1
0x140317344: 448d40f0                 lea      r8d, [rax - 0x10]
0x140317348: 896f24                   mov      dword ptr [rdi + 0x24], ebp
0x14031734b: e89af80200               call     0x140346bea
0x140317350: c687a700000080           mov      byte ptr [rdi + 0xa7], 0x80
0x140317357: eb27                     jmp      0x140317380
0x140317359: c74720000000a0           mov      dword ptr [rdi + 0x20], 0xa0000000
0x140317360: b838000000               mov      eax, 0x38
0x140317365: 66894706                 mov      word ptr [rdi + 6], ax
0x140317369: 48c7474802000000         mov      qword ptr [rdi + 0x48], 2
0x140317371: 66c7472800ff             mov      word ptr [rdi + 0x28], 0xff00
0x140317377: 66c7473800ff             mov      word ptr [rdi + 0x38], 0xff00
0x14031737d: 896f24                   mov      dword ptr [rdi + 0x24], ebp
0x140317380: 488b5c2430               mov      rbx, qword ptr [rsp + 0x30]
0x140317385: 488b6c2438               mov      rbp, qword ptr [rsp + 0x38]
0x14031738a: 488b742440               mov      rsi, qword ptr [rsp + 0x40]
0x14031738f: 4883c420                 add      rsp, 0x20
0x140317393: 5f                       pop      rdi
0x140317394: c3                       ret      
0x140317395: 0f1f00                   nop      dword ptr [rax]
