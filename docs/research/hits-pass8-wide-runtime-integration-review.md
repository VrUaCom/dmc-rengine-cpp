# HITS Raw Reverse Pass 8 — Wide Runtime Integration Review

Date: 2026-08-14

Canonical EXE target: `dmc3.exe`

SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Google Drive authority:
- `DMC Rengine — HITS Raw Reverse Pass 8 — Wide Runtime Integration Review — 2026-08-14`
- `DMC Rengine — HITS Reverse Core Mega Synthesis — Pass 7 — 2026-08-07`
- `DMC Rengine — HITS Canonical Preservation Registry`

No proprietary game bytes are committed.

## Purpose

This review reconciles the HITS reverse/core work through Pass 7C with the newer DMC3 Vanilla Deep Research Wave 2 resource/stage-runtime findings. It also reviews the merged HITS Pass 5/6 implementation against the stronger runtime architecture now available.

The goal is to prevent a false architectural merge between:

1. the general DMC3 resource lifecycle/runtime; and
2. the stage-local HITS collision runtime.

## Current HITS authority retained

The following findings remain valid and are not superseded by Wave 2:

- main stage PAC members 3 and 6 feed two independent stage-local HITS runtimes;
- source 0 maps to member 3 and remains the default static source;
- source 1 maps to member 6 and is explicitly selected by specialized query paths;
- HITS runtime initialization remains anchored to the recovered stage/PAC consumer path and `0x1402D3060`;
- exact HITS file layout and spatial writer equivalence from Pass 5 remain valid;
- source-1 corpus properties and raw flags evidence from Pass 6 remain valid;
- Pass 7/7B/7C dynamic-category, mask, source-switching and specialized query ABI findings remain valid.

## Wave-2 resource-runtime reconciliation

Wave 2 adds a separate higher-level resource/load manager:

- pool base: `0x140C99D30`;
- 363 entries;
- entry stride `0x48`;
- seven partitions with counts `[4,136,60,28,1,128,6]`;
- minimum stable fields:
  - group index `+0x00`;
  - state `+0x04`;
  - u16 subtype/index/variant `+0x08`;
  - source/resource descriptor pointer `+0x18`;
  - loaded payload `+0x20`;
  - owned allocation/container state `+0x28`;
- lifecycle:
  - `0` free/unstarted;
  - `1` loading;
  - `2` I/O complete / pending postprocess;
  - `3` ready/postprocessed;
  - `4` cancellation/teardown pending;
- state 2 performs typed post-load normalization before state 3;
- confirmed typed helpers include MOD, EFM, SCM and SHW, with recursive PAC/PNST traversal.

This manager is a distinct higher-level resource lifecycle and must not be represented as the HITS runtime itself.

## Canonical layered model after review

```text
Resource/VFS request
    -> general 363-entry resource lifecycle
    -> raw bytes / container traversal
    -> typed post-load normalization
    -> ready resource
    -> stage-specific PAC consumer
    -> main stage PAC member 3 / member 6
    -> HITS initializer
    -> HITS source 0 / source 1
    -> StageCollisionWorld
    -> static HITS query layer
    -> dynamic collider layer
    -> query-specific result/correction propagation
    -> gameplay consumer
```

The exact instruction-level handoff from a Wave-2 state-3 ready resource to the stage/PAC HITS construction path is still `RESEARCH REQUIRED`.

## Stage/resource identity correction relevant to HITS coverage

Wave 2 corrects the old assumption that one 110x4 table is the whole Stage descriptor universe.

Current evidence:

- `StageResourceCell` minimum ABI:
  - `u16 kind16 @ +0x00`;
  - unresolved bytes `+0x02..+0x07`;
  - path pointer `@ +0x08`;
  - stride `0x10`;
- four cells form one `0x40` descriptor;
- Bank A: 110 descriptors at `0x1405C4AA0`;
- Bank B: 79 descriptors at `0x1405C3080`;
- total observed descriptors: 189;
- numeric selector table: 193 entries at `0x1405C4440`;
- group-base table: 10 pointers at `0x1405C4A50`.

Therefore future HITS corpus coverage must be keyed by descriptor/resource identity and physical PAC identity, not by an `st001`-centric or 110-row-only model.

Recommended coverage identity:

```text
descriptor/resource_set identity
    -> resolved main/script PAC
    -> physical PAC identity + SHA-256
    -> member 3 -> source 0
    -> optional member 6 -> source 1
```

## Pass 5/6 implementation review

Merged PR #45 remains correct and should not be reverted.

Reviewed production findings retained:

