# GDSpaces L3 R1 — leaf/no-unwind and completion-callback alias pass — 2026-08-26

## Authority

Canonical analysis executable:

- `dmc3.exe`
- size `6,356,432` bytes
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- preferred image base `0x140000000`

Primary layer: **[L3] Original Runtime / Resource Lifecycle**.

This pass continues the R1 state-writer census after `l3-r1-direct-writer-census-2026-08-26.md`. It does not reopen the already recovered central lifecycle spine and does not claim L3 complete.

## Why this pass exists

The previous whole-image census was intentionally bounded by PE x64 unwind/runtime-function entries plus direct references to LoadedResource registry base `0x140C99D30`. That filter rejects a useful false-positive class, but it can miss:

1. leaf functions with no `.pdata` unwind entry;
2. a state writer that receives an already-derived record pointer from a caller;
3. indexed/derived addressing with no direct registry-base reference in the writer itself;
4. callback functions whose record identity is encoded as a compact argument rather than a pointer.

The most important known example is normal completion callback `0x1401B8DC0`: it is itself a leaf/no-unwind function.

## Method

The pass used the reacquired canonical raw executable and full disassembly to:

1. enumerate exact immediate state-like writes of the form `dword [object + 0x04] <- 0..4`;
2. identify candidates outside PE unwind-covered function bodies;
3. recover the leaf-function start using padding/function-boundary context;
4. inspect every direct caller and destination-object provenance;
5. separately trace `0x1401B8DC0` from registration through scheduler storage and callback invocation;
6. require exact LoadedResource registry/record provenance before promotion.

A matching `+0x04` field is not sufficient evidence. The destination must be `0x140C99D30 + index*0x48` or a proven alias to such a record.

## Leaf/no-unwind candidate census

The bounded exact-immediate leaf class reduces to six function bodies.

### 1. `0x1401B8DC0` — TRUE LoadedResource writer

Normal branch:

```text
context -> EAX
base = 0x140C99D30
[base + context + 0x04] = 2
```

This is the already canonical normal materialization-completion state-2 writer.

Its registration and argument ABI are strengthened below.

### 2. `0x14027A760` — REJECTED as LoadedResource writer

The state-like write is:

```text
[RCX + 0x04] = 3
```

Both direct callers prove `RCX` is a subobject at `parent + 0x63D0`:

- caller around `0x1401E955E`: `lea rcx,[rdi+0x63D0]`;
- caller around `0x1401EAC35`: `add rcx,0x63D0`.

The function also writes other local fields such as `+0x30/+0x33`. No LoadedResource record provenance exists.

**Status:** non-L3 higher-level object state machine for this census.

### 3. `0x1402CC470` — REJECTED as LoadedResource writer

The function initializes a much larger object, including fields through at least `+0x330`, matrix blocks and multiple pointers. A direct caller around `0x14005A52B` first initializes/allocates a `0x340`-byte object and then calls this constructor.

The `+0x04 = 0` write is therefore an object-constructor field, not LoadedResource state.

### 4. `0x1402CCE80` — REJECTED as LoadedResource writer

This is another large-object initializer with matrix/pointer fields extending far beyond a `0x48` LoadedResource record. Direct callers around `0x14005C4AB` and `0x14005C5CA` pass pointers held in a separate object/table and clear those table slots after the call.

The `+0x04 = 0` write is not a LoadedResource state write.

### 5. `0x14032D3E0` — REJECTED as LoadedResource writer

This leaf builds a packed handle/descriptor-like value:

```text
qword [RCX+0x00] = 0
qword [RCX+0x08] = 0
encoded value derived from EDX/R9D
[RCX+0x04] = 1
[RCX+0x00] = encoded value
return RCX+0x10
```

Direct callers pass heap/subobject pointers from unrelated structures, including pointers loaded from arrays/fields around `0x1402F3932`, `0x1403336DD`, `0x1403342B7` and `0x14033621D`.

No canonical LoadedResource record provenance exists.

### 6. `0x140333A20` — REJECTED as LoadedResource writer

This is a small clamp/setter:

```text
if EDX < 0: [RCX+4] = 0
else:       [RCX+4] = min(EDX,8)
```

Its direct caller around `0x1402DFFE9` supplies global object `0x1405D1880`, not a LoadedResource record.

## Leaf-census result

For this bounded class:

- **one** true LoadedResource state writer exists: the already-known `0x1401B8DC0`;
- **five** leaf/no-unwind state-like candidates are rejected by destination/caller provenance;
- **zero new LoadedResource state writers** are promoted.

