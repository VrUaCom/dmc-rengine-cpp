# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Primary execution program:** L1 final same-lineage acceptance  
**Merged supporting authority:** #233 connected-artifact access, #235 L2 R2B v2 + mount-topology correction, #230/#242 materialization-completion boundary  
**Pending branch truth:** #226 RCP/grey boundary, #238 Pocket reconciliation, #240 L3 R1 final review, #241 successful-mount topology implementation

This is the dependency-driven execution roadmap for GDSpaces as one resource-runtime program. L1/L2/L3 remain separate ownership layers. Supporting work from another layer is allowed only when it discharges a concrete dependency of the current vertical acceptance path or closes a bounded layer gate.

## 1. Canonical layer ownership

### L1 — Resource Materialization

```text
physical/container bytes
 -> exact acquisition
 -> transform/decompression
 -> materialized bytes + provenance
 -> nested PAC/PNST expansion
 -> editable child identity
 -> bounded edit
 -> bottom-up rebuild/repack
 -> reopen/rematerialize
```

L1 owns exact product byte materialization, representation-preserving authoring and authored-byte rematerialization.

### L2 — Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> discovery/attempt topology
 -> actual successful mounted sources
 -> provider/source/volume selection
 -> fallback/ambiguity/failure semantics
 -> exact selected ResourceRef identity
```

L2 owns which source/member is selected. Discovery is not successful mounting.

### L3 — Original Runtime / Lifecycle

```text
selected/materialized bytes
 -> FileSlot/AsyncIO request lifetime
 -> LoadedResource state progression
 -> typed post-load / construction
 -> manager-ready visibility
 -> claims/cache/ownership
 -> cancellation/reset/release/shutdown
 -> consumer handoff/effect
```

L3 owns original runtime request/materialization scheduling, publication, typed-ready and lifecycle/consumer semantics.

**Stage Assembly / Stage Ops is downstream and is not L3.**

## 2. Orthogonal planes — not L4

Validation and control remain cross-cutting:

- **V/LV** — validation, live observation and promotion authority;
- **RCP** — Resource Control Plane for root/dependency planning, pending/ready coordination, claims and transition orchestration;
- **TYPE/ID** — descriptor/type and cross-layer identity mapping;
- **RT-IO** — FileSlot/AsyncIO substrate at the L1/L3 seam;
- **MEM/BACKING** — allocation/backing substrate;
- **BOOTSTRAP** — startup/resource-service substrate;
- **ERROR** — per-owner failure/recovery matrix.

Draft #226/#223 work must not create a fourth execution layer or move L1/L2/L3 ownership.

Useful L3 accounting labels:

- **L3A — Typed Construction / Dependency**;
- **L3B — Ownership / Lifecycle**.

These are subdomains only.

## 3. Global evidence rules

1. GitHub `main` is implementation truth; open PRs are branch truth until merged.
2. Reverse claims are valid only for their exact artifact/range/scope.
3. Synthetic/public CI proves bounded product/tool behavior, not original-game equivalence.
4. A Pocket member receipt proves the exact local archive snapshot/member/materialization encoded in it, not the original protected resolver winner or consumer.
5. Canonical analysis EXE and protected distribution/original-execution candidate are separate authorities.
6. Independent PASS receipts cannot be joined by filename alone; final acceptance requires one reconciled selected-source/request lineage.
7. Documentation synchronization never creates a completion claim.

## 4. Current vertical acceptance target

```text
real selected request/source lineage
 -> [L2] actual successful selected provider/volume/member identity
 -> [L1] artifact-bound exact member materialization + ByteProvenance
 -> [L1] explicit representation classification
 -> [L1] supported top-level or nested edit/rebuild
 -> [L1/L2] next-volume publication + actual selected authored winner
 -> [L1] exact authored rematerialization
 -> [L3] original lifecycle/materialization terminal condition
 -> [L3] manager-ready / typed-ready state where applicable
 -> deterministic consumer-visible effect attributable to authored bytes
 -> rollback / retail immutability
 -> [V] final cross-stack verdict
```

A crash-free launch is insufficient. `state3` alone is not a universal semantic-success receipt.

## 5. Track A — L1 final acceptance

**Internal product implementation:** CLOSED for the current representative DMC3-HD acceptance scope.

Canonical capabilities include:

- NBZ classic ZIP indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA + ByteProvenance;
- artifact-bound member observation;
- atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + recursive expansion;
- size-changing relative-slot reflow;
- nested root-to-leaf slot-path authoring;
- exact untouched-sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-volume NBZ overlay authoring;
- canonical reopen/rematerialization;
- protected executable preflight;
- direct-retail acquisition and product closure orchestration.

### L1 real-evidence sequence

```text
L1-C exact real selected/member receipt
 -> L1-D representation classification
 -> L1-E one supported real edit/rebuild
 -> L1-F next-volume reopen/rematerialization
 -> L1-G #209 original DMC3 consumption + rollback
 -> L1-H final cross-stack/V audit
 -> L1 COMPLETE / 100%