- header `+0x3C` = spatial table relative offset;
- header `+0x40` = triangle array relative offset;
- both offsets relative to `fileBase + 8`;
- pointer table starts directly at `0x44` in the common layout;
- no unknown eight-byte spatial prefix;
- no two global guard dwords before the triangle array;
- triangle array follows the final cell-list `-1` terminator directly;
- inclusive 13-axis triangle-vs-AABB SAT assignment;
- raw 32-bit flags are exposed without speculative semantic enum conversion;
- source 0/source 1 profiles remain evidence-safe and semantically neutral.

No code rollback is warranted from the Pass-8 review.

## Recovered-game vs product architecture boundary

The newer repository direction represented by the Recovered Game Source Tree is the correct destination for original-game collision ABI reconstruction.

Recommended ownership boundary:

```text
dmc_rengine_core
  formats::hits
    parser
    writer
    spatial model
    editor-safe abstractions

recovered-game/dmc3
  collision
    StageCollisionWorld
    source selector
    original query ABI
    dynamic category registry
    static/dynamic arbitration
    source 2
    runtime lifecycle handoff

profiles::dmc3_hd
  addresses
  artifact hashes
  masks
  evidence anchors

GDSpaces
  ResourceId
  provenance
  resolution
  materialized bytes

HITS Editor / ModViz / Stage Ops
  consumers of the above contracts
```

Do not move recovered original-game ownership into GDSpaces. GDSpaces remains the resource API and evidence-aware loader, not the collision runtime.

## Documentation supersession review

Historical HITS receipts contain claims that were correct at the time but have since been superseded, including:

- exact original spatial-builder equivalence being open;
- exact spatial writer remaining open;
- required preservation of an unknown eight-byte spatial prefix;
- global guard dwords before the triangle array;
- topology edits being permanently limited by `requires_spatial_rebuild`.

These historical records must remain traceable, but current authority is Pass 5 / PR #45 and the later preservation registry updates.

They should be treated as `CORRECTED`, `REJECTED` or `SUPERSEDED`, not silently deleted.

## Coverage expansion unlocked by Wave 2

The existing 16-HITS corpus remains sufficient evidence for the Pass-5 byte-identical writer result.

However, it is not complete ecosystem coverage after discovery of the full 189-descriptor universe.

New corpus sweep target:

```text
all available 189 descriptors
    -> resolve main/script PAC
    -> detect member 3 HITS
    -> detect optional member 6 HITS
    -> PAC SHA-256
    -> HITS SHA-256
    -> triangle count
    -> grid dimensions
    -> raw flags distribution
    -> source-1 presence
    -> duplicate HITS payload detection
    -> cross-descriptor resource reuse
```

This is a coverage expansion, not a reopening of the already verified Pass-5 writer algorithm.

## Remaining high-value reverse targets

The remaining HITS reverse/core closure is concentrated in six areas:

1. `0x14005E7A0`
   - complete input/output ABI;
   - no-hit initialization;
   - write set;
   - candidate metric;
   - static/dynamic arbitration;
   - equality/tie-break behavior;
   - caller-visible output.

2. `0x14005B460`
   - internal body;
   - channel-list entry ABI;
   - candidate production;
   - ownership/lifetime;
   - return semantics.

3. `0x14005FEC0`
   - exact source-1 output ABI and field semantics.

4. `0x1400601E0`
   - exact in/out layout;
   - fourth-component semantics;
   - accumulation across multiple contacts.

5. source 2
   - exact type bound from the global root;
   - backing data/runtime object;
   - lifetime;
   - selection path;
   - relationship to static/dynamic collision.

6. general resource lifecycle -> HITS construction handoff
   - exact transition from Wave-2 ready resource state to the stage/PAC HITS initializer path.

## Implementation freeze retained

Until the remaining ABI/arbitration evidence is recovered:

- do not invent one universal original `CollisionResult`;
- do not invent gameplay enum names for raw HITS flags;
- do not rename source 1 to camera/navigation/boundary/player collision;
- do not model source 2 as a third stage-PAC HITS member;
- do not conflate the 363-entry resource manager with `StageCollisionWorld` or `HitsRuntime`;
- do not put recovered collision ownership in GDSpaces.

## Status

Pass 8 is a wide architectural and evidence review. It does not supersede Pass 5/6 file-format proof or Pass 7B/7C ABI findings; it reconciles them with the stronger Wave-2 stage/resource runtime.

Current conclusion:

- HITS binary format and spatial writer are substantially closed at file level;
- the architectural layering is now clearer;
- the highest-value remaining work is original runtime ABI, arbitration, source-2 ownership, resource-to-HITS construction handoff and full descriptor-universe coverage.
