# GDSpaces L3 R1 — final research / reverse / approval review — 2026-08-26

## Decision

**R1 static writer census: APPROVED as BOUNDED-CLOSED / CONTRADICTION-GATED for the canonical analysis image.**

This approval is deliberately narrower than Layer-3 completion.

It means the current static reverse has saturated the declared `LoadedResource +0x04` writer problem strongly enough to stop broad discovery and proceed to the next L3 reverse question. It does **not** replace original-process V1–V7 receipts, cross-build/profile evidence, R2 field ownership, R3 typed/factory/dependency work, or R4 shared-owner breadth.

R1 must be reopened if later evidence supplies a contradictory state write with exact `LoadedResource` record provenance.

## Authority

Canonical instruction-reverse executable:

- `dmc3.exe`
- size `6,356,432` bytes
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- PE32+ x64
- preferred ImageBase `0x140000000`

Canonical registry authority:

```text
LoadedResource base = 0x140C99D30
record count        = 363
record stride       = 0x48
state field         = record + 0x04
```

Raw group topology remains:

```text
counts = [4,136,60,28,1,128,6]
bases  = [0,4,140,200,228,229,357,363]
```

This review was performed after the direct-base, leaf/no-unwind, derived-alias, higher-level-state exclusion, residual state-writer and provenance-closure passes. It intentionally challenges the proposed closure instead of assuming those passes were sufficient.

## Reviewed evidence chain

Canonical preceding documents:

1. `l3-r1-direct-writer-census-2026-08-26.md`;
2. `l3-r1-leaf-alias-pass-2026-08-26.md`;
3. `l3-r1-derived-alias-pass-2026-08-26.md`;
4. `l3-r1-higher-level-state-exclusion-pass-2026-08-26.md`;
5. `l3-r1-residual-state-writer-pass-2026-08-26.md`;
6. `l3-r1-provenance-closure-pass-2026-08-26.md`.

The final review adds independent challenge passes for:

- startup/bulk-zero state initialization;
- one-hop interprocedural hidden writer candidates;
- producer-return/member-slot alias propagation;
- partial-width and non-enum `+0x04` writes;
- lock/atomic mutation forms;
- completion/cancellation callback-address surfaces;
- malformed completion context behavior;
- possible state-domain values outside canonical `0..4`.

## 1. Initialization is two distinct phases

### Static/startup pre-zero

`0x140010670` performs:

```text
RCX = 0x140C99D30
EDX = 0
R8D = 0x6768
call 0x140346BEA
```

It therefore zeroes the full `0x6768`-byte manager object before calling manager construction `0x1401B78F0` and registering CRT destructor `0x14034E840`.

This is **process/static initialization**, not an ordinary lifecycle transition.

The `0x6768` manager region is larger than `363 * 0x48 = 0x6618`, so this operation must not be mislabeled as only a per-record state reset.

### Runtime registry initialization

`0x1401B8380` separately iterates all 363 records. With `RBX = record+0x28`, it executes:

```text
0x1401B83A5: dword [RBX-0x24] = 0
```

which is exactly:

```text
record.state = 0
```

then resets the backing subobject and assigns the seven group IDs from the raw count/base tables.

**Promotion:** startup whole-manager zeroing and runtime per-record state0 initialization are distinct evidenced phases and remain distinct in the recovered model.

## 2. Bulk-zero / memset hidden-writer challenge

A whole-image direct-call review of the canonical zeroing primitive `0x140346BEA` was cross-checked against resource-aware functions.

The resource-aware call surface contains four relevant call sites in three functions:

- `0x140010670` — exact whole-manager zero described above;
- two calls inside `0x140046C40` — zero freshly allocated independent `0x60`-byte objects before their own vtable/field initialization;
- `0x140211E50` — zero a separate owner buffer at `owner+0x6438`, sized from an owner count.

Only `0x140010670` targets the canonical LoadedResource manager.

**Result:** no second runtime bulk-zero path is evidenced that silently resets `LoadedResource.state` outside the documented startup/runtime/reset writers.

## 3. Interprocedural hidden-writer challenge

A broad one-hop screen asks a deliberately dangerous question:

> can a resource-aware caller pass an object to a helper that has no direct registry reference, while that callee writes its incoming `arg+0x04`?

Two previously unclassified write-capable targets survived the first syntactic screen.

### `0x140059390` — REJECTED

This function reads and rewrites a compact four-dword state at offsets `0x00/0x04/0x08/0x0C` using independent multiplicative/additive transforms:

```text
[rcx+0x00] *= 0x41C64E6D; +0x3039
[rcx+0x04] *= 0x7FFFFFFF; +0x62C137
[rcx+0x08] *= 0x2004D71B; +0x2C9D9
[rcx+0x0C] *= 0x2BFFE969; +0x105D7B69
```

It then mixes those values to produce a result.

