# HITS Pass 10 — Slice 9: Primitive Descriptor Ownership and Runtime Bridge

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED / CI VALIDATION ACTIVE**

## Purpose

Slice 9 closes the ownership path between collision-manager source tables, the primitive descriptor selected for one runtime object, and constructor dispatcher `0x1402CC530`.

It also corrects an important transient research mistake: runtime object `+0x20` is **not** the primitive descriptor pointer.

## Manager source initialization — `0x14005C260`

Canonical body:
- `0x14005C260..0x14005C318`;
- 184 bytes;
- SHA-256 `9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7`.

Relevant ABI writes:
- argument 2 (`RDX`) -> manager `+0x108` entry-table pointer;
- argument 3 (`R8`) -> manager `+0x110` primitive-descriptor-table pointer.

## Entry table and descriptor resolution

Both runtime-object builder paths `0x14005C630` and `0x14005C740` resolve the same primitive descriptor:

1. `entry = manager+0x108 + entry_index * 4`;
2. `descriptor_index = u16(entry+0x02)`;
3. `descriptor = manager+0x110 + descriptor_index * 0x50`.

Therefore:
- entry stride = `0x04`;
- entry raw flag byte = `+0x00`;
- entry transform-selector byte = `+0x01`;
- entry descriptor-index field = `u16 +0x02`;
- primitive descriptor stride = **`0x50`**.

The `0x50` descriptor stride is now directly confirmed from runtime pointer arithmetic, not inferred from one shape layout.

Canonical builder bodies:
- `0x14005C630..0x14005C731`, 257 bytes, SHA `97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a`;
- `0x14005C740..0x14005C83D`, 253 bytes, SHA `0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6`.

The two builders differ in transform-source resolution but not in primitive descriptor resolution.

## Runtime object initializer — `0x14005C8D0`

Canonical body:
- `0x14005C8D0..0x14005C99D`;
- 205 bytes;
- SHA-256 `f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe`.

Windows x64 argument bridge:
- arg 2 / `RDX` -> runtime object `+0x110` entry/source pointer;
- arg 3 / `R8` -> runtime object **`+0x118` primitive descriptor pointer**;
- arg 4 / `R9` -> runtime object being initialized;
- stack arg 5 -> runtime object **`+0x20` transform pointer**.

## Constructor dispatcher correction

`0x1402CC530` first reads runtime object `+0x20` and, when non-null, uses it as the source for the runtime transform/matrix at object `+0x30`.

It then separately reads runtime object `+0x118`, reads the descriptor's first byte, and dispatches values `0..6` to the seven constructor branches.

Therefore the canonical ownership model is:

`manager entry -> descriptor index -> manager descriptor table (0x50 stride) -> runtime object +0x118 -> CC530 type dispatch`

while transform ownership is separate:

`transform source -> runtime object +0x20 -> runtime matrix +0x30`.

## Transform-source variants

`0x14005C630` resolves an optional transform indirectly through an external source selected by `entry+0x01` and then uses that source object's `+0x110` pointer.

`0x14005C740` resolves an optional transform directly as an indexed external table with `0x40` stride using the same `entry+0x01` selector.

Neither transform path changes which primitive descriptor is selected from manager `+0x110`.

## Correction

**REJECTED transient model:** `runtime_object+0x20` is the primitive descriptor pointer.

**CURRENT:**
- `runtime_object+0x20` = transform pointer/source;
- `runtime_object+0x118` = primitive descriptor pointer;
- descriptor first byte selects constructor type `0..6`;
- manager primitive descriptors have exact stride `0x50`.

This correction was caught before the incorrect `+0x20 descriptor` statement was promoted into project authority.

## Implementation

- `include/dmc_rengine/profiles/dmc3/hits_primitive_descriptor_ownership_evidence.hpp`
- `tests/hits_primitive_descriptor_ownership_evidence_tests.cpp`
- CTest `hits_primitive_descriptor_ownership_evidence`

All public descriptors remain exact-canonical-SHA gated; packed SHA `81c7...c7d6` receives no canonical VA/offset ownership evidence.

## Next reverse boundary

With ownership closed, type-5 upstream research should now search the **manager `+0x110` descriptor table population/source**, specifically descriptors whose first byte is `5`, rather than global immediate-`5` instructions or transform-table data.
