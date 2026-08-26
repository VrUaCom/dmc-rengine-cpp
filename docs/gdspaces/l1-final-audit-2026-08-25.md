# GDSpaces Layer 1 — Final Pre-Level-E Audit

**Audit date:** 2026-08-25  
**Reconciled:** 2026-08-26 against `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Layer:** L1 — Resource Materialization  
**Verdict:** INTERNAL PRODUCT PATH CLOSED; L1 COMPLETE remains blocked by real-retail / original-game evidence.  
**EXE boundary authority:** `l1-exe-boundary-review-2026-08-26.md`  
**L1/L3 handoff authority:** `l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md`  
**Canonical L3 raw authority:** `l3-boundary-audit-2026-08-26.md` + `l3-raw-exe-pass-2026-08-26.md`

## 1. Purpose

This audit answers one question:

> Is there any remaining **internal GDSpaces implementation gap** that must be closed before a representative protected-DMC3 Level-E acceptance run can decide Layer-1 completion?

The answer remains **no known mandatory internal implementation gap** for the currently evidenced DMC3-HD L1 acceptance scope.

That does **not** mean L1 is COMPLETE. The remaining mandatory gates are real-retail and original-process evidence executions.

The 2026-08-26 raw-EXE reconciliation changes one important boundary statement without changing the acceptance verdict:

```text
L2 selected identity
 -> L1 exact byte acquisition/read/seek/decompression/materialized-byte authority
 -> 0x1401B8CA0 materialization/lifecycle seam
 -> L3 request/scheduler ownership
 -> L3 state 0 -> 1 -> 2 -> typed post-load -> 3
```

`LoadedResource state 1 -> 2` is therefore **L3 lifecycle completion**, not the terminal L1 boundary.

FileSlot is a boundary subsystem: byte-read mechanics can support L1, while request ownership, scheduling, completion, cancellation and close lifetime are L3.

## 2. Current canonical L1 product chain

Current `main` contains one coherent product path:

```text
protected DMC3 executable preflight
 -> executable-relative numbered-volume observation
 -> recovered first-gap/runtime-domain bootstrap policy
 -> canonical RuntimeResourceResolver selection
 -> exact selected retail NBZ identity
 -> artifact-bound central-entry/member observation
 -> STORE or raw-DEFLATE materialization
 -> CRC / size / SHA / ByteProvenance receipt
 -> PAC/PNST canonical parse + sparse/alias-preserving expansion
 -> top-level or nested slot-path authored replacement
 -> size-changing bottom-up relative-slot reflow
 -> byte-exact untouched sibling preservation
 -> atomic/no-replace rebuilt artifact publication
 -> next-contiguous DMC3-N.nbz overlay authoring
 -> staged canonical NBZ reopen validation
 -> canonical resolver with authored higher volume
 -> exact rebuilt-container rematerialization
 -> exact authored child verification
 -> closure SHA/provenance summary
 -> external original-game Level-E consumption/rollback receipt
```

The final arrow is intentionally external and remains open in issue #209.

## 3. Mandatory gate matrix

### L1-A — Publication integrity

**Status: CLOSED / CANONICAL**

Closed by the shared no-replace publication contract and subsequent CLI adoption. Product publication requires private staging, complete write before visibility, staged validation where applicable and final no-replace publication.

Relevant promotion: #194 and consumers merged afterwards.

### L1-B — Artifact-stable retail member acquisition

**Status: CLOSED / CANONICAL**

Artifact-bound central/member observations prevent stale metadata from being reused after same-size archive mutation. STORE and raw-DEFLATE method 8 are both covered by focused regressions.

Relevant promotion: #195, #196, #197.

### L1-C — Direct-retail representative provenance

**Status: IMPLEMENTATION READY / REAL RECEIPT REQUIRED**

Use:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The request, not a pre-guessed archive path, is the authority. The receipt must preserve the actual resolver winner, selected volume/archive/member identity, archive identity, central metadata, transform and materialized SHA/size.

`obj\em000.pac` remains a high-value request but is not mandatory if another representative request provides a stronger deterministic consumer effect.

### L1-D — Retail representation classification

**Status: REAL RECEIPT REQUIRED**

Classify the exact bytes from L1-C. Only use an existing writer if the observed representation is inside its evidenced domain. Otherwise create a new bounded evidence gate.

### L1-E — Bounded edit and bottom-up rebuild

**Status: PRODUCT IMPLEMENTATION CLOSED; REAL-RETAIL RECEIPT REQUIRED**

Current authoring supports:

- size-changing PAC/PNST slot reflow;
- sparse/empty/alias preservation;
- byte-exact untouched physical spans;
- nested root-to-leaf slot-path authoring;
- bottom-up parent reflow;
- chained SHA receipts;
- atomic/no-replace output publication.

Relevant promotion: #201 and #213.

### L1-F — Next-volume NBZ publication and canonical reopen

**Status: PRODUCT IMPLEMENTATION CLOSED; REAL-RETAIL RECEIPT REQUIRED**

Current product path supports deterministic next-volume authoring, staged canonical NBZ reopen, higher-volume resolver selection, exact rebuilt-container rematerialization and exact authored-child verification.

Relevant promotion: #188, #194, #198, #208.

### L1-G — Original DMC3 consumption

**Status: OPEN / EXTERNAL LEVEL-E**

Issue #209 remains the principal completion blocker:

```text
closure receipt
 -> exact generated DMC3-N.nbz
 -> controlled copy into protected installation
 -> post-copy SHA equality
 -> launch protected distribution executable
 -> deterministic request/consumer path
 -> observable effect attributable to authored bytes
 -> clean transition/exit
 -> remove only test overlay
 -> verify original retail artifacts unchanged
