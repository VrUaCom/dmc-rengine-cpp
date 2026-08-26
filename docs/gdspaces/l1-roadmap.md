# GDSpaces Layer 1 Roadmap

**Status:** INTERNAL PRODUCT PATH CLOSED / REAL ACCEPTANCE OPEN  
**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Primary tracking:** #100, #182, #209  
**Final pre-Level-E audit:** `l1-final-audit-2026-08-25.md`  
**Post-audit reconciliation:** `l1-real-device-member-evidence-reconciliation-2026-08-26.md`  
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

Current canonical main includes:

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
- nested root-to-leaf slot-path authoring, e.g. `0/2/1`, with bottom-up parent reflow and chained SHA receipts;
- verified immutable NBZ copy rebuild;
- deterministic next-contiguous STORE NBZ overlay authoring;
- staged NBZ reopen and exact member verification before publication;
- protected distribution executable preflight;
- protected retail authoring closure orchestration through resolver rematerialization;
- process-instance-bound L2 R2B v2 tooling from #235;
- normal materialization completion callback/context ABI from #230;
- materialization dependency model narrowing and documentation synchronization from #242;
- Windows + Ubuntu CI for promoted product paths.

These close the known mandatory **L1 implementation** work for the current representative DMC3-HD acceptance scope.

### L2 topology correction that affects final L1 evidence

Merged #235 proves that **filename discovery is not proof of successful mount topology**. The first missing numbered filename terminates discovery, but an existing archive may fail registration and discovery may continue. The actually successful mounted set may therefore be sparse.

Successful archive registrations still prepend, preserving:

```text
higher successful volume -> lower successful volume -> physical
```

Issue #237 tracks the product correction and PR #241 is the active implementation separating discovery/attempt authority from successful runtime mount outcomes. Until #241 merges it is pending branch truth. Final L1 acceptance must validate the actual successful selected/reopened source lineage and must never infer successful mounting from filename presence alone.

### Pocket GDS evidence bridge

A second execution surface exists in **Pocket GDS / GDSpace Manager**: a real device can hold a large NBZ locally, run the canonical mobile `ArchiveSession`/`NbzZipSource` materialization path and emit a byte-free exact-member receipt without transferring the whole archive through the connected ChatGPT/Drive channel.

Pocket GDS PR #2 supplies the mobile receipt seam. DMC Rengine PR #238 reconciles that seam into this L1 evidence model and remains pending. This is an evidence bridge, not a second format implementation and not an original-process substitute.

## 3. Materialization completion dependency bridge — merged #230/#242

This cross-layer seam is supporting evidence for the final vertical proof and must be stated precisely.

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

**CLOSED / CANONICAL**

Closed by #194 and all subsequent authoring/acquisition seams using the shared final no-replace publication contract.

### L1-B — artifact-stable retail acquisition

**CLOSED / CANONICAL**

Closed by #195–#197 and consumed by the direct-retail acquisition command.

### L1-C — representative source/member provenance

**IMPLEMENTATION CLOSED / REAL SAME-LINEAGE RECEIPT OPEN**

Canonical protected-install path:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The game request, not a pre-guessed archive path, is the selection authority for this path. The receipt must preserve the actual selected provider/archive/member identity, central-entry metadata, materialized SHA/size and byte transform.

If the acceptance topology includes mount failures, selected identity must come from the actual successful-mounted set rather than an inferred contiguous filename set. Issue #237 tracks this distinction and PR #241 is its pending product implementation.

`obj\em000.pac` remains a high-value target but is not mandatory if another representative resource gives a stronger deterministic authoring/consumer receipt.

#### Pocket real-device sub-receipt

Pocket GDS PR #2 adds `gdspaces.l1.member-acquisition-receipt.v1` over the existing canonical `ArchiveSession::export_node` path. When run against the actual NBZ on-device it can preserve:

- NBZ snapshot SHA/size;
- canonical ResourceIdentity;
- central index / nested slot identity where available;
- compression metadata;
- full ByteProvenance;
- exact materialized/exported SHA/size;
- representation class;
- producer build/core provenance.

This can close the **member-byte acquisition/materialization sub-gate** without transporting the 960 MB NBZ into this connected execution container.

It does **not** by itself prove protected original-process resolver selection for a game request. To promote it into the final L1-C lineage, bind the same archive/member identity to the accepted L2/protected-install selection authority rather than matching only by filename.

### L1-D — retail representation classification

**REAL RECEIPT OPEN / MOBILE EXECUTION PATH READY**

Classify the exact bytes obtained in L1-C. A Pocket receipt may provide direct representation evidence from the canonical materialized node and ByteProvenance.

