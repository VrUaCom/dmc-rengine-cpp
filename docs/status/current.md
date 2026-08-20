# Current Project Status

**Snapshot date:** 2026-08-20  
**Repository generation:** evidence-backed C++20 reconstruction platform  
**Version:** 0.2.0  
**Canonical snapshot base:** `main` at `4cf6b34258e95bc6fde19979036c82ba0104d270`  
**Latest canonical GDSpaces promotion:** PR #147  
**Overall status:** GDSpaces Layer 1 is the current execution priority; many bounded reverse/implementation slices are canonical, but no major subsystem has whole-game behavioral-equivalence proof.

## Canonical authority split

- GitHub `main` is implementation truth for reviewed, promoted code and documentation.
- Google Drive is research/artifact truth where newer reverse material exists before promotion.
- Active branches/PRs are branch truth only until merged.
- Synthetic tests prove bounded contracts, not original-game equivalence.

## GDSpaces layer model

Canonical boundaries are defined in [GDSpaces decompilation-layer classification](../gdspaces/decompilation-layer-classification.md).

- **L1 — Resource Materialization:** storage/container bytes -> exact materialized bytes -> nested extraction -> WorkingCopy -> rebuild/repack -> reopen/round-trip.
- **L2 — Resource Resolution:** logical request -> candidate construction -> normalization -> provider/volume selection -> exact resource identity.
- **L3 — Original Runtime/Lifecycle:** original FileSlot/scheduler/callback/LoadedResource/typed-postload/release/unload behavior beyond the minimum L1 byte-read contract.
- **V — Validation:** cross-cutting receipts/equivalence support, not a fourth layer.

## Hypothetical reverse-progress index

The authoritative operational scale is [GDSpaces reverse-progress scale](../gdspaces/reverse-progress-scale.md).

| Layer | Index | Status |
|---|---:|---|
| **L1 Resource Materialization** | **88%** | ACTIVE / NOT COMPLETE |
| **L2 Resource Resolution** | **94%** | HIGH / NOT COMPLETE |
| **L3 Original Runtime / Lifecycle** | **72%** | ADVANCED / NOT COMPLETE |
| **V Validation** | **60%** | SUPPORTING / NOT A LAYER |

These percentages are coverage indices, not equivalence probabilities. `99%` may still be NOT COMPLETE. `100% / COMPLETE` is allowed only when mandatory gates are zero or evidence-pruned, closing work is canonical in `main`, representative real-corpus receipts exist, required Windows+Ubuntu validation is green, and no architecture-changing contradiction remains.

## L1 canonical capability now in main

Strong canonical Layer-1 coverage includes:

- NBZ classic-ZIP indexing/source materialization;
- STORE and raw-DEFLATE materialization with CRC/size/budget validation;
- ByteProvenance and source/materialized coordinate separation;
- PAC and PNST structural parsing with sparse/empty/alias topology preservation;
- recursive PAC/PNST expansion;
- WorkingCopy and parser-gated edit enablement where required;
- same-size `layout-preserving-packed` PAC/PNST authoring;
- validated same-size nested PAC/PNST reintegration with alias-conflict arbitration;
- runtime-synth size-changing PAC/PNST authoring using the recovered `.lst` 64-byte layout;
- typed verified `RuntimeSynthResult -> ExactChildImage` authority for nested size-changing composition;
- deterministic STORE-only next-volume NBZ overlay authoring;
- canonical reopen/reparse of generated overlay output;
- authored-byte receipts kept distinct from original ByteProvenance.

Canonical writer/reference documents:

- [Runtime-synth PAC/PNST writer](../gdspaces/dmc3-runtime-synth-relative-slot-writer.md)
- [Nested PAC/PNST reintegration](../gdspaces/dmc3-nested-relative-slot-reintegration.md)
- [NBZ STORE overlay writer](../gdspaces/dmc3-nbz-store-overlay-writer.md)
- [Loose-container `.lst` path](../gdspaces/dmc3-loose-container-list.md)

## Mandatory L1 closure path