```

No new synthetic-only feature may displace this sequence unless the real run exposes a concrete missing implementation dependency.

### Connected access boundary

Merged #233 establishes:

- protected `dmc3.exe` is locatable;
- executable-relative `data/dmc3/dmc3-0.nbz` is locatable;
- observed NBZ size = `960,358,951` bytes;
- connected raw materialization ceiling = `268,435,456` bytes.

This is a transport/access limitation, not archive absence and not an L1 parser failure.

### Pocket evidence bridge

Pocket GDS can materialize an exact member where the full NBZ is already local on-device and emit `gdspaces.l1.member-acquisition-receipt.v1` without transferring proprietary bytes through the connected environment.

Pending DMC Rengine PR #238 reconciles that receipt into L1 evidence. It may close the exact member-byte/materialization sub-gate and representation-classification evidence for that local snapshot, but cannot prove:

- protected original-process selected provider;
- actual successful original mount topology;
- protected executable authority;
- original-game consumption;
- rollback.

Final promotion must bind the member/archive identity to the accepted selected-source lineage without filename laundering.

## 6. Track B — L2 closure

Merged internal/tooling authority:

- #215/#204 — type-0 physical-provider post-`0x0C` static reverse + controlled product model;
- #219 — legacy bounded explicit-PID runtime mapping tooling;
- #221 — selected-provider content-candidate normalizer/validator/artifact binder;
- #235 — process-instance-bound R2B v2 tooling and discovery-vs-successful-mount correction.

### L2-R2A — real-retail `0x0E` collision census

The recovered archive index uses normalized-string-only qsort/bsearch comparison with no equal-key secondary tie-break. Therefore uniqueness-dependent selection claims require an exact archive-SHA-bound retail member surface and a zero/non-zero collision census.

A single-member Pocket receipt cannot close this gate. A complete cryptographically bound central-directory/member-name derivative could.

### L2-R2B v2 — protected runtime mapping

Promotable evidence now requires one exact protected process instance and:

- exact PID;
- non-zero OS-derived process creation FILETIME from the same process handle;
- module base/path/image identity;
- canonical EXE artifact independently read by the validator;
- seven mandatory canonical `0x40` anchor windows:
  - `0x2FCA0` OpenGameResource;
  - `0x326D20` physical registration;
  - `0x326DA0` archive registration;
  - `0x327430` ResourceMountResolve;
  - `0x327800` physical-open anchor;
  - `0x328160` archive normalized lookup;
  - `0x328290` archive wrapper/open.

Legacy v1 receipts are historical and are not R2B v2 promotion authority. The real seven-anchor protected-process packet remains open.

### L2 topology correction — discovery != successful mounts

Merged #235 proves:

```text
VolumeBootstrapPlan = numbered filename discovery / registration attempts
RuntimeMountTopology = actually successful mounted sources
```

The first missing filename bounds discovery only. An existing numbered archive can fail registration while discovery continues, so successful mounts can be sparse.

Successful archive registrations prepend, therefore precedence among successful mounts remains:

```text
higher successful volume -> lower successful volume -> physical
```

Issue #237 tracks the correction. Pending PR #241 implements this separation in product topology. Until merge it remains branch truth and cannot be described as current-main code.

### L2-R3 — trusted selected-provider identity

Merged #221 defines a strict content candidate and binder, not trusted original-process evidence.

Promotion requires:

1. real R2B v2 mapping from the same protected process instance;
2. trusted runtime publisher/observer origin;
3. zero-loss trace (`trace_complete=true`, `dropped_event_count=0`);
4. exact observer artifact binding;
5. exact actually-successful mounted archive SHA/size binding;
6. exact selected provider/archive/member identity;
7. fail-closed handling for provider/backend failure.

Fresh EXE authority preserves that an archive normalized lookup hit followed by wrapper/open failure at `0x140328290` is terminal null/cleanup, not a clean lower-volume miss.

### L2 final sequence

```text
exact retail collision census
 + merge/reconcile #241
 + real protected R2B v2 packet
 -> trusted zero-loss R3 selected identity
 -> exact successful-mounted archive/member binding
 -> exact-head Windows + Ubuntu validation
 -> final L2 audit
 -> L2 COMPLETE only if all gates agree
```

## 7. Track C — L3 closure

Current static spine is advanced but Layer 3 is not complete.

### Materialization completion dependency — merged #230/#242

Canonical normal completion ABI:

```text
0x1401B84E0
 -> registers 0x1401B8DC0
 -> one u32 context = record_ptr - 0x140C99D30
 -> valid contexts = index * 0x48 for 363 records
```

Normal `0x1401B8DC0` receives no raw transport status pointer, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata.

Therefore lower transport/materialization success/failure must already be terminal before normal state2 publication, or the queued completion must be suppressed/removed.

**FIFO insertion order alone is not a proven dependency barrier.** No generic fan-in/outstanding-child counter is evidenced.

Exact-byte priority:

1. `0x1402EF4D0` queued materialization job identity/type/callees/context consumer;
2. relevant materialization case in `0x1402EF790`, including persistence/re-poll/terminal retirement;
3. reacquire historical `0x1400333E0` status/poll hypothesis;
4. reacquire historical `0x140033390` terminal cleanup/release hypothesis;
5. `0x1400335A0` lower transport terminal writes;
6. identify the mechanism blocking/suppressing normal `0x1401B8DC0` on incomplete/failure;
7. `0x1402EF460` pending scheduler clear/rollback and queued-completion suppression;
8. only then apply the proven mechanism to `.lst` child/recursive failure ordering.

Layer ownership:

- exact FileSlot byte-read mechanics may support L1;
- FileSlot/AsyncIO request ownership/scheduling/callback lifetime/cancellation = L3;
- `0x1401B8CA0` = explicit L1/L3 materialization-success seam;
- LoadedResource states = L3.

### L3 R1 — state writer census

Pending PR #240 proposes the exact canonical `LoadedResource +0x04` writer census as `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`.

Until merge this is branch truth. If promoted, it closes only R1 for the exact canonical artifact/scope. It does not complete L3.

Remaining after R1 include R2-R5 and V1-V7, including field/backing ownership, family dependency/factory edges, lifecycle breadth and original-process consumer receipts.

### L3 vertical minimum for L1

L1 does not require full L3 closure before its representative acceptance. It does require enough original-process evidence to prove that the exact authored L1 byte identity reached the intended consumer without confusing:

```text
manager_ready_state3
!= family_semantic_ready
!= consumer_effect_observed
```

## 8. Track D — RCP / EXE grey boundary

Draft PR #226 models an orthogonal Resource Control Plane and explicitly preserves the three-layer model.

RCP work is permitted when it:

- closes request-origin or dependency-emission ambiguity needed by V;
- establishes ownership boundaries that prevent L1/L2/L3 leakage;
- resolves a concrete dependency of the active L1/L2/L3 acceptance path.

It must not displace the L1 real-evidence sequence merely to broaden reverse coverage.

Current bounded RCP queue remains:

1. upstream request-origin census;
2. StageCfg dependency-preload reacquisition;
3. Type/Descriptor identity xref;
4. factory/resource-set demand edges;
5. ownership hierarchy breadth;
6. manager-ready/family-ready/consumer-effect taxonomy;
7. dependency-aware LV/V integration only after required static graph edges are bounded.

Historical/unmerged Wave-3 evidence may define reacquisition targets but cannot silently become current authority.

## 9. Current priority order

### P0 — finish L1 real acceptance

1. obtain exact real member/materialization receipt;
2. bind it to accepted actual selected/successful source lineage;
3. classify exact representation;
4. execute one supported bounded real edit/rebuild;
5. publish/reopen/rematerialize the next-volume overlay;
6. run #209 deterministic original-game consumption + rollback;
7. run final cross-stack/V audit.

### P1 — supporting L2 closure

1. review/merge or reconcile #241;
2. acquire exact archive-bound retail member surface and run `0x0E` collision census;
3. run real protected R2B v2 seven-anchor capture;
4. capture trusted zero-loss R3 selected identity;
5. final L2 audit.

### P1 — supporting L3 closure

1. close the exact #242 materialization terminal-condition dependency bytes;
2. review/merge #240 only at bounded R1 scope if no contradiction exists;
3. proceed to R2 field/backing ownership instead of reopening broad R1 discovery;
4. capture original-process lifecycle/consumer receipts bound to exact L1/L2 identity.

### P2 — RCP/grey-boundary expansion

Proceed only where it resolves an active identity/dependency/ownership ambiguity. No speculative L4 work.

## 10. Explicit freezes / non-blockers

Do not reactivate absent direct evidence/dependency:

- binary AFS backend from `.afs/` strings alone;
- original DMC3 PACK runtime authority from historical product parser code;
- Capcom offline writer equivalence;
- exhaustive malformed-input equivalence outside the accepted representative path;
- complete `.lst` corpus/lifetime semantics unless the chosen acceptance actually selects `.lst`;
- Stage Ops/ModViz as a substitute for core L1/L2/L3 acceptance.

Bounded reverse gaps such as `0x140328540` ZIP stream initialization and `0x140328FE0` compressed seek/reset become P0 only if the chosen real acceptance path depends on them.

## 11. Completion rules

`L1 COMPLETE`, `L2 COMPLETE` and `L3 COMPLETE` are separate gate verdicts.

- L1 completion requires one same-lineage real materialization/edit/rebuild/rematerialization/original-consumer/rollback proof.
- L2 completion requires real retail collision evidence, real R2B v2, trusted selected identity, successful-mount topology reconciliation and final audit.
- L3 completion requires bounded static ownership plus representative original-process lifecycle/consumer validation beyond R1.
- V issues promotion only from reconciled evidence; percentages are planning aids only.
- RCP/V/LV never create an L4 completion state.

## 12. Documentation synchronization rule

Any merged implementation, real receipt or contradiction that changes current truth must update together:

- `docs/gdspaces/l1-roadmap.md`;
- this master roadmap;
- relevant current L2/L3 roadmap/review successor;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- canonical Drive Architecture / Layer Classification / Technical Status / Audit documents;
- tracking issues #100, #182, #209 and the relevant L2/L3 ledger.

Historical evidence/pass documents are preserved as chronology and are superseded by explicit reconciliation/addendum rather than silently rewritten.
