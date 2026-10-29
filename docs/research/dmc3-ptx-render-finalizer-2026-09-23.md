# PTX render descriptor finalizer

Date: 2026-09-23. Branch: **Ада-Астра**.
Canonical executable SHA-256:
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

**Result: the bounded finalizer path and its resource publication step are
statically confirmed.** The two observed direct callers are the real materializer
paths at `0x140336B6B` and `0x140336CAB`. The downstream draw and shutdown
consumers of its registry are not established yet.

## Finalizer behavior

`0x140331A80` takes a context whose record count is
`dword[context+0x200] * dword[context+0x204]`. For a positive count, it walks
qword record pointers from the context. Each record supplies words at `+0x04`,
`+0x06`, `+0x08`, `+0x0A`, `+0x0C`, and a resource pointer at `+0x10`. The
finalizer resets record dword `+0x3C` to zero and sets `+0x40` to one before
building the render descriptor. It then stores the descriptor pointer at
`+0x30`, marks dword `+0x38` as one, and registers the resource pointer. A
nonpositive product returns `AL=1` without entering the loop.

Descriptor construction uses `0x14032D6B0`, which clears `0xA0` bytes at the
selected descriptor address and fills its fields from record metadata and the
resource pointer. The finalizer then calls `0x140033370` with the signed record
word at `+0x06` and the resource pointer. That helper stores the pointer in
registry `0x1405E1830` at index `identifier >> 2`, then writes the identifier to
resource dword `+0x50`.

## Lifecycle boundary

Both direct callers are materializer variants; each calls the finalizer after
its record loop. The bounded static evidence does not identify code that reads
registry `0x1405E1830`, nor prove when entries are cleared or when pointed
resources are released. This pass confirms publication and per-record state
changes, but not draw visibility, GPU acceptance, or shutdown ownership.

## Artifacts

- [Canonical finalizer audit](../../scripts/reverse/audit_ptx_render_finalizer.py)
- [Instruction and caller evidence](../../data/reverse/ptx-render-finalizer-20260923/evidence.json)
- [Materializer implementation and prior evidence](dmc3-ptx-materializer-2026-09-16.md)

The audit checks canonical instruction boundaries, direct targets, descriptor
zeroing, registry indexing, and the two materializer callsites. It is static
analysis, not runtime differential execution.
