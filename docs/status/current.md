# Current Project Status

**Snapshot date:** 2026-08-02  
**Repository generation:** clean C++ foundation  
**Version:** 0.2.0  
**Overall status:** foundation green; read-only resource/stage/binary layer in progress

## Milestone result

The foundational milestone is complete.

Final validation Build #170 passed on:

- Windows — configure, compile, and all tests;
- Ubuntu — configure, compile, and all tests.

Validation PR #1 is closed as completed provenance. It was not merged because all implementation commits already exist in `main`; the branch contained only CI validation markers.

## Implemented in this repository

### Core and build

- C++20/CMake core library and CLI;
- Windows/Linux GitHub Actions matrix;
- CMake Ninja and Visual Studio presets;
- consistent compiler warnings;
- Release tests with compiler-level `/UNDEBUG` / `-UNDEBUG`;
- formatting and editor configuration;
- MIT license and changelog.

### Artifact and Evidence Registry

- internal SHA-256 implementation;
- known-vector tests;
- `ArtifactIdentity` with role, size, and SHA-256;
- confidence states: hypothesis, candidate, low, medium, high, confirmed, corrected, rejected;
- evidence locations, records, registry, supersession links;
- versioned `EvidencePacket` validation;
- deterministic JSON export;
- public evidence directory;
- canonical DMC3 HD Phase 12 executable Evidence Packet.

### Binary and EXE foundation

- bounds-checked binary `Reader`;
- little-endian integer/float reads, slices, strings, and signature probes;
- generic read-only PE32/PE32+ parser;
- machine, image base, entry point, subsystem, headers, and sections;
- checked file offset ↔ RVA and RVA → VA conversion;
- malformed/truncated/range/overlap diagnostics;
- generic `KnownExecutableTarget`;
- DMC3 profile-specific Phase 12 target registry;
- independent SHA-256 and parsed-metadata target matching;
- synthetic PE tests with no original game bytes.

### Binary Inspector domain

- `ByteRange` with overflow/range checks;
- structural `Region` and `RegionKind`;
- `OwnershipClaim`;
- `Document` over a supplied `ResourceRef`;
- deterministic region ordering;
- union coverage;
- unknown byte gaps;
- exact overlap conflicts;
- evidence/type identifiers;
- source-independent synthetic tests.

### GDSpaces foundation

- stable `ResourceId`;
- `ResourceRef` and `ResourcePayload`;
- diagnostics;
- abstract `ISource`;
- safe read-only `LocalDirectorySource`;
- root-containment/path traversal rejection;
- deterministic `SourceRegistry`;
- `ResourceGraph` and typed relations;
- `OpenRouter` with explicit context overrides;
- canonical `GameProfile` values for DMC1/2/3/Launcher/unknown;
- centralized `ResourceClassifier`;
- magic recognition for PE, PAC, SCM, DCA, HITS$, and DDS;
- path/extension fallback classification;
- post-read magic correction in `LocalDirectorySource`;
- typed `StageBundle`;
- conservative `StageBundleAssembler`;
- explicit-category precedence;
- unknown preservation and duplicate diagnostics;
- revisioned `WorkingCopy`;
- expected-byte edit guards;
- variable-length replacement, history, reset, and undo;
- immutable source bytes separated from editor state.

### Patch foundation

- `GuardedPatchPlan`;
- source SHA-256 guard;
- expected-byte guards;
- range and overlap validation;
- atomic fixed-size in-memory patching;
- no direct original-file writes.

### CLI

```text
dmc-rengine version
dmc-rengine doctor
dmc-rengine scan <directory>
dmc-rengine hash <path>
dmc-rengine route <format>
dmc-rengine inspect-exe <path>
```

The CLI reads local files through GDSpaces. `inspect-exe` prints SHA-256, PE metadata, sections, known-target recognition, and metadata consistency.

### Project process and Canon

- governance, maintainers, contribution, security, support, conduct, and clean-room policies;
- issue and pull-request templates;
- DMC Rengine Constitution;
- SDD specifications 001–008;
- ADR system;
- current status, blockers, risks, phase map, and machine-readable status;
- historical timeline, canonical decisions, deprecated architecture, migrated findings, and artifact provenance;
- public brand Canon: Order of the Inverted Triangle, Monks of Reverse, Sect of Neuroslop;
- active implementation backlog in issues #2–#5.

## CI investigation history

The validation process found and corrected real cross-platform issues:

1. An escaped-newline test expectation was incorrect.
2. MSVC Release removed side-effectful `assert(...)` expressions through `NDEBUG`.
3. A forced include was proven unreliable because a later `<cassert>` could redefine `assert`.
4. The stable solution is compiler-level `/UNDEBUG` / `-UNDEBUG` for every test target.
5. Multiple subsequent Windows/Ubuntu matrix runs passed, including final Build #170.

## Historical functionality not yet migrated

- NBZ volume source;
- AFS/PAC/PNST container expansion;
- Binary Inspector typed fields, annotations, selection, diff, and entropy;
- Stage Ops format editors;
- production Item Editor capabilities;
- ModViz desktop UI and dual-mode editor;
- EXE phases 13–16 tooling and recovered source seeds;
- MCP/Obsidian/MemPalace runtime integration.

These remain migration inputs, not current implementation claims.

## Active next milestone: 0.3 container and stage read-only slice

Critical work:

1. strict Evidence Packet JSON import — issue #2;
2. generic read-only container parser/source foundation — issue #3;
3. first EXE-backed `st001` StageBundle — issue #4;
4. Binary Inspector fields, annotations, and owner lookup — issue #5;
5. migrate additional historical findings into public Evidence Packets;
6. create synthetic PAC/PNST/container malformed-input corpus;
7. add sanitized PE/evidence JSON CLI reports;
8. define validated export manifests only after container read paths stabilize.

## Explicitly not complete

- full DMC3 executable decompilation;
- recompilable game executable;
- production NBZ/AFS/PAC/PNST parser/writer suite;
- game-backed `st001` bundle assembly;
- complete Binary Inspector;
- Stage Ops migration;
- complete ModViz/Menu Editor;
- production Item Editor migration;
- public binary releases.

## Architecture health

- Single resource API: enforced.
- Second resolver: absent from implemented flows.
- Tool-owned local file loading: absent from hash/EXE paths.
- Direct original-data writes: absent.
- Proprietary repository content: prohibited.
- Evidence model and public target packet: implemented.
- Artifact SHA-256: implemented.
- Working-copy boundary: implemented.
- Guarded patch seed: implemented.
- Profile-aware classifier: implemented at foundation level.
- StageBundle assembly model: implemented with synthetic tests.
- Binary Inspector region/ownership foundation: implemented.
- Container expansion: not implemented.

## Milestone gate

Version 0.2 foundation is green. New feature work should preserve Build #170's cross-platform baseline and must not introduce container/source resolution outside GDSpaces.
