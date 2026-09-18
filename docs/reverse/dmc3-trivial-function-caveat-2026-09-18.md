# A two-byte function, seventeen vtables, and a limit on our own instruments

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-trivial-function-caveat`.

I went to read `0x0629C0` because the correction note had called it out: read and
seek keep their archive branch inside the file layer while the size dispatcher
tail-jumps outside it, and I wrote that "whatever computes an archive entry's
size does not live with the rest of the archive code." That sentence implied an
architectural fact. There is no architectural fact. There is a coincidence of
offsets, and the way I came to believe otherwise is worth more than the answer.

## What `0x0629C0` is

```text
0629C0  8B 41 20   mov eax,[rcx+0x20]
0629C3  C3         ret
```

Two bytes of body. Against the entry layout, `[entry+0x20]` is decoded struct
`+0x18`, which is record `+0x18`, the uncompressed size. So the size of an
archive entry is its uncompressed size and the dispatcher's tail jump is exactly
right.

It sits in a run of four such getters at sixteen-byte stride — `+0x20`, `+0x2C`,
`+0x28`, `+0x34` — in a region with no `.pdata` coverage at all.

## And what else it is

Searching the image for its address as a qword finds it in **17 pointer tables
in `.rdata`**. Two of them, dumped in context, settle what those tables are:

```text
0x4C68B8 -> .rdata 0x50E0C8      0x4CDCC8 -> .rdata 0x511598
0x4C68C0 -> .text  0x061780      0x4CDCD0 -> .text  0x0D8C44
0x4C68C8 -> .text  0x0629C0      0x4CDCD8 -> .text  0x0629C0
0x4C68D0 -> .text  0x196020      0x4CDCE0 -> .text  0x196020
0x4C68D8 -> .text  0x0629E0      0x4CDCE8 -> .text  0x0629E0
0x4C68E0 -> .text  0x0629D0      0x4CDCF0 -> .text  0x0629D0
0x4C68E8 -> .text  0x196010      0x4CDCF8 -> .text  0x196010
0x4C68F0 -> .text  0x0629F0      0x4CDD00 -> .text  0x0629F0
0x4C68F8 -> .text  0x061800      0x4CDD08 -> .text  0x061800
0x4C6900 -> .text  0x061980      0x4CDD10 -> .text  0x061980
0x4C6908 -> .text  0x062860      0x4CDD18 -> .text  0x062860
0x4C6910 -> .text  0x0622C0      0x4CDD20 -> .text  0x0622C0
0x4C6918 -> .text  0x0628D0      0x4CDD28 -> .text  0x0628D0
```

An `.rdata` pointer then a run of `.text` pointers is a complete-object locator
followed by a vtable, and the two vtables agree on every slot but the first. So
`0x0629C0` is a virtual accessor, slot one, shared by a large class family that
overrides only slot zero.

The project has measured this territory systematically already — evidence packet
`dmc3-hdc-vtable-slots` carries 702 base-and-slot pairs and the override ratios —
so the inheritance pattern here is an instance of something known, not a finding.

## Why the two uses cannot be the same thing

The archive entry is `calloc(1, 0x50)` by `0x328290`, and `[entry+0x00]` holds a
`_strdup` of the archive's path, freed and nulled on realisation. **There is no
vtable pointer at offset zero.** The entry is not an instance of that class
family, and `0x3272A0` reaches `0x0629C0` by a direct tail jump rather than
through any table.

Two two-byte functions with identical bodies, folded by the linker, or one helper
the compiler reused: the bytes do not say which, and it does not matter. What
matters is that the reference set of `0x0629C0` spans unrelated types, joined
only by both wanting the dword at `+0x20`.

## The limit this puts on our instruments

This reverse leans hard on the call graph. The file-access note's central claim
was a coupling ratio — 774 callers in against 17 callees out. The reachability
work brackets the image with caller sets and dispatch closures, and reports 1,641
functions no path reaches. Those are good measures and they are not in question
here, because they concern functions with real bodies.

They carry no semantic weight over a function whose body is one instruction. A
trivial function's callers are not the users of a concept; they are whoever
needed that offset. Any statement of the form "X is called from these places,
therefore X belongs to this subsystem" is unsound when X is two bytes long, and
the sentence I wrote in the correction note was exactly that form.

A census to size the exposure, and I ran it this time rather than asserting it.
Scanning `.text` for one-instruction `rcx`-relative getters immediately followed
by `ret` — six encodings, `mov eax`, `mov rax`, `movzx` from a word and from a
byte, `movss`, and `lea` — and requiring the preceding byte to be padding or a
return, finds **33, every one of them outside `.pdata`**:

| form | found |
| --- | --- |
| `mov eax,[rcx+d]` | 15 |
| `movss xmm0,[rcx+d]` | 8 |
| `movzx eax,byte[rcx+d]` | 5 |
| `mov rax,[rcx+d]` | 3 |
| `movzx eax,word[rcx+d]` | 1 |
| `lea rax,[rcx+d]` | 1 |

That is a **lower bound**, not a count: the padding requirement will miss a
getter that follows something else, and only six encodings were scanned. What is
exact is the second column of the summary — 33 of 33 outside `.pdata` — which is
the third instance today of the same blind spot, after the two missing ZIP
signature decoders and the third dispatcher.

## What is still unread

- Whether the linker folded these or the compiler emitted one. Deciding it would
  need the duplicate-body census to be run over function bodies rather than
  `.pdata` ranges; a first attempt found 107 groups of byte-identical ranges
  across 367 entries, but many of those ranges are one to fourteen bytes and are
  chained-unwind fragments rather than functions, so the test as run does not
  decide anything and is not reported as though it did.
- The inflater at `0x328820`, still.
- `0x328210`, the archive teardown.
- Sections 1C, 1E and 1F of the 2026-09-05 reconciliation.
