# DMC3 HD PTX texture-count capacity — 2026-10-04

## Scope

This pass connects three already recovered pieces of the canonical HD texture runtime:

1. the fixed PTX payload pointer table;
2. the texture parser's framing-dependent record count;
3. the global PTX record pool.

The result distinguishes **storage capacity** from a validated input maximum.

Canonical executable SHA-256:
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## PTX payload storage

Recovered runtime payload:

```text
+0x000..+0x1FF  64 qword record pointers
+0x200          texture_count
+0x204          records_per_texture
total           0x208 = 520 bytes
```

Therefore one payload has **64 record-pointer slots**.

Status: EXE_CONFIRMED.

## Framing-dependent record fanout

Parser `0x1403365B0` prepares the 0x40-byte materialization descriptor.

### Descriptor + DDS fallback

For the non-TM2 fallback path, `0x1403366BF..0x1403366C8` writes:

```text
descriptor +0x3C = 1
```

The downstream materializer `0x1403366E0` uses signed word `descriptor+0x3C` as its record-iteration count.

Thus the descriptor+DDS fallback materializes exactly **one PTX runtime record per texture**.

This is the framing observed throughout the preserved model/stage texture specimens used by current Rengine texture work.

Status: EXE_CONFIRMED plus CORPUS_CONFIRMED framing.

### TM2 branch

When the source begins with `TM2\0`, the parser copies source word `+0x04` into descriptor word `+0x3C`.

The downstream record count therefore can differ from one on this branch.

This pass intentionally does not rename source word `+0x04` beyond its proven role in controlling materialization-record count.

Status: EXE_CONFIRMED_PRESERVED_TECHNICAL_ROLE.

## Descriptor+DDS payload capacity

For the retail-observed descriptor+DDS path:

```text
records_per_texture = 1
payload pointer capacity = 64 records
=> in-structure texture capacity = 64 textures
```

This is a runtime ABI/storage capacity.

It is **not** equivalent to a validated file-format input limit: the reviewed original loader increments/copies according to source counts and does not expose a local clean `count <= 64` rejection before the pointer-table write. DMC Rengine's bounded model adds a host-side out-of-range guard; that host guard must not be projected back into the EXE.

Classification:

- 64 texture records/pointers per payload: EXE_CONFIRMED;
- 1 record per descriptor+DDS texture: EXE_CONFIRMED;
- 64 descriptor+DDS textures fitting one payload: DERIVED_ABI_CAPACITY;
- graceful original-EXE rejection at texture 65: NOT_CONFIRMED.

## Global PTX pool

The canonical global pool contains:

```text
128 records
record stride = 0x50 bytes
record array bytes = 0x2800
```

For one-record descriptor+DDS textures this corresponds to an absolute storage pool of **128 simultaneously allocated texture records** before considering lifecycle, reservations, failed partial allocations and other payloads.

This does not guarantee that 128 complete textures can all be rendered simultaneously. The pool is global and shared by active users; placement/reservation state can reduce availability.

Status: EXE_CONFIRMED pool capacity; runtime simultaneous-texture interpretation remains bounded.

## Retail comparison

The hash-bound v6 DDS corpus currently shows extracted texture sibling sets up to **17 textures**.

For the descriptor+DDS one-record path:

```text
17 / 64 = 26.5625% of one payload's pointer capacity
```

This is only a corpus comparison. The complete game/NBZ population still needs census before 17 can be called the largest retail texture bundle globally.

## Failure behavior

Existing materializer evidence shows non-transactional failure behavior for some PTX allocation failures:

- record exhaustion can leave partially reserved state;
- some failure classes roll back, others do not;
- global/scene reset may later clean state, but that is separate lifecycle evidence.

Therefore deliberately exceeding storage capacity is not considered a safe probing technique without isolation and rollback.

## Current limit table

| Domain | Value | Status |
|---|---:|---|
| PTX record pointers per payload | 64 | EXE_CONFIRMED |
| descriptor+DDS records per texture | 1 | EXE_CONFIRMED |
| descriptor+DDS textures fitting pointer table | 64 | DERIVED_ABI_CAPACITY |
| global PTX records | 128 | EXE_CONFIRMED |
| global record stride | 0x50 / 80 bytes | EXE_CONFIRMED |
| largest extracted retail sibling texture set in bounded corpus | 17 | CORPUS_CONFIRMED |
| clean reject at texture #65 | unknown | OPEN |

## Next gates

1. Trace all PTX payload owners to determine realistic simultaneous-payload count.
2. Close pool reservation values for each scene/profile state.
3. Test 18..64 descriptor+DDS textures with controlled authored bundles.
4. Test 65 only in an isolated instrumented run because the original loader lacks a proven local safe rejection.
5. Expand the texture census to the complete retail NBZ population.