This closes the exact immediate `dword [object+0x04] <- 0..4` leaf/no-unwind candidate class at the reviewed direct-call surface.

It does **not** close non-immediate state writes, indirect function-pointer registrations, derived aliases that never use a literal `+0x04` immediate form, or every possible code-generation pattern in the image.

## `0x1401B8DC0` completion callback — recovered scheduler ABI

### Registration in `0x1401B84E0`

After materialization dispatch succeeds, acquisition performs:

```text
base = 0x140C99D30
record.state = 1
callback = 0x1401B8DC0
context = low32(record_ptr - base)
argument_count = 1
scheduler = 0x1402EF580
```

The relevant ordering is therefore:

```text
materialization success
 -> state 1
 -> compute record-relative context
 -> queue 1B8DC0(context)
```

### Queue storage in `0x1402EF580`

The scheduler uses `0x88`-byte queue records. For the queued callback it stores, at the observed bounded path:

```text
queue_record +0x00 : scheduler record state/type
queue_record +0x04 : argument-count/dispatch metadata
queue_record +0x08 : callback pointer
queue_record +0x10... : copied u32 callback arguments
```

For `1B8DC0`, argument count is exactly one and the one dword is the record-relative context supplied by `1B84E0`.

### Callback invocation in `0x1402EF790`

When a queued callback record is dispatched with one argument, the scheduler executes the equivalent of:

```text
ECX = dword(queue_record + 0x10)
call qword(queue_record + 0x08)
```

Thus `0x1401B8DC0` receives the same 32-bit record-relative context calculated by acquisition.

## Exact normal-context domain

The LoadedResource registry is:

```text
base   = 0x140C99D30
count  = 363
stride = 0x48
```

For a valid canonical record index `i`:

```text
context = i * 0x48
0 <= i <= 362
0 <= context <= 0x65D0
```

Every valid normal context is therefore:

- aligned to `0x48`;
- even;
- low bit = `0`.

The normal callback branch reconstructs the record as:

```text
record = 0x140C99D30 + context
record.state = 2
```

## Odd/tagged `CL & 1` branch — boundary correction

`0x1401B8DC0` begins by testing the low bit of its context.

The odd branch mechanically does:

```text
if (CL & 1) {
    EAX = 0;
    dword [EAX + 4] = 2;
    return;
}
```

That means an odd context would target address `0x00000004`, not another LoadedResource record.

Current direct-xref evidence finds only one direct registration reference to `0x1401B8DC0`: the acquisition path in `0x1401B84E0`. That path can only supply `i*0x48`, so its normal callback context can never enter the odd branch.

**Promotion:** the odd branch is **outside the recovered canonical normal acquisition-registration domain**.

Do not invent a gameplay/resource meaning for it. Its higher-level intent (defensive trap, impossible tagged case, legacy generic callback convention, or another role) is not proven. The mechanical invalid-address behavior is observed; semantic intent remains unresolved.

This supersedes treating the low-bit branch as a plausible second normal state-2 record path.

## R1 status after this pass

### Bounded/closed classes

- central direct state-writer spine;
- direct-base + unwind-bounded immediate writer census;
- exact-immediate leaf/no-unwind `+0x04 <- 0..4` candidate class;
- normal `1B8DC0` registration/context/storage/invocation ABI;
- odd-context branch excluded from the canonical normal acquisition-registration domain.

### Still genuinely open

1. non-immediate state writes where value reaches `record+0x04` through a register;
2. caller-propagated record pointers into non-leaf helpers without direct registry references;
3. indexed/derived aliases whose writer expression does not expose the canonical base;
4. indirect callback/function-pointer registrations not represented by a direct `1B8DC0` xref;
5. any family-specific state authority that survives exact destination-provenance review.

The next R1 pass should therefore be data-flow driven from known record-producing/accepting helpers (`1B8DF0`, fixed-family record selectors/wrappers, `1B84E0`, release/reset helpers), rather than another global textual search for `+0x04`.

## Layer-map implication

The canonical layer map remains unchanged:

```text
L2 selected provider identity
 -> L1 exact materialized bytes
 -> L3 lifecycle ownership/state/ready/release
```

This pass strengthens only L3. It does not advance L1 retail Level-E evidence or L2 protected-process selected-provider evidence.

## Completion claim

**R1 is narrower, not complete. L3 is not complete.**

No new state meaning, resource family or lifecycle equivalence is promoted beyond the bounded evidence above.