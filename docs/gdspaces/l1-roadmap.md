# GDSpaces Layer 1 Roadmap

**Status:** INTERNAL PRODUCT PATH CLOSED / EXTERNAL ACCEPTANCE OPEN  
**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`  
**Primary tracking:** #100, #182, #209  
**Final pre-Level-E audit:** `l1-final-audit-2026-08-25.md`  
**Canonical EXE boundary review:** `l1-exe-boundary-review-2026-08-26.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

L1 answers:

> Can DMC Rengine obtain the exact bytes selected for a DMC3 resource, preserve their provenance, safely edit an evidenced representation, rebuild the required nested container/archive stack, reopen/rematerialize the authored result, and prove that the original protected game consumed those authored bytes?

Synthetic tests, a working resolver, a valid parser or a crash-free game launch are not sufficient for `L1 COMPLETE`.

## 1. Canonical L1 boundary

Product acceptance chain:

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

Original EXE review boundary:

```text
L2 selected logical/provider/member identity
 -> L1 materialization submission
 -> L1 whole-file/FileSlot transport
 -> L1 physical backend OR NBZ member backend
 -> L1 STORE direct OR raw-DEFLATE materialization
 -> L1 caller-owned destination bytes
 -> L1 packed representation OR .lst synthesis
 -> L1 resource-level completion/fan-in
 -> L1 state 1 -> 2
 ===== END L1 =====
 -> L3 typed post-load / state 2 -> 3 / consumer lifecycle
```

`FileSlot`/AsyncIO therefore participates in L1 where it transports the selected bytes. Pool ownership, cancellation/release/reset policy and typed-ready lifecycle remain L3. `0x1401B84E0` is explicitly cross-layer rather than wholly owned by one layer.

L2 may supply exact selected identity and L3 may supply original consumer/lifecycle evidence. Their broader completion is not required for L1 unless a concrete L1 acceptance path activates one of their unresolved boundaries.

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
- recovered/promoted type-0 physical-provider post-`0x0C` boundary through #215;
- guarded canonical-analysis EXE window acquisition packet;
- protected-runtime RVA mapping acquisition tooling promoted through #219, while the real original-process mapping receipt remains open;
- Windows + Ubuntu CI for promoted product paths.

These close the known mandatory **internal implementation** work for the current representative DMC3-HD L1 acceptance scope.

## 3. Gate status

### L1-A — publication integrity

**CLOSED / CANONICAL**

Closed by #194 and all subsequent authoring/acquisition seams using the shared final no-replace publication contract.

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

`verify-dmc3-l1-authoring` composes the protected preflight, retail acquisition, top-level PAC/PNST authoring, next-volume overlay and resolver/rematerialization checks into one product closure receipt. Nested authoring is also canonical through `rebuild-relative-slot-path`; closure orchestration may be widened to slot paths as a usability refinement, but this is not a correctness prerequisite for a representative acceptance resource that fits the current closure command.

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

The canonical detailed review is [L1 EXE Boundary Review — 2026-08-26](l1-exe-boundary-review-2026-08-26.md).

Do **not** restart these without contradictory direct evidence:

- numbered-volume bootstrap / first-gap behavior;
- generic basename candidate construction and archive-first/physical-second attempt order;
- archive normalized lookup/index behavior;
- type-0 post-`0x0C` physical final-open static contract promoted by #215;
- whole-file caller-owned-destination transfer family `0x1400333F0/3C0/500/5A0`;
- bounded FileSlot / ReadRequest transport architecture;
- ZIP EOCD/central/member identity path;
- `ZipEntryRead 0x140328F50` STORE-vs-compressed split;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior;
- compressed seek reset+replay architecture and raw seek architecture;
- ZIP member teardown ownership architecture;
- primary `.lst` packed-first grammar/layout/recursive synthesis structure.

Important corrected labels:

- `0x1402EF4D0` = **resource materialization submission/scheduling wrapper**, not proven exact-path resolver, final provider open or raw file reader;
- `0x1400335A0` = transport/whole-file completion callback;
- `0x1401B8DC0` = resource scheduler/materialization completion handoff to state 2, **not** a raw I/O callback;
- `.lst` synchronous temporary-text acquisition is **not** proven to be the synchronous-style wrapper around `0x1402EF920`.

### Current reverse priority when evidence is available

1. **materialization fan-in/completion semantics** between submitted child/direct work and `0x1401B8DC0` state-2 publication;
2. **transport failure -> resource scheduler/materialization failure mapping**;
3. **`.lst` temporary allocation/free identity and failure cleanup**;
4. `.lst` malformed/truncated/recursion failure propagation if real loose-list acceptance activates it;
5. FileSlot/ReadRequest partial-read/error/cancellation breadth where required by a compatibility claim;
6. complete `0x140328540` and `0x140328FE0` exact-body/error breadth only when a concrete acceptance claim requires it.

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
1. run direct-retail acquisition on a protected DMC3 installation
2. preserve acquisition/provenance receipt
3. classify exact retail representation
4. perform one bounded real edit through the supported authoring domain
5. generate next-contiguous NBZ and require canonical reopen/resolver/rematerialization
6. preserve product closure receipt
7. execute #209 original-game consumption + rollback
8. run final L1 audit
9. mark L1 100% / COMPLETE only if every mandatory receipt is valid
```

While external Level-E is blocked, supporting static reverse follows the priority in section 5 and must remain bounded to evidence-backed seams.

## 8. Environment boundary

The connected automation environment currently does not expose all exact raw protected-install artifacts needed to execute the protected-install Level-E run here.

Current main does expose guarded canonical-analysis EXE window acquisition infrastructure, so static EXE targets should be reacquired through the packet plan rather than by ad hoc range extraction.

This external evidence boundary does not justify substituting synthetic CI for L1-G.

## 9. Documentation synchronization

When L1 evidence or reverse boundaries change, synchronize:

- this roadmap;
- `l1-exe-boundary-review-2026-08-26.md`;
- `l1-final-audit-2026-08-25.md` or its completion successor;
- `decompilation-layer-classification.md`;
- `dmc3-loose-container-list.md` when `.lst` boundaries change;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- `docs/reverse/gdspaces-blocked-window-acquisition.md`;
- `data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json`;
- issues #100, #182 and #209.

Percentage estimates are planning aids only; mandatory gates remain the completion authority.