```

A crash-free launch does not close this gate.

### L1-H — Final cross-stack acceptance audit

**Status: OPEN / DEPENDS ON REAL RECEIPTS**

Before `L1 COMPLETE / 100%`, verify one coherent lineage of:

- executable authority;
- exact retail resolver/archive/member provenance;
- representation classification;
- authored replacement and rebuilt-container identity;
- generated overlay identity;
- canonical higher-volume winner;
- exact rematerialized bytes;
- original consumer observation;
- rollback / retail immutability;
- exact-head Windows + Ubuntu CI;
- contradiction-free code/docs/issues.

## 4. Internal implementation verdict

The following are **not remaining mandatory internal L1 blockers**:

- NBZ STORE/raw-DEFLATE materialization;
- bounded ZIP source handling at the supported product scope;
- artifact snapshot binding;
- publication race safety;
- first-gap volume read semantics;
- PAC/PNST structural parsing and recursive expansion;
- sparse/empty/alias identity preservation;
- size-changing and nested slot-path reflow;
- verified immutable NBZ copy rebuild;
- next-volume overlay writer;
- product reopen/rematerialization;
- resolver composition required by the closure receipt;
- protected executable preflight;
- output isolation from the retail executable tree;
- type-0 physical-provider post-`0x0C` bounded static final-open contract promoted by #215.

If a real-retail representation falls outside these domains, that creates a new evidence-derived bounded gate rather than invalidating the existing internal closure.

## 5. Reconciled L1/L3 EXE handoff

Canonical raw-EXE evidence now directly establishes the lower-L3 acquisition ordering in `0x1401B84E0`:

```text
record +0x18 <- descriptor/type authority
 -> prepare record +0x28 backing
 -> record +0x20 <- loaded/materialized payload
 -> call 0x1401B8CA0
 -> only on success: state <- 1
 -> schedule completion callback 0x1401B8DC0
```

Therefore:

- `0x1401B8CA0` is the explicit L1/L3 seam;
- its representation/materialization mechanics are L1 evidence;
- its success/failure result gates L3 state1 publication;
- `0x1401B8DC0` is L3 normal completion `state 1 -> 2`;
- `0x1401B92D0` performs typed post-load -> optional callback -> `state 3`.

Do not restore the older shorthand `L1 ends at state1->2`.

## 6. Bounded supporting reverse gaps

These are legitimate reverse targets but are **not automatic L1 completion blockers**.

### `0x1402EF4D0` exact body/callees/load-context semantics

Safe bounded label: **resource materialization submission/scheduling wrapper**.

Still open:

- exact lower callees;
- exact relationship to whole-file/FileSlot paths and higher scheduler;
- first concrete consumer/numeric domain of inherited load-context/mode;
- exact failure return semantics.

### Completion ordering / dependency barrier

No generic child/outstanding-work **fan-in counter** is currently evidenced.

Safe open question:

> What exact dependency/order prevents L3 lifecycle advancement before required L1 materialization is valid?

Possible mechanisms include synchronous ordering, scheduler ordering, nested callbacks, another status/dependency object or an explicit counter. None is promoted without direct dataflow.

### Scheduler rollback / transport interaction

`0x1402EF580` is scheduler enqueue, `0x1402EF790` scheduler worker, and preserved evidence identifies `0x1402EF460` as pending scheduled-entry clear/rollback.

`0x1402EF460` is not OS AsyncIO cancellation authority. Exact interaction with already-running FileSlot/ReadRequest/backend work remains open.

### `.lst` child failure / temporary buffer

The grammar/layout/recursive synthesis is strong. Open breadth is child-population failure ordering, temporary allocator/free identity, malformed/recursion cleanup and partial parent lifetime.

### ZIP/FileSlot exact error tails

Complete ZIP initializer/compressed seek bodies and exhaustive partial-read/malformed/cancellation behavior remain claim-dependent bounded breadth.

## 7. Completion scope rule

`L1 COMPLETE` means the evidenced DMC3-HD materialization/authoring path is validated end to end at its declared scope.

It does **not** mean:

- every malformed input is bit-for-bit Capcom-equivalent;
- every hypothetical binary format is supported;
- Capcom offline authoring tools were reconstructed;
- L2 provider-selection evidence or L3 lifecycle breadth is complete.

Those claims have separate gates.

## 8. Remaining critical path

```text
1. run direct-retail acquisition on protected DMC3
2. preserve exact provenance receipt
3. classify exact retail representation
4. perform one supported bounded real edit
5. generate next-contiguous NBZ through closure workflow
6. require canonical resolver/reopen/rematerialization success
7. preserve product closure receipt
8. execute #209 original-game consumption + rollback
9. run final cross-stack audit
10. only then mark L1 100% / COMPLETE
```

While external acceptance is blocked, focused static handoff work follows:

```text
0x1402EF4D0 exact body/callees/load-context
 -> L1 byte-materialization -> L3 request/scheduler handoff
 -> completion ordering/dependency barrier
 -> scheduler rollback / transport-failure interaction
 -> .lst child failure/temp cleanup where activated
```

Focused packet: `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`.

## 9. Environment boundary

The canonical analysis executable is **not currently an unresolved project blocker**: the exact `e454...` raw EXE has already been reacquired and used in the 2026-08-26 L3 static audits.

The remaining external blocker is access to the exact protected-install **real corpus and original process** needed for real-retail provenance, protected-runtime mapping and Level-E execution.

This external evidence limitation does not justify weakening `L1 COMPLETE` criteria.
