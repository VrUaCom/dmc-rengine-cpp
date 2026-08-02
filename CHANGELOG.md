# Changelog

All notable changes to the clean C++ generation of DMC Rengine will be documented here.

The project is pre-1.0 and may change APIs rapidly. Historical legacy-project work is recorded in `docs/history/` rather than presented as releases of this repository.

## [Unreleased]

### Added

#### Core and build

- cross-platform Windows/Ubuntu CI validation with final Build #170 green;
- CMake presets, warnings, formatting, and editor configuration;
- compiler-level Release test assertion invariant;
- SHA-256 implementation and known-vector tests;
- bounds-checked binary reader.

#### Evidence and Canon

- confidence model, locations, records, and `EvidenceRegistry`;
- `ArtifactIdentity` and versioned `EvidencePacket`;
- deterministic Evidence Packet JSON export;
- public evidence registry;
- canonical DMC3 HD Phase 12 target Evidence Packet;
- full historical timeline, canonical decisions, deprecated architecture, migrated findings, and artifact provenance.

#### GDSpaces

- `ResourceId`, `ResourceRef`, `ResourcePayload`, and diagnostics;
- safe read-only `LocalDirectorySource`;
- `SourceRegistry`, `ResourceGraph`, and `OpenRouter`;
- DMC1/2/3/Launcher game profiles;
- centralized path/extension/magic resource classification;
- post-read magic correction;
- typed `StageBundle` and conservative `StageBundleAssembler`;
- revisioned `WorkingCopy` with expected-byte guards, variable-size edits, history, reset, and undo.

#### EXE and patching

- generic read-only PE32/PE32+ parser;
- checked file offset, RVA, and VA conversions;
- PE section/range diagnostics;
- generic known executable target model;
- DMC3 Phase 12 known-target registry;
- CLI target recognition by SHA-256 and PE metadata;
- `GuardedPatchPlan` with source hash, expected bytes, range, overlap, and atomicity checks.

#### Binary Inspector domain

- overflow-safe `ByteRange`;
- structural regions and kinds;
- ownership claims;
- union coverage;
- unknown gaps;
- exact region conflicts;
- source-independent synthetic tests.

#### CLI

- `version`;
- `doctor`;
- `scan`;
- `hash`;
- `route`;
- `inspect-exe`.

#### Process and documentation

- MIT license;
- governance, maintainer, contribution, security, support, conduct, and clean-room policies;
- issue and pull-request templates;
- DMC Rengine Constitution;
- SDD specifications 001–008;
- ADR system;
- status, phase map, blockers, risks, JSON status, and first weekly report;
- public brand Canon, Order of the Inverted Triangle, Monks of Reverse, and Sect of Neuroslop;
- active issues #2–#5 for the 0.3 milestone.

### Fixed

- corrected Evidence JSON escaped-newline test expectation;
- fixed Windows Release test crashes caused by `NDEBUG` removing side-effectful `assert` expressions;
- rejected an unreliable forced-include assertion workaround and standardized `/UNDEBUG` / `-UNDEBUG` for test targets.

## [0.1.0] — 2026-08-02

### Added

- initial C++20/CMake foundation;
- minimal CLI;
- initial `ResourceId` model;
- cross-platform build workflow;
- initial architecture, roadmap, reverse-engineering rules, and README;
- proprietary-data exclusions.
