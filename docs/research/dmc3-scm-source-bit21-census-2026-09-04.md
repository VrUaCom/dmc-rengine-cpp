# DMC3 HD SCM source object flag `0x00200000` — whole-image census 2026-09-04

## Target

- executable: `dmc3.exe`;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size: `6,356,432`;
- ImageBase: `0x140000000`.

## Question

The current SCM corpus contains source object flag bit 21 (`0x00200000`) on 19 objects. The SCM object initializer preserves source flags into baseline/effective runtime state, but the bounded consumers previously recovered did not decode this bit.

This pass asks whether a wider executable census exposes a real consumer of **serialized SCM object `u32 +0x10` bit 21**.

## Whole-image exact-immediate census

The disassembly contains 11 exact textual uses of immediate `0x200000`:

```text
0x1400304CC  add rax, 0x200000
0x1401E2D5E  test [rbx+0x3FF4], 0x200000
0x1401E3C2C  test [rbx+0x3FF4], 0x200000
0x14020364D  or global, 0x200000
0x140203F83  or global, 0x200000
0x1402043B1  test [rbx+0x3FF4], 0x200000
0x14020484C  or global, 0x200000
0x1402F4C21  and eax, 0x200000
0x140303F2F  and eax, 0x200000
0x140339D10  cmp rcx, 0x200000
0x140339E20  cmp rdx, 0x200000
```

Immediate equality alone is not provenance evidence. Every plausible hit must be traced back to the bitfield being tested.

## Rejected hit: `0x1402F4C21`

The branch reads:

```text
runtimeRecord +0x304
  -> AND 0x00200000
```

But `runtimeRecord+0x304` is initialized independently in the same subsystem:

```text
0x1402F395C  eax = constructor/runtime input +0x128
0x1402F3962  eax &= 0xF0000000
0x1402F396B  runtimeRecord+0x304 = eax
```

Therefore this bitfield is not the serialized SCM object `+0x10` baseline/effective flag word.

Status:

```text
REJECTED_AS_SCM_OBJECT_FLAG_CONSUMER
```

## Rejected hit: `0x140303F2F`

This hit is located inside the broader SCM manager/runtime subsystem, but pointer provenance again separates it from object flags:

```text
0x140303F22 manager
0x140303F29 eax = manager+0xE0
0x140303F2F eax &= 0x00200000
0x140303F38..43 optional call 0x140306560
```

`manager+0xE0` is established separately by `0x1402F9570`:

```text
0x1402F9621 manager+0x108 -> 0x1402FD650
0x1402F9631 manager+0xE0 = returnValue
0x1402F963B..48 manager+0xE0 |= 1
0x1402F9652..61 manager+0xE0 |= external manager flag argument
```

It is therefore a manager-level bitfield, not serialized object `+0x10`.

Status:

```text
REJECTED_AS_SCM_OBJECT_FLAG_CONSUMER
```

## Bit-test census

A focused search for x86 bit-test operations using bit index 21 (`0x15`) in the local `0x14030xxxx` SCM/runtime region found only `BTS` operations that **set manager bit 21** at:

```text
0x140302CF9
0x140302D59
```

Those operations mutate `manager+0xE0` after other source conditions (`0x200` / `0x400` in a separate control word). They do not test serialized object bit 21.

No `BT/BTR/BTC` consumer of baseline/effective SCM object flags for bit 21 was recovered in this census.

## Current bounded conclusion

For serialized SCM object flags:

```text
source object +0x10 bit 21 = 0x00200000
```

Evidence remains:

- `DATA_CONFIRMED`: observed on 19 corpus objects;
- `RUNTIME_PRESERVED`: copied into baseline/effective object flag state;
- `BOUNDED_CONSUMERS_NEGATIVE`: known object-state helper and mesh descriptor paths do not decode it;
- `WHOLE_IMAGE_IMMEDIATE_HITS_PROVENANCE_REJECTED`: both nearby exact `AND 0x200000` candidates operate on unrelated runtime/manager bitfields;
- `SEMANTIC_ROLE_UNRESOLVED`.

Canonical code policy therefore remains:

```text
PRESERVE BIT EXACTLY
DO NOT EXPOSE A SEMANTIC EDITOR TOGGLE
DO NOT CALL IT UNUSED OR RESERVED GLOBALLY
```

The correct current label is:

```text
PRESERVED_UNDECODED_WITH_BOUNDED_NEGATIVE_EVIDENCE
```

## Remaining path if further closure is required

A stronger future closure would require one of:

1. a dynamic trace comparing otherwise-identical objects with bit21 clear/set;
2. a recovered producer that assigns bit21 by semantic object class;
3. an indirect consumer where the bit is transformed without an immediate `0x200000` mask;
4. controlled original-game mutation of bit21 with render/update/state instrumentation.

Until then no gameplay/render semantic name is justified.
