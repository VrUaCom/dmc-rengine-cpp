# GDSpaces Layer 1 — Final Pre-Level-E Audit

**Audit date:** 2026-08-25  
**Reconciled:** 2026-08-26 against `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`  
**Layer:** L1 — Resource Materialization  
**Verdict:** INTERNAL PRODUCT PATH CLOSED; L1 COMPLETE remains blocked by real-retail / original-game evidence.  
**EXE boundary authority:** `l1-exe-boundary-review-2026-08-26.md`

## 1. Purpose

This audit reconciles Layer 1 after the publication, artifact-stability, retail-acquisition, authoring, nested-reflow and protected-retail closure work merged through PR #213, and remains valid after the 2026-08-26 EXE boundary reconciliation.

It answers one question:

> Is there any remaining **internal GDSpaces implementation gap** that must be closed before a representative protected-DMC3 Level-E acceptance run can decide Layer-1 completion?

The answer on the current canonical head is **no known mandatory internal implementation gap** for the currently evidenced DMC3-HD L1 acceptance scope.

That does **not** mean L1 is already COMPLETE. The remaining mandatory gates are evidence executions against a real protected retail installation and the original game process.

The 2026-08-26 EXE review refines the reverse boundary but does not weaken this acceptance rule: byte transport/materialization through the resource-level `state 1 -> 2` handoff is L1; typed post-load, `state 2 -> 3` and lifecycle are L3.

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

Closed by the shared `publish_validated_file_no_replace` / `publish_bytes_no_replace` contract and subsequent CLI adoption.

Properties now required by canonical product seams:

- private staging ownership;
- complete write before visibility;
- optional staged validation before visibility;
- final no-replace publication;
- no `exists() -> ofstream` non-overwrite claim;
- no unconditional deletion of a path after publication ownership may have changed;
- repeated publication leaves the first artifact byte-exact.

Relevant promotion: #194 and consumers merged afterwards.

### L1-B — Artifact-stable retail member acquisition

**Status: CLOSED / CANONICAL**

`NbzZipArtifactSerializationBinder` plus `NbzZipArtifactMemberObserver` bind selected central metadata and selected stored/member bytes to the exact observed archive artifact identity.

Same-size archive mutation cannot inherit stale metadata and silently produce a valid member receipt.

STORE and raw-DEFLATE method 8 are both covered by focused regressions.

Relevant promotion: #195, #196, #197.

### L1-C — Direct-retail representative provenance

**Status: IMPLEMENTATION READY / REAL RECEIPT REQUIRED**

The command:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

already performs canonical resolver selection and emits artifact-bound provenance.

The implementation gate is closed. The mandatory remaining item is execution against a real protected DMC3 installation and preservation of its receipt.

`obj\em000.pac` remains a high-value request, not a hard-coded archive-member assumption. Another representative request may satisfy the gate if it has a deterministic authoring/consumer path and the receipt records the actual resolver winner.

### L1-D — Retail representation classification

**Status: REAL RECEIPT REQUIRED**

This cannot be completed from synthetic fixtures or transformed corpus alone.

After L1-C, the exact retail bytes must be classified as either:

- inside an already evidenced writer domain;
- another supported representation with its own evidence;
- outside current authoring authority, in which case a new bounded evidence gate is required.

No conversion may be inferred solely from DDS/TM2/runtime familiarity.

### L1-E — Bounded edit and bottom-up rebuild

**Status: PRODUCT IMPLEMENTATION CLOSED; REAL-RETAIL RECEIPT REQUIRED**

Canonical product authoring now includes:

- same-level size-changing PAC/PNST slot reflow;
- sparse/empty slot preservation;
- alias-partition preservation;
- byte-exact untouched physical spans;
- nested root-to-leaf slot-path authoring such as `0/2/1`;
- bottom-up reflow of every traversed parent;
- chained SHA receipts binding every parent changed span to the next nested output;
- atomic/no-replace output publication.

