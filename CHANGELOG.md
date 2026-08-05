# Changelog

All notable changes to the clean C++ generation of DMC Rengine are documented here.

The project is pre-1.0 and may change APIs rapidly. Historical research is recorded in `docs/history/`, public Evidence Packets, and Drive authority records rather than presented as completed releases.

## [Unreleased]

### Added

#### Core and build

- C++20/CMake core library and CLI;
- Windows/Ubuntu CI validation;
- CMake presets, warning policy, formatting, and editor configuration;
- compiler-level `/UNDEBUG` / `-UNDEBUG` test invariant;
- SHA-256 implementation and known-vector tests;
- bounds-checked binary reader;
- expanded CTest integration stack, reaching 67 validated tests per platform for the Pass 32 promotion.

#### Evidence and Canon

- confidence model, locations, records, tags, supersession, and `EvidenceRegistry`;
- `ArtifactIdentity` and versioned `EvidencePacket`;
- deterministic Evidence Packet JSON export;
- strict untrusted Evidence Packet JSON import;
- parser size/depth/count limits, duplicate-key and duplicate-ID rejection, and cross-reference validation;
- CLI `validate-evidence`;
- public packets for the canonical DMC3 executable, Item runtime, and PC-save Pass 31/32 findings;
- Drive/GitHub reverse-authority registry and implementation receipts.

#### GDSpaces and integration

- `ResourceId`, `ResourceRef`, `ResourcePayload`, and diagnostics;
- safe read-only `LocalDirectorySource` with root-containment protection;
- `SourceRegistry`, `ResourceGraph`, `OpenRouter`, and game profiles;
- centralized path/extension/magic classification and post-read correction;
- typed `StageBundle` and deterministic `StageBundleAssembler`;
- generic read-only container contracts, parser registry, synthetic slot-container fixtures, stable child identity, empty-slot preservation, diagnostics, and graph edges;
- revisioned `WorkingCopy` with expected-byte edits, variable-size replacement, history, reset, and undo;
- Project Workspace, Project Graph, append-only workspace events, and deterministic manifests;
- canonical tool and format capability registries;
- shared Stage Ops/ModViz stage views.

#### Binary Inspector domain

- overflow-safe `ByteRange`;
- structural regions and kinds;
- typed fields and parent-child structures;
- ownership claims;
- annotations and evidence links;
- owner, field, and annotation selection context;
- union coverage, unknown gaps, structural conflicts, and ownership conflicts;
- deterministic metadata manifests;
- format adapters including the canonical HITS model.

#### EXE and patching

- generic read-only PE32/PE32+ parser;
- checked file offset, RVA, and VA conversions;
- PE section/range diagnostics;
- known executable target model and DMC3 Phase 12 registry;
- target recognition by SHA-256 and PE metadata;
- Evidence Address Resolver and executable workspace manifests;
- `GuardedPatchPlan` with source hash, expected bytes, ranges, overlap, and atomicity checks;
- evidence-gated patch-plan compilation;
- copied-output in-memory patch execution;
- output SHA-256 and verified rollback plans;
- manifests proving that the original file was not written.

#### Stage and formats

- DMC3 110 × 4 stage-table descriptor;
- `st001` role plan, path normalization, and resource matching;
- `DMC3StageWorkspaceBuilder` and Stage Workspace manifests;
- compiled modules and tests for HITS, DCA, LIG2, and Stage TXT;
- Resource Analyzer integration.

#### Item and Trial Chamber

- Item Workspace and Item runtime Evidence Packet;
- runtime requests, graph nodes, events, and manifests;
- validation plans and requirements;
- evidence-gated Item runtime patch compilation;
- guarded copied-output and rollback provenance.

#### Source integration and custom builds

- `SourceModificationPackage`;
- `IntegrationProject` state and dependency/conflict graph;
- deterministic source-integration manifests;
- `CustomBuildIdentity` and `CustomBuildRecord`;
- compiler, linker, target, flags, dependency-lock, and recovered-source identity;
- source-unit/source-line/recovered-symbol mappings to output offsets, RVAs, and VAs;
- test, release, attestation, revocation, and rollback gates;
- EXE reopen lineage by executable SHA-256.

#### HITS

- correction of the obsolete `HITS$`/fixed-marker model;
- header-driven `HITS` parser;
- exact `0x38` triangle-plane records;
- spatial grid and signed `-1`-terminated reference lists;
- source 0/member 3 and source 1/member 6 identity;
- Binary Inspector semantic adapter;
- runtime-derived grid conversion, flattening, broadphase, deduplication, and reject-mask behavior;
- candidate and contact result contracts;
- topology-preserving safe editing;
- normal and plane-D recomputation;
- deterministic DMC Rengine SAT spatial writer;
- canonical parser/writer round trips and stable surface identity;
- spatial corpus differential validator;
- deterministic per-cell/per-surface JSON reports and precision/recall/Jaccard metrics;
- GDSpaces-backed `compare-hits-spatial` CLI with SHA-256 identities and unique bit-exact geometry mapping across reorder.

#### DMC3 PC save

- exact `0x4A30` file model;
- 21 integrity envelopes;
- global, summary, and detailed-payload record layouts;
- four-byte `recordState + checksum` trailers;
- one's-complement end-around-carry checksum validation and generation;
- packed-BCD date/time handling;
- rejection of the former standalone `0x28` block interpretation;
- conservative open-semantic boundaries;
- Pass 31 and Pass 32 Evidence Packets, tests, CI, and Drive receipts.

#### CLI

- `version`;
- `doctor`;
- `scan`;
- `hash`;
- `validate-evidence`;
- `route`;
- `inspect-exe`;
- `list-tools`;
- `list-formats`;
- `integration-status`;
- `inspect-workspace`;
- `compare-hits-spatial`.

#### Process and documentation

- MIT license;
- governance, maintainer, contribution, security, support, conduct, and clean-room policies;
- issue and pull-request templates;
- DMC Rengine Constitution;
- SDD specifications and ADR system;
- architecture, phase map, blockers, risks, JSON status, history, and Canon documents;
- public brand Canon defining the Sect of Neuroslop as the DMC Rengine community, the Monks of Binary Code as creators and recognized core contributors, and the Order of the Inverted Triangle as the core Team alias;
- public Long Descent, Monastery, chamber, ritual, campaign, and evidence-presentation vocabulary;
- GitHub implementation truth separated from newer Drive research truth;
- current documentation reconciled to `main` after merged PR #42.

### Fixed

- corrected Evidence JSON escaped-newline test expectation;
- fixed Windows Release test crashes caused by `NDEBUG` removing side-effectful `assert` expressions;
- rejected an unreliable forced-include assertion workaround and standardized `/UNDEBUG` / `-UNDEBUG` for test targets;
- removed stale status claims that strict Evidence import, Binary Inspector fields, guarded copy execution, source integration, HITS runtime/writer work, and PC-save Pass 31/32 were still planned;
- documented that `HITS$` and `0x18060001` as a universal record marker are rejected historical assumptions;
- removed the obsolete public-lore model that treated the Sect as an AI-only inner wing and used `Monks of Reverse` as the contributor identity.

### Research boundaries

- production PAC/PNST/NBZ/AFS source expansion remains incomplete;
- the first game-backed `st001` StageBundle remains open;
- HITS Capcom offline-builder equivalence is not confirmed;
- Wide Pass 33 remains research-ready and product-promotion-pending;
- full DMC3 decompilation and a working rebuilt executable are not complete.

## [0.1.0] — 2026-08-02

### Added

- initial C++20/CMake foundation;
- minimal CLI;
- initial `ResourceId` model;
- cross-platform build workflow;
- initial architecture, roadmap, reverse-engineering rules, and README;
- proprietary-data exclusions.
