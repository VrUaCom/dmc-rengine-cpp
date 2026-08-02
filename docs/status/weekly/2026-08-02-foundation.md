# Weekly Foundation Report — 2026-08-02

## Executive summary

The public clean C++ generation of DMC Rengine moved from a minimal repository seed to a tested cross-platform foundation with real resource, evidence, executable, patch, stage, and Binary Inspector domain contracts.

**Milestone result:** green.

Final GitHub Actions Build #170 passed configure, compile, and all tests on Windows and Ubuntu. Validation PR #1 was closed as completed provenance.

## Delivered

### Repository and governance

- MIT license;
- governance and maintainer policies;
- contribution, security, support, conduct, and clean-room policies;
- issue and pull-request templates;
- CMake presets, formatting rules, changelog, ADRs, and documentation indexes;
- DMC Rengine Constitution and specifications 001–008.

### Canon and evidence

- full project timeline and architecture decision ledger;
- deprecated architecture record preventing PAC Editor/PAC Manager return;
- migrated findings and artifact provenance registry;
- confidence model and `EvidenceRegistry`;
- versioned `EvidencePacket`;
- deterministic JSON export;
- SHA-256 artifact identity;
- public DMC3 HD Phase 12 canonical target Evidence Packet.

### GDSpaces

- `ResourceId`, `ResourceRef`, `ResourcePayload`, diagnostics;
- source interface and safe read-only local directory source;
- path traversal/root-containment protection;
- `SourceRegistry`, `ResourceGraph`, and `OpenRouter`;
- canonical DMC1/2/3/Launcher profiles;
- centralized extension/path/magic classifier;
- post-read magic correction;
- typed `StageBundle` and conservative `StageBundleAssembler`;
- revisioned `WorkingCopy` with expected-byte guards, variable-size edits, history, reset, and undo.

### EXE and patching

- generic bounds-checked PE32/PE32+ parser;
- section and address mapping;
- file offset ↔ RVA and RVA → VA conversion;
- generic known executable target model;
- DMC3 Phase 12 target registry;
- CLI target recognition by SHA-256 plus independent PE metadata consistency check;
- atomic fixed-size `GuardedPatchPlan` with hash, expected-byte, range, and overlap guards.

### Binary Inspector foundation

- reusable bounds-checked binary reader;
- byte ranges;
- structural regions;
- ownership claims;
- union coverage;
- unknown gaps;
- exact overlap conflicts;
- evidence/type IDs;
- source-independent tests.

### CLI

```text
dmc-rengine version
dmc-rengine doctor
dmc-rengine scan <directory>
dmc-rengine hash <path>
dmc-rengine route <format>
dmc-rengine inspect-exe <path>
```

All file acquisition for hashing and PE inspection flows through GDSpaces.

## CI investigation and corrections

The validation process exposed and resolved two real classes of test failure:

1. Evidence JSON escaped-newline expectation was incorrect.
2. MSVC Release defined `NDEBUG`, removing side-effectful `assert(...)` expressions and causing a core-test crash.

A forced include was tested and rejected because a later `<cassert>` could redefine `assert`. The stable solution is compiler-level `/UNDEBUG` / `-UNDEBUG` for all test targets.

Subsequent matrix runs passed, including final Build #170.

## Architecture assessment

### Healthy

- GDSpaces is the only implemented resource-access path.
- EXE/hash CLI does not reopen files inside tool-specific modules.
- Source bytes remain immutable.
- Working copies and guarded patches are separate explicit concepts.
- Unknown resources and stage members remain visible.
- Binary Inspector domain owns structure/ownership, not source resolution.
- Game/version-specific target metadata is outside the generic PE parser.
- No proprietary game files are present.

### Still missing

- strict Evidence JSON import;
- generic container source/parser contracts;
- NBZ/AFS/PAC/PNST child exposure;
- game-backed `st001` StageBundle;
- Binary Inspector fields/annotations/owner lookup;
- validated export manifests;
- Stage Ops, ModViz, and Item Editor migration;
- recompilation pipeline.

## Active backlog

- issue #2 — strict Evidence Packet JSON import;
- issue #3 — read-only container parser/source foundation;
- issue #4 — first EXE-backed `st001` StageBundle;
- issue #5 — Binary Inspector fields, annotations, and owner lookup.

## Proposed work for the next development week

### Priority 1 — Container foundation

Implement generic container entry/document/result contracts and original synthetic fixtures. Do not begin production PAC writing or revive PAC Manager.

### Priority 2 — Evidence import

Add strict, bounded JSON parsing and CLI validation for public Evidence Packets.

### Priority 3 — `st001` vertical slice

Represent the confirmed four-column stage table and connect user-supplied sources to one deterministic typed bundle.

### Priority 4 — Binary Inspector depth

Add typed fields, nested structures, annotations, and owner lookup on top of the green document model.

## Main risk

The repository grew rapidly but remains coherent because each new module was validated through Windows/Ubuntu CI. The next risk is schema guessing in container work. Real PAC/PNST/NBZ/AFS behavior must be added only through narrow evidence-backed subsets and synthetic malformed-input tests.

## Final status

```text
Foundation 0.2: GREEN
Next milestone: 0.3 container and stage read-only slice
Old PAC Editor/PAC Manager architecture: excluded
Original game data writes: absent
Full decompilation/recompilation: not complete
```
