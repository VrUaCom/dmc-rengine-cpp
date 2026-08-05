# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*  
> **Built by the Sect of Neuroslop and the Monks of Binary Code.**

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing, and eventually recompiling Devil May Cry 3: Special Edition from the HD Collection.

It is an evidence-backed platform connecting resource identity, executable research, binary inspection, stage reconstruction, guarded modification, source integration, testing, and long-term source recovery.

## Current state

**Status snapshot:** 2026-08-05  
**Project version:** 0.2.0  
**Reviewed implementation base:** `main` at `1f77e2076a79216e015a3ddc83b1d1ed89c121c8`

The repository is well beyond its original scaffold. The active C++ tree includes:

- C++20/CMake core library and CLI;
- Windows and Ubuntu validation;
- SHA-256 artifact identity;
- versioned Evidence Packets with deterministic export and strict untrusted JSON import;
- bounds-checked binary and PE32/PE32+ inspection;
- GDSpaces identity, sources, classification, graph, routing, typed bundles, and working copies;
- generic read-only container contracts and synthetic container fixtures;
- Binary Inspector regions, typed fields, ownership, annotations, selection context, conflicts, and manifests;
- DMC3 stage-table descriptors, resource matching, Stage Workspace construction, and shared Stage Ops/ModViz views;
- Item runtime evidence, requests, validation plans, guarded patch compilation, copied-output execution, and verified rollback;
- source modification packages, Integration Projects, Custom Build identity, source-to-binary mappings, and executable reopen lineage;
- HITS parsing, runtime-derived grid behavior, safe editing, deterministic spatial rebuilding, corpus comparison, and CLI reports;
- reviewed DMC3 PC-save Pass 31/32 record-envelope and checksum ABI.

The repository does **not** claim full DMC3 decompilation, a production archive writer suite, Capcom HITS offline-builder equivalence, a complete desktop UI, or a working rebuilt game executable.

## Implementation truth and research truth

DMC Rengine C++ intentionally separates two authority layers:

- **GitHub `main`** is the source of truth for reviewed, implemented, tested product code.
- **Google Drive reverse research** is the source of truth for the newest research findings and recovered-source snapshots before product promotion.

Drive research currently extends through Wide Pass 33. Reviewed product promotion currently extends through Wide Pass 32. Newer research is migrated narrowly through Evidence Packets, reviewed C++, tests, CI, and provenance receipts rather than bulk-imported.

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

PAC, PNST, NBZ, and AFS are internal container layers, not top-level product architecture. Legacy PAC Editor/PAC Manager logic is intentionally excluded.

## Central engineering laws

> **All tools receive resources through GDSpaces.**

> **No claim without evidence.**

> **No original-file write without an explicit working-copy/export contract.**

> **Recovered C++ is an evidence-backed reconstruction, not leaked or original Capcom source.**

These laws prevent duplicated identities, competing loaders, false compatibility claims, and unsafe binary modification.

## Notable implemented vertical slices

### HITS collision

The current implementation supersedes the rejected historical `HITS$`/fixed-marker assumption.

Implemented:

- header-driven `HITS` parsing;
- exact `0x38` triangle-plane records;
- spatial grids and signed `-1`-terminated reference lists;
- source 0/member 3 and source 1/member 6 identity;
- Binary Inspector semantic regions and fields;
- EXE-backed grid conversion, flattening, broadphase, deduplication, and reject-mask behavior;
- topology-preserving safe edits;
- normal and plane-D recomputation;
- deterministic DMC Rengine SAT spatial writer;
- parser/writer round trips;
- stable surface identity;
- spatial differential reports and `compare-hits-spatial` CLI.

The writer is structurally tested. Equivalence with Capcom's unknown offline builder remains **RESEARCH REQUIRED** until real corpus and game-runtime validation are complete.

### DMC3 PC save

Reviewed Pass 31/32 support includes:

- exact file size `0x4A30`;
- 21 integrity envelopes;
- one `0x138` global record;
- ten `0x40` summary records;
- ten `0x70C` payload records;
- four-byte `recordState + checksum` trailers;
- one's-complement end-around-carry validation with valid fold `0xFFFF`;
- packed-BCD date/time handling;
- compile-time layouts, diagnostics, tests, Evidence Packets, and Drive/GitHub provenance.

Wide Pass 33 payload semantics remain research-ready and product-promotion-pending.

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

The nearest open product gates are:

1. advance generic container contracts into a production read-only PAC/PNST/NBZ/AFS path;
2. assemble the first legal local game-backed `st001` StageBundle;
3. perform real HITS corpus comparison and controlled game-runtime validation;
4. promote Wide Pass 33 through a narrow Evidence Packet, reviewed C++, tests, and CI;
5. compile and behaviorally compare the first isolated recovered subsystem;
6. advance toward controlled recompilation milestones.

Complete UI and public release automation remain downstream of these evidence and integration gates.

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