Relevant promotion: #201 and #213.

The remaining mandatory work is to run this path on the exact real-retail representation selected in L1-C/L1-D.

### L1-F — Next-volume NBZ publication and canonical reopen

**Status: PRODUCT IMPLEMENTATION CLOSED; REAL-RETAIL RECEIPT REQUIRED**

Canonical product behavior now includes:

- deterministic next-contiguous runtime volume selection;
- output-only authored NBZ generation;
- staged canonical `NbzZipSource` reopen verification;
- atomic/no-replace publication;
- canonical resolver re-mount with higher-volume precedence;
- exact rematerialized rebuilt-container comparison;
- exact authored target-child comparison in the closure workflow.

Relevant promotion: #188, #194, #198, #208.

### L1-G — Original DMC3 consumption

**Status: OPEN / EXTERNAL LEVEL-E**

This is the principal remaining completion blocker.

Issue #209 defines the required run:

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

A crash-free launch does not close this gate.

### L1-H — Final cross-stack acceptance audit

**Status: OPEN / DEPENDS ON REAL RECEIPTS**

This audit can become the completion audit only after L1-C through L1-G have evidence receipts from the same controlled acceptance lineage.

At that point the final audit must verify:

- exact executable authority;
- exact retail archive/member provenance;
- representation classification;
- authored replacement identity;
- rebuilt container identity;
- generated overlay identity;
- canonical higher-volume winner;
- exact rematerialized bytes;
- original consumer observation;
- rollback / retail immutability;
- exact-head Windows + Ubuntu CI on final canonical code;
- no unresolved contradiction that changes the claimed materialization architecture.

Only then may status become `L1 COMPLETE / 100%`.

## 4. Internal implementation verdict

The following are **not remaining mandatory internal L1 blockers** on this head:

- NBZ STORE materialization;
- NBZ raw-DEFLATE materialization;
- central/local bounded ZIP source handling at the supported product scope;
- artifact snapshot binding;
- publication race safety;
- first-gap volume read semantics;
- PAC structural parsing;
- PNST structural parsing/classification;
- sparse/empty relative-slot identity;
- same-level size-changing reflow;
- nested PAC/PNST child traversal;
- nested size-changing slot-path authoring;
- verified immutable NBZ copy rebuild;
- next-volume overlay writer;
- product reopen/rematerialization;
- product resolver composition required by the closure receipt;
- protected executable authority preflight;
- output isolation from the retail executable tree;
- type-0 physical-provider post-`0x0C` static final-open contract at the bounded recovered scope, promoted by #215.

If a new real-retail representation falls outside these supported domains, that becomes a **new evidence-derived bounded gate**, not permission to retroactively mark all current implementation work incomplete.

## 5. Bounded reverse gaps that do not block L1 unless activated by acceptance evidence

The canonical reverse matrix and corrected labels are maintained in `l1-exe-boundary-review-2026-08-26.md`.

### Materialization fan-in / completion semantics

This is now the highest-value open L1 EXE handoff seam.

`0x1400335A0` is a transport/whole-file completion callback. `0x1401B8DC0` is a higher resource scheduler/materialization completion handoff registered through `0x1402EF580` and publishes the normal `state 1 -> 2` transition.

Open exact breadth includes:

- outstanding child/direct submission aggregation;
- parent completion condition;
- nested `.lst` child participation in completion;
- one-child-failure behavior;
- partial destination lifetime on failure;
- transport-failure mapping into resource-level completion/failure;
- exact conditions that suppress state2 publication.

These are bounded reverse details. They become a mandatory blocker only if a concrete acceptance claim depends on unresolved behavior rather than the successful representative path.

### `.lst` temporary-buffer lifetime/error breadth

The `.lst` text is known to be loaded synchronously into aligned temporary storage before bounded parsing, but there is no direct evidence equating that loader with the synchronous-style wrapper around `0x1402EF920`.

