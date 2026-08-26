# GDSpaces Layer 1 Roadmap

**Status:** INTERNAL PRODUCT PATH CLOSED / REAL ACCEPTANCE OPEN  
**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Primary tracking:** #100, #182, #209  
**Final pre-Level-E audit:** `l1-final-audit-2026-08-25.md`  
**Materialization dependency authority:** `materialization-completion-dependency-pass2-2026-08-26.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

L1 answers:

> Can DMC Rengine obtain the exact bytes selected for a DMC3 resource, preserve their provenance, safely edit an evidenced representation, rebuild the required nested container/archive stack, reopen/rematerialize the authored result, and prove that the original protected game consumed those authored bytes?

Synthetic tests, a working resolver, a valid parser, a successful mobile archive open or a crash-free game launch are not sufficient for `L1 COMPLETE`.

Open PRs #226, #238, #240 and #241 remain branch truth until merged. Their findings may support planning but may not be described as current-main implementation authority.

## 1. Canonical L1 boundary

```text
selected physical source identity
 -> archive/container identity
 -> exact member/span acquisition
 -> transform/decompression
 -> materialized bytes + ByteProvenance
 -> PAC/PNST nested expansion
 -> exact editable child identity
 -> bounded edit
 -> bottom-up container rebuild
 -> next-volume NBZ publication
 -> canonical resolver/reopen/rematerialization
 -> product closure receipt
 -> original DMC3 Level-E consumption + rollback receipt
 -> final V:L1 verdict
```

L2 supplies selected-provider/source identity where that authority is required. L1 owns byte materialization and authoring. L3 supplies original request/consumer/lifecycle evidence where needed. Validation binds those layers into one same-lineage acceptance proof; unrelated receipts must not be composed by filename alone.

## 2. Canonical product capabilities on current main

Canonical main includes:

- classic NBZ/ZIP indexing and bounded central/member acquisition;
- STORE and raw-DEFLATE method-8 materialization;
- CRC, size, SHA and explicit `ByteProvenance`;
- artifact-bound serialization/member observation preventing stale snapshot receipts;
- numbered-volume filename discovery / first-gap product support for the accepted clean path;
- canonical runtime resolver composition and higher-successful-volume precedence for the represented mounted set;
- atomic/no-replace artifact publication with staged validation;
- direct-retail resolver-based member extraction + provenance sidecar;
- PAC and PNST sparse/empty/alias-preserving relative-slot parsing;
- recursive PAC/PNST expansion;
- size-changing relative-slot reflow with byte-exact untouched physical spans;
- nested root-to-leaf slot-path authoring with bottom-up parent reflow and chained SHA receipts;
- verified immutable NBZ copy rebuild;
- deterministic next-contiguous STORE NBZ overlay authoring;
- staged NBZ reopen and exact member verification before publication;
- protected distribution executable preflight;
- protected retail authoring closure orchestration through resolver rematerialization;
- process-instance-bound L2 R2B v2 tooling from #235;
- normal materialization completion callback/context ABI from #230;
- materialization dependency narrowing/document synchronization from #242;
- Windows + Ubuntu CI for promoted product paths.

These close the known mandatory **L1 implementation** work for the current representative DMC3-HD acceptance scope.

### L2 topology correction relevant to final L1 evidence

Merged #235 proves that **filename discovery is not proof of successful mount topology**. The first missing numbered filename terminates discovery, but an existing archive may fail registration and discovery may continue. The actually successful mounted set may therefore be sparse.

Successful archive registrations still prepend, preserving:

```text
higher successful volume -> lower successful volume -> physical
```

Issue #237 tracks the product correction and PR #241 is the active implementation separating discovery/attempt authority from successful runtime mount outcomes. Until #241 merges it is pending branch truth. Final L1 acceptance must validate the actual successful selected/reopened source lineage and must never infer successful mounting from filename presence alone.

### Pocket GDS evidence bridge

Pocket GDS / GDSpace Manager can hold a large NBZ locally, execute canonical mobile GDSpaces materialization and emit a byte-free exact-member receipt without transferring the full archive through the connected environment.

Pending DMC Rengine PR #238 reconciles that evidence seam. It is an evidence bridge, not a second format implementation and not an original-process substitute.

## 3. Materialization completion dependency bridge — merged #230/#242

Canonical normal completion flow:

```text
0x1401B84E0
 -> registers 0x1401B8DC0
 -> one u32 context = record_ptr - 0x140C99D30
 -> valid contexts = index * 0x48 for 363 records
