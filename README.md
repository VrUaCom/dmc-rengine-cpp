# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*  
> **Built by the Sect of Neuroslop and the Monks of Binary Code.**

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing and progressively recompiling Devil May Cry 3: Special Edition from the HD Collection.

The project connects exact artifact identity, executable research, resource resolution/materialization, binary inspection, stage reconstruction, guarded authoring, recovered source, validation and long-term recompilation under one evidence-first architecture.

## Current state

**Status snapshot:** 2026-09-02  
**Reviewed implementation base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`

GDSpaces is tracked as three separate resource-runtime ownership layers:

```text
L2 — Resource Resolution
     ADVANCED / INCOMPLETE

L1 — Resource Materialization
     INCOMPLETE / NOT 100%
     product authoring capability is advanced

L3 — Original Runtime / Lifecycle
     INCOMPLETE
     R1 static writer census bounded-closed
     R2 field/backing ownership ACTIVE
```

The repository already contains substantial reviewed implementation and reverse evidence, including:

- C++20/CMake core and CLI with Windows + Ubuntu validation;
- SHA-256 artifact/evidence infrastructure;
- GDSpaces ResourceId/ResourceRef/SourceRegistry/ByteProvenance/WorkingCopy;
- canonical NBZ ZIP indexing and STORE/raw-DEFLATE product materialization;
- PAC/PNST sparse/empty/alias-preserving parsing and recursive expansion;
- bounded same-size and size-changing PAC/PNST authoring/reintegration;
- nested root-to-leaf PAC/PNST slot-path authoring;
- verified NBZ copy rebuild and next-volume overlay generation;
- atomic/no-replace publication and exact reopen/rematerialization checks;
- recovered DMC3 runtime resolver/type identity contracts;
- current naming architecture separating physical identity, extracted ordinals, historical labels, semantic evidence and presentation;
- strong LoadedResource lifecycle/state reverse with L3-R1 closed for the canonical analysis image;
- Binary Inspector, EXE evidence, Stage/Item/HITS/save and guarded-modification foundations.

The project does **not** claim full DMC3 decompilation, whole-game behavioral equivalence, complete original materialization/lifecycle equivalence, Capcom offline-writer equivalence, a complete desktop editor, or a behaviorally equivalent rebuilt executable.

## Canonical GDSpaces architecture

```text
logical game/resource request
        |
        v
[L2] Resource Resolution
     choose exact provider/source/volume/member identity
        |
        v
[L1] Resource Materialization
     produce/reproduce exact bytes + provenance
     edit/rebuild/repack/reopen/rematerialize
        |
        v
[L3] Original Runtime / Lifecycle
     scheduler/callback ownership
     LoadedResource states
     typed-ready / claims / cancellation / release / reset
        |
        v
[DOMAIN] Stage Ops / ModViz / editors
```

Validation/live observation is cross-cutting; it is not a speculative L4.

### Core engineering laws

> **L2 selects. L1 materializes. L3 owns lifecycle.**

> **All product resource access goes through GDSpaces authority.**

> **Recovered original-game code belongs to the Recovered Game Source Tree/profile evidence domain.**

> **No claim without evidence.**

> **No implicit retail-file mutation.**

> **No second resolver, materializer, lifecycle truth or scene truth.**

## Current critical work

### L3 — R2 active

With R1 current-main writer reconciliation closed, the active static lifecycle frontier is family/group ownership of:

```text
LoadedResource +0x08
               +0x10 where applicable
               +0x18
               +0x20
               +0x28
```

including initialization/finalization/release ordering and the SCM `mesh +0x28` reconciliation.

### L2 — successful mount topology correction

Current product code still needs the recovered distinction:

```text
filename discovery / registration attempt
!=
successful linked runtime mount
```

Only successful registrations should become resolver topology.

### L1 — original reverse + real acceptance

Current product authoring capability remains advanced, but the Layer-1 status is not complete. The active chain is:

```text
original byte/result reverse required by claimed scope
 -> direct-retail selected-member provenance
 -> exact representation classification
 -> supported real edit/rebuild
 -> next-volume reopen/rematerialization
 -> original DMC3 consumer-visible effect
 -> rollback
 -> final L1 audit
