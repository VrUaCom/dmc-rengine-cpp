# DMC3 Vanilla Deep Research Wave 2

**Status:** CANONICAL RESEARCH ADDENDUM (pre-roadmap)  
**Scope:** direct cross-build reverse of Stage resolution, resource lifecycle, HD media translation, and runtime post-load processing.  
**Rule:** this document extends/supersedes relevant claims in the earlier vanilla baseline where stronger direct evidence is recorded here. It does **not** change the product roadmap.

## 1. Artifact and cross-build control

Research was reproduced against both executable artifacts:

- protected distribution build: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`;
- canonical unpacked analysis build: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

The static Stage banks, selector/group layer, and OGG descriptor table are byte-identical between both builds. The protected build has an extra `.bind` section and protected `.text`; exact code/XREF evidence remains anchored to `e454…`, while static-data identity is independently corroborated by both artifacts.

## 2. Corrected Stage descriptor ABI

**CORRECTED:** the previous `0x10` cell interpretation `path @ +0x00 / unknown @ +0x08` is not the true semantic boundary.

Direct canonical code reads `WORD [cell+0x00]` and the path pointer from `[cell+0x08]`.

Minimum evidence-safe ABI:

```cpp
struct StageResourceCell {
    uint16_t kind16;       // +0x00 — exact semantic enum/name still open
    uint8_t unknown02_07[6];
    const char* path;      // +0x08
}; // 0x10

