# Changelog

All notable changes to the clean C++ generation of DMC Rengine are documented here.

The project is pre-1.0 and may change APIs rapidly. Historical research is recorded in `docs/history/`, public Evidence Packets, and Drive authority records rather than presented as completed releases.

## [Unreleased]

### Added

#### Switch dispatch and prologue recovery

- switch-table recovery behind register-indirect jumps: candidate bases from RIP-relative `lea` targets *and* non-RIP 32-bit displacements, since a compiler holding the image base in a register reaches tables through `base + disp32`; a candidate is accepted only when consecutive entries land inside the same function, and both offset readings are tried;
- 637 tables and 6,867 block addresses recovered on the canonical target, lifting walk coverage from 85.8 to **98.6 percent** of the unwind extent and cutting functions with no structural referrer from 2,101 to 848;
- unwind code decoding: per-function prologue size, stack reservation, pushed/saved register counts, frame-pointer register and handler flags, with an unrecognised operation stopping the walk rather than desynchronising it;
- decoder now reports displacement width and operand shape (`register_indirect`, `rip_relative_lea`);
- **corrections:** the code-graph and reachability evidence records are superseded with post-switch figures; `is_leaf_frame` no longer treats a function that establishes a frame pointer as a leaf.

#### Function-level reverse

- `X86LengthDecoder`: bounded, fail-closed x86-64 instruction length decoder reporting control-flow role, RIP-relative displacement and direct branch displacement, cross-validated against GNU objdump at 875,067 in-function positions with 99.93 percent agreement;
- `CodeGraphBuilder`: recursive-descent walks that fold `UNW_FLAG_CHAININFO` continuation ranges into the function that owns them, and never enter the switch jump tables MSVC embeds inside function extents;
- `FunctionMapBuilder`: joins the code graph against the import, export and RTTI tables — callers/callees, reachability, vtable slot bindings, imported symbols called (including through unwind-less thunks) and referenced literals;
- `dmc-rengine map-functions` emitting a deterministic JSON function map;
- **correction:** the canonical target's 12,235 exception-directory entries resolve to 7,389 functions plus 4,846 continuation ranges; the evidence record supersedes the earlier inventory claim;
- function-level results recorded as evidence: 593,458 instructions, 26,322 call edges, 2,412 attributed functions, 3,588 of 13,894 vtable slots bound to code.

#### Executable structural reverse

- `PeReader` extended with COFF timestamp/characteristics, DLL characteristics, alignments, checksum, code/data sizes and the data-directory array;
- `PeDirectoryReader`: imports (delay imports and ordinal-only entries included), exports with forwarder detection, the x64 exception directory as a function inventory, the debug directory with CodeView GUID/age/PDB identity, base relocations, TLS with its callback array, and the resource tree — each recovered independently so one malformed table does not cost the others;
- `RttiScanner`: MSVC type descriptors, complete-object locators, class hierarchy and base-class descriptors, and vtables, grouped by type with per-subobject vtable offsets; locators are accepted only when their self-reference matches, and MSVC decorated names are reconstructed with an explicit completeness flag;
- `dmc-rengine analyze-exe` emitting a deterministic JSON analysis report;
- synthetic PE32+ fixtures covering every directory, the RTTI hierarchy and their failure paths;
- structural reverse of the canonical DMC3 HD target recorded as evidence: build identity, 12,235 unwind-backed function ranges over 89.1 percent of `.text`, the 26-module import surface, the `dmc3_main` export, and 396 polymorphic types across 915 vtables.

#### Runtime host layer