```

### Cross-layer vertical proof

The highest-value integrated receipt is one same-resource lineage:

```text
[L2] original selected provider/member
 -> [L1] exact materialized bytes/provenance
 -> authored rebuild/rematerialization
 -> [L3] original ready/use lifecycle
 -> deterministic consumer-visible effect
 -> rollback
```

## Important evidence boundaries

- `.afs/` strings such as `GData.afs/` are logical namespace evidence, not proof of a binary AFS backend.
- Historical GDSpaces PACK parsing does not prove original DMC3 PACK runtime authority.
- `.index` is historical extraction/naming evidence, not original runtime lookup authority on the recovered path.
- Display names, semantic suffixes and embedded aliases do not retarget physical ResourceId/write authority.
- Runtime type identity is not one universal detector: registry probe, container post-load dispatcher and family-mask classifier are separate evidence sites.
- A product writer that creates game-accepted output is not automatically equivalent to Capcom's external/offline authoring tool.
- Product safety may intentionally be stricter than unsafe original wrap/short-success/failure-swallowing behavior.
- Manager state3 is not automatically universal family-semantic success or a consumer-visible gameplay receipt.
- Synthetic CI proves bounded composition only; trusted original-process receipts are required for original equivalence claims.

## Build

```bash
cmake -S . -B build -DDMC_RENGINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Ninja presets:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Visual Studio preset:

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-release
ctest --preset vs2022-release
```

## Project navigation

- [Documentation index](docs/README.md)
- [GDSpaces master roadmap](docs/gdspaces/master-roadmap.md)
- [Layer 1 roadmap](docs/gdspaces/l1-roadmap.md)
- [Layer 2 roadmap](docs/gdspaces/l2-roadmap.md)
- [Layer 3 roadmap](docs/gdspaces/l3-roadmap.md)
- [Layer classification](docs/gdspaces/decompilation-layer-classification.md)
- [Current status](docs/status/current.md)
- [Machine-readable status](docs/status/canonical-status.json)
- [Blockers](docs/status/blockers.md)
- [Risk register](docs/status/risks.md)
- [Phase map](docs/status/phase-map.md)
- [Architecture](docs/architecture.md)
- [GDSpaces contract](docs/gdspaces-contract.md)
- [Evidence Packets](docs/evidence/evidence-packets.md)
- [PE Inspector](docs/exe/pe-inspector.md)
- [Guarded Patching](docs/patch/guarded-patching.md)
- [Specifications](specs/README.md)
- [Constitution](.specify/memory/constitution.md)

## Public lore

**The Sect of Neuroslop / Секта Нейрошлаку** is the DMC Rengine community. **The Monks of Binary Code / Монахи Бінарного Коду** are evidence-backed core contributors. **The Order of the Inverted Triangle** is a lore-facing alias for the core team.

All lore names are fictional branding and never replace technical identity, evidence status, repository permissions, legal policy or professional security controls.

- [Lore](docs/brand/lore.md)
- [Sect of Neuroslop](docs/brand/sect-of-neuroslop.md)
- [Naming system](docs/brand/naming-system.md)
- [Brand glossary](docs/brand/glossary.md)

## Legal and repository policy

This repository does not contain Capcom game binaries, proprietary game assets, extracted archives, copyrighted resource blobs, leaked source code or unauthorized distributions. Users must provide legally obtained game files locally.

Recovered structures, behavior models and source units are independent research findings and must not be represented as leaked/original Capcom source.

DMC Rengine is an independent community research/modding project and is not affiliated with or endorsed by Capcom.

## Motto

> **No claim without evidence. No tool outside the graph. No second resolver.**
