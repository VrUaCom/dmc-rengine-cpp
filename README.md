# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*  
> **Built by the Sect of Neuroslop and the Monks of Binary Code.**

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing, and eventually recompiling Devil May Cry 3: Special Edition from the HD Collection.

It is an evidence-backed platform connecting resource identity, executable research, binary inspection, stage reconstruction, guarded modification, source integration, testing, and long-term source recovery.

## Current state

**Status snapshot:** 2026-08-20  
**Project version:** 0.2.0  
**Reviewed implementation base:** `main` at `4cf6b34258e95bc6fde19979036c82ba0104d270`  

The repository is well beyond its original scaffold. Current reviewed code includes:

- C++20/CMake core library and CLI;
- Windows and Ubuntu validation;
- SHA-256 artifact identity and evidence infrastructure;
- GDSpaces resource identity, sources, classification, graph, routing, WorkingCopy and provenance;
- canonical NBZ STORE/raw-DEFLATE materialization;
- canonical PAC/PNST structural parsing and recursive expansion;
- same-size PAC/PNST authoring and validated nested reintegration;
- runtime-synth size-changing PAC/PNST authoring using the recovered `.lst` 64-byte layout;
- typed verified nested size-changing runtime-synth composition;
- deterministic STORE-only next-volume NBZ overlay authoring and reopen;
- Binary Inspector domain models;
- PE/EXE inspection and executable evidence acquisition;
- Stage/Item/HITS/save/source-integration verticals;
- guarded modification and rollback infrastructure.

The repository does **not** claim full DMC3 decompilation, whole-game behavioral equivalence, Capcom offline-packer equivalence, lossless retail-NBZ repack, a complete desktop UI, or a working rebuilt game executable.

### GDSpaces reverse status

GDSpaces reverse progress is tracked by a canonical **hypothetical reverse-coverage index**. It is not an equivalence percentage.

| Layer | Index | Status |
|---|---:|---|
| **L1 — Resource Materialization** | **88%** | ACTIVE / NOT COMPLETE |
| **L2 — Resource Resolution** | **94%** | HIGH / NOT COMPLETE |
| **L3 — Original Runtime / Lifecycle** | **72%** | ADVANCED / NOT COMPLETE |
| **V — Validation** | **60%** | SUPPORTING / NOT A LAYER |

A layer reaches `100% / COMPLETE` only when every mandatory gate is closed or evidence-pruned, the closing work is in canonical `main`, representative real-corpus receipts exist, required Windows+Ubuntu validation is green, and no architecture-changing contradiction remains.

For L1, the remaining mandatory closure path is:

```text
real .lst / representative real PNST evidence
        +
real child <-> slot / intrinsic-byte authority
        +
no-loss retail NBZ metadata + repack tier
        ↓
representative real size-changing edit
        ↓
bottom-up rebuild
        ↓
retail/overlay NBZ
        ↓
reopen
        ↓
canonical compare
        ↓
controlled original-game consumption receipt
        ↓
L1 100% / COMPLETE
```

See [GDSpaces reverse-progress scale](docs/gdspaces/reverse-progress-scale.md) and [GDSpaces layer classification](docs/gdspaces/decompilation-layer-classification.md) for the exact rules and boundaries.

## Implementation truth and research truth

DMC Rengine C++ intentionally separates two authority layers:

- **GitHub `main`** is the source of truth for reviewed, implemented, tested product code.
- **Google Drive reverse research** is the source of truth for newer research findings and recovered-source snapshots before product promotion.

Research findings are migrated narrowly through evidence, reviewed C++, tests, CI, provenance receipts, and explicit promotion boundaries rather than being bulk-imported.

## The public lore

**The Sect of Neuroslop / Секта Нейрошлаку** is the DMC Rengine community: reverse engineers, modders, programmers, artists, testers, researchers, tool builders, and supporters gathered around the project.

**The Monks of Binary Code / Монахи Бінарного Коду** are the creators and evidence-backed core contributors who build, maintain, test, document, and reconstruct DMC Rengine systems. The title is recognition of accepted work, not a permission level or social rank.

**The Order of the Inverted Triangle** is a lore-facing alias for the core DMC Rengine team. It is not the whole community. Its development home is the **Monastery of Binary Code**.

The shared engineering journey is **The Long Descent**: moving from visible behavior through resources, structures, ownership, functions, and recovered source toward **The Return**—behavior-tested source entering a controlled build.

All Sect, Monk, Order, Monastery, ritual, chamber, and canon language is fictional public branding. It does not replace technical identities, evidence states, repository permissions, legal language, or professional security controls.

- [DMC Rengine Lore](docs/brand/lore.md)
- [Sect of Neuroslop](docs/brand/sect-of-neuroslop.md)
- [Naming System](docs/brand/naming-system.md)
- [Brand Glossary](docs/brand/glossary.md)

## Canonical architecture

