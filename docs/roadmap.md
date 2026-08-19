# Roadmap

**Snapshot date:** 2026-08-08

The roadmap prioritizes evidence-backed vertical completion over breadth. Legacy PAC Editor/PAC Manager logic is excluded; container formats remain internal to GDSpaces.

## Foundation — maintained

- C++20 and CMake baseline;
- Windows and Ubuntu CI;
- artifact SHA-256 identity;
- Evidence Packets and confidence/correction vocabulary;
- no proprietary game data in the repository;
- working-copy and guarded-patch safety contracts.

## Milestone A — GDSpaces production identity and read path

- production read-only PAC/PNST subset;
- NBZ/AFS exposure through GDSpaces;
- nested child classification and diagnostics;
- `.index` linkage only as evidence-backed metadata;
- [Resource Identity v1](gdspaces/resource-identity-v1.md);
- deterministic local integration reports.

**Exit gate:** a legal local stage resource set resolves through one canonical resource path with no tool-local resolver.

## Milestone B — Game-backed Stage Workspace

- resolve `st001` from legal local data;
- assemble one complete/diagnostic `StageBundle`;
- preserve unknown and partial resources;
- prove Stage Ops, ModViz, Binary Inspector, and EXE links reuse canonical `ResourceId` values;
- begin [Stage Semantic Graph v1](stage/stage-semantic-graph.md).

**Exit gate:** Stage Ops consumes one game-backed typed bundle without opening loose paths independently.

## Milestone C — Binary Inspector Wave 2 and Reverse Core bridge

- artifact-identity-keyed Analysis Cache;
- generic offset/order/overlap/alignment/stride diagnostics;
- unknown-region feature analysis;
- versioned binary templates;
- deterministic analysis-result JSON;
- bridge regions/fields/ownership/annotations to durable Reverse Core objects.

**Exit gate:** an inspected binary range can be promoted into a stable reverse object without copying identity/evidence into a second database.

## Milestone D — Reverse Core v0.1

- canonical objects: BinaryArtifact, AddressRange, Function, DataObject, RecoveredType, EvidenceRecord, Hypothesis, Experiment, TaskClaim, Reconstruction, ValidationReceipt, Subsystem;
- ownership/claim protocol over the existing MCP/Kanban workflow;
- recovered-source tree contract exportable to VS Code/CMake;
- EXE Editor and Binary Inspector bridges;
- deterministic provenance manifests.

See [Reverse Core](reverse-core/README.md).

**Exit gate:** the schema and claim/source contracts support one real subsystem without DMC-specific concepts leaking into the reusable layer.

## Milestone E — First behavior-tested recovered subsystem

Follow the [EXE Reconstruction Pipeline](exe/reconstruction-pipeline.md):

1. select one bounded subsystem with strong evidence;
2. bind exact executable artifact/range identities;
3. recover functions/data/types and ABI hypotheses;
4. compile as an isolated reviewed C++ target;
5. behaviorally compare against the canonical executable/boundary;
6. record a ValidationReceipt;
7. promote, correct, or reject findings explicitly.

**Exit gate:** first complete `binary -> recovered C++ -> build -> behavioral validation` loop.

This gate precedes mass decompilation.

## Milestone F — Stage semantic integration

- Stage -> Room -> Geometry -> Collision -> Lighting -> Camera -> Door/Transition -> Event -> Effect -> Audio -> Runtime links;
- HITS real-corpus comparison and controlled runtime validation;
- broader CAM/effect/model/collision relationships;
- deterministic semantic graph manifests.

**Exit gate:** one stage slice can be reasoned about across formats and runtime references without format-local identity silos.

## Milestone G — ModViz Menu Editor vertical slice

Use the [Red Orb counter](modviz/menu-editor.md) as the first complete Menu Editor proof:

- GDSpaces-backed HUD resource;
- hierarchy/mesh/digit/UV editing;
- runtime-value preview;
- linked EXE formatting/limit evidence;
- guarded patch request where required;
- validated working-copy/export path.

**Exit gate:** first visual HUD edit travels through existing identity, evidence, working-copy, validation, and export contracts without a second resolver.

## Milestone H — Controlled source integration and recompilation

- additional behavior-tested recovered modules;
- dependency and replacement/rebinding boundaries;
- deterministic composite builds;
- source-to-output address mappings;
- runtime validation and regression receipts;
- first working rebuilt executable milestone when evidence supports the claim.

## Parallel research promotion

Continue narrow promotion of Drive research such as Wide Pass 33 and selected recovered-source units through:

`immutable artifacts -> Evidence Packet -> reviewed C++ -> tests -> CI -> provenance receipt`.

Do not bulk-import recovered source snapshots.

## Deferred behind evidence gates

- complete desktop UI breadth;
- production container write/repack suite;
- signed public binary releases;
- broad automatic decompilation at scale.
