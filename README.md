# DMC Rengine — DMC3 HD Reverse Engineering & Recompilation Research Framework

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*  
> **Built by the Sect of Neuroslop and the Monks of Binary Code.**

**DMC Rengine** is an open-source C++20 framework for reverse engineering, decompiling, editing and progressively recompiling **Devil May Cry 3: Special Edition from the Devil May Cry HD Collection (DMC3 HD)**.

The project combines executable research, DMC3 binary/file-format documentation, resource materialization, archive inspection, guarded authoring, recovered-source work and validation under one evidence-first architecture. It is intended to support long-term DMC3 reconstruction and safe modding tooling without presenting unverified behavior as fact.

**Public documentation / discovery site:** [DMC Rengine on GitHub Pages](https://vruacom.github.io/dmc-rengine-cpp/). The Pages site is a readable discovery layer; `main`, canonical documentation and evidence records remain the technical authority.

## What DMC Rengine can do today

The canonical repository already contains substantial reviewed implementation and research infrastructure, including:

- C++20/CMake core and CLI with Windows + Ubuntu validation;
- SHA-256 artifact/evidence infrastructure and explicit provenance handling;
- GDSpaces resource resolution/materialization foundations;
- NBZ ZIP indexing/materialization and next-volume overlay work;
- PAC/PNST sparse/empty/alias-preserving parsing, recursive expansion and bounded reintegration;
- Binary Inspector and executable-analysis infrastructure;
- evidence-backed DMC3 format documentation and machine-readable registries;
- guarded modification/reintegration paths for explicitly supported subsets.

Canonical built-in Native Reader modules are currently documented for:

```text
DDS
PTX
HITS
DCA
LIG / LIG2
Stage TXT
SCM
MOD
SHW
PE / EXE
```

PAC and PNST remain container parsers, while NBZ is handled as a source/materialization layer. EFM and MOT have active research/history but are **not** presented as canonical Native Reader modules until promotion closes their evidence and integration gates.

For the live implementation/reverse status, use **[Current Project Status](docs/status/current.md)**. That document, together with `main` and the evidence records, is authoritative over summaries in this README.

## What the project does not claim

DMC Rengine does **not** currently claim:

- full DMC3 decompilation;
- whole-game behavioral equivalence;
- Capcom offline-writer equivalence;
- a complete desktop editor;
- a behaviorally equivalent rebuilt executable.

Completion is gate-based. A parser, green synthetic test, structural match or successful bounded writer does not by itself promote a subsystem to complete.

## DMC3 HD formats and archives

DMC Rengine maintains a growing evidence-backed knowledge base for Devil May Cry 3 HD resources and runtime behavior.

Start with:

- [Public DMC3 HD file-format and archive index](docs/formats/public-index.md)
- [DMC Rengine / DMC3 public FAQ](docs/discovery/faq.md)
- [DMC3 HD format documentation](docs/formats/README.md)
- [DMC3 HD format and resource-purpose catalog](docs/formats/dmc3-hd-format-catalog.md)
- [DMC3 HD format presence census](docs/formats/dmc3-hd-format-presence-census.md)
- [GDSpaces contract](docs/gdspaces-contract.md)
- [Canonical status](docs/status/current.md)

Documented/researched families include SCM, MOD, SHW, HITS, PAC, PNST, DDS/PTX, archive/resource infrastructure and executable/runtime evidence. Filename extensions or short ASCII tags alone are never treated as sufficient semantic proof.

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
- Synthetic CI proves bounded product/tool composition only; original-game equivalence requires the applicable direct evidence/receipt gates.
- `st001` is a regression/compatibility fixture, not the complete Stage identity model.

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
- [Current project status](docs/status/current.md)
- [DMC3 public FAQ](docs/discovery/faq.md)
- [Public DMC3 HD format and archive index](docs/formats/public-index.md)
- [DMC3 HD format documentation](docs/formats/README.md)
- [Public discovery strategy](docs/discovery/README.md)
- [Public release readiness](docs/releases/readiness.md)
- [Canonical GDSpaces L1 roadmap](docs/gdspaces/l1-roadmap.md)
- [Project roadmap](docs/roadmap.md)
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

## Public discovery policy

DMC Rengine uses search/discovery metadata to expose real project capabilities, not to inflate claims. The project identity should consistently bind **DMC Rengine**, **DMC3**, **Devil May Cry 3**, **HD Collection**, **reverse engineering**, **file formats**, **decompilation** and long-term **recompilation** research where semantically accurate.

See [Public Discovery Strategy](docs/discovery/README.md) for the metadata, README, documentation-site and external-discovery plan.

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