```

Normal `0x1401B8DC0` receives no:

- raw transport status pointer;
- error flag;
- byte count;
- FileSlot/ReadRequest handle;
- child/outstanding-work metadata.

Therefore the lower materialization success/error dependency cannot be decided inside normal `0x1401B8DC0`. By normal state2 publication time, lower transport/materialization success must already be terminal, or the queued completion must have been suppressed/removed.

**FIFO insertion order alone is not a proven dependency barrier.** If the earlier materialization scheduler job merely submits asynchronous I/O and retires, a later completion record could run too early. No generic original fan-in/outstanding-child counter is currently evidenced.

Focused exact-byte acquisition order:

1. `0x1402EF4D0` queued materialization job identity/type, callees and inherited load-context consumer;
2. relevant `0x1402EF790` dispatch case, persistence/re-poll/terminal retirement;
3. reacquire historical `0x1400333E0` status/poll role as hypothesis until fresh bytes;
4. reacquire historical `0x140033390` terminal cleanup/release role as hypothesis until fresh bytes;
5. `0x1400335A0` lower transport success/error terminal writes;
6. identify what prevents normal `0x1401B8DC0` on incomplete or failed transport;
7. `0x1402EF460` pending higher-scheduler clear/rollback and queued-completion suppression;
8. only then apply the confirmed direct-resource mechanism to `.lst` child/recursive failure ordering.

Layer ownership is unchanged:

- exact FileSlot byte-read mechanics may support L1;
- FileSlot/AsyncIO request ownership, scheduling, callback lifetime and cancellation are L3;
- `0x1401B8CA0` is the explicit L1/L3 materialization-success seam;
- LoadedResource state semantics remain L3.

This seam does not reopen closed L1 product authoring capabilities and does not make L3 complete.

## 4. Gate status

### L1-A — publication integrity

**CLOSED / CANONICAL** — closed by #194 and subsequent authoring/acquisition seams using the shared final no-replace publication contract.

### L1-B — artifact-stable retail acquisition

**CLOSED / CANONICAL** — closed by #195–#197 and consumed by the direct-retail acquisition command.

### L1-C — representative source/member provenance

**IMPLEMENTATION CLOSED / REAL SAME-LINEAGE RECEIPT OPEN**

Canonical protected-install path:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The game request, not a pre-guessed archive path, is the selection authority. The receipt must preserve the actual selected provider/archive/member identity, central-entry metadata, materialized SHA/size and byte transform.

If the topology includes mount failures, selected identity must come from the actual successful-mounted set. Issue #237 / pending PR #241 owns that product distinction.

`obj\em000.pac` remains a high-value target but is not mandatory if another representative resource gives a stronger deterministic authoring/consumer receipt.

#### Pocket real-device sub-receipt

Pocket GDS PR #2 adds `gdspaces.l1.member-acquisition-receipt.v1`. When run against the actual NBZ on-device it can preserve archive SHA/size, ResourceIdentity, central/nested identity, compression metadata, ByteProvenance, exact exported SHA/size, representation class and producer/core provenance.

This can close the **member-byte acquisition/materialization sub-gate**. It does **not** by itself prove protected original-process resolver selection. Final L1-C promotion must independently bind the same archive/member identity to the accepted selected-source authority.

### L1-D — retail representation classification

**REAL RECEIPT OPEN / MOBILE EXECUTION PATH READY**

Classify the exact bytes obtained in L1-C. Only use an existing writer if the observed representation is inside its evidenced domain. If not, create a new bounded evidence gate.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT IMPLEMENTATION CLOSED / REAL SAME-LINEAGE RECEIPT OPEN**

Current writers support top-level and nested PAC/PNST slot paths, size-changing reflow, sparse/alias preservation and exact untouched-sibling validation. A materialization receipt alone does not close E.

### L1-F — next-volume publication + canonical reopen

**PRODUCT IMPLEMENTATION CLOSED / REAL SAME-LINEAGE RECEIPT OPEN**

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged canonical reopen
 -> atomic/no-replace publication
 -> RuntimeResourceResolver higher-successful-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

The #235/#241 correction does not invalidate a verified all-success topology; it forbids claiming mount success from filename presence alone.

### L1-G — original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E / MANDATORY** — tracking #209.

Required run:

```text
same-lineage product closure receipt
 -> exact generated DMC3-N.nbz
 -> controlled copy into retail data/dmc3
 -> post-copy SHA verification
 -> protected distribution launch
 -> deterministic request/consumer path
 -> observable effect attributable to authored bytes
 -> clean transition/exit
 -> remove only test overlay
 -> verify original retail artifacts unchanged