Only use an existing writer if the observed representation is inside its evidenced domain. If not, stop and create a new bounded evidence gate; do not force the bytes through a convenient writer.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT IMPLEMENTATION CLOSED / REAL SAME-LINEAGE RECEIPT OPEN**

Current writers support top-level and nested PAC/PNST slot paths, size-changing reflow, sparse/alias preservation and exact untouched-sibling validation.

Relevant promotion includes #201 and #213.

A real materialization receipt alone does not close E. The selected exact representation must be edited through an evidenced writer and the source/output/reflow identities preserved.

### L1-F — next-volume publication + canonical reopen

**PRODUCT IMPLEMENTATION CLOSED / REAL SAME-LINEAGE RECEIPT OPEN**

Current clean-path product flow supports:

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged canonical reopen
 -> atomic/no-replace publication
 -> RuntimeResourceResolver higher-successful-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

`verify-dmc3-l1-authoring` composes protected preflight, retail acquisition, top-level PAC/PNST authoring, next-volume overlay and resolver/rematerialization checks into one product closure receipt. Nested authoring is canonical through `rebuild-relative-slot-path`.

The #235/#241 discovery-vs-successful-mount correction does not invalidate clean successful topology. It does mean the final receipt must not claim that filename presence alone proves all mounts succeeded.

### L1-G — original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E / MANDATORY**

Canonical tracking: #209.

Required controlled run:

```text
same-lineage product closure receipt
 -> exact generated DMC3-N.nbz
 -> verify destination absent
 -> controlled copy into retail data/dmc3
 -> post-copy SHA == closure overlay SHA
 -> launch protected distribution executable
 -> deterministic request / consumer path
 -> observable effect attributable to authored bytes
 -> clean transition/exit
 -> remove only test overlay
 -> verify original retail artifacts unchanged
```

A crash-free launch alone fails this gate. A Pocket receipt alone also fails this gate.

### L1-H — final cross-stack audit

**OPEN / DEPENDS ON L1-C..G REAL RECEIPTS**

Before `L1 COMPLETE`:

- exact executable authority is recorded;
- selected-provider/archive/member provenance exists;
- successful-mounted topology assumptions used by the selected path are evidenced rather than inferred from filename presence;
- exact materialized member identity exists;
- representation classification is explicit;
- real authored replacement/rebuild receipt exists;
- generated overlay identity is exact;
- canonical resolver/rematerialization succeeds on that real artifact;
- original DMC3 consumer observation exists;
- rollback proves retail immutability;
- materialization terminal-condition ordering is consistent with the bounded original L1/L3 seam used by the proof;
- exact-head Windows + Ubuntu CI is green on final canonical code/docs;
- #100, #182, #209 and current status agree;
- no unresolved contradiction changes the claimed L1 architecture or accepted representation scope.

Only then may V promote **L1 = 100% / COMPLETE**.

## 5. Commands / execution surfaces on the closure path

Canonical desktop/product commands:

```text
preflight-dmc3-game-test <exe-dir>
extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>
rebuild-relative-slot-path <container-file> <slot/path> <replacement-file> <output-file>
build-nbz-copy <source-nbz> <central-index> <replacement-file> <output-nbz>
build-dmc3-overlay <exe-dir> <game-request> <authored-file> <output-dir>
verify-dmc3-l1-authoring <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>
```

Real-device acquisition bridge after Pocket GDS PR #2:

```text
Open actual NBZ in GDSpace Manager
 -> search/navigate representative member
 -> Export
 -> canonical materialization/export
 -> gdspaces.l1.member-acquisition-receipt.v1
```

The desktop commands are product authoring/validation seams. Pocket GDS is a mobile execution surface over the canonical GDSpaces reader/materializer snapshot. Neither surface claims to reproduce Capcom's offline writer implementation.

## 6. Supporting EXE/L2/L3 boundaries

Do **not** restart these without contradictory direct evidence:

- generic basename candidate construction and archive-first/physical-second attempt order;
- archive `0x0E` normalized lookup/index behavior and normalized-string-only comparator;
- bounded FileSlot / AsyncIO whole-file materialization spine;
- ZIP stored-vs-inflated path and raw-DEFLATE core behavior;
- PAC/PNST recursive typed traversal;
- primary `.lst` packed-first selection/synthesis structure;
- exact type-0 physical final-open edge recovered under #215;
- normal `0x1401B8DC0` u32 completion-context ABI from #230.

Current L2 corrections relevant to acceptance:

- #235: R2B v2 binds live windows to one process instance using PID + process creation FILETIME + module identity and derives seven canonical anchors from the canonical EXE artifact;
- #235: first missing archive filename bounds **discovery**, not independently the successful-mounted set;
- issue #237 / pending PR #241: product correction separates discovered archive attempts from successful mounted topology in failure cases;
- retail `0x0E` normalized-key collision census remains mandatory because the recovered qsort/bsearch comparator has no equal-key secondary tie-break.

