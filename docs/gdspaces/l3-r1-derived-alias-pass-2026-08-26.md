# GDSpaces L3 R1 — derived-record aliases and family release writers — 2026-08-26

## Authority

Canonical analysis executable:

- `dmc3.exe`
- size `6,356,432` bytes
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- preferred image base `0x140000000`

Primary layer: **[L3] Original Runtime / Resource Lifecycle**.

This pass follows `l3-r1-direct-writer-census-2026-08-26.md` and `l3-r1-leaf-alias-pass-2026-08-26.md`. Its target is the R1 class deliberately left open by those passes: a LoadedResource record is derived by index or propagated through a higher-level object, so the final `record+0x04` write does not need a direct registry-base reference in the writer instruction sequence.

No new state meaning is invented. The goal is writer authority and provenance.

## Raw seven-group table revalidation

The canonical raw executable directly contains the group topology in `.data`.

At VA `0x140581A10`, seven little-endian `u16` counts are:

```text
[4, 136, 60, 28, 1, 128, 6]
```

Raw bytes:

```text
04 00 88 00 3C 00 1C 00 01 00 80 00 06 00
```

At VA `0x140581A20`, eight little-endian cumulative base indices are:

```text
[0, 4, 140, 200, 228, 229, 357, 363]
```

Raw bytes:

```text
00 00 04 00 8C 00 C8 00 E4 00 E5 00 65 01 6B 01
```

The final base `363` equals the full registry record count. This directly binds the already recovered seven-group topology to canonical `.data` rather than leaving it as summary-only inference.

Registry record address remains:

```text
record(i) = 0x140C99D30 + i * 0x48
```

## Class A — acquisition-failure rollback writers

These writers are not generic release entrypoints. They are rollback edges after an attempted family acquisition has already selected a canonical LoadedResource record.

### Group-base 228 singleton path — `0x1401B8D60`

The helper loads base index from `0x140581A28` (`228`), derives the exact record at `registry + 228*0x48`, and calls `0x1401B84E0` when the record is not already ready.

If acquisition fails, it releases `record+0x28` through `0x140337710`; only when that backing release returns success does it write:

```text
0x1401B8DA4: record.state = 0
```

### Group-5 first-free path — `0x1401B8DF0`

This helper uses:

- count `0x140581A1A = 128`;
- base `0x140581A2A = 229`.

It scans canonical records `229..356` for the first state0 slot, initializes family metadata and calls `0x1401B84E0`.

On acquisition failure it releases `record+0x28`; on successful release:

```text
0x1401B8E60: record.state = 0
```

This is the same group whose no-free-slot path has already been bounded as a hard original runtime invariant rather than safe product behavior.

### Group-base 357 indexed path — `0x1401B9270`

The helper loads base `0x140581A2C = 357`, derives `record(357 + selector)`, writes family selector metadata at `record+0x08`, and calls acquisition when not already state3.

If acquisition fails, backing release success gates:

```text
0x1401B92B8: record.state = 0
```

### Promotion

`0x1401B8DA4`, `0x1401B8E60` and `0x1401B92B8` are therefore **LoadedResource state0 writers with acquisition-failure rollback semantics** on their bounded family paths.

They are separate from ordinary owner release, state4 deferred cleanup and forced reset.

## Class B — fixed-family indexed ready-record release helpers

Four helpers take the canonical registry base as `RCX`, derive a fixed-family record through the `.data` base-index table, require state3, release `record+0x28`, and set state0 only when backing release succeeds.

Direct callers on the reviewed surface explicitly load:

```text
RCX = 0x140C99D30
```

before calling these helpers. Therefore the destination provenance does not rely only on arithmetic resemblance.

### `0x1401B98D0` — base index 0

Loads `0x140581A20 = 0` and derives:

```text
index = base0 + u8(DL) + u8(R8B)
record = registry + index*0x48
```

If `record.state == 3`, backing release success gates:

```text
0x1401B9914: record.state = 0
```

### `0x1401B9990` — base index 4

Loads `0x140581A22 = 4` and derives:

```text
index = base1 + u8(DL)*0x22 + u8(R8B)
```

Backing release success gates:

```text
0x1401B99D7: record.state = 0
```

### `0x1401B99F0` — base index 140

Loads `0x140581A24 = 140` and derives:

```text
index = base2 + u8(DL)*0x0F + s8(R8B)
```

Backing release success gates:

```text
0x1401B9A3B: record.state = 0
```

### `0x1401B9930` — base index 200

Loads `0x140581A26 = 200` and derives:

```text
index = base3 + u8(DL)*7 + s8(R8B)
```

Backing release success gates:

```text
0x1401B997B: record.state = 0
```

### Promotion

These are **family-specific indexed LoadedResource release writers** for the fixed registry partitions beginning at bases `0`, `4`, `140` and `200`.

Do not attach stronger gameplay names to those partitions from this pass alone. The mechanical partition/index/release authority is sufficient for R1.

## Class C — caller-propagated group-5 record aliases

