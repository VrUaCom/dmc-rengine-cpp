# DMC Rengine

> **Reverse the engine. Rebuild the possibilities.**  
> *Descend to the bytes. Return with the source.*

DMC Rengine is an open-source C++20 framework for reverse engineering, reconstructing, editing, validating, and progressively recompiling Devil May Cry 3: Special Edition from the HD Collection.

## Current state — 2026-08-15

**Merged product baseline:** `main` at `562e14179723598e09e58c9baded998a2b79e1a3`  
**Completion authority:** [`docs/status/completion-and-evidence-policy.md`](docs/status/completion-and-evidence-policy.md)

DMC Rengine already contains substantial real implementation and reverse evidence. However:

> **No major end-to-end subsystem is currently `COMPLETE` or proven behaviorally equivalent to the original DMC3 runtime.**

Individual parsers, ABIs, writers, recovered functions, workspaces and integration slices may be implemented, tested, EXE-confirmed, bounded-closed or validated without promoting the containing subsystem to complete.

Green CI proves the tested branch/build contract. It does not by itself prove original-game behavior.

## Truth layers

DMC Rengine intentionally separates several authorities:

- **raw canonical artifact/runtime evidence** — strongest reverse evidence for the exact artifact;
- **Evidence Packets / reconciliation records** — sanitized reproducible evidence;
- **Recovered Game Source Tree** — evidence-backed executable reconstruction, not automatically original Capcom source;
- **active PR heads** — branch-scoped implementation truth only;
- **GitHub `main`** — merged product implementation truth and may lag active stacked work;
- **Google Drive research** — research history and newer reverse material; older documents can be superseded without being erased.

Agent consensus is not evidence.

## Canonical architecture

```text
canonical dmc3.exe / runtime evidence
        │
        ├── Reverse Core
        │     evidence / claims / reconstruction identities / validation
        │
        ├── Recovered Game Source Tree
        │     reconstructed original DMC3 runtime behavior
        │
        └── Stage / resource executable authority
                    │
                    ▼
                 GDSpaces
          resource resolution / bytes / provenance
                    │
                    ▼
                 Stage Ops
          scene assembly / operational state
              ┌─────┴─────┐
              ▼           ▼
      Semantic Graph     ModViz
      derived index      editor

Binary Inspector = bytes / structure / ownership / evidence
EXE Editor       = executable/reconstruction editing frontend
Build & Test Lab = validation / guarded modification / receipts
```

### Ownership boundaries

- **GDSpaces** is the only product resource resolver/materializer. It does not own original DMC3 runtime functions.
- **Recovered Game Source Tree** owns reconstructed original-game functions, types, factories, caches and lifecycle behavior.
- **Reverse Core** owns generic reverse/evidence/reconstruction infrastructure, not DMC-specific gameplay code.
- **Stage Ops** owns product-side stage/scene assembly and operational state.
- **Stage Semantic Graph** is a derived representation of Stage Ops state, not a second scene assembler.
- **ModViz** consumes Stage Ops state for editing; it does not resolve resources or build a competing scene model.
- **Binary Inspector** consumes supplied bytes/structure/evidence; it does not reopen game sources independently.
- **EXE Editor** is a frontend/editor over executable evidence and recovered-source identities; it is not an independent reconstruction truth store.

PAC, PNST, NBZ and AFS are resource/container layers inside the resource architecture, not top-level editors. Legacy PAC Editor/PAC Manager architecture is excluded.

## Stage identity — no `st001` architecture

The current Wave-2 executable-derived model distinguishes:

- Bank A: **110 observed descriptors**;
- Bank B: **79 observed descriptors**;
- total: **189 observed descriptors**;
- separate **193-entry selector space**;
- separate **10-pointer group-base table**;
- numeric selection through `stageId / 100` and `stageId % 100` group/selector indirection.

Three identities remain separate:

1. `resource_set_id / catalog_entry_id`;
2. `numeric_stage_id`;
3. semantic/gameplay Stage identity only when separately evidenced.

**189 descriptors are not automatically 189 gameplay stages. `st001` is only regression/compatibility data.**

## Current subsystem status