This is a small PRNG/hash/random-state transform, not a `LoadedResource` record mutator. The object contract itself disproves a lifecycle-state interpretation.

### `0x1403097D0` — REJECTED

The function explicitly establishes:

```text
RBP = RSP + 0x20
```

and candidate write `0x140309834` is:

```text
[rbp+0x04] = converted coordinate/value
```

The same stack frame holds local fields at `+0x00/+0x08/+0x10`; later logic derives `0x380`-stride and `0x1A0`-stride resource-internal records.

**Result:** the write is a stack local, not the incoming `RCX` object and not a `LoadedResource` record.

### Interprocedural result

Neither surviving generic one-hop target is a hidden lifecycle writer.

This independently supports the earlier provenance pass rather than merely repeating it.

## 4. Exact record-producer -> stored alias -> callee mutation challenge

The review seeds provenance from the known record-producing/record-selecting set, including:

- `0x1401B8DF0`;
- `0x1401B8EA0`;
- `0x1401B8D60`;
- `0x1401B8F50`;
- `0x1401B8FF0`;
- `0x1401B90B0`;
- `0x1401B9160`;
- `0x1401B9270`.

Observed ownership propagation repeatedly follows:

```text
record-producing call
 -> RAX = LoadedResource*
 -> store exact RAX in a higher-level owner slot
 -> later reload exact slot
 -> inspect/use/release record
```

The final challenge follows those exact stored pointers into direct callees and asks whether the callee mutates incoming `pointer+0x04`.

The write-capable target collapses to the already-canonical ordinary release helper:

```text
0x1401B9530
```

whose semantics remain:

```text
state == 3
 -> release backing
 -> if release succeeded
 -> state = 0
```

**Result:** no new hidden state writer is introduced through the reviewed record-producer/member-slot one-hop ownership surface.

This is the important distinction between an offset-coincidence scan and actual pointer provenance.

## 5. Partial-width and non-enum adjacency challenge

The final review does not assume every lifecycle write must look like `DWORD [x+4] = 0..4`. Residual resource-aware `+0x04` mutations with other widths/values were reviewed for object provenance.

Representative exclusions:

### `0x1401BAEC6`

```text
[rsi+0x04] = source+0x34
```

`RSI` is a caller-supplied output vector/result structure; neighboring writes fill output `+0x00/+0x08/+0x0C`. It is not a registry record.

### `0x14028C9F2`

```text
[rdi+0x04] = -1
```

`RDI` is a separate higher-level request/entry state. The same owner stores the real `LoadedResource*` in a different slot (`owner + index*8 + 0x310`) and reads canonical `record+0x04` through that separate pointer.

### `0x1402B4843`

```text
byte [rbx+0x04] = 1
```

`RBX` advances through a separate compact entry array at stride `0x06`. The same function stores exact `LoadedResource*` values separately at `owner + index*8 + 0x118` and reads their dword state independently.

Therefore the byte field cannot be canonical `LoadedResource.state`.

### Result

No partial-width or non-enum `+0x04` mutation survives exact LoadedResource provenance on the reviewed resource-aware surface.

## 6. Atomic/lock-form challenge

No provenance-backed `LOCK` / `CMPXCHG` / `XCHG` mutation of the canonical `record+0x04` field is evidenced.

Apparent `XCHG` decodes around the `0x1402787xx` dispatch area fall inside an embedded jump-table/data region after executable branch bodies; they are not promoted as state-mutating instructions.

The canonical state writer map therefore remains ordinary memory stores on the bounded image rather than an unobserved atomic state machine.

## 7. Completion callback registration surface

### Normal completion callback `0x1401B8DC0`

Whole-image disassembly exposes exactly one conventional code reference materializing this callback:

```text
0x1401B8572: LEA RDX -> 0x1401B8DC0
```

This is the already-canonical acquisition path inside `0x1401B84E0`.

Raw-image search also finds:

- zero exact 64-bit absolute pointer literals equal to VA `0x1401B8DC0`;
- zero exact 32-bit RVA literals equal to `0x001B8DC0`.

No second static callback-table registration is evidenced.

### Deferred cancellation callback `0x1401B8F00`

Whole-image disassembly likewise exposes one conventional code reference:

```text
0x1401B8491: LEA RDX -> 0x1401B8F00
```

This is the canonical cancellation registration path in `0x1401B8430`.

The exact-code-reference result is the authority; coincidental 32-bit byte sequences elsewhere in data/unwind structures are not treated as function-pointer evidence.

## 8. `0x1401B8DC0` low-bit branch — final classification

Normal acquisition registers callback context as:

```text
context = low32(record_ptr - 0x140C99D30)
```

and scheduler `0x1402EF580/0x1402EF790` later passes that one `u32` value in `ECX`.

For the 363-record registry:

```text
context = index * 0x48
index   = 0..362
range   = 0..0x65D0
```

Every valid normal context is therefore even and has low bit zero.

The odd branch in `0x1401B8DC0` mechanically performs:

```text
if (CL & 1) {
    RAX = 0;
    dword [RAX+4] = 2;
    return;
}
```

So an odd context attempts a write to virtual address `0x4` rather than to the registry.

**Final static classification:** this is a malformed/non-normal-context fault path outside the recovered canonical acquisition registration domain. No semantic name such as “alternate state2 transition” or “sentinel record” is promoted. Under normal user-mode null-page mapping the write is faulting; the raw evidence requirement is only the attempted VA `0x4` write.

R1 may be reopened if an exact runtime/cross-build path is later shown to register an odd context deliberately.

## 9. State-domain contradiction check

The canonical writer map publishes only states `0..4`.

Higher-level objects that use overlapping values plus `5`, `6`, `7` or other values have been separated by exact pointer/object provenance. The large `0x1401C83xx..0x1401C98xx` family is the strongest example: it stores exact `LoadedResource*` values in separate fields while maintaining its own `object+0x04` state domain including state5.

No provenance-backed canonical `LoadedResource.state > 4` writer is evidenced by the static passes.

This remains contradiction-gated by future original-process/cross-build evidence.

## 10. Canonical R1 writer map approved by review

### Startup/static initialization

- `0x140010670` — bulk zero full manager before construction; not an ordinary runtime transition.

### Runtime state 0 initialization

- `0x1401B8380` / write at `0x1401B83A5` — explicit per-record state0 initialization.

### State 1

- `0x1401B84E0` / `0x1401B8569` — acquisition publication after materialization success.

### State 2

- `0x1401B8DC0` / normal even-context path — completion publication.

### State 3

- `0x1401B92D0` — typed post-load -> optional callback -> state3.

### State 4

- `0x1401B8430` — canonical cancellation; source domain only state1/state2.

### State 0 teardown/rollback families

Keep distinct:

- acquisition rollback: `1B8DA4`, `1B8E60`, `1B92B8`;
- deferred state4 cleanup: `1B8F00`;
- ordinary owner release: `1B9530` / write near `1B9546`;
- fixed-family indexed release: `1B9914`, `1B997B`, `1B99D7`, `1B9A3B`;
- stored group5 aliases: `1B946D`, `1B96CC`, `1B9B9A`, `1B9C83`;
- forced group reset: `1B95B9`;
- forced full reset: `1B9609`.

These paths intentionally retain their different ordering/success policies.

## 11. Approval checklist

| R1 closure question | Result |
|---|---|
| Canonical central writers recovered? | PASS |
| Direct-base whole-image class reviewed? | PASS |
| Leaf/no-unwind class reviewed? | PASS |
| Indexed/derived record aliases reviewed? | PASS |
| Stored/caller-propagated state0 aliases reviewed? | PASS |
| Higher-level overlapping state machines separated? | PASS |
| Residual immediate state1–4 writes reviewed? | PASS |
| Non-immediate direct-registry writes reviewed? | PASS |
| Producer/member-slot cross-function aliases challenged? | PASS |
| Generic one-hop hidden writer candidates reviewed? | PASS |
| Bulk-zero/memset paths reviewed? | PASS |
| Partial-width/non-enum adjacency reviewed? | PASS |
| Atomic/lock mutation form challenged? | PASS |
| Normal completion callback static refs bounded? | PASS |
| Cancellation callback static refs bounded? | PASS |
| Odd completion context classified without invented semantics? | PASS |
| Contradictory provenance-backed state >4 writer found? | NO |

## 12. Approval boundary

### Approved

For the exact canonical analysis executable identified above:

**R1 = STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED.**

Broad state-writer discovery should stop.

### Reopen R1 only when at least one of these occurs

1. a later R2/R3 reverse pass yields a concrete additional write to `record+0x04` with exact record provenance;
2. an original-process V1–V7 trace observes a transition inconsistent with the approved writer map;
3. a different executable build/profile yields a contradictory exact writer;
4. a previously unknown runtime-computed callback/alias path is recovered and reaches `record+0x04`;
5. direct new binary evidence contradicts a writer classification above.

A vague numeric `+0x04` resemblance is not sufficient to reopen R1.

## Next work order

Only after this approval is merged/reconciled may L3 static reverse advance to **R2 family-complete field/backing ownership**:

```text
+0x08 selector/index metadata
+0x18 descriptor/source authority
+0x20 consumer payload alias
+0x28 backing/owned subobject
+ stable adjacent fields
```

R2 must preserve the R1 writer map as an input invariant and report any contradiction immediately.

## Layer completion

**Layer 3 remains NOT COMPLETE.**

R1 approval closes one static sub-gate only. R2–R5 and original-process V1–V7 remain mandatory.