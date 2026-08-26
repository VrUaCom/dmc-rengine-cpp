# GDSpaces Layer 1 — Final Pre-Level-E Audit

**Audit date:** 2026-08-25  
**Reconciled:** 2026-08-26 against current-main resource/runtime authority through merged #228/#230/#233  
**Layer:** L1 — Resource Materialization  
**Verdict:** INTERNAL PRODUCT PATH CLOSED; L1 COMPLETE remains blocked by real-retail / original-game evidence.  
**Materialization completion authority:** `materialization-completion-boundary-pass-2026-08-26.md` + `materialization-completion-dependency-pass2-2026-08-26.md`

## 1. Purpose

This audit asks:

> Is there any remaining **internal GDSpaces implementation gap** that must be closed before a representative protected-DMC3 Level-E acceptance run can decide Layer-1 completion?

The answer remains **no known mandatory internal implementation gap** for the currently evidenced representative DMC3-HD L1 acceptance scope.

That does **not** mean L1 is COMPLETE. The remaining mandatory gates are evidence executions against real retail artifacts and the original protected game process.

The 2026-08-26 completion-ordering passes refine supporting EXE reverse without weakening the Level-E acceptance rule.

## 2. Current canonical L1 product chain

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

The final arrow remains open under issue #209.

## 3. Mandatory gate matrix

### L1-A — Publication integrity

**CLOSED / CANONICAL**

Shared staged validation + atomic/no-replace publication is canonical across the authoring/evidence path.

### L1-B — Artifact-stable retail member acquisition

**CLOSED / CANONICAL**

Archive/member/hash observations are artifact-bound; stale same-path observations cannot silently become provenance-grade receipts.

### L1-C — Direct-retail representative provenance

**IMPLEMENTATION READY / REAL RECEIPT REQUIRED**

Canonical command:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The receipt must record actual resolver winner, selected volume/archive identity, central-entry metadata, materialized SHA/size and transform/provenance.

A preferred request such as `obj\em000.pac` is not a pre-guessed archive member authority.

### L1-D — Exact retail representation classification

**REAL RECEIPT REQUIRED**

Classify the exact bytes acquired in L1-C. If they fall outside a current writer domain, stop and create a new bounded evidence gate rather than forcing them through an unrelated serializer.

### L1-E — Bounded edit and bottom-up rebuild

**PRODUCT IMPLEMENTATION CLOSED / REAL-RETAIL RECEIPT REQUIRED**

Current product authoring includes same-level and nested PAC/PNST size-changing reflow, sparse/alias preservation, exact untouched-span validation and explicit output publication.

### L1-F — Next-volume NBZ publication + canonical reopen/rematerialization

**PRODUCT IMPLEMENTATION CLOSED / REAL-RETAIL RECEIPT REQUIRED**

Current product path supports next-contiguous overlay generation, staged NBZ reopen, higher-volume resolver selection and exact authored-child rematerialization checks.

### L1-G — Original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E**

Required run:

```text
closure receipt
 -> exact generated DMC3-N.nbz
 -> controlled copy into protected installation data/dmc3
 -> post-copy SHA equality
 -> launch protected distribution executable
 -> deterministic request/consumer path
 -> observable effect attributable to authored bytes
 -> clean transition/exit
 -> remove only test overlay
 -> verify original retail artifacts unchanged
```

A crash-free launch alone does not close this gate.

### L1-H — Final cross-stack acceptance audit

**OPEN / DEPENDS ON REAL RECEIPTS**

Before `L1 COMPLETE / 100%`:

- exact executable authority is recorded;
- direct-retail archive/member provenance exists;
- retail representation is classified explicitly;
- real authored replacement/rebuild receipt exists;
- generated overlay identity is exact;
- canonical resolver/rematerialization succeeds on that real artifact;
- original DMC3 consumer observation exists;
- rollback proves retail immutability;
- exact-head Windows + Ubuntu validation is green;
- issues/docs/code/evidence agree;
- no unresolved contradiction changes the accepted L1 scope.

## 4. Internal implementation verdict

The following are **not mandatory internal L1 blockers** for the current representative scope:

- NBZ STORE/raw-DEFLATE materialization;
- bounded central/local ZIP source handling;
- artifact snapshot binding;
- atomic/no-replace publication;
- first-gap volume read semantics;
- PAC/PNST sparse/empty/alias-preserving parsing;
- recursive expansion;
- same-level and nested size-changing reflow;
- verified immutable NBZ copy rebuild;
- next-volume overlay writer;
- product reopen/rematerialization;
- protected executable preflight;
- output isolation from the retail tree.