struct StageResourceDescriptor {
    StageResourceCell script; // +0x00
    StageResourceCell cfg;    // +0x10
    StageResourceCell effect; // +0x20
    StageResourceCell sound;  // +0x30
}; // 0x40
```

Consequences:

- old `0x1405C4AA8` is the first **path field**, not the true row base;
- Bank A row base is `0x1405C4AA0`;
- a reader that starts at `0x1405C4AA8` can still obtain valid paths while being semantically shifted by eight bytes.

## 3. Complete Stage descriptor banks

Two descriptor banks cover all **189** observed `stNNN` identities:

- **Bank A:** `VA 0x1405C4AA0`, 110 rows: `000–012`, `100–146`, `200–241`, `300–307`;
- **Bank B:** `VA 0x1405C3080`, 79 rows: `308–313`, `400–446`, `448–449`, `600–612`, `900–910`.

Total: `110 + 79 = 189` descriptors.

These are runtime resource descriptors; this is **not** a claim that DMC3 has 189 independent gameplay stages.

## 4. Numeric Stage ID resolver

A **193-entry selector table** exists at `VA 0x1405C4440`, followed by a **10-pointer group-base table** at `VA 0x1405C4A50`.

Direct code around `0x1401B985D / 0x1401B9860`:

1. reads numeric Stage ID;
2. computes `stageId / 100` and `stageId % 100`;
3. selects `groupBase[hundreds]`;
4. indexes the dense selector by remainder;
5. obtains a Stage resource descriptor;
6. selects a role cell — the observed sound path adds `+0x30`;
7. invokes the resource loader.

Therefore this is **EXE CONFIRMED**:

```text
numeric Stage ID
-> group-base / selector indirection
-> StageResourceDescriptor
-> role cell
-> logical resource path
-> loader
```

Observed selector-space behavior includes fallback bases for numeric families 5, 7, and 8 and a duplicate descriptor around the missing `447` position. Exact gameplay semantics of these fallback positions remain `RESEARCH REQUIRED`.

## 5. Cross-stage resource aliasing

`st600–st612` prove that a Stage descriptor can own its own script/config while reusing effect/sound dependencies from another stage:

```text
st600 -> st004_effect / snd_r004
st601 -> st006_effect / snd_r006
st602 -> st126_effect / snd_r126
st603 -> st113_effect / snd_r113
st604 -> st003_effect / snd_r003
st605 -> st300_effect / snd_r300
st606 -> st204_effect / snd_r204
st607 -> st228_effect / snd_r228
st608 -> st216_effect / snd_r216
st609 -> st233_effect / snd_r233
st610 -> st131_effect / snd_r131
st611 -> st128_effect / snd_r128
st612 -> st300_effect / snd_r300
```

Canonical consequence: **resource identity and dependency ownership cannot be inferred from filename family alone.**

## 6. `.lst` fallback and list-driven PAC expansion

For `kind16 == 0`, loader logic first queries the original resource path. If unavailable it rewrites the extension to `.lst` and queries again.

The list path contains explicit `dummy` and `lst` tokens and can:

- skip/zero `dummy` entries;
- recurse into nested `.lst` references;
- rewrite non-list entries to `.pac` before existence/read operations.

Confirmed boundary: `kind16 == 0` supports list-driven PAC expansion fallback with dummy/nested-list semantics. Full text grammar, ordering, ownership, and failure behavior remain `RESEARCH REQUIRED`.

## 7. Higher-level 363-entry resource/load manager

A higher-level runtime pool exists at `VA 0x140C99D30`:

- **363 entries**;
- stride `0x48`;
- distinct from the low-level VFS `100 × 0x20` FileSlot pool.

Initialization partitions it into seven groups:

```text
counts:     [4, 136, 60, 28, 1, 128, 6]
offsets:    [0, 4, 140, 200, 228, 229, 357, 363]
```

Minimum stable fields:

```text
+0x00 group index
+0x04 state
+0x08 u16 subtype/index/variant
+0x18 source/resource descriptor pointer
+0x20 loaded buffer/payload
+0x28 owned allocation/container state
```

Direct state lifecycle:

```text
0 free / unstarted
1 I/O scheduled / loading
2 I/O complete / pending post-process
3 ready / postprocessed
4 cancellation / teardown pending
```

Observed transitions:

```text
scheduler       0 -> 1
I/O completion  1 -> 2
post-load loop  2 -> 3
teardown        active -> 4
cleanup         4 -> 0
```

This is direct Level-C/Level-D evidence that DMC3 has a higher-level resource lifecycle manager above raw archive/file reads.

### Partial group evidence

- group 0 (`4`) is associated with player-object descriptor sets;
- group 1 (`136 = 4×34`) includes player weapon/object resources;
- group 2 (`60 = 4×15`) includes enemy object resources;
- group 3 (`28 = 4×7`) includes player motion resources;
- group 4 (`1`) is bound to a sound-system/global descriptor set;
- group 5 (`128`) is a generic free-slot load group used by the numeric Stage resolver and many other callers;
- group 6 (`6`) is used by scene/system initialization, exact semantic identity still open.

These names remain evidence-bounded; the full subtype contracts are not yet recovered.

## 8. Typed post-load normalization / fixup

When a resource entry reaches state 2, the runtime does **not** immediately expose the raw bytes as ready. It performs a typed post-load normalization pass before state 3.

The dispatcher recognizes at least:

```text
MOD  -> 0x1402FE3B0
EFM  -> 0x1402F7A90
SCM  -> 0x1403051B0
SHW  -> 0x1403204C0
PNST -> recursively visit non-empty members and run the same dispatcher
```

PAC roots are likewise traversed member-by-member before the same typed fixup path is applied.

The `MOD/EFM/SCM/SHW` helpers visibly convert stored relative offsets into in-memory pointers by adding the resource base and writing resolved pointers back in place.

Canonical runtime pipeline is therefore refined to:

```text
request
-> byte acquisition
-> container/root traversal
-> typed in-place fixup / normalization
-> ready resource
-> consumers
```

A parser that only returns bytes or PAC/PNST entries has **not** reproduced this phase of the game resource runtime.

## 9. HD audio translation ABI

The executable contains:

- 154 legacy ADX logical names;
- a separate 154-record OGG descriptor table at `VA 0x14055C610`;
- descriptor stride `0x10`.

Minimum descriptor:

```cpp
struct OggDescriptor {
    const char* filename;
    uint32_t loopStartMs;
    uint32_t loopEndMs;
}; // 0x10
```

Code around `0x140031D80` constructs/rewrites the requested name to `.ogg`, lowercases the basename, searches the OGG descriptor table by string, and stores the matched descriptor index in the sound runtime object.

Code around `0x140032A80` reads descriptor `+0x08/+0x0C` and passes them to `FMOD_Channel_SetLoopPoints` with millisecond time units. `0xFFFFFFFF` is the no-explicit-loop sentinel path.

Therefore:

- `ADX -> OGG` is **runtime basename translation**, not integer index equivalence between catalogs;
- loop metadata is owned by EXE descriptors and is not dependent on OGG comment tags.

## 10. HD video translation ABI

Legacy SFD names remain in the EXE while the HD distribution physically stores WMV files.

Direct media code:

1. copies the legacy path;
2. locates the last extension separator;
3. replaces the extension with `.wmv`;
4. passes the translated path to the media loader.

Therefore `SFD -> WMV` is **EXE CONFIRMED runtime extension translation**.

## 11. StageSet ABI heterogeneity

The 35 `CStageSet*` RTTI names do not represent one uniform object ABI.

Observed families include:

- actor+collision StageSets with a `CStageSet` secondary subobject near `+0x110`;
- lighter `CWork`-based `Color/Light/Se/Yure` types with `CStageSet` near `+0x60`;
- `CNonPlayer`-derived `Heart/Lungs/Nausika` variants with `CStageSet` near `+0x180`;
- StageSet-like names such as `Shl00` and `TruckEnemy` that do not expose `CStageSet` as an RTTI base.

The future Stage Semantic Graph and recovered C++ must preserve concrete inheritance/subobject evidence instead of forcing one generic StageSet structure.

## 12. Corrections to older Canon / implementation assumptions

- **CORRECTED:** one `110×4` Stage table -> two descriptor banks totaling 189 descriptors plus selector/group indirection.
- **CORRECTED:** cell `path @ +0 / unknown @ +8` -> minimum ABI `kind16 @ +0 / path @ +8`.
- **CORRECTED:** `st600–612` lack effect/sound -> descriptor-level cross-stage aliases provide those dependencies.
- **CONFIRMED:** the original 110-row table content is real, but it is one bank rather than the complete Stage descriptor universe.
- **REFINED:** logical resource -> physical file is insufficient; HD audio/video require explicit translation metadata/logic.
- **REFINED:** raw bytes -> typed resource is insufficient; typed post-load in-place normalization is a separate runtime phase.

## 13. Research still open after Wave 2

Highest-value open questions:

- exact semantics of `StageResourceCell.kind16` and remaining cell bytes;
- complete `.lst` grammar, ownership, recursion, lifetime, and error rules;
- semantic identities of all seven 363-pool groups and exact per-group subtype contracts;
- completion-callback fields and ownership in the `0x48` resource entry;
- cache/reuse/refcount behavior for duplicate requests and stage transitions;
- higher-level factory/object construction after normalized bytes become ready;
- complete `ResourceTypeInfo` / `LoadedResource` type domain;
- lifecycle behavior across room transition, stage transition, restart, menu return, and shutdown;
- EventTbl and scene-transition linkage to the numeric Stage resolver;
- exact selector fallback semantics for invalid/missing numeric Stage IDs.

## Preservation rule

This Wave 2 record is a pre-roadmap research authority. Roadmap work must be derived from the reconciled research model and later stronger evidence; these findings are architecture constraints, not optional implementation ideas.
