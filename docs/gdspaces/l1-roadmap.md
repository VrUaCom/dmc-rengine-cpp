# GDSpaces Layer 1 Roadmap

**Status:** INTERNAL PRODUCT PATH CLOSED / REAL ACCEPTANCE OPEN  
**Snapshot date:** 2026-08-26  
**Reconciled main base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`  
**Primary tracking:** #100, #182, #209  
**Final pre-Level-E audit:** `l1-final-audit-2026-08-25.md`  
**Post-audit reconciliation:** `l1-real-device-member-evidence-reconciliation-2026-08-26.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

L1 answers:

> Can DMC Rengine obtain the exact bytes selected for a DMC3 resource, preserve their provenance, safely edit an evidenced representation, rebuild the required nested container/archive stack, reopen/rematerialize the authored result, and prove that the original protected game consumed those authored bytes?

Synthetic tests, a working resolver, a valid parser, a successful mobile archive open or a crash-free game launch are not sufficient for `L1 COMPLETE`.

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

Current `main` includes:

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
- Windows + Ubuntu CI for promoted product paths.

These close the known mandatory **L1 implementation** work for the current representative DMC3-HD acceptance scope.

Fresh L2 reverse in #235 adds one cross-stack constraint that must remain visible to L1 acceptance: **filename discovery is not proof of successful mount topology**. Original bootstrap can continue after an existing archive fails mount initialization. Clean success still yields higher successful volume -> lower successful volume -> physical, but failure cases can produce a sparse mounted set. Product correction is tracked in open #237. The final L1 acceptance run must validate the actual successful selected/reopened source lineage; it must not infer mount success solely from pre-gap filename presence.

A second execution surface exists in **Pocket GDS / GDSpace Manager**: a real device can hold a large NBZ locally, run the canonical mobile `ArchiveSession`/`NbzZipSource` materialization path and emit a byte-free exact-member receipt without transferring the whole archive through the connected ChatGPT/Drive channel. This is an evidence bridge, not a second format implementation and not an original-process substitute.

## 3. Gate status

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

If the acceptance topology includes mount failures, selected identity must come from the actual successful-mounted set rather than an inferred contiguous filename set. #237 owns the product correction for that L2 failure-case distinction.

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

The #235/#237 discovery-vs-successful-mount correction does not invalidate clean successful topology. It does mean the final receipt must not claim that filename presence alone proves all mounts succeeded.

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
- exact-head Windows + Ubuntu CI is green on final canonical code/docs;
- #100, #182, #209 and current status agree;
- no unresolved contradiction changes the claimed L1 architecture or accepted representation scope.

Only then may V promote **L1 = 100% / COMPLETE**.

## 4. Commands / execution surfaces on the closure path

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

## 5. Supporting EXE/L2 boundaries

Do **not** restart these without contradictory direct evidence:

- generic basename candidate construction and archive-first/physical-second attempt order;
- archive `0x0E` normalized lookup/index behavior and normalized-string-only comparator;
- bounded FileSlot / AsyncIO whole-file materialization spine;
- ZIP stored-vs-inflated path and raw-DEFLATE core behavior;
- PAC/PNST recursive typed traversal;
- primary `.lst` packed-first selection/synthesis structure;
- exact type-0 physical final-open edge recovered under #215.

Current L2 corrections relevant to acceptance:

- #235: R2B v2 binds live windows to one process instance using PID + process creation FILETIME + module identity and derives seven canonical anchors from the canonical EXE artifact;
- #235: first missing archive filename bounds **discovery**, not independently the successful-mounted set;
- #237: product correction remains open to separate discovered archive attempts from successful mounted topology in failure cases;
- retail `0x0E` normalized-key collision census remains mandatory because the recovered qsort/bsearch comparator has no equal-key secondary tie-break.

Still-open reverse breadth is activated only when a completion claim depends on it:

1. complete ZIP stream initializer `0x140328540` body/lifetime;
2. complete compressed seek/reset `0x140328FE0` behavior;
3. exhaustive malformed/partial-read error-code equivalence;
4. dynamic `.lst` allocation/free/error/cycle behavior where a real loose-list acceptance path requires it.

These bounded gaps do not automatically block a representative packed-NBZ/PAC/PNST L1 Level-E receipt.

## 6. Explicit non-blockers / freezes

- **Binary AFS:** `.afs/` strings are evidenced logical namespaces, not a binary-backend proof.
- **PACK:** historical product parsing is not original DMC3 runtime authority absent a direct dependency.
- **Capcom offline writer equivalence:** not required for L1 product authoring completion.
- **Stage Ops / ModViz:** downstream tools; they do not define L1 truth.
- **Exhaustive malformed-input equivalence:** remains separate reverse breadth unless the claimed acceptance scope explicitly includes it.
- **Real `.lst` corpus:** mandatory only for a claim covering real loose-list consumption or when the representative acceptance path actually selects `.lst`.
- **Connected 960 MB transfer failure:** this is a transport limitation, not evidence that `DMC3-0.nbz` is absent or invalid.
- **#237 failure-case topology correction:** this is primarily L2; it blocks L1 only if the selected acceptance lineage relies on an unresolved mount-failure topology. Clean successful topology remains an admissible representative path when actually verified.

## 7. Remaining work order

No new synthetic-only feature may displace this path unless real evidence reveals a concrete missing dependency.

```text
1. produce an exact real member/materialization receipt
   - canonical protected-install command, or
   - Pocket real-device receipt plus independent selected-identity binding
2. preserve acquisition/provenance and representation classification
3. bind the selected resource to the protected request/successful-source lineage
4. perform one bounded real edit through the supported authoring domain
5. generate next-contiguous NBZ and require canonical reopen/resolver/rematerialization
6. preserve product closure receipt
7. execute #209 original-game consumption + rollback
8. run final L1 cross-stack/V audit
9. mark L1 100% / COMPLETE only if every mandatory receipt is valid
```

## 8. Environment boundary — corrected 2026-08-26

The protected artifacts are **locatable** in the connected Drive corpus:

- protected `dmc3.exe` is present;
- executable-relative `data/dmc3/dmc3-0.nbz` is present with observed size `960,358,951` bytes.

The connected execution environment cannot currently ingest the full retail NBZ because the observed Drive/Files raw transfer/materialization ceiling is `268,435,456` bytes. Therefore direct execution of the desktop protected-install closure command inside this chat environment remains blocked by transport, not by artifact discovery.

Pocket GDS provides an out-of-band real-device path to materialize and hash individual members where the full NBZ is already local. That narrows L1-C/D evidence acquisition but does not supply the protected original-process Level-E run.

Synthetic CI must not substitute for L1-G.

## 9. Documentation synchronization

When a real receipt changes L1 status, synchronize:

- this roadmap;
- `l1-final-audit-2026-08-25.md` or its completion successor/addendum;
- `l1-real-device-member-evidence-reconciliation-2026-08-26.md`;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- issues #100, #182 and #209.

Percentage estimates are planning aids only; mandatory gates remain the completion authority.