If a real-retail resource falls outside these supported domains, that becomes a new evidence-derived bounded gate rather than retroactively invalidating closed implementation work.

## 5. Materialization completion ordering / dependency bridge — bounded supporting reverse

Merged #228 corrects the old shorthand `fan-in/completion`: **no generic child/outstanding-work fan-in counter is currently evidenced**.

Canonical layer ownership remains:

- FileSlot exact byte-read mechanics may support L1;
- FileSlot/AsyncIO request ownership/scheduling/callback lifetime is L3;
- `0x1401B8CA0` is the L1/L3 materialization-success seam;
- LoadedResource states are L3.

Merged #230 proves normal `0x1401B8DC0` receives one u32 registry-relative context and writes state2. It does not receive raw transport status/error, byte count, FileSlot handle or child/outstanding-work metadata.

### Consequence

The success/error dependency must be resolved **before normal `0x1401B8DC0` dispatch**, or the queued completion must be suppressed/removed before execution.

FIFO insertion order alone is insufficient if a previous materialization job can submit asynchronous transport and retire immediately.

### Current focused state-machine question

```text
0x1402EF4D0 materialization job/submission
 -> lower whole-file/FileSlot transport
 -> terminal condition / job-retirement rule
 -> queued 0x1401B8DC0(record-context)
 -> state2
```

Current hypotheses include persistent scheduler polling, lower-callback-driven terminal state, a separate scheduler dependency gate, synchronous completion before success, or another directly evidenced mechanism.

No counter hypothesis is privileged.

### Historical Pass-90 anchors

Historical derivative evidence identified:

- `0x1400333E0` — candidate status/poll helper;
- `0x140033390` — candidate terminal load-state release/close helper.

These roles are **reacquisition targets**, not newly promoted canonical semantics.

### Revised raw-pass order

```text
1. 0x1402EF4D0 exact queued job identity/type and load-context consumer
2. 0x1402EF790 materialization-job persistence/re-poll/retirement
3. 0x1400333E0 pending/success/error domain
4. 0x140033390 terminal cleanup/release
5. 0x1400335A0 lower transport writes into that state
6. identify what blocks/suppresses normal B8DC0 on failed/incomplete transport
7. 0x1402EF460 higher scheduler clear/rollback
8. .lst child/recursive failure ordering
```

## 6. Cancellation/control boundary

`0x1402EF460` is retained as **pending scheduler-entry clear/rollback**. Do not label it OS `CancelIo`/AsyncIO cancellation without direct lower-I/O interaction.

The open question includes whether it can remove a queued normal completion and what happens to already-running FileSlot/ReadRequest work.

## 7. `.lst` remaining breadth

The `.lst` grammar/layout and `0x1401B85C0` in-place recursive synthesis are already strong.

Open breadth is child submission/failure/recursive ordering, temporary allocation/free/failure cleanup and real loose-list validation when that representation is activated.

Do not re-reverse grammar before the direct-resource completion dependency model is closed.

## 8. Other bounded non-blockers

These become mandatory only if the chosen acceptance scope depends on them:

- remaining ZIP initializer/seek exact error/lifetime breadth;
- exhaustive malformed/partial-read original error equivalence;
- binary AFS or original-runtime PACK absent direct supported-path evidence;
- Capcom offline writer equivalence.

## 9. Completion scope rule

`L1 COMPLETE` means the declared DMC3-HD resource materialization/authoring scope is validated end to end with real lineage through original-game consumption and rollback.

It does not mean every malformed input, hypothetical format, writer implementation or full L2/L3 lifecycle is equivalent.

## 10. Remaining critical path

```text
1. obtain/export one real retail selected member with exact provenance
2. classify its exact representation
3. perform one supported bounded edit/rebuild
4. generate next-contiguous NBZ and prove canonical rematerialization
5. preserve the product closure receipt
6. execute #209 original-game consumption + rollback
7. run final cross-stack audit
8. only then mark L1 100% / COMPLETE
```

While Level-E evidence is unavailable, supporting static reverse follows the focused terminal dependency pass above.

## 11. Environment boundary

The connected evidence surface can identify the protected distribution executable and the co-located retail `dmc3-0.nbz`, but the observed archive is approximately 960 MB and exceeds the current connected raw-transfer/materialization ceiling. No exact protected-process Level-E receipt is produced by this audit.

This is an external evidence/transport boundary, not justification for weakening L1 completion criteria.
