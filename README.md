# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*  
> **Built by the Sect of Neuroslop and the Monks of Binary Code.**

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing and progressively recompiling Devil May Cry 3: Special Edition from the HD Collection.

The project connects exact artifact identity, executable research, resource materialization, binary inspection, stage reconstruction, guarded authoring, recovered source, validation and long-term recompilation under one evidence-first architecture.

## Current state

**Status snapshot:** 2026-08-25  
**Version:** 0.2.0  
**Canonical implementation base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`  
**Primary execution program:** **GDSpaces Layer 1 — Resource Materialization**  

The repository already contains substantial reviewed implementation:

- C++20/CMake core and CLI with Windows + Ubuntu validation;
- SHA-256 artifact/evidence infrastructure;
- GDSpaces ResourceId/ResourceRef/SourceRegistry/ByteProvenance/WorkingCopy;
- canonical NBZ ZIP indexing and STORE/raw-DEFLATE materialization;
- PAC/PNST relative-slot parsing with sparse/empty/alias identity preservation;
- recursive PAC/PNST expansion;
- bounded same-size and size-changing PAC/PNST authoring/reintegration;
- synthetic full nested A-to-Z NBZ rebuild/reopen composition;
- transformed DDS-bearing texture framing and bounded size-changing writer for the evidenced safe subset;
- original DMC3 non-TM2 serialized `gfxTexture` relocation compatibility checks for writer output;
- numbered `DMC3-N.nbz` bootstrap/precedence reconstruction;
- shared atomic/no-replace publication and artifact-bound retail member acquisition;
- next-contiguous STORE NBZ overlay generation and canonical resolver selection validation;
- protected-distribution vs unpacked-analysis executable authority roles;
- merged product-side protected-retail authoring closure orchestration;
- Binary Inspector, EXE evidence, Stage/Item/HITS/save and guarded-modification foundations.

The project does **not** claim full DMC3 decompilation, whole-game behavioral equivalence, Capcom offline-writer equivalence, a complete desktop editor, or a behaviorally equivalent rebuilt executable.

## GDSpaces L1 — current critical path

The canonical cross-layer roadmap is [docs/gdspaces/master-roadmap.md](docs/gdspaces/master-roadmap.md); the detailed L1 gates are in [docs/gdspaces/l1-roadmap.md](docs/gdspaces/l1-roadmap.md).

Completion is **gate-based**, not percentage-based. Current product hardening and external acceptance are:

```text
#212 bind the closure receipt to its artifact-bound acquisition sidecar
 -> #210 harden recursive PAC/PNST slot-path authoring receipts
 -> direct-retail resolver/acquisition receipt
 -> retail representation classification
 -> representative real edit/rebuild/overlay/rematerialization
 -> #209 original DMC3 consumption + rollback receipt
 -> final L1 cross-stack audit
```

`main` already contains the product seams for artifact-stable acquisition, bounded PAC/PNST authoring, next-volume publication and canonical reopen. PRs #210 and #212 are branch truth until merged. Neither product CI nor the orchestration command substitutes for the still-missing exact retail and original-game receipts.

## Canonical architecture

- **GDSpaces — The Archive:** only product resource resolver/materializer/provenance authority.
- **Recovered Game Source Tree:** reconstructed original DMC3 functions, ABI, ownership and lifecycle behavior.
- **Reverse Core:** generic artifact/range/function/type/claim/reconstruction/validation infrastructure.
- **EXE Editor — The Scriptorium:** frontend over executable mappings, recovered-source identities and guarded patch/rebuild requests.
- **Binary Inspector — The Reliquary:** byte/structure/evidence inspection; never a source resolver.
- **Stage Ops — The Theatre:** product-side stage/scene assembly and operational workspace authority.
- **Stage Semantic Graph:** evidence-aware representation/index emitted from Stage Ops state.
- **ModViz — The Observatory:** scene/model/menu editor and visualization consumer over Stage Ops/Semantic Graph.
- **Build & Test Lab — The Trial Chamber:** reproducibility, validation, generated outputs and behavioral receipts.

### Core engineering laws

> **All product resource access goes through GDSpaces.**

> **Recovered original-game code belongs to the Recovered Game Source Tree.**

> **No claim without evidence.**

> **No implicit retail-file mutation.**

> **No second resolver or scene truth.**

## Important evidence boundaries

- `.afs/` strings such as `GData.afs/` are logical namespace evidence, not proof of a binary AFS backend.
- Historical GDSpaces PACK parsing does not prove original DMC3 PACK runtime authority.
- A product writer that creates game-accepted output is not automatically equivalent to Capcom's external/offline authoring tool.
- Product materialization and StageBundle/Stage Ops state are not automatically original game-ready state 3.
- Synthetic CI proves bounded composition only; direct-retail/game-backed receipts are required for L1 closure.
- `st001` is a regression/compatibility fixture, not the complete Stage identity model.

## Supporting EXE reverse frontier for GDS

Major bootstrap/candidate/archive-index/ZIP-read/inflate/LoadedResource/post-load boundaries are already strongly recovered and should not be restarted without contradictory direct evidence.

The remaining GDS-relevant exact reverse frontier includes:

- final type-0 physical-provider Win32 filename/case/open/failure semantics after `0x0C` normalization;
- complete ZIP stream initializer `0x140328540` body/lifetime;
- complete compressed seek/reset/reinflate `0x140328FE0` behavior;
- malformed/partial-read error equivalence where required by a promoted claim;
- dynamic `.lst` lifetime/error/cycle behavior only if real loose-container acceptance depends on it.

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
- [Canonical GDSpaces L1 roadmap](docs/gdspaces/l1-roadmap.md)
- [Project roadmap](docs/roadmap.md)
- [Current status](docs/status/current.md)
- [Machine-readable status](docs/status/canonical-status.json)
- [Blockers](docs/status/blockers.md)
- [Risk register](docs/status/risks.md)
- [Phase map](docs/status/phase-map.md)
- [Architecture](docs/architecture.md)
- [GDSpaces contract](docs/gdspaces-contract.md)
- [Layer classification](docs/gdspaces/decompilation-layer-classification.md)
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
