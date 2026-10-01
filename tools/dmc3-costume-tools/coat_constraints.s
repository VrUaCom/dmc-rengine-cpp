# Dante coat node constraints (new section .dmcx, VA 0x140DAC000).
#
# Called from both coat setups of CPlDante's load (costume byte +0x3E9E: 1
# and 4 take 0x14021526F, the others 0x140214F74), at 0x1402151C3 and
# 0x140215373, after the coat joint table is bound (vtbl+0x150 ->
# 0x14030F850, which clears every joint +0x100), the capsule shapes are
# written to player +0xB630 and the cloth chain is parsed, just before the
# capsule setter 0x1402CA2F0. It replaces `lea rdx, [r14+0x1880]` there.
# r14 = player. Every volatile register is dead at the site except rdx, which
# is rebuilt before returning.
#
# Player PAC slot 15 (optional; the retail game never reads it):
#   +0x00 'CCNS'  +0x04 version 1  +0x08 node count (<= 16)
#   +0x0C capsule count (<= 6)
#   +0x10 node count x 0x50: u32 coat node, u32 body joint, u64 reserved,
#                            f32[16] offset matrix (row-vector, DMC3 layout)
#   then capsule count x 0x40: u32 shape index (0..5), 12 reserved bytes,
#        f32[4] A, f32[4] B, f32[4] radius -> player +0xB630 + index*0x50
#        + 0x10 (shape: A +0x10, B +0x20 in host-joint space, radius +0x30;
#        0x1402C97F0 reads them per frame)
# Each entry gets a constraint in mode 1 (0x1402CBBE0: world = offset x
# *host), as CEm028 init 0x140130480 builds for Nevan's sleeves:
#   +0x00 vtable 0x1404CC1F8, +0x20 enabled, +0x28 mode 1,
#   +0x30 host world pointer (body joint +0x110), +0x80..+0xBF offset.
# The skeleton update 0x14030E680 then calls the constraint instead of
# local x parent for that node; its children compose from it as usual.
# Coat joints: player +0xA0D0 + 8*node (39 allocated by 0x1401DE820).
# Body joints: player +0x1880 + 8*joint (96 allocated).

        .intel_syntax noprefix
# Absolute addresses come from the link (coat_patch.py passes them with
# ld --defsym) so that RIP-relative operands resolve against VA 0x140DAC000:
#   GETTER   0x1401B82C0  pac slot getter (table, 0, player id, slot)
#   PACTABLE 0x140C99D30  its table (lea at 0x140215274)
#   CVTABLE  0x1404CC1F8  constraint vtable (ctor 0x1400F7AC0)
#   OWNERS   0x140DAC400  u64[4] player owning each pool
#   RRIDX    0x140DAC420  u32 round-robin pool index
#   POOL     0x140DAD000  4 pools x 16 constraints x 0xC0

        .text
        .globl _start
_start:
        push    rbx
        push    rsi
        push    rdi
        push    r12
        sub     rsp, 0x28
        lea     rcx, [rip + PACTABLE]
        xor     edx, edx
        movzx   r8d, word ptr [r14 + 0x78]
        mov     r9d, 15
        call    GETTER
        test    rax, rax
        jz      .Ldone
        cmp     dword ptr [rax], 0x534E4343     # 'CCNS'
        jne     .Ldone
        cmp     dword ptr [rax + 4], 1
        jne     .Ldone
        mov     esi, dword ptr [rax + 8]
        cmp     esi, 16
        ja      .Ldone
        mov     r12, rax
        lea     rbx, [rax + 0x10]

        # Pool: the one this player already owns, else a free one, else the
        # next in round-robin order.
        lea     r10, [rip + OWNERS]
        xor     ecx, ecx
.Lfind_owner:
        cmp     qword ptr [r10 + rcx*8], r14
        je      .Lhave_pool
        inc     ecx
        cmp     ecx, 4
        jb      .Lfind_owner
        xor     ecx, ecx
