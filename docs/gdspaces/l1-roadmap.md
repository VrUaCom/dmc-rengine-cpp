# GDSpaces Layer 1 Roadmap

**Status:** **INCOMPLETE / NOT 100% — original-runtime reverse + naming validation + real acceptance open**  
**Snapshot date:** 2026-08-31  
**Reconciliation base:** `main@08231d669666d2bdfefe3d74f123600ca365cc3d`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes  
**Primary tracking:** #100, #182, #209; historical reverse checkpoint #258; landed naming/type checkpoint #268

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

The old wording `INTERNAL PRODUCT PATH CLOSED` is superseded. Product capabilities are advanced and the main materialization/authoring spine is mature, but fresh canonical-EXE reverse proved that the original byte/result/failure path was not exhaustively recovered. Naming and scoped runtime type evidence are now main-landed, while real-corpus/replay validation remains open. L1 must not be reported as `COMPLETE / 100%` until the mandatory reverse, naming and real acceptance gates below are closed.

## 1. Canonical L1 question and boundary

L1 answers:

> Given an already selected resource identity, can DMC Rengine obtain the exact bytes and representation, preserve physical identity/provenance, expand and name nested children without authority laundering, safely author a supported edit, rebuild the required container/archive stack, reopen/rematerialize the authored result, and prove that the original DMC3 consumer used those bytes?

Canonical ownership cut:

```text
[L2] exact selected provider / volume / member identity
 -> [L1] materialized size / capacity / allocation
 -> [L1] exact acquisition / decompression / transfer
 -> [L1] exact destination bytes + ByteProvenance
 -> [L1] PAC/PNST topology + physical child identity
 -> [L1] naming evidence reconciliation / presentation
 -> [L1] bounded edit + bottom-up rebuild
 -> [L1] authored NBZ publication + reopen/rematerialization
 -> [L1] native terminal byte/result semantics
 ===== END L1 BYTE / MATERIALIZATION AUTHORITY =====
 -> [L3] callback/lifecycle publication and original consumer visibility
```

A helper can participate in more than one layer. Classification follows concrete semantics, not function ownership by name.

## 2. Current product capabilities

Current `main` already contains, at bounded/evidenced scope:

- classic NBZ/ZIP indexing and bounded member acquisition;
- STORE and raw-DEFLATE method-8 materialization;
- CRC/size/SHA and explicit `ByteProvenance`;
- artifact-bound archive/member observations;
- numbered-volume first-gap/runtime namespace behavior;
- canonical resolver composition and higher-volume precedence;
- staged atomic/no-replace publication;
- resolver-selected direct-retail acquisition tooling;
- PAC/PNST sparse/empty/alias-preserving parsing and recursive expansion;
- same-size and size-changing relative-slot authoring;
- nested root-to-leaf slot-path authoring with bottom-up parent rebuild;
- byte-exact untouched-span/sibling preservation;
- verified immutable NBZ copy rebuild;
- deterministic next-contiguous STORE NBZ overlay authoring;
- staged NBZ reopen/rematerialization verification;
- protected-distribution preflight and product closure orchestration;
- runtime-synth `.lst` direct-child `0x800` transfer extents vs recursive `0x40` complete-image structural extents (#255);
- original synthesized-image zero initialization/padding model;
- canonical L1 naming architecture landed in `main`;
- exact `.index` rule: extraction entry `N` binds to extracted ordinal `N`, i.e. the N-th populated physical payload, not physical slot N;
- independent naming authority domains for external `.index`, embedded aliases, enclosing-container stored names, semantic-format evidence, display projection and export projection;
- exact `ResourceId` runtime-to-L1 naming bridge with fail-closed physical-identity equality;
- no-`.index` deterministic derived display as presentation only, without manufacturing historical extraction authority;
- scoped runtime type evidence separated into independent original-code paths rather than one global magic detector: three-byte registry probe, PAC/PNST container dispatcher, and four-byte family-mask classifier;
- explicit provenance separation between structural, generic magic and instruction-backed runtime type evidence.

The latest direct runtime-type corrections are on `main@08231d6`; its exact-head CI is currently pending and must be green before this reconciliation is promoted.

These capabilities prove product maturity. They do **not** prove exhaustive original-runtime equivalence or original-game consumption.

## 3. Mandatory reverse gate — L1-R

**STATUS: OPEN / substantially narrowed.**

The 2026-08-27/28 raw canonical-EXE pass corrected the old completion claim.

### Confirmed / corrected original behavior

- cached materialized-size source is now bounded: physical slots use low-32-bit `GetFileSize`; NBZ uses central-directory uncompressed size;
- whole-file direct transfer extent is derived through `0x1400333C0 -> 0x1402EF620` in 32-bit arithmetic;
- the original arithmetic is wrap-prone and mathematical `ceil(size/0x800)` is valid only in the safe positive domain;
- lower reads may terminate at EOF/no-progress/error before the cached/planned size;
- `0x1402EF4D0` is type-2 queue admission, not the byte-producing body;
- `0x1402EF790` consumes admitted materialization jobs;
- `0x1401B85C0` ignores direct child enqueue failures and recursive-writer failures;
- `0x1401B8CA0` has branch-dependent boolean meaning;
- `0x1401B84E0` ignores failure of the final type-3 completion enqueue `0x1402EF580`;
- no single upstream writer/setup boolean proves that every expected child job was admitted or completed;
- `.lst` scan/token ceilings (`0x1FC0`, `0x100`) are original bounds, not clean original error enums;
- for admitted jobs on the canonical static normal path, type-2 work and the normal type-3 callback share one lane/FIFO;
- status `2` remains pending, status `4` retries the same type-2 job without retirement, status `3` retires it and permits the later callback to become current;
- original status `3` does not independently require actual transferred bytes to equal planned bytes, so a short no-error transfer can permit later normal lifecycle completion;
- cancellation queued-work suppression belongs to L3; the static L1/L3 ownership seam is now bounded.

Canonical evidence:

- `l1-writer-failure-width-reconciliation-2026-08-28.md`;
- `l1-terminal-l3-completion-seam-2026-08-28.md`;
- `../../data/reverse/dmc3-l1-writer-failure-width-2026-08-28.v1.json`;
- `../../data/reverse/dmc3-l1-terminal-l3-completion-seam-2026-08-28.v1.json`.

### Reverse still open

Before an exhaustive original-L1 claim:

1. exact recursive `.lst` cycle/depth behavior;
2. recursive allocation/free lifetime and residual allocator/backend failure branches;
3. representative real `.lst` corpus receipt if real loose-list equivalence is claimed;
4. final contradiction sweep across L1 byte/materialization functions and the bounded L1/L3 seam.

Dynamic current-slot cancellation/concurrency, broader transition/reset/shutdown behavior and typed-ready lifecycle remain L3 breadth unless a concrete L1 acceptance run activates them.

## 4. Naming / identity gate — L1-N

**STATUS: MAIN-LANDED / VALIDATION OPEN.**

The #251-#262 naming stack, semantically retained parts of #254, and the #268 derived-display/runtime-type correction are now integrated into `main`. The historical PRs remain evidence/review checkpoints rather than separate implementation authorities.

The canonical model keeps these authorities separate:

```text
ResourceId
physical_slot_index
extracted_ordinal
external .index raw/normalized label + folder marker
embedded_alias
enclosing_container_stored_name
semantic_format + sealed provenance
canonical_display_name
legacy extraction representation
safe host export projection
```

Canonical `.index` rule:

```text
.index entry N
 == extracted ordinal N
 == N-th populated payload in physical slot order
```

It is not a direct physical-slot mapping.

`RuntimeNamingBridge` links runtime resolution to L1 naming only by exact complete `ResourceId` equality. Filename/display/alias/semantic-name fallback joins are forbidden.

The no-`.index` fallback now derives only a deterministic presentation name from physical container identity + populated ordinal + independently evidenced semantic extension. It remains synthetic presentation, not historical extraction evidence or write authority.

Runtime type evidence is now explicitly scoped rather than collapsed into one global detector:

- registry/resource-registration three-byte content probe;
- PAC/PNST materialized-child dispatcher;
- four-byte higher-level family-mask classifier.

Therefore the earlier global shorthand “exactly five runtime tags” is superseded; only the narrower three-byte registry probe has that five-tag boundary.

Still open before naming completion:

- representative real retained effect-corpus replay/reconciliation;
- global naming coverage/collision report across representative PAC/PNST families;
- exact historical `.index` producer/extractor lineage recovered or explicitly bounded unresolved;
- real-retail selected runtime identity -> exact L1 parent identity receipt;
- replay/export/reopen validation for retained historical extraction representations;
- final naming/type-evidence contradiction audit after the direct instruction-level corrections now on main.

## 5. Product / real-acceptance gates

### L1-A — publication integrity

**CLOSED / CANONICAL.**

Shared staged atomic/no-replace publication is integrated.

### L1-B — artifact-stable member acquisition

**CLOSED / CANONICAL at product scope.**

Archive/member identity and provenance are bound to the successfully materialized artifact.

### L1-C — direct-retail representative provenance

**IMPLEMENTATION AVAILABLE / REAL RECEIPT OPEN.**

The request is authority. Record the actual resolver winner; do not predeclare an archive/member path.

### L1-D — exact retail representation classification

**REAL RECEIPT OPEN.**

Classify the exact bytes from L1-C and use only a writer whose representation domain is evidenced.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT CAPABILITY ADVANCED / REAL-RETAIL RECEIPT OPEN.**

PAC/PNST same-size, size-changing and nested authoring exist. The exact retail representation determines whether that writer is valid for the acceptance resource.

### L1-F — next-volume publication + canonical reopen

**PRODUCT CAPABILITY AVAILABLE / REAL-RETAIL RECEIPT OPEN.**

Required chain:

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged canonical reopen
 -> higher-volume resolver winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### L1-G — original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E / FINAL MATERIALIZATION ACCEPTANCE.**

Canonical tracking: #209. A crash-free launch is insufficient. The receipt must attribute a deterministic original-game consumer effect to the authored bytes and prove rollback / retail immutability.

### L1-H — final cross-stack audit

**OPEN.**

Requires L1-R, applicable L1-N gates, real acquisition/classification/edit/rebuild/rematerialization receipts, Level-E consumption + rollback, exact-head Windows+Ubuntu validation, synchronized canonical docs/issues and no unresolved contradiction changing the supported scope.

Only then may the project state **L1 = COMPLETE / 100%**.

## 6. Original behavior vs product safety

GDSpaces is not required to reproduce unsafe original defects.

| Original runtime evidence | GDSpaces product rule |
| --- | --- |
| 32-bit size/offset wrap possible | checked overflow, fail closed |
| writer may ignore child enqueue failure | successful product receipt must not launder rejected work |
| completion enqueue result may be ignored | product completion authority remains explicit |
| scan/token bounds lack clean original error enums | explicit fail-closed diagnostics |
| original short status-3 transfer may permit lifecycle completion | exact-byte product receipts validate their declared exactness contract |

Original behavior is reverse truth. Product hardening is product truth. Neither may be mislabeled as the other.

## 7. Current work order

```text
1. land canonical status/evidence reconciliation for the confirmed #258 findings on current main
2. finish residual recursive .lst + allocator/backend reverse
3. run final original-L1 contradiction sweep
4. finish naming/type-evidence real-corpus, producer-lineage and replay validation
5. obtain representative real-retail selected identity + acquisition provenance
6. classify exact retail representation
7. perform one bounded real edit + rebuild + canonical rematerialization
8. execute #209 original-game consumption + rollback
9. final cross-stack audit
10. only then mark L1 COMPLETE / 100%
```

No broad L2/L3/tooling feature should displace this sequence unless it directly closes one of these dependencies.

## 8. Evidence-gated freezes / non-blockers

Unless a real dependency activates them:

- `.afs/` strings are logical namespace evidence, not proof of a binary AFS backend;
- the Web DMC Rengine PACK parser is product history, not original DMC3 runtime authority;
- Capcom offline packer equivalence is not required for safe DMC Rengine authoring;
- Stage Ops / ModViz are downstream consumers and do not define L1 truth;
- exhaustive malformed-input parity is separate breadth unless the acceptance scope explicitly claims it.

## 9. Completion rule

Percentage estimates are planning aids only. Mandatory evidence gates are the completion authority. A green build, a parser, a writer, a synthetic round-trip or a successful game launch without attributable consumption evidence cannot independently mark L1 complete.
