# Current Project Status

**Snapshot date:** 2026-08-02  
**Repository generation:** clean C++ foundation  
**Version:** 0.2.0  
**Overall status:** functional foundation under final matrix validation

## Implemented in this repository

### Core and build

- C++20/CMake core library and CLI;
- Windows/Linux GitHub Actions workflow;
- CMake developer and Visual Studio presets;
- common warning configuration;
- Release test configuration that keeps assertions active;
- formatting and editor configuration.

### Artifact and evidence layer

- internal SHA-256 implementation;
- standard known-vector tests;
- `ArtifactIdentity` with role, size, and SHA-256;
- evidence confidence states;
- evidence locations and records;
- in-memory `EvidenceRegistry`;
- versioned `EvidencePacket` validation;
- deterministic JSON export;
- correction/supersession fields.

### Binary and EXE foundation

- reusable bounds-checked binary `Reader`;
- little-endian integer and float reads;
- checked slices, fixed strings, and ASCII signatures;
- generic read-only PE32/PE32+ reader;
- machine, image base, entry point, subsystem, headers, and sections;
- checked file offset ↔ RVA and RVA → VA conversions;
- malformed/truncated/overlap diagnostics;
- synthetic PE32+ fixture tests.

### GDSpaces foundation

- stable `ResourceId`;
- `ResourceRef` and `ResourcePayload`;
- diagnostics;
- abstract `ISource`;
- safe read-only `LocalDirectorySource`;
- root-containment/path traversal rejection;
- deterministic `SourceRegistry` enumeration;
- `ResourceGraph` and typed relations;
- `OpenRouter` with tool targets and context overrides;
- typed `StageBundle` with partial diagnostics;
- revisioned `WorkingCopy`;
- expected-byte edit guards;
- variable-length replacement, history, reset, and undo;
- immutable original bytes separated from editable state.

### Patch foundation

- `GuardedPatchPlan`;
- source SHA-256 guard;
- expected-byte guards;
- range and overlap validation;
- atomic fixed-size in-memory patch application;
- no direct file writes.

### CLI

- `version`;
- `doctor`;
- `scan <directory>`;
- `hash <path>`;
- `route <format>`;
- `inspect-exe <path>`.

File hashing and PE inspection acquire data through GDSpaces rather than tool-owned file loading.

### Process and repository

- MIT license;
- governance and maintainer policy;
- contribution guide;
- security, support, conduct, and clean-room policies;
- issue and pull-request templates;
- DMC Rengine Constitution;
- SDD specifications 001–005;
- ADR system;
- human and machine-readable status;
- blockers and risk register;
- full historical timeline, canonical decisions, deprecated architecture, migrated findings, and artifact provenance;
- public lore/brand Canon including the Order of the Inverted Triangle, Monks of Reverse, and Sect of Neuroslop.

## CI findings and current validation

### Validation run 1

- Ubuntu configure/build: passed;
- all code compiled;
- one Evidence JSON test assertion used the wrong escaped-newline expectation;
- test corrected.

### Validation run 2

- Ubuntu configure/build/test: passed;
- Windows configure/build: passed;
- Windows core test crashed because Release `NDEBUG` removed `assert(...)` expressions that contained setup calls;
- root cause corrected in the common CMake test helper by undefining `NDEBUG` for test targets.

### Validation run 3

- queued/in progress for the complete current foundation, including binary reader and WorkingCopy tests.

The code has already compiled on both GNU C++ and MSVC. The remaining milestone gate is one green Windows/Linux test matrix for the latest head.

## Implemented historically but not yet migrated

- read-only NBZ volume source;
- NBZ/AFS/PAC/PNST navigation and expansion;
- Binary Inspector advanced structure/ownership features;
- Stage Ops format editors;
- Item Editor practical feature set;
- EXE research phases 12–16 tooling and recovered seed units;
- ModViz UI and dual-mode product shell;
- MCP/Obsidian/MemPalace runtime integration.

These are migration inputs, not claims about the current C++ repository.

## Planned next

1. complete green matrix validation;
2. remove side-effectful `assert` usage through a permanent test helper;
3. add Evidence Packet JSON import with strict diagnostics and limits;
4. add sanitized PE JSON reports and known-target hash metadata;
5. implement a generic parser/result interface;
6. create synthetic PAC/PNST fixture generators without claiming unsupported real layouts;
7. add profile-aware resource classification;
8. build StageBundle assembly service;
9. migrate EXE-backed `st001` stage identity;
10. design Binary Inspector region/ownership domain types.

## Explicitly not complete

- full DMC3 executable decompilation;
- recompilable game executable;
- production NBZ/AFS/PAC/PNST parser/writer suite;
- complete stage reconstruction;
- complete Binary Inspector migration;
- complete ModViz/Menu Editor;
- production Item Editor migration;
- public binary release artifacts.

## Architecture health

- Single resource API: enforced by current public flow.
- Tool-owned file loading: absent from EXE/hash CLI paths.
- Direct original-data writes: absent.
- Proprietary repository content: prohibited.
- Evidence confidence and packet model: implemented.
- SHA-256 artifact identity: implemented.
- Working-copy contract: implemented at foundation level.
- Guarded Patch Engine seed: implemented.
- Bounds-checked binary reader: implemented.
- Full profile-aware classifier: not yet implemented.
- Container expansion: not yet implemented.

## Definition of the current milestone

The 0.2 milestone is complete when the latest code builds and all tests pass on Windows and Linux, status/specification documents remain synchronized, and no subsystem bypasses the defined GDSpaces contracts.