.Lfind_free:
        cmp     qword ptr [r10 + rcx*8], 0
        je      .Lhave_pool
        inc     ecx
        cmp     ecx, 4
        jb      .Lfind_free
        lea     r11, [rip + RRIDX]
        mov     ecx, dword ptr [r11]
        lea     edx, [rcx + 1]
        and     edx, 3
        mov     dword ptr [r11], edx
.Lhave_pool:
        mov     qword ptr [r10 + rcx*8], r14
        imul    rdi, rcx, 0xC00
        lea     r11, [rip + POOL]
        add     rdi, r11

.Lentry:
        test    esi, esi
        jz      .Lcapsules
        mov     eax, dword ptr [rbx]
        cmp     eax, 39
        jae     .Lnext
        mov     edx, dword ptr [rbx + 4]
        cmp     edx, 96
        jae     .Lnext
        mov     r8, qword ptr [r14 + rax*8 + 0xA0D0]
        test    r8, r8
        jz      .Lnext
        mov     r9, qword ptr [r14 + rdx*8 + 0x1880]
        test    r9, r9
        jz      .Lnext
        mov     r9, qword ptr [r9 + 0x110]
        test    r9, r9
        jz      .Lnext
        xorps   xmm0, xmm0
        movaps  xmmword ptr [rdi + 0x00], xmm0
        movaps  xmmword ptr [rdi + 0x10], xmm0
        movaps  xmmword ptr [rdi + 0x20], xmm0
        movaps  xmmword ptr [rdi + 0x30], xmm0
        movaps  xmmword ptr [rdi + 0x40], xmm0
        movaps  xmmword ptr [rdi + 0x50], xmm0
        movaps  xmmword ptr [rdi + 0x60], xmm0
        movaps  xmmword ptr [rdi + 0x70], xmm0
        lea     r11, [rip + CVTABLE]
        mov     qword ptr [rdi], r11
        mov     byte ptr [rdi + 0x20], 1
        mov     dword ptr [rdi + 0x28], 1
        mov     qword ptr [rdi + 0x30], r9
        movups  xmm0, xmmword ptr [rbx + 0x10]
        movaps  xmmword ptr [rdi + 0x80], xmm0
        movups  xmm0, xmmword ptr [rbx + 0x20]
        movaps  xmmword ptr [rdi + 0x90], xmm0
        movups  xmm0, xmmword ptr [rbx + 0x30]
        movaps  xmmword ptr [rdi + 0xA0], xmm0
        movups  xmm0, xmmword ptr [rbx + 0x40]
        movaps  xmmword ptr [rdi + 0xB0], xmm0
        mov     qword ptr [r8 + 0x100], rdi
        add     rdi, 0xC0
.Lnext:
        add     rbx, 0x50
        dec     esi
        jmp     .Lentry

.Lcapsules:
        # rbx now points past the node records.
        mov     esi, dword ptr [r12 + 0xC]
        cmp     esi, 6
        ja      .Ldone
.Lcapsule:
        test    esi, esi
        jz      .Ldone
        mov     eax, dword ptr [rbx]
        cmp     eax, 6
        jae     .Lnext_capsule
        imul    rax, rax, 0x50
        lea     rax, [r14 + rax + 0xB630]
        movups  xmm0, xmmword ptr [rbx + 0x10]
        movups  xmmword ptr [rax + 0x10], xmm0
        movups  xmm0, xmmword ptr [rbx + 0x20]
        movups  xmmword ptr [rax + 0x20], xmm0
        movups  xmm0, xmmword ptr [rbx + 0x30]
        movups  xmmword ptr [rax + 0x30], xmm0
.Lnext_capsule:
        add     rbx, 0x40
        dec     esi
        jmp     .Lcapsule

.Ldone:
        add     rsp, 0x28
        pop     r12
        pop     rdi
        pop     rsi
        pop     rbx
        lea     rdx, [r14 + 0x1880]
        ret