The next class does not recompute `registry + index*0x48` at release time. A higher-level object stores an exact LoadedResource pointer at `object+0x10`, and later lifecycle states use that alias.

### Wrapper pair `0x1401B94C0` / `0x1401B9420`

`0x1401B94C0` initializes a higher-level object and calls dynamic group-5 acquisition:

```text
RCX = 0x140C99D30
call 0x1401B8DF0
object+0x10 = returned_record
```

A direct higher-level caller uses the same subobject address `parent+0x1298` for both initialization (`1B94C0`) and later lifecycle update (`1B9420`).

`0x1401B9420` loads:

```text
record = qword(object+0x10)
```

then calls `0x140337710(record+0x28)`. On success:

```text
0x1401B946D: record.state = 0
object+0x10 = null
```

This closes the provenance chain:

```text
group5 acquisition -> exact record pointer -> object+0x10 alias -> release -> state0
```

### Stateful wrapper `0x1401B9660`

Within the same function, one state acquires a group-5 record through `0x1401B8DF0` and stores:

```text
0x1401B98A1: object+0x10 = returned_record
```

A later state loads the same alias, releases `record+0x28`, and on success writes:

```text
0x1401B96CC: record.state = 0
object+0x10 = null
```

The same function also reads `record.state` through that alias before advancing its higher-level state machine.

### Stateful wrapper `0x1401B9A50`

This function likewise calls `0x1401B8DF0` and stores:

```text
0x1401B9AC1: object+0x10 = returned_record
```

Later branches load that exact pointer and perform release-to-state0 sequences:

```text
0x1401B9B8F: backing release
0x1401B9B9A: record.state = 0 on success

0x1401B9C78: backing release
0x1401B9C83: record.state = 0 on success
```

Both paths then clear `object+0x10`.

### Promotion

`0x1401B946D`, `0x1401B96CC`, `0x1401B9B9A` and `0x1401B9C83` are **LoadedResource state0 writers via caller-propagated/stored record aliases**, not higher-level lookalike `+0x04` fields.

This is the exact R1 class that the previous direct-base and leaf passes could not prove.

## Class D — central and forced release writers retained separately

For completeness, these already-canonical writers remain distinct and are not collapsed into the new family/alias classes:

- `0x1401B8F00`: deferred state4 cleanup, state0 before backing release;
- `0x1401B9530`: ordinary owner release, backing release first and state0 only on success;
- `0x1401B9560`: group reset, iterates using count/base tables, release then unconditional state0;
- `0x1401B95E0`: full 363-record reset, release then unconditional state0.

The new pass reinforces why one universal `ReleaseLoadedResource()` semantic would be wrong.

## Rejected nearby non-LR copy — `0x1401B9EE0`

A nearby non-immediate `+0x04` write at `0x1401B9F0C` copies `source+0x38` into `destination+0x04`.

Destination provenance rejects it as a LoadedResource record writer:

- one caller passes `RCX = parent/subobject +0x28`;
- another passes `RCX = parent +0x1298`, the higher-level wrapper object also used with `1B9420/1B94C0`;
- another passes `RCX = parent +0x1B2D0`.

The function copies four leading bytes, initializes its own `+0x10/+0x18/+0x28/+0x38` fields and participates in a larger higher-level object contract. Its `+0x04` field is therefore not promoted as LoadedResource state.

## Bounded state0 writer map after this pass

Within the recovered LoadedResource lifecycle cluster, state0 publication now has evidence-separated origins:

### Acquisition rollback

- `1B8DA4`
- `1B8E60`
- `1B92B8`

### Deferred cancellation cleanup

- `1B8F00`

### Ordinary owner release

- `1B9546` inside `1B9530`

### Fixed-family indexed release

- `1B9914`
- `1B997B`
- `1B99D7`
- `1B9A3B`

### Stored/caller-propagated record aliases

- `1B946D`
- `1B96CC`
- `1B9B9A`
- `1B9C83`

### Forced reset

- `1B95B9` group reset
- `1B9609` full reset

These writers do not all share the same ordering or failure semantics.

## R1 effect

This pass closes a substantial portion of the previously open **indexed/derived/caller-propagated state0 writer class**.

It also converts the seven-group base/count topology into direct raw `.data` evidence used by the family release formulas.

### Still open for R1

1. non-state0, non-immediate writes to `record+0x04` whose state value is carried in a register or table;
2. record aliases propagated outside the reviewed `0x1401B8xxx..0x1401B9xxx` lifecycle cluster;
3. indirect function-pointer/callback registration paths that can publish state changes without a direct static xref;
4. any additional family-specific writer that survives exact destination provenance;
5. whole-image contradiction sweep after those classes are exhausted.

The next pass should prioritize **value-flow into `record+0x04` for states 1/2/3/4 outside the canonical central writers**, because the state0 release/rollback surface is now much more complete.

## Completion claim

**R1 materially advanced; not complete. L3 not complete.**

No L1, L2 or dynamic V1–V7 acceptance is promoted by this static writer census.