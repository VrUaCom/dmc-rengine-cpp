# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing, and eventually recompiling Devil May Cry 3: Special Edition from the HD Collection.

It is not another isolated format editor or a loose collection of binary patches. DMC Rengine is being built as one evidence-backed platform connecting resource identity, executable research, scene reconstruction, visual editing, guarded modification, testing, and long-term source recovery.

## The public lore

The community-facing fictional identity of DMC Rengine is **The Order of the Inverted Triangle**. Contributors may informally call themselves **Monks of Reverse**: researchers who descend from visible behavior to bytes, structure, ownership, and recovered source.

The Order is a creative brand layer, not a real religion, cult, or secret organization. The engineering discipline behind it is real:

- observe before changing;
- preserve evidence;
- separate hypotheses from confirmed facts;
- reproduce before claiming;
- leave a trace another researcher can follow.

Read the full [DMC Rengine Lore](docs/brand/lore.md), [Naming System](docs/brand/naming-system.md), and [Brand Voice Guide](docs/brand/voice-and-marketing.md).

## Canonical architecture

- **GDSpaces — The Archive** is the only resource access API.
- **Spider Hub — The Nexus** connects tools, resources, specifications, evidence, and project memory.
- **EXE Editor — The Scriptorium** owns executable analysis, decompilation evidence, recovered C/C++, and guarded patch planning.
- **Binary Inspector — The Reliquary** consumes bytes, regions, diagnostics, ownership, and evidence; it does not resolve game sources independently.
- **Stage Ops — The Theatre** receives typed `GDStageBundle` objects.
- **ModViz — The Observatory** contains a 3D Scene/Model Editor and a Menu/HUD Editor.
- **Item Editor — The Forge** is a GDSpaces client for typed item authoring and runtime-linked validation.
- **Build & Test Lab — The Trial Chamber** owns repeatable builds, tests, patch validation, and regression evidence.
- **Reverse Canon** is the versioned body of accepted findings.
- **Evidence Registry** binds claims to hashes, offsets, bytes, traces, runtime observations, and tests.

PAC, PNST, NBZ, and AFS are internal container layers—the **Sealed Archives**—not top-level product architecture. The legacy PAC Editor/PAC Manager worldview is intentionally excluded from the new design.

## The central law

> **All tools receive resources through GDSpaces.**

Within the project lore, an editor that creates its own competing resource resolver commits **The Heresy of the Second Resolver**. In engineering terms, this prevents duplicated identity systems, conflicting loaders, unstable paths, and incompatible ownership models.

## Project status

This repository is the clean C++ foundation for the next generation of DMC Rengine. It intentionally does not import the legacy project wholesale. Existing knowledge and proven behavior will be migrated through documented contracts, evidence, tests, and reviewed implementation phases.

The project is in the foundation and controlled migration stage. Full decompilation and a rebuilt executable are long-term goals, not current claims.

## Initial implementation path

1. Establish the C++20/CMake foundation.
2. Define stable resource identities and GDSpaces contracts.
3. Build read-only source mounting and typed resource discovery.
4. Integrate Binary Inspector, Stage Ops, ModViz, and EXE evidence as clients.
5. Add safe working-copy, validation, patch-plan, and export pipelines.
6. Migrate proven format and runtime knowledge through tests and evidence packets.
7. Progress from evidence-backed decompilation units toward recompilable engine modules.

The overall campaign is called **The Long Descent**: from archive mapping to resource recovery, behavior recovery, isolated recompilation, and eventually a controlled rebuildable engine.

## Repository map

- [`docs/architecture.md`](docs/architecture.md) — canonical technical architecture.
- [`docs/roadmap.md`](docs/roadmap.md) — staged implementation roadmap.
- [`docs/reverse-engineering-rules.md`](docs/reverse-engineering-rules.md) — evidence and confidence rules.
- [`docs/brand/lore.md`](docs/brand/lore.md) — Order, monks, symbols, mottos, and public lore.
- [`docs/brand/naming-system.md`](docs/brand/naming-system.md) — product and subsystem names.
- [`docs/brand/voice-and-marketing.md`](docs/brand/voice-and-marketing.md) — messaging, taglines, ranks, and visual language.
- `include/dmc_rengine/` — public C++ interfaces.
- `src/` — implementation and CLI.
- `tests/` — reproducible unit and integration tests.
- `.github/workflows/` — cross-platform CI.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Legal and repository policy

This repository does not contain Capcom game binaries, `dmc3.exe`, proprietary game assets, extracted archives, copyrighted resource blobs, or redistributed reverse-engineered binary data. Users must provide legally obtained game files locally.

Recovered structures, names, and source units must be documented as independent research findings and must not be represented as leaked or original Capcom source code.

DMC Rengine is an independent, community-driven research and modding project and is not affiliated with or endorsed by Capcom.

## Motto

> **No claim without evidence. No tool outside the graph. No second resolver.**
