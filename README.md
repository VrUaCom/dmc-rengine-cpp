# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing, and eventually recompiling Devil May Cry 3: Special Edition from the HD Collection.

It is not another isolated format editor or a loose collection of binary patches. DMC Rengine is being built as one evidence-backed platform connecting resource identity, executable research, binary inspection, stage reconstruction, visual editing, guarded modification, testing, and long-term source recovery.

## Current state

The repository has moved beyond a placeholder scaffold. The current 0.2 foundation includes:

- C++20/CMake core library and CLI;
- Windows/Linux CI;
- SHA-256 artifact identity;
- versioned Evidence Packets and deterministic JSON export;
- bounds-checked binary reading primitives;
- generic read-only PE32/PE32+ inspection;
- GDSpaces resource identity, references, payloads, diagnostics, sources, graph, routing, and typed stage bundles;
- safe read-only local directory mounts with root-containment checks;
- revisioned `WorkingCopy` editing with expected-byte guards and undo;
- atomic `GuardedPatchPlan` validation by source hash, ranges, expected bytes, and overlap;
- synthetic tests with no original game bytes;
- project Constitution, SDD specifications, governance, security, clean-room policy, Canon migration, blockers, risks, and status JSON.

Full DMC3 decompilation, production container writers, the complete UI, and a rebuilt executable remain long-term goals. They are not current completion claims.

## The public lore

The community-facing fictional identity of DMC Rengine is **The Order of the Inverted Triangle**. Contributors may informally call themselves **Monks of Reverse**: researchers who descend from visible behavior to bytes, structure, ownership, and recovered source.

**The Sect of Neuroslop / Секта Нейрошлаку** is the satirical AI-assisted experimentation wing. Its rule is simple: generated output is not evidence until it survives triage, testing, correction, and review.

The Order and Sect are creative brand layers, not real religions, cults, or secret organizations.

- [DMC Rengine Lore](docs/brand/lore.md)
- [Sect of Neuroslop](docs/brand/sect-of-neuroslop.md)
- [Naming System](docs/brand/naming-system.md)
- [Brand Glossary](docs/brand/glossary.md)

## Canonical architecture

- **GDSpaces — The Archive** is the only resource access API.
- **Spider Hub — The Nexus** connects tools, resources, specifications, evidence, and project memory.
- **EXE Editor — The Scriptorium** owns executable analysis, decompilation evidence, recovered C/C++, and guarded patch planning.
- **Binary Inspector — The Reliquary** consumes bytes, regions, diagnostics, ownership, and evidence; it does not resolve game sources independently.
- **Stage Ops — The Theatre** receives typed `StageBundle` objects.
- **ModViz — The Observatory** contains a 3D Scene/Model Editor and a Menu/HUD Editor.
- **Item Editor — The Forge** is a GDSpaces client for typed item authoring and runtime-linked validation.
- **Build & Test Lab — The Trial Chamber** owns repeatable builds, tests, patch validation, and regression evidence.
- **Reverse Canon** is the versioned body of accepted, corrected, and rejected findings.
- **Evidence Registry** binds claims to artifact hashes, offsets, RVA/VA locations, symbols, runtime observations, and tests.

PAC, PNST, NBZ, and AFS are internal container layers—the **Sealed Archives**—not top-level product architecture. The legacy PAC Editor/PAC Manager worldview is intentionally excluded from the new design.

## The central law

> **All tools receive resources through GDSpaces.**

Within the project lore, a tool that creates its own competing resource resolver commits **The Heresy of the Second Resolver**. In engineering terms, this prevents duplicated identities, conflicting loaders, unstable paths, and incompatible ownership models.

## CLI

```text
dmc-rengine version
dmc-rengine doctor
dmc-rengine scan <directory>
dmc-rengine hash <path>
dmc-rengine route <format>
dmc-rengine inspect-exe <path>
```

`hash` and `inspect-exe` acquire files through the read-only GDSpaces local-source path rather than opening them inside tool-specific modules.

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

## Initial implementation path

1. Stabilize the C++ foundation and CI.
2. Complete artifact identity and Evidence Packet import/validation.
3. Expand read-only PE analysis and EXE evidence models.
4. Build synthetic container fixtures and generic parser interfaces.
5. Implement NBZ/AFS/PAC/PNST resource expansion through GDSpaces.
6. Assemble the first EXE-backed `st001` StageBundle.
7. Migrate Binary Inspector's structure and ownership model.
8. Migrate Stage Ops, Item Editor, and ModViz through shared working-copy and patch contracts.
9. Recover isolated executable systems into compilable, behavior-tested C++ units.
10. Advance toward controlled recompilation milestones.

The overall campaign is called **The Long Descent**: from archive mapping to resource recovery, behavior recovery, isolated recompilation, and eventually a controlled rebuildable engine.

## Legal and repository policy

This repository does not contain Capcom game binaries, `dmc3.exe`, proprietary game assets, extracted archives, copyrighted resource blobs, leaked source code, or unauthorized distributions. Users must provide legally obtained game files locally.

Recovered structures, names, and source units are independent research findings and must not be represented as leaked or original Capcom source code.

DMC Rengine is an independent, community-driven research and modding project and is not affiliated with or endorsed by Capcom.

## Motto

> **No claim without evidence. No tool outside the graph. No second resolver.**
