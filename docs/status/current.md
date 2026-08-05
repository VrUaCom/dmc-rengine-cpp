# Current Project Status

**Snapshot date:** 2026-08-05  
**Repository generation:** evidence-backed C++ platform  
**Version:** 0.2.0  
**Snapshot base:** `main` at `1f77e2076a79216e015a3ddc83b1d1ed89c121c8`  
**Overall status:** cross-platform foundation green; integrated domain stack active; production archive sources, game-backed stage assembly, runtime validation, and recompilation remain open

## Executive result

DMC Rengine C++ is no longer a placeholder scaffold. The repository contains a functioning C++20 platform with a core library, CLI, cross-platform tests, Evidence Packets, GDSpaces contracts, Binary Inspector models, PE/EXE inspection, stage and item integration, guarded modification workflows, source-integration provenance, a substantial HITS vertical slice, and the reviewed DMC3 PC-save Pass 31/32 ABI.

GitHub `main` is the implementation truth for reviewed code. Google Drive remains the research truth for newer reverse-engineering results and recovered-source snapshots. Wide Pass 33 exists in Drive research but has not yet been promoted into the reviewed product tree.

## Validation baseline

The latest reviewed promotion is PR #42, merged on 2026-08-04.

Its recorded gates passed on both platforms:

- Ubuntu — 67/67 tests;
- Windows — 67/67 tests;
- strict Pass 32 Evidence Packet validation;
- reverse-authority manifest validation;
- final immutable review CI;
- Drive implementation-receipt readback.

All future changes must preserve Windows and Ubuntu builds and the rule that no tool creates a second resource resolver outside GDSpaces.

## Implemented in this repository

### Core and build

- C++20/CMake core library and CLI;
- Windows/Ubuntu GitHub Actions validation;
- Ninja and Visual Studio presets;
- `/W4 /permissive-` on MSVC;
- `-Wall -Wextra -Wpedantic -Wconversion` on GCC/Clang;
- compiler-level `/UNDEBUG` / `-UNDEBUG` for test targets;
- synthetic tests containing no proprietary game bytes.

### Artifact identity and Evidence

- internal SHA-256 and known-vector tests;
- `ArtifactIdentity` with role, size, and SHA-256;
- confidence states: hypothesis, candidate, low, medium, high, confirmed, corrected, rejected;
- evidence locations, records, registry, tags, and supersession links;
- versioned `EvidencePacket` validation;
- deterministic JSON export;
- strict untrusted JSON import with parser limits and cross-reference validation;
- CLI `validate-evidence`;
- public Evidence Packets for the canonical executable, Item runtime, and PC-save Pass 31/32 findings.

### GDSpaces and integration foundation

- stable `ResourceId`, `ResourceRef`, and `ResourcePayload`;
- safe read-only `LocalDirectorySource` with root-containment guards;
- deterministic `SourceRegistry`;
- profile-aware `ResourceClassifier` and post-read magic correction;
- `ResourceGraph`, `OpenRouter`, and canonical tool/format registries;
- typed `StageBundle` and `StageBundleAssembler`;
- generic read-only container contracts, parser registry, synthetic slot-container fixtures, child identity, empty-slot preservation, diagnostics, and parent-child graph edges;
- revisioned `WorkingCopy` with expected-byte guards, variable-length replacement, history, reset, and undo;
- shared Project Workspace, Project Graph, workspace events, and deterministic manifests.

The generic container foundation is implemented. Production PAC/PNST/NBZ/AFS source expansion remains incomplete.

### Binary Inspector domain

- overflow-safe `ByteRange`;
- structural regions and region kinds;
- typed fields and parent-child structures;
- ownership claims;
- annotations and evidence links;
- selection lookup for field, owner, and annotation context;
- union coverage, unknown gaps, structural conflicts, and ownership conflicts;
- deterministic metadata manifest export;
- adapters for integrated formats including HITS.

The desktop hex/visual UI, entropy UI, and full diff interaction remain future product work.

### PE, EXE, and address evidence

- bounds-checked binary reader;
- generic read-only PE32/PE32+ parser;
- sections, machine, image base, entry point, subsystem, and malformed-input diagnostics;
- checked file offset ↔ RVA ↔ VA conversion;
- known executable target model and canonical DMC3 Phase 12 target;
- SHA-256 plus PE-metadata matching;
- Evidence Address Resolver;
- executable workspace manifests;
- EXE reopen lineage by executable SHA-256;
- source-unit, source-line, and recovered-symbol mappings to output offsets/RVAs/VAs in the Custom Build model.

Full DMC3 decompilation and a behaviorally equivalent rebuilt executable are not complete.

### Stage and format integration

Compiled modules and tests exist for:

- HITS;
- DCA;
- LIG2;
- Stage TXT;
- resource analysis;
- the DMC3 stage-table descriptor;
- stage-resource matching;
- `StageBundleAssembler`;
- `DMC3StageWorkspaceBuilder`;
- Stage Workspace manifests;
- shared Stage Ops/ModViz views.