- new responsibility boundary: platform, fixed-step frame loop and rendering device abstraction for playable targets (Specification 010);
- `IPlatform` with `HeadlessPlatform` (deterministic, display-free) and `AndroidPlatform` (lifecycle- and surface-aware);
- `FrameClock` fixed-step accumulator with a bounded catch-up budget that drops a stall instead of replaying it;
- `IRenderDevice` abstraction, implemented `NullRenderDevice` reference backend, and a `RenderBackendRegistry` in which declared-but-unimplemented backends fail closed rather than substituting `null`;
- `ResourceBridge`: the layer's only door to resource bytes, over `SourceRegistry`, with residency accounting and typed failures;
- `StageHost`: runtime mirror of a Stage Ops `StageBundle` that projects a deterministic draw list and never synthesizes geometry;
- `RuntimeApplication` loop handling surface loss/recreation, suspend/resume and focus throttling;
- separate `DMCRengine::Runtime` target on C++23 by default, selectable across C++20/23/26 with automatic step-down, keeping the core library on its C++20 baseline;
- `std::expected` and `std::move_only_function` used where the toolchain provides them, with API-identical fallbacks otherwise;
- `dmc-rengine-runtime-probe` host tool and 7 runtime test suites, green in both the standard and fallback builds;
- Android Gradle project, JNI bridge and Java shell (minSdk 26, targetSdk 36, `arm64-v8a` + `x86_64`);
- `Runtime` CI workflow covering C++20/23 on Ubuntu and Windows, a forward-looking C++26 job, and an Android APK job.

#### Core and build

- C++20/CMake core library and CLI;
- Windows/Ubuntu CI validation;
- CMake presets, warning policy, formatting, and editor configuration;
- compiler-level `/UNDEBUG` / `-UNDEBUG` test invariant;
- SHA-256 implementation and known-vector tests;
- bounds-checked binary reader;
- expanded CTest integration stack, reaching 68 validated tests per platform after Binary Inspector Cross-Port Wave 1.
- optional CLI (`DMC_RENGINE_BUILD_CLI`) and runtime (`DMC_RENGINE_BUILD_RUNTIME`) targets so the Android NDK build excludes the desktop tool and the CTest suite.

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
- annotations and Evidence links;
- owner, field, and annotation selection context;
- selected-range overlap context across regions, fields, ownership, and annotations;
- union coverage, unknown gaps, structural conflicts, and ownership conflicts;
- deterministic metadata manifests;
- format adapters including the canonical HITS model;
- deterministic offset-aligned byte diff with equal, modified, inserted, and removed spans;
- byte-diff summary counters and stable left/right ranges;
- Shannon entropy maps with configurable windows and step size;
- entropy-window zero ratio, unique-byte count, and visualization bands;
- explicit heuristic boundaries for entropy and non-resynchronizing diff behavior;
- web-to-C++20 Binary Inspector capability parity matrix and staged cross-port roadmap.

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
- Binary Inspector Web → C++20 cross-port rules, parity tracking, and Wave 1–4 plan;
- current status and machine-readable state reconciled after PR #47.

### Fixed

- corrected Evidence JSON escaped-newline test expectation;
- fixed Windows Release test crashes caused by `NDEBUG` removing side-effectful `assert` expressions;
- rejected an unreliable forced-include assertion workaround and standardized `/UNDEBUG` / `-UNDEBUG` for test targets;
- removed stale status claims that strict Evidence import, Binary Inspector fields, guarded copy execution, source integration, HITS runtime/writer work, and PC-save Pass 31/32 were still planned;
- documented that `HITS$` and `0x18060001` as a universal record marker are rejected historical assumptions;
- removed the obsolete public-lore model that treated the Sect as an AI-only inner wing and used `Monks of Reverse` as the contributor identity;
- removed the new Binary Inspector aggregate-initialization warning by fully initializing `ByteDiffResult`;
- corrected status documentation that still described Binary Inspector Diff and entropy analysis as unimplemented after Wave 1.

### Research boundaries

- production PAC/PNST/NBZ/AFS source expansion remains incomplete;
- the first game-backed `st001` StageBundle remains open;
- HITS Capcom offline-builder equivalence is not confirmed;
- Binary Inspector diff is currently offset-aligned and not structure-aware or resynchronizing;
- Binary Inspector Analysis Cache, generic diagnostics, unknown-region analysis, templates, EXE bridges, and native UI remain open;
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
