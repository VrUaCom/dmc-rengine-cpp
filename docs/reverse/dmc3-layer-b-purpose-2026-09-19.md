# Why layer B exists: it is the back end of two open modes nothing selects

2026-09-19. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-layer-b-purpose`.

The file-access note of 2026-09-18 found two independent file layers over one
platform and closed on "nothing here says why the engine has both." The
mount-registration note added a second axis separating them and left the question
open. `0x2FF12` answers it.

## The third path is layer B

`OpenGameResource` has three resolution paths. The prefixed mount search runs when
mode bit 1 is clear. With bit 1 set, bit 9 chooses between the other two, and
both of them end in the same place:

```text
bit 9 set   -> derive the executable's directory, build a path at 0x2F210,
               then 0x2FEFA: call 0x49120
bit 9 clear -> 0x2FF12: build a descriptor at 0x2E7D0; if its field at +0x10
               is null, release the slot and fail; otherwise
               0x2FF4C: call 0x49120
```

`0x49120` is the function the file-access note's import census named as layer B's
owner of `CreateFileA` and `GetFileSize`. Its **only two callers in the image are
those two sites**, and there is no indirect path in: its address does not appear
as a qword anywhere in the file, nor do those of layer B's read and seek entry
points.

So layer B's open is not an alternative the engine chooses between at runtime. It
is the implementation of two `OpenGameResource` modes, and the mount search is the
implementation of the third.

## And no caller selects those modes

Both modes require mode bit 1 to be set. Every one of `OpenGameResource`'s three
direct callers loads `edx = 1` — established in `l2-exe-reconciliation-2026-08-26`
§1.1 and re-confirmed here. Bit 1 is therefore never set anywhere in the image,
and **layer B's open is unreachable from the shipped call surface**.

That is the answer to the question the file-access note left open, and it also
explains the shape that note measured. Layer B looked like a library nobody used
— 16 callers in against 10 callees out, two users per dependency where layer A
had forty-six — because its entry point is the back end of a mode the game never
asks for.

## Not the same as dead

Layer B's other two entry points do have callers. `0x49200` is called from
`0x2F989` and from a thunk at `0x2FF70` that `0x33820E` reaches, and `0x33820E`
is inside one of `OpenGameResource`'s three callers. `0x49290` is called from
`0x2F953` and `0x2FC90`. Whether those callers are themselves reachable is **not**
established here: the function-map report committed in `evidence/` is a partial
artefact of 2,889 functions with its depth fields absent, so it cannot answer the
question, and I did not substitute a guess for it.

What can be said is narrower and still useful: the open is unreachable, so any
live use of layer B's read or seek must obtain its handle by a route this pass did
not find.

## The two layers differ in interface style, not just in shape

The read and seek calls settle what kind of API layer B is. `0x2F930` and
`0x2F972` operate on a transfer descriptor and call:

```text
0x2F953  mov ecx,[rsi+0x30] ; mov edx,[rsi+0x10] ; xor r8d,r8d ; call 0x49290
0x2F989  mov ecx,[rsi+0x30] ; mov rdx,rdi ; mov r8d,ebx     ; call 0x49200
```

The first argument is a **32-bit value loaded from the descriptor**, not a
pointer. Layer A's read, seek and size all take a pointer to a reader object or an
archive entry; layer B's take an integer. So layer B is an id-based API over a
handle table and layer A is a pointer-based one, which is a sharper distinction
than the coupling ratio the file-access note used and explains how two layers can
coexist without either wrapping the other: they do not have compatible calling
conventions to wrap.

`0x2F972`'s loop is worth one line because it is the shape a port must match: read
`[desc+0x20]` bytes into `[desc+0x18]`, advance both by what came back, stop when
the count reaches zero, treat a negative return as failure and a zero return as
the end.

## What is still unread

- Where a layer-B id comes from, given that its open is unreachable.
- `0x2E7D0` and `0x2E5F0`, the descriptor's constructor and destructor, beyond
  their positions in the failure path.
- What writes the slot byte at `+0x19` that the success path reads; not this
  path, which does not touch it.
- Section 1F of the 2026-09-05 reconciliation.
- The 1,641 functions no path reaches, as a population. The report in `evidence/`
  cannot be used for that question either, for the reason above.