The confirmed 110 × 4 stage-table descriptor and `st001` resource plan are implemented. The complete legal local game-backed `st001` StageBundle remains open in issue #4.

### Item, Trial Chamber, and guarded modification

- Item Workspace and item-runtime Evidence Packet;
- runtime requests, validation plans, requirements, graph nodes, events, and manifests;
- evidence-gated patch-plan compilation;
- `GuardedPatchPlan` with exact source hash, expected bytes, ranges, and overlap checks;
- in-memory copy execution only;
- output SHA-256;
- generated rollback plans verified to restore the original bytes exactly;
- manifests recording `original_file_write_performed=false`.

The Item layer produces guarded requests; it does not patch the executable directly.

### Source integration and custom-build lineage

- `SourceModificationPackage`;
- `IntegrationProject` state and dependency/conflict graph;
- deterministic source-integration manifests;
- `CustomBuildIdentity` and `CustomBuildRecord`;
- compiler, linker, target, flags, dependency-lock, and recovered-source identity;
- source-to-binary mappings;
- mandatory test and release gates;
- distribution, attestation, revocation, and rollback identity;
- EXE Editor reopen context using executable SHA-256.

These models define the future composite source-build contract. They do not claim that a working rebuilt DMC3 executable exists today.

### HITS vertical slice

The active HITS implementation supersedes the rejected historical `HITS$`/record-marker model.

Implemented:

- header-driven `HITS` parser;
- exact `0x38` triangle-plane records;
- spatial grid and signed `-1`-terminated reference lists;
- source 0/member 3 and source 1/member 6 identity;
- Binary Inspector semantic adapter;
- EXE-backed grid conversion, flattening, broadphase, reference deduplication, and reject-mask behavior;
- typed candidate/contact contracts;
- topology-preserving safe edits;
- normal and plane-D recomputation;
- deterministic DMC Rengine spatial writer using triangle-vs-cell SAT;
- canonical serialization and parser round trips;
- stable surface identities;
- spatial corpus differential validator;
- deterministic per-cell/per-surface reports and precision/recall/Jaccard metrics;
- GDSpaces-backed `compare-hits-spatial` CLI with SHA-256 identities and unique bit-exact mapping across reorder.

The DMC Rengine SAT writer is structurally tested. Equivalence with Capcom's unknown offline builder is not confirmed and requires real corpus plus game-runtime validation.

### DMC3 PC-save ABI

Pass 31 and Pass 32 are promoted into reviewed C++.

Implemented and evidence-gated:

- file size `0x4A30` / 18,992 bytes;
- 21 integrity envelopes;
- one `0x138` global record (`0x134` body + four-byte trailer);
- ten `0x40` summary records (`0x3C` body + trailer);
- ten `0x70C` payload records (`0x708` body + trailer);
- trailer ABI `u16 recordState + u16 checksum`;
- rejection of the former standalone `0x28` block interpretation;
- compatibility-tag observation at `+0x108 = 0xDEC0`;
- one's-complement end-around-carry checksum and valid fold `0xFFFF`;
- packed-BCD date/time handling;
- conservative raw boundaries for unresolved semantics;
- compile-time layout assertions, diagnostics, and regression tests;
- Pass 32 Evidence Packet and Drive/GitHub provenance receipts.

Wide Pass 33 payload semantics are research-ready in Drive but product-promotion-pending.

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

## Active GitHub work

- issue #3 — advance the generic container foundation into a production read-only PAC/PNST/NBZ/AFS subset;
- issue #4 — complete the first game-backed `st001` StageBundle;
- issue #13 — reconcile into real HITS corpus and runtime validation; its obsolete `HITS$` assumptions must not be reused.

There are no open pull requests at this snapshot.

## Explicitly not complete

- production PAC/PNST/NBZ/AFS read/write suite;
- game-backed `st001` end-to-end assembly;
- proof that the HITS writer matches Capcom's offline builder;
- complete Stage Ops or ModViz desktop UI;
- complete production Item Editor UI/export flow;
- bulk promotion of the recovered-source skeleton;
- full DMC3 executable decompilation;
- a recompilable, behaviorally validated DMC3 executable;
- public signed binary release pipeline.

## Current critical path

```text
production read-only container subset
  → legal local game-backed st001 StageBundle
  → broader stage/runtime corpus validation
  → narrow Evidence Packet promotions from Drive research
  → behavior-tested recovered subsystems
  → controlled recompilation milestones
```

Parallel HITS work should prioritize real corpus comparison and controlled game-runtime validation rather than stronger compatibility claims.

## Milestone gate

The 0.2 C++ foundation and integrated domain stack are green. The next milestone should preserve the existing natural architecture, remove documentation drift, and complete demonstrable game-backed vertical slices without bulk-importing unreviewed recovered source.