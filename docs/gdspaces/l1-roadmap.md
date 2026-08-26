# GDSpaces Layer 1 Roadmap

**Status:** INTERNAL PRODUCT PATH CLOSED / EXTERNAL ACCEPTANCE OPEN  
**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Primary tracking:** #100, #182, #209  
**Final pre-Level-E audit:** `l1-final-audit-2026-08-25.md`  
**Materialization completion authority:** `materialization-completion-boundary-pass-2026-08-26.md` + `materialization-completion-dependency-pass2-2026-08-26.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

L1 answers:

> Can DMC Rengine obtain the exact bytes selected for a DMC3 resource, preserve their provenance, safely edit an evidenced representation, rebuild the required nested container/archive stack, reopen/rematerialize the authored result, and prove that the original protected game consumed those authored bytes?

Synthetic tests, a working resolver, a valid parser or a crash-free game launch are not sufficient for `L1 COMPLETE`.

## 1. Canonical L1 boundary

```text
physical source bytes
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
```

L2 may supply exact selected identity and L3 may supply original consumer/lifecycle evidence. Their broader completion is not required for L1 unless a concrete L1 acceptance path activates one of their unresolved boundaries.

FileSlot is classified by behavior, not wholesale. Exact byte-read mechanics can be L1 support; original request ownership/scheduler/callback lifecycle and LoadedResource states remain L3. `0x1401B8CA0` is the explicit L1/L3 materialization-success seam.

## 2. Canonical product capabilities on current main

Current `main` includes:

- classic NBZ/ZIP indexing and bounded central/member acquisition;
- STORE and raw-DEFLATE method-8 materialization;
- CRC, size, SHA and explicit `ByteProvenance`;
- artifact-bound serialization/member observation preventing stale snapshot receipts;
- recovered contiguous numbered-volume / first-gap runtime namespace;
- canonical runtime resolver composition and higher-volume precedence;
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
- guarded canonical-analysis EXE acquisition packet plans;
- Windows + Ubuntu CI for promoted product/documentation paths.

These close the known mandatory **internal implementation** work for the current representative DMC3-HD L1 acceptance scope.

## 3. Gate status

### L1-A — publication integrity

**CLOSED / CANONICAL**

Closed by #194 and subsequent authoring/acquisition seams using the shared final no-replace publication contract.

### L1-B — artifact-stable retail acquisition

**CLOSED / CANONICAL**

Closed by #195–#197 and consumed by the direct-retail acquisition command.

### L1-C — direct-retail representative provenance

**IMPLEMENTATION CLOSED / REAL RECEIPT OPEN**

Use:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The request, not a pre-guessed archive path, is the input authority. The receipt must preserve the actual resolver winner, selected volume/archive identity, central-entry metadata, materialized SHA/size and byte transform.

`obj\em000.pac` remains a high-value target but is not mandatory if another representative resource gives a stronger deterministic authoring/consumer receipt.

Connected Drive evidence locates the protected distribution executable and co-located `data/dmc3/dmc3-0.nbz`. The observed archive is 960,358,951 bytes. The current connected raw-transfer/materialization channel cannot ingest that full file because of the observed 268,435,456-byte ceiling. This is a transport boundary, not artifact absence.

### L1-D — retail representation classification

**REAL RECEIPT OPEN**

Classify the exact bytes obtained in L1-C. Only use an existing writer if the observed retail representation is inside its evidenced domain.

If not, stop and create a new bounded evidence gate; do not force the bytes through a convenient writer.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT IMPLEMENTATION CLOSED / REAL-RETAIL RECEIPT OPEN**

Current writers support top-level and nested PAC/PNST slot paths, size-changing reflow, sparse/alias preservation and exact untouched-sibling validation.

Relevant promotion includes #201 and #213.

### L1-F — next-volume publication + canonical reopen

**PRODUCT IMPLEMENTATION CLOSED / REAL-RETAIL RECEIPT OPEN**

Current product path supports:

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged canonical reopen
 -> atomic/no-replace publication
 -> RuntimeResourceResolver higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

`verify-dmc3-l1-authoring` composes the protected preflight, retail acquisition, top-level PAC/PNST authoring, next-volume overlay and resolver/rematerialization checks into one product closure receipt. Nested authoring is canonical through `rebuild-relative-slot-path`.

### L1-G — original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E / FINAL MATERIALIZATION ACCEPTANCE**

Canonical tracking: #209.

Required controlled run:

```text
product closure receipt
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

A crash-free launch alone fails this gate.

### L1-H — final cross-stack audit

**OPEN / DEPENDS ON L1-C..G REAL RECEIPTS**

Before `L1 COMPLETE`:

- exact executable authority is recorded;
- direct-retail archive/member provenance exists;
- representation classification is explicit;
- real authored replacement/rebuild receipt exists;
- generated overlay identity is exact;
- canonical resolver/rematerialization succeeds on that real artifact;
- original DMC3 consumer observation exists;
- rollback proves retail immutability;
- exact-head Windows + Ubuntu CI is green on final canonical code/docs;
- #100, #182, #209 and current status agree;
- no unresolved contradiction changes the claimed L1 architecture or accepted representation scope.

Only then may the project state **L1 = 100% / COMPLETE**.

## 4. Commands on the closure path

```text
preflight-dmc3-game-test <exe-dir>
extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>
rebuild-relative-slot-path <container-file> <slot/path> <replacement-file> <output-file>
build-nbz-copy <source-nbz> <central-index> <replacement-file> <output-nbz>
build-dmc3-overlay <exe-dir> <game-request> <authored-file> <output-dir>
verify-dmc3-l1-authoring <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>
```

The commands are product authoring/validation seams. They do not claim to reproduce Capcom's offline writer implementation.

## 5. Supporting EXE reverse boundaries

Do **not** restart these without contradictory direct evidence:

- numbered-volume bootstrap / first-gap behavior;
- generic basename candidate construction and archive-first/physical-second attempt order;
- archive normalized lookup/index behavior;
- bounded whole-file caller-owned-destination transfer family;
- central FileSlot / ReadRequest transport architecture;
- ZIP stored-vs-inflated path and raw-DEFLATE core behavior;
- PAC/PNST recursive typed traversal;
- primary `.lst` packed-first selection/synthesis structure;
- merged #230 normal `0x1401B8DC0` one-u32 callback/context ABI.

### Materialization completion ordering / dependency bridge

Merged #228/#230/#242 replace broad `fan-in/completion` wording with a narrower evidence-backed target.

No generic original outstanding-child / fan-in counter is currently established.

Normal `0x1401B8DC0` receives exactly one u32 registry-relative context and no raw transport status/error, bytesRead, FileSlot handle or child/outstanding-work metadata. Therefore success/error eligibility must already be resolved before normal B8DC0 dispatch, or the queued completion must be suppressed/removed.

FIFO insertion order alone is insufficient if an earlier materialization scheduler job can submit asynchronous work and retire before lower transport becomes terminal.

Current exact-byte priority:

```text
1. 0x1402EF4D0 — queued materialization job identity/type, callees, inherited load-context consumer
2. relevant 0x1402EF790 case — persistence/re-poll/terminal retirement
3. 0x1400333E0 — historical status/poll anchor
4. 0x140033390 — historical terminal release/close anchor
5. 0x1400335A0 — lower transport status/error writes
6. determine what blocks/suppresses normal 0x1401B8DC0 on failed/incomplete transport
7. 0x1402EF460 — pending higher scheduler clear/rollback
8. .lst child/recursive failure ordering after the direct-resource mechanism is closed
```

`0x1400333E0` and `0x140033390` remain **reacquisition hypotheses** until fresh canonical `e454...` bytes confirm their exact roles. `0x1402EF460` is not promoted as OS AsyncIO cancellation.

### Other bounded reverse breadth

Still-open reverse breadth is activated only when a completion claim depends on it:

1. materialization terminal dependency/error-to-completion-suppression details above;
2. `.lst` child/recursive failure and temporary cleanup behavior when a real loose-list path activates it;
3. complete ZIP stream initializer `0x140328540` body/lifetime/error breadth when required;
4. complete compressed seek/reset `0x140328FE0` error breadth when required;
5. exhaustive malformed/partial-read error-code equivalence.

These bounded gaps do not automatically block a representative packed-NBZ/PAC/PNST L1 Level-E receipt.

## 6. Explicit non-blockers / freezes

- **Binary AFS:** `.afs/` strings are evidenced logical namespaces, not a binary-backend proof.
- **PACK:** historical product parsing is not original DMC3 runtime authority absent a direct dependency.
- **Capcom offline writer equivalence:** not required for L1 product authoring completion.
- **Stage Ops / ModViz:** downstream consumers; they do not define L1 truth.
- **Exhaustive malformed-input equivalence:** remains separate reverse breadth unless the claimed acceptance scope explicitly includes it.
- **Real `.lst` corpus:** mandatory only for a claim covering real loose-list consumption or when the representative acceptance path actually selects `.lst`.

## 7. Remaining work order

No new synthetic-only feature may displace this path unless real evidence reveals a concrete missing dependency.

```text
1. export/acquire one real retail selected member with exact provenance
2. classify exact retail representation
3. perform one bounded real edit through the supported authoring domain
4. generate next-contiguous NBZ and require canonical reopen/resolver/rematerialization
5. preserve product closure receipt
6. execute #209 original-game consumption + rollback
7. run final L1 audit
8. mark L1 100% / COMPLETE only if every mandatory receipt is valid
```

While external Level-E is blocked, supporting static reverse follows the materialization-completion order in section 5.

## 8. Environment boundary

Connected Drive evidence exposes the protected distribution executable and the co-located retail `dmc3-0.nbz`, but the observed archive is 960,358,951 bytes while the current raw-transfer/materialization ceiling is 268,435,456 bytes.

A range/member export path, local operator extraction or another exact artifact surface is required for the real L1-C receipt. This external evidence boundary does not justify substituting synthetic CI for L1-G.

## 9. Documentation synchronization

When L1 evidence or reverse boundaries change, synchronize:

- this roadmap;
- `l1-final-audit-2026-08-25.md` or its completion successor;
- `materialization-completion-boundary-pass-2026-08-26.md` and Pass 2 when the completion bridge changes;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- the EXE acquisition packet/runbook;
- issues #100, #182 and #209.

Percentage estimates are planning aids only; mandatory gates remain the completion authority.