```text
L1 = 88%
    |
    +-- artifact/evidence gates
    |     +-- representative real .lst receipt
    |     +-- representative real PNST/raw-container receipt on current parser
    |     +-- representative real child <-> slot / intrinsic-byte authority
    |
    +-- code/serialization gates
          +-- raw retail-NBZ serialization metadata authority
          +-- metadata-preserving no-loss retail-NBZ repack tier

then

representative real size-changing edit
    -> bottom-up nested rebuild
    -> root PAC/PNST
    -> retail/overlay NBZ
    -> reopen
    -> canonical compare
    -> controlled original-game consumption receipt
    -> L1 100% / COMPLETE
```

### Artifact status

Current connected Drive metadata does not expose a raw `.lst` filename and does not currently expose the exact representative PNST file requested for a fresh current-parser receipt. Historical Phase 12-16 raw packages are documented as unavailable in Drive. These are artifact-availability gates, not reasons to redo already-strong parser reverse from scratch.

### Retail NBZ boundary

`NbzZipSource` is a materialization authority, not yet a lossless retail serialization authority. The current entry model does not preserve all source ZIP metadata needed for no-loss repack, including the complete raw central/local metadata envelope where required. STORE-overlay authoring and retail repack therefore remain separate capabilities.

The next no-loss tier must preserve or bind exact source serialization information such as local framing, central records, EOCD/comment data, version/time/attribute/extra/comment fields, and opaque local-region bytes where descriptors/padding/gaps may exist. It must not infer Capcom compressor/offline-builder equivalence.

## Scope corrections that remain canonical

- AFS/PACK do not block DMC3 HD L1 absent new direct raw/backend evidence.
- `.afs/` strings are logical namespace evidence, not proof of a binary AFS container backend.
- Stage Ops/ModViz semantics do not count toward L1 completion.
- Original resource factory/cache/refcount/async/unload/shutdown semantics beyond minimum materialization are L3.
- Descriptor identity, numeric Stage identity, and semantic gameplay identity remain distinct.
- `st001` is a regression/compatibility fixture, not the architecture's single Stage model.

## Other major project areas

### EXE / Recovered Game Source Tree

The project has verified executable acquisition and multiple bounded recovered-runtime slices. Full DMC3 decompilation, full resource lifecycle equivalence, progressive recompilation and a behaviorally equivalent rebuilt executable remain incomplete.

### Stage Ops / ModViz

Shared operational/domain state and editor ownership boundaries exist, but evidence-backed gameplay semantics for all camera/door/enemy/effect/event/runtime links are not complete. ModViz must remain a consumer/editor over Stage Ops rather than creating a second scene truth.

### Binary Inspector

Binary Inspector remains the byte/structure/evidence inspection authority. Revision/source lineage must stay synchronized with WorkingCopy and Stage Ops derived state.

### HITS / collision

A large bounded HITS implementation/reverse body exists. Whole original-builder/runtime equivalence is not claimed; remaining claims continue to require representative corpus and controlled runtime evidence.

## Current execution priority

1. Finish Layer-1 no-loss retail-NBZ serialization/repack authority without contaminating ordinary materialization ownership.
2. Reacquire missing representative `.lst`/PNST/raw-container artifacts where legally available.
3. Prove real child-to-slot/intrinsic-byte authority for representative size-changing nested resources.
4. Execute one real size-changing `materialize -> edit -> rebuild -> NBZ -> reopen -> compare` receipt.
5. Obtain a controlled original-game consumption receipt.
6. Promote `L1 = 100% / COMPLETE` only in the same canonical change that closes the final mandatory gates.
7. Then move primary execution accounting to the next layer while preserving L1 regressions.

## Navigation

- [Root project README](../../README.md)
- [Documentation index](../README.md)
- [GDSpaces reverse-progress scale](../gdspaces/reverse-progress-scale.md)
- [GDSpaces decompilation-layer classification](../gdspaces/decompilation-layer-classification.md)
- [Blockers](blockers.md)
- [Risk register](risks.md)
- [Phase map](phase-map.md)
- [Machine-readable status](canonical-status.json)

No statement in this document upgrades a bounded implementation or synthetic regression into whole-game equivalence.