- **GDSpaces:** substantial structural/lookup/materialization/provenance work; full request-to-unload typed-postload/factory/cache/lifetime equivalence is **NOT COMPLETE**.
- **Recovered Game Source Tree:** selected units compile/test; broad original-game behavioral equivalence is **NOT COMPLETE**.
- **Reverse Core:** architecture/evidence direction active; mature generic claim/ValidationReceipt program is **NOT COMPLETE**.
- **Stage Ops:** substantial active PR #91 implementation; complete domain coverage and vanilla lifecycle/game-ready equivalence are **NOT COMPLETE**.
- **Semantic Graph / ModViz:** real projection/editor slices exist; complete target coverage/product flows are **NOT COMPLETE**.
- **HITS/collision:** strong format/spatial/writer/runtime reverse exists; whole collision/runtime/original-builder equivalence is **NOT COMPLETE**.
- **Binary Inspector:** substantial domain/analysis foundation; complete native product workflow is **NOT COMPLETE**.
- **EXE Editor / decompilation / recompilation:** selected evidence/recovered/build-lineage work exists; full decompilation and a behaviorally equivalent rebuilt executable are **NOT COMPLETE**.
- **Item/HUD/editor flows:** bounded guarded slices exist; complete production editor/export/runtime equivalence is **NOT COMPLETE**.

## HITS current boundary

The historical `HITS$`/fixed-marker model is rejected. Current product/reverse work includes header-driven `HITS`, exact `0x38` triangle/plane records, spatial reconstruction, deterministic DMC Rengine writing, source0/source1 ownership, and multiple EXE-backed runtime slices.

Later Pass-10 work in PR #85 supersedes the older statement that the whole top-level `0x14005E7A0 / 0x14005B460 / 0x14005FEC0 / 0x1400601E0` set remains unknown:

- `0x14005E7A0` is bounded-closed at the combined-query wrapper contract;
- `0x14005B460` is reclassified into the separate dynamic-world update pipeline;
- `0x14005FEC0` and `0x1400601E0` are bounded-closed at their stated top-level contracts;
- later Pass-10 slices validate common contact-normal semantics and primitive descriptor ownership.

That does **not** make HITS/collision complete. Deeper primitive producers/helpers, source2 backing/lifetime/live semantics, controlled runtime comparison, modified-topology game validation and Capcom offline-builder equivalence remain open.

## Canonical executable project evidence

Current project evidence identifies the canonical executable as:

```text
SHA-256: e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
size:    6,356,432 bytes
```

The older `3,735,552` size paired with that SHA is corrected by PR #92. A statement that a pass independently re-hashed or re-disassembled the executable still requires the raw artifact to have actually been mounted for that pass.

## Central engineering laws

> **All product resource resolution goes through GDSpaces.**

> **No claim without evidence. Agent consensus is not evidence.**

> **Bounded closure does not imply subsystem completion.**

> **Recovered C++ is evidence-backed reconstruction, not automatically original source.**

> **No original-file write without an explicit WorkingCopy/validation/export contract.**

## Build

```bash
cmake -S . -B build -DDMC_RENGINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Public tests and Evidence Packets must not contain proprietary game bytes.

## Current execution frontier

```text
artifact/evidence integrity
  -> resource lifecycle / typed-postload / factory / cache / unload closure
  -> evidence-bounded recovered-runtime behavioral receipts
  -> representative Bank-A / Bank-B / shared / alias / partial Stage validation
  -> Stage Ops complete domain/runtime bridge
  -> deeper HITS primitive/source2/runtime validation
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

A fully decompiled, behaviorally equivalent rebuilt DMC3 executable does not exist today.

## Project navigation

- [Completion & Evidence Policy](docs/status/completion-and-evidence-policy.md)
- [Current status](docs/status/current.md)
- [Machine-readable status](docs/status/canonical-status.json)
- [Phase map](docs/status/phase-map.md)
- [Blockers](docs/status/blockers.md)
- [Risk register](docs/status/risks.md)
- [Architecture](docs/architecture.md)
- [Roadmap](docs/roadmap.md)
- [Reverse-engineering rules](docs/reverse-engineering-rules.md)
- [GDSpaces contract](docs/gdspaces-contract.md)
- [Documentation index](docs/README.md)

## Legal

This repository does not contain Capcom game binaries, proprietary game assets, extracted archive payloads, leaked source code, or unauthorized distributions. Users provide legally obtained game files locally.

DMC Rengine is an independent community research/modding project and is not affiliated with or endorsed by Capcom.

> **No claim without evidence. No second resolver. No completion claim without the gate.**