- **GDSpaces — The Archive** is the only resource access API.
- **Spider Hub — The Nexus** connects tools, resources, specifications, evidence, events, and project provenance.
- **EXE Editor — The Scriptorium** owns executable analysis, recovered-source evidence, address mappings, guarded patch requests, and rebuilt-output lineage.
- **Binary Inspector — The Reliquary** consumes supplied bytes, structures, ownership, diagnostics, annotations, and evidence; it does not resolve game sources independently.
- **Stage Ops — The Theatre** receives typed shared Stage Workspace state.
- **ModViz — The Observatory** consumes the same stage/resource state for scene/model and future Menu/HUD editing.
- **Item Editor — The Forge** produces typed item changes and evidence-backed runtime requests; it does not patch the executable directly.
- **Build & Test Lab — The Trial Chamber** owns validation plans, guarded patch compilation, copied-output execution, rollback, and regression evidence.
- **Reverse Canon** is the versioned body of accepted, corrected, rejected, and research-required findings.
- **Evidence Registry** binds claims to artifact hashes, byte ranges, RVA/VA locations, symbols, runtime observations, tests, and supersession records.

PAC, PNST and NBZ are internal resource/container layers. A binary AFS backend is evidence-gated and must not be inferred from `.afs/` logical namespaces. Legacy PAC Editor/PAC Manager logic is intentionally excluded.

## Central engineering laws

> **All tools receive resources through GDSpaces.**

> **No claim without evidence.**

> **No original-file write without an explicit working-copy/export contract.**

> **Recovered C++ is an evidence-backed reconstruction, not leaked or original Capcom source.**

These laws prevent duplicated identities, competing loaders, false compatibility claims, and unsafe binary modification.

## CLI

```text
dmc-rengine version
dmc-rengine doctor
dmc-rengine scan <directory>
dmc-rengine hash <path>
dmc-rengine validate-evidence <path>
dmc-rengine route <format>
dmc-rengine inspect-exe <path>
dmc-rengine list-tools
dmc-rengine list-formats
dmc-rengine integration-status
dmc-rengine inspect-workspace <path> [--stage] [--menu]
dmc-rengine compare-hits-spatial <original> <candidate> [report.json]
```

Local files are acquired through GDSpaces-backed sources rather than tool-specific filesystem loaders.

## Build

Generic CMake workflow:

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

## Current frontier

The project-wide resource priority is **GDSpaces Layer 1 — Resource Materialization**. L2/L3 findings remain preserved but do not count as L1 completion unless they directly close a materialization boundary.

Current L1 frontier:

1. preserve the raw retail NBZ serialization envelope needed for no-loss repack;
2. implement and validate the metadata-preserving retail-NBZ repack tier without conflating it with STORE-overlay authoring;
3. reacquire representative legal `.lst` and PNST/raw-container artifacts where current connected storage lacks them;
4. prove representative real child-to-slot and intrinsic-byte authority for size-changing nested writes;
5. run a representative legal `materialize -> edit -> bottom-up rebuild -> NBZ -> reopen -> canonical compare` receipt;
6. obtain a controlled original-game consumption receipt for authored output;
7. only then promote `L1 = 100% / COMPLETE`.

Complete UI, progressive recompilation, and whole-game equivalence remain downstream.

## Project navigation

- [Documentation index](docs/README.md)
- [Current status](docs/status/current.md)
- [Machine-readable status](docs/status/canonical-status.json)
- [Phase map](docs/status/phase-map.md)
- [Blockers](docs/status/blockers.md)
- [Risk register](docs/status/risks.md)
- [Architecture](docs/architecture.md)
- [Canonical decisions](docs/history/canonical-decisions.md)
- [Deprecated architecture](docs/history/deprecated-architecture.md)
- [Migrated reverse findings](docs/history/migrated-findings.md)
- [GDSpaces contract](docs/gdspaces-contract.md)
- [GDSpaces layer classification](docs/gdspaces/decompilation-layer-classification.md)
- [GDSpaces reverse-progress scale](docs/gdspaces/reverse-progress-scale.md)
- [GDSpaces runtime-synth PAC/PNST writer](docs/gdspaces/dmc3-runtime-synth-relative-slot-writer.md)
- [GDSpaces nested PAC/PNST reintegration](docs/gdspaces/dmc3-nested-relative-slot-reintegration.md)
- [GDSpaces NBZ STORE overlay writer](docs/gdspaces/dmc3-nbz-store-overlay-writer.md)
- [Evidence Packets](docs/evidence/evidence-packets.md)
- [PE Inspector](docs/exe/pe-inspector.md)
- [Guarded Patching](docs/patch/guarded-patching.md)
- [Specifications](specs/README.md)
- [Constitution](.specify/memory/constitution.md)

## Legal and repository policy

This repository does not contain Capcom game binaries, `dmc3.exe`, proprietary game assets, extracted archives, copyrighted resource blobs, leaked source code, or unauthorized distributions. Users must provide legally obtained game files locally.

Recovered structures, names, behavior models, and source units are independent research findings and must not be represented as leaked or original Capcom source code.

DMC Rengine is an independent, community-driven research and modding project and is not affiliated with or endorsed by Capcom.

## Motto

> **No claim without evidence. No tool outside the graph. No second resolver.**
