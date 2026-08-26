# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*  
> **Built by the Sect of Neuroslop and the Monks of Binary Code.**

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing and progressively recompiling Devil May Cry 3: Special Edition from the HD Collection.

The project connects exact artifact identity, executable research, resource materialization, binary inspection, stage reconstruction, guarded authoring, recovered source, validation and long-term recompilation under one evidence-first architecture.

## Current state

**Status snapshot:** 2026-08-27  
**Version:** 0.2.0  
**Canonical authority reconciled through:** merged PR #242  
**Primary execution program:** **GDSpaces Layer 1 — final real same-lineage acceptance**  

Current status:

- **L1 Resource Materialization:** internal product path closed for the current representative DMC3-HD acceptance scope; real same-lineage acceptance remains open;
- **L2 Resource Resolution:** static/tooling authority advanced; real retail collision census, real R2B v2, trusted selected identity and successful-mount topology reconciliation remain open;
- **L3 Original Runtime/Lifecycle:** static spine advanced; materialization terminal-condition dependency and broader lifecycle/consumer validation remain open;
- **RCP / V / LV:** orthogonal control/validation architecture, not L4; draft work remains branch truth until merged.

The repository already contains substantial reviewed implementation:

- C++20/CMake core and CLI with Windows + Ubuntu validation;
- SHA-256 artifact/evidence infrastructure;
- GDSpaces ResourceId/ResourceRef/SourceRegistry/ByteProvenance/WorkingCopy;
- canonical NBZ ZIP indexing and STORE/raw-DEFLATE materialization;
- PAC/PNST relative-slot parsing with sparse/empty/alias identity preservation;
- recursive PAC/PNST expansion;
- bounded same-size, size-changing and nested PAC/PNST authoring/reintegration;
- byte-exact untouched-sibling preservation;
- verified immutable NBZ copy rebuild;
- next-contiguous STORE NBZ overlay generation and canonical reopen/rematerialization;
- direct-retail acquisition and protected-distribution product closure orchestration;
- numbered-volume resolver/bootstrap reverse and process-instance-bound R2B v2 tooling;
- protected-distribution vs canonical-analysis executable authority separation;
- Binary Inspector, EXE evidence, Stage/Item/HITS/save and guarded-modification foundations.

The project does **not** claim full DMC3 decompilation, whole-game behavioral equivalence, Capcom offline-writer equivalence, a complete desktop editor, or a behaviorally equivalent rebuilt executable.

## GDSpaces L1 — current critical path

Canonical roadmaps:

- [L1 roadmap](docs/gdspaces/l1-roadmap.md)
- [L1/L2/L3 master roadmap](docs/gdspaces/master-roadmap.md)

Completion is **gate-based**, not percentage-based. Current path:

```text
exact real selected/member lineage
 -> exact member materialization + ByteProvenance
 -> representation classification
 -> supported real edit/rebuild
 -> next-volume publication + canonical reopen/rematerialization
 -> original DMC3 deterministic consumer-visible effect
 -> rollback / retail immutability
 -> final cross-stack/V audit
```

Issue #209 remains the mandatory original-game consumption/rollback gate.

### Connected artifact boundary

Merged #233 establishes that protected `dmc3.exe` and executable-relative `data/dmc3/dmc3-0.nbz` are locatable. The retail NBZ is observed at `960,358,951` bytes while the connected raw materialization ceiling is `268,435,456` bytes.

This is a **transport/access limitation**, not archive absence and not an L1 parser failure.

Pocket GDS can provide an out-of-band exact-member materialization receipt when the archive is already local on-device. That receipt does not by itself prove the protected original-process resolver winner, successful original mount topology or original-game consumption.

## L2 — current correction and evidence frontier

Merged #235 hardens R2B to a process-instance-bound v2 model and proves a critical topology distinction:

```text
numbered filename discovery / registration attempts
!= actual successful mounted sources
```

The first missing numbered filename bounds discovery only. An existing archive may fail registration while discovery continues, so successful mounts may be sparse. Successful archive registrations prepend, preserving:

```text
higher successful volume -> lower successful volume -> physical
```

Issue #237 tracks the product correction; PR #241 is pending and is not canonical `main` truth until merged.

A promotable R2B v2 packet requires one exact protected process instance with PID, non-zero OS process creation FILETIME, module identity and seven canonical anchor windows. A real packet has not yet been promoted.

The retail `0x0E` normalized-key collision census also remains mandatory because the recovered archive comparator has no equal-key secondary tie-break.

## L1/L3 materialization completion seam

Merged #230/#242 establish the normal completion ABI:

```text
0x1401B84E0
 -> registers 0x1401B8DC0
 -> one u32 registry-relative LoadedResource context
```

Normal `0x1401B8DC0` does **not** receive raw transport status/error, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata.

Therefore lower materialization success/failure must already be terminal before normal state2 publication, or the queued completion must be suppressed/removed.

**FIFO insertion order alone is not a proven dependency barrier.** No generic original fan-in/outstanding-child counter is claimed.

The exact-byte reverse priority is `0x1402EF4D0` -> relevant `0x1402EF790` persistence/retirement -> reacquire historical `0x1400333E0`/`0x140033390` hypotheses -> `0x1400335A0` terminal writes -> identify incomplete/failure completion suppression -> `0x1402EF460` rollback/clear -> only then `.lst` recursive failure ordering.

## Canonical architecture

- **GDSpaces — The Archive:** only product resource resolver/materializer/provenance authority.
- **Recovered Game Source Tree:** reconstructed original DMC3 functions, ABI, ownership and lifecycle behavior.
- **Reverse Core:** generic artifact/range/function/type/claim/reconstruction/validation infrastructure.
- **EXE Editor — The Scriptorium:** frontend over executable mappings, recovered-source identities and guarded patch/rebuild requests.
- **Binary Inspector — The Reliquary:** byte/structure/evidence inspection; never a source resolver.
- **Stage Ops — The Theatre:** downstream product-side stage/scene assembly and operational workspace authority.
- **Stage Semantic Graph:** evidence-aware representation/index emitted from Stage Ops state.
- **ModViz — The Observatory:** scene/model/menu editor and visualization consumer over Stage Ops/Semantic Graph.
- **Build & Test Lab — The Trial Chamber:** reproducibility, validation, generated outputs and behavioral receipts.
- **RCP / V / LV:** orthogonal orchestration and validation surfaces; never a fourth execution layer.

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
- Product materialization and Stage Ops state are not automatically original game-ready/consumer-success state.
- Synthetic CI proves bounded composition only; direct-retail/original-process receipts remain mandatory where a completion gate requires them.
- Independent PASS receipts must not be joined by filename alone.
- Open PRs remain branch truth until merged.

## Supporting EXE reverse frontier for GDS

Do not restart already-bounded bootstrap/candidate/archive-index/type-0/ZIP/LoadedResource work without contradictory direct evidence.

Current high-value exact frontier includes:

- #242 materialization terminal-condition dependency mechanism;
- real protected-process R2B v2 seven-anchor capture;
- trusted original-process selected identity;
- retail `0x0E` normalized-key collision census;
- successful-mount topology observation/reconciliation;
- complete ZIP stream initializer `0x140328540` only if activated by the chosen acceptance path;
- complete compressed seek/reset/reinflate `0x140328FE0` only if activated;
- dynamic `.lst` lifetime/error/cycle behavior only if the representative path actually selects `.lst`.

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
- [Canonical GDSpaces master roadmap](docs/gdspaces/master-roadmap.md)
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