Current L3 supporting correction:

- #230/#242: normal state2 completion cannot inspect lower transport status; safe promotion depends on the pre-dispatch terminal-condition/dependency mechanism;
- pending #240: R1 `LoadedResource +0x04` writer census may become statically bounded-closed if merged, but this does not close broader L3 R2-R5/V1-V7.

Still-open reverse breadth is activated only when a completion claim depends on it:

1. complete ZIP stream initializer `0x140328540` body/lifetime;
2. complete compressed seek/reset `0x140328FE0` behavior;
3. exhaustive malformed/partial-read error-code equivalence;
4. dynamic `.lst` allocation/free/error/cycle behavior where a real loose-list acceptance path requires it.

These bounded gaps do not automatically block a representative packed-NBZ/PAC/PNST L1 Level-E receipt.

## 7. Explicit non-blockers / freezes

- **Binary AFS:** `.afs/` strings are evidenced logical namespaces, not a binary-backend proof.
- **PACK:** historical product parsing is not original DMC3 runtime authority absent a direct dependency.
- **Capcom offline writer equivalence:** not required for L1 product authoring completion.
- **Stage Ops / ModViz:** downstream tools; they do not define L1 truth.
- **Exhaustive malformed-input equivalence:** remains separate reverse breadth unless the claimed acceptance scope explicitly includes it.
- **Real `.lst` corpus:** mandatory only for a claim covering real loose-list consumption or when the representative acceptance path actually selects `.lst`.
- **Connected 960 MB transfer failure:** transport limitation, not evidence that `DMC3-0.nbz` is absent or invalid.
- **#241 failure-case topology correction:** primarily L2; it blocks L1 only if the selected acceptance lineage relies on unresolved mount-failure topology. A clean successful topology remains admissible only when actually verified.
- **#240 R1 review:** even if promoted, it is L3 bounded static closure, not L1 completion and not full L3 completion.
- **#226 RCP / V-LV architecture:** orthogonal orchestration/validation, not a fourth execution layer.

## 8. Remaining work order

No new synthetic-only feature may displace this path unless real evidence reveals a concrete missing dependency.

```text
1. produce an exact real member/materialization receipt
   - canonical protected-install command, or
   - Pocket real-device receipt plus independent selected-identity binding
2. preserve acquisition/provenance and representation classification
3. bind the selected resource to the protected request/actual-successful-source lineage
4. perform one bounded real edit through the supported authoring domain
5. generate next-contiguous NBZ and require canonical reopen/resolver/rematerialization
6. preserve product closure receipt
7. execute #209 original-game consumption + rollback
8. run final L1 cross-stack/V audit
9. mark L1 100% / COMPLETE only if every mandatory receipt is valid
```

Supporting work that may run without displacing the above:

```text
- review/merge #241 if its successful-mount topology contract survives exact-head CI
- run real R2B v2 protected-process capture from #235 tooling
- close the exact materialization terminal-condition dependency bytes from #242's acquisition order
- review/merge #240 only at its bounded R1 static scope
```

## 9. Environment boundary — corrected 2026-08-27

The protected artifacts are **locatable** in the connected Drive corpus:

- protected `dmc3.exe` is present;
- executable-relative `data/dmc3/dmc3-0.nbz` is present with observed size `960,358,951` bytes.

The connected execution environment cannot currently ingest the full retail NBZ because the observed Drive/Files raw transfer/materialization ceiling is `268,435,456` bytes. Therefore direct execution of the desktop protected-install closure command inside this chat environment remains blocked by transport, not by artifact discovery.

Pocket GDS provides an out-of-band real-device path to materialize and hash individual members where the full NBZ is already local. That narrows L1-C/D evidence acquisition but does not supply protected original-process selected identity, Level-E consumption/rollback or full L2/L3 evidence.

Synthetic CI must not substitute for L1-G.

## 10. Documentation synchronization

When a real receipt or merged reverse/implementation changes current status, synchronize:

- this roadmap;
- `l1-final-audit-2026-08-25.md` or its completion successor/addendum;
- `l1-real-device-member-evidence-reconciliation-2026-08-26.md`;
- `materialization-completion-dependency-pass2-2026-08-26.md` only by explicit supersession/addendum, not by rewriting historical evidence;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- `master-roadmap.md` where cross-layer dependency ordering changes;
- issues #100, #182 and #209.

Percentage estimates are planning aids only; mandatory gates remain the completion authority. Documentation synchronization itself never creates a completion claim.