```

A crash-free launch or Pocket receipt alone fails this gate.

### L1-H — final cross-stack audit

**OPEN / DEPENDS ON L1-C..G REAL RECEIPTS**

Before `L1 COMPLETE`:

- exact executable authority is recorded;
- selected-provider/archive/member provenance exists;
- successful-mounted topology assumptions are evidenced rather than inferred;
- exact materialized member identity and representation classification exist;
- real authored replacement/rebuild and generated overlay identities are exact;
- canonical resolver/rematerialization succeeds on the real artifact;
- original DMC3 consumer observation exists;
- rollback proves retail immutability;
- materialization terminal-condition ordering is consistent with the bounded L1/L3 seam;
- exact-head Windows + Ubuntu CI is green;
- #100, #182, #209 and current status agree;
- no unresolved contradiction changes the accepted scope.

Only then may V promote **L1 = 100% / COMPLETE**.

## 5. Remaining work order

```text
1. produce an exact real member/materialization receipt
2. bind it to the protected request / actual-successful-source lineage
3. classify the exact representation
4. perform one bounded real edit through an evidenced writer
5. generate next-contiguous NBZ and require canonical reopen/rematerialization
6. execute #209 original-game consumption + rollback
7. run final L1 cross-stack/V audit
8. mark L1 COMPLETE only if every mandatory receipt is valid
```

Supporting work may run only where it discharges a dependency:

- review/merge #241 if its successful-mount contract survives validation;
- acquire real R2B v2 protected-process evidence using #235 tooling;
- close #242's exact materialization terminal-condition dependency mechanism;
- review/merge #240 only at its bounded R1 static scope.

## 6. Explicit non-blockers / freezes

- Binary AFS is not inferred from `.afs/` namespace strings.
- Historical PACK parsing is not original DMC3 runtime authority absent direct evidence.
- Capcom offline writer equivalence is not required for L1 authoring acceptance.
- Stage Ops / ModViz do not define L1 truth.
- Exhaustive malformed-input equivalence remains separate reverse breadth unless activated by the accepted path.
- Real `.lst` corpus/lifetime semantics are mandatory only if the representative acceptance actually selects `.lst`.
- Connected 960 MB transfer failure is a transport limitation, not archive absence.
- #241 is primarily L2 and blocks L1 only if the chosen lineage depends on unresolved mount-failure topology.
- #240 is bounded L3 static work, not L1 completion and not full L3 completion.
- #226 RCP / V-LV architecture is orthogonal orchestration/validation, not L4.

## 7. Environment boundary

Merged #233 establishes that protected `dmc3.exe` and executable-relative `data/dmc3/dmc3-0.nbz` are locatable. The NBZ is observed at `960,358,951` bytes, while the connected Drive/Files raw materialization ceiling is `268,435,456` bytes.

Pocket GDS can materialize individual members where the archive is already local. The protected game PC/process remains required for original selected identity, R2B/R3 evidence, #209 consumption/rollback and original lifecycle evidence.

Synthetic CI must not substitute for L1-G.

## 8. Documentation synchronization

When a real receipt or merged reverse/implementation changes current truth, synchronize:

- this roadmap;
- `master-roadmap.md`;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- relevant completion/audit successor or explicit addendum;
- canonical Drive Architecture / Layer Classification / Technical Status / Audit documents;
- issues #100, #182 and #209.

Historical evidence/pass documents remain chronology and are superseded by explicit reconciliation/addendum rather than silently rewritten. Documentation synchronization itself never creates a completion claim.