Allocation/free identity, failure cleanup, malformed/truncated propagation and original recursion failure behavior remain open. A real `.lst` corpus receipt is mandatory only if L1 completion is claimed for real loose-list consumption or if the representative acceptance path selects `.lst`.

### FileSlot / ReadRequest error breadth

The transport architecture, 100×`0x20` FileSlot pool and `ReadRequestV2` callback ABI are strong. Exact partial-read/error/cancellation breadth remains bounded and should be closed when a compatibility claim requires it.

### Complete ZIP initializer / compressed-seek body breadth

`0x140328540` and `0x140328FE0` retain exact-body/error-state details, but their architecture is already strong: lazy realization is known and compressed seek is reset + reinflate/discard replay. They are no longer automatic first-priority reverse targets.

### Exhaustive malformed ZIP/error equivalence

Not required for a successful representative materialization receipt. It remains bounded reverse breadth and must be completed before any claim of exhaustive malformed-input original equivalence.

### Binary AFS / PACK

Binary AFS is not evidenced as a DMC3-HD binary backend on this path. Historical PACK parser evidence does not establish original DMC3 runtime PACK authority. Both remain evidence-gated non-blockers absent a direct dependency.

### Capcom offline writer equivalence

Not an L1 completion requirement. DMC Rengine writers are product authoring implementations constrained by evidenced read/runtime behavior and explicit receipts.

## 6. EXE boundary corrections that are now canonical

Do not use the following superseded shorthand:

- `0x1402EF4D0` as a packed-file reader, exact-path resolver or final provider open. Safe label: **resource materialization submission/scheduling wrapper**.
- `0x1401B8DC0` as a raw I/O callback. Raw transport completion is represented by callbacks such as `0x1400335A0`; `0x1401B8DC0` is the resource-level scheduler/materialization handoff.
- `.lst synchronous temporary load == 0x1402EF920`. The two synchronous behaviors are independently evidenced but not directly tied.
- FileSlot/AsyncIO as wholly L3. The byte-transport portion is L1; lifecycle/ownership/cancellation/reset breadth remains L3.
- type-0 physical final-open semantics as still open after #215.

## 7. Completion scope rule

`L1 COMPLETE` means the evidenced DMC3-HD Layer-1 materialization architecture and the promoted supported authoring path are validated end to end at the declared scope.

It does **not** mean:

- every malformed input behaves bit-for-bit like Capcom;
- every hypothetical binary format is supported;
- every resource family has a bespoke editor;
- Capcom's offline authoring tools were reconstructed;
- complete L2/L3 closure is achieved.

Those claims have their own gates.

## 8. Remaining critical path

The mandatory acceptance work order remains:

```text
1. run direct-retail acquisition on protected DMC3
2. preserve exact provenance receipt
3. classify the selected retail representation
4. perform one bounded real edit through the supported top-level or nested slot-path writer
5. generate next-contiguous NBZ through the closure workflow
6. require canonical resolver/reopen/rematerialization success
7. preserve product closure receipt
8. execute #209 original-game consumption + rollback
9. run final cross-stack audit
10. only then mark L1 100% / COMPLETE
```

While this external sequence is blocked, supporting reverse follows:

```text
materialization fan-in/completion
 -> transport-to-resource error mapping
 -> .lst temp allocation/free/failure cleanup
 -> acceptance-activated FileSlot/error breadth
 -> acceptance-activated ZIP exact-body breadth
```

No synthetic-only feature is allowed to displace the real acceptance path unless evidence reveals a concrete missing dependency.

## 9. Environment boundary

The current connected automation environment does not expose all exact raw protected-install artifacts required to execute the protected-install Level-E run here.

Current main does contain guarded canonical-analysis EXE window acquisition infrastructure. Static reverse reacquisition must use that authority/plan rather than ad hoc unbound byte ranges.

This is an **external evidence blocker**, not justification for changing `L1 COMPLETE` criteria.
