# Current Project Status

**Snapshot date:** 2026-08-31  
**Canonical implementation synchronized through:** `main@d358a2e69a98b13d36d42b594c353afd6546ffb8`  
**Active L1 reconciliation:** PR #269 / `ada/l1-status-reconcile-20260831`  
**Latest L1 naming/type checkpoints:** #268 landed by fast-forward; subsequent instruction-level type-family corrections are also synchronized into this branch  
**Primary execution program:** GDSpaces Layer 1 original-materialization reverse + naming/type evidence reconciliation + real acceptance  
**Overall status:** **L1 INCOMPLETE / NOT 100%; L2 INCOMPLETE; L3 INCOMPLETE.** Product capabilities are advanced, but no layer may be promoted to complete from implementation or synthetic CI alone. Exact-head CI for the final #269 reconciliation head is required before promotion.

## Authority split

- GitHub `main` is canonical implemented/product truth.
- Canonical instruction-reverse executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.
- Reverse claims are artifact/range/scope bound.
- Synthetic/public CI proves bounded product behavior only.
- Original-game equivalence requires original-process evidence.
- Product safety/hardening must not be mislabeled as recovered original behavior.
- Unsafe original implementation behavior must not be copied into GDSpaces merely for literal parity.

## GDSpaces layer model

- **L1 — Resource Materialization:** selected-resource size/capacity, allocation, exact bytes, transfer/decompression, nested topology, physical child identity, naming/type evidence reconciliation, bounded authoring/rebuild/repack and exact reopen/rematerialization.
- **L2 — Resource Resolution:** logical request, candidates, normalization, provider/volume/source/member selection and exact selected identity.
- **L3 — Original Runtime/Lifecycle:** queue/callback ownership, LoadedResource state publication, typed post-load, ready visibility, cancellation/reset/release/shutdown and consumer lifecycle.
- **L1/L3 seam:** terminal L1 byte/result state gates normal L3 lifecycle publication; upstream queue/writer booleans are not automatically terminal-byte receipts.
- Validation is cross-cutting.

## L1 current state — INCOMPLETE

### Product capabilities already integrated

Current `main` includes:

- NBZ classic ZIP indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observation;
- resolver-selected direct-retail acquisition tooling;
- staged atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + recursive expansion;
- same-size, size-changing and nested relative-slot authoring;
- byte-exact untouched-span/sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-contiguous NBZ overlay authoring;
- staged canonical NBZ reopen/rematerialization;
- higher-volume resolver verification;
- protected distribution preflight + product closure orchestration;
- runtime-synth `.lst` direct `0x800` transfer extents vs recursively synthesized `0x40` structural extents, with zero-filled synthesized image;
- consolidated L1 naming architecture and exact extracted-ordinal `.index` mapping;
- sealed separation of external `.index`, embedded aliases, enclosing stored names, semantic evidence, display and export projection;
- exact `ResourceId` runtime-to-L1 naming bridge;
- no-`.index` semantic derived display that remains presentation-only;
- instruction-backed runtime type evidence split by recovered call path rather than one global detector;
- current primary 3D/render-family reverse documentation synchronized through `main@d358a2e`.

These are strong product capabilities, not a completion claim.

### Fresh original-runtime reverse that invalidates the old status

The confirmed 2026-08-27/28 canonical-EXE pass proves that `L1 INTERNAL PRODUCT PATH CLOSED` was too strong.

Confirmed/corrected:

- `0x1402EF4D0` is type-2 queue admission, not byte completion;
- `0x1401B85C0` ignores direct child enqueue and recursive-writer failures;
- `0x1401B8CA0` has branch-dependent boolean meaning;
- `0x1401B84E0` ignores failure of type-3 completion enqueue `0x1402EF580`;
- therefore outer writer/setup `true` does not prove that all expected work was admitted or completed;
- original chunk/planner arithmetic is 32-bit and wrap-prone;
- scan/token ceilings are original bounds, not clean original error enums;
- admitted type-2 materialization and the normal type-3 callback share one lane/FIFO on the bounded static path;
- status `2` is pending, status `4` retries without retirement, status `3` retires type-2 work;
- status `3` does not independently require actual bytes == planned bytes, so original short-success can permit later normal lifecycle completion;
- cancellation queued-work suppression is L3 ownership; the static normal L1/L3 seam is now bounded.

Canonical evidence:

- `../gdspaces/l1-writer-failure-width-reconciliation-2026-08-28.md`
- `../gdspaces/l1-terminal-l3-completion-seam-2026-08-28.md`
- `../../data/reverse/dmc3-l1-writer-failure-width-2026-08-28.v1.json`
- `../../data/reverse/dmc3-l1-terminal-l3-completion-seam-2026-08-28.v1.json`

### L1 reverse still open

- exact recursive `.lst` cycle/depth behavior;
- recursive allocation/free lifetime and residual allocator/backend failure branches;
- representative real `.lst` corpus if real loose-list equivalence is claimed;
- final contradiction sweep over the recovered L1 byte/materialization path.

Dynamic current-slot cancellation/concurrency and broader transition/reset/shutdown behavior remain L3 unless a concrete L1 acceptance receipt activates them.

### L1 naming / type-evidence state

The #251-#262 naming stack, semantically valid #254 contributions and #268 derived-display/runtime-type work are now in `main`; historical PRs remain evidence checkpoints.

Canonical rules now include:

- one physical `ResourceId` independent of naming/display;
- `physical_slot_index` separate from `extracted_ordinal`;
- `.index entry N == extracted ordinal N == N-th populated payload`, not physical slot N;
- external `.index`, embedded alias, enclosing stored name and semantic-format evidence remain separate authorities;
- safe host export is separate from historical extraction evidence;
- runtime-to-L1 naming joins only on exact complete `ResourceId` equality;
- no-`.index` derived display remains synthetic presentation, not historical extraction or write authority;
- runtime type evidence is scoped to separate recovered instruction paths.

The old shorthand that the runtime has one global “exactly five tags” detector is superseded. Current main distinguishes at least:

1. a three-byte registry/resource-registration content probe;
2. a PAC/PNST materialized-child dispatcher, including evidence-bounded EFW/EFE sentinels;
3. a four-byte higher-level family-mask classifier, including MCV evidence.

Still open:

- representative real effect-corpus replay/reconciliation;
- global naming coverage/collision census;
- historical `.index` producer/extractor lineage or explicit unresolved bound;
- real-retail runtime-selected identity -> exact L1 parent identity receipt;
- historical extraction replay/export/reopen validation;
- final naming/type-evidence contradiction audit after the latest instruction-level corrections.

### L1 real acceptance still open

Required vertical chain:

```text
real retail selected identity / provenance
 -> exact representation classification
 -> supported bounded real edit / rebuild
 -> next-volume publication
 -> canonical reopen / rematerialization
 -> original DMC3 consumer-visible effect
 -> rollback / retail immutability
 -> final L1 audit
```

Issue #209 remains the final original-game Level-E gate. A crash-free launch is insufficient.

Therefore **L1 is not COMPLETE and is not 100%**.

## Product safety vs original behavior

| Recovered original behavior | Product stance |
| --- | --- |
| 32-bit wrap can produce negative/zero extents | checked overflow / fail closed |
| loose writer can swallow child enqueue failure | successful product receipt must preserve explicit failure |
| completion enqueue can be ignored by original setup | no authority laundering into product success |
| scan/token bounds lack clean error status | explicit fail-closed diagnostics |
| short status-3 transfer can permit original lifecycle completion | exact-byte product receipts validate declared exactness |

## L2 current frontier

**Status: INCOMPLETE.**

Static resolver/provider work and bounded runtime-mapping tooling are advanced. Real retail collision/member evidence, protected-process mapping and trusted original selected-provider identity remain evidence gates. L2 supports the L1 vertical proof but does not replace L1 byte/materialization closure.

Canonical analysis VAs/RVAs must not be applied to a different protected distribution build without independent runtime mapping.

## L3 current frontier

**Status: INCOMPLETE.**

The static LoadedResource / typed-ready / release spine is strong. The static normal L1-terminal -> L3 completion seam is bounded, but dynamic current-slot cancellation/concurrency, transitions, reset, shutdown and original-process receipts remain open.

## Current critical path

```text
1. land #269 status/evidence reconciliation after final current-head review and CI
2. obtain trustworthy canonical-analysis bytes/disassembly for the residual .lst frontier
3. finish residual recursive .lst + allocator/backend reverse without substituting product guards for original behavior
4. run final original-L1 contradiction sweep
5. finish naming/type-evidence real-corpus, producer-lineage and replay validation
6. obtain representative real-retail selected identity + acquisition provenance
7. classify exact representation
8. perform supported real edit/rebuild/rematerialization
9. execute original-game consumption + rollback (#209)
10. final L1 cross-stack audit
11. only then mark L1 COMPLETE / 100%
```

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

The canonical analysis executable identity and prior canonical-EXE reverse packets are well established, but the **current connected session does not expose the complete raw canonical analysis image** (`e454...`, 6,356,432 bytes). The accessible raw `dmc3.exe` artifact is the protected/distribution build (`81c7...`, 6,567,320 bytes), which is not instruction-reverse authority for canonical analysis VAs.

Accordingly, this session does not promote any new instruction-level claim for recursive `.lst` cycle/depth/allocation/free behavior from the protected build. That frontier remains open until the canonical raw image, a trustworthy bounded disassembly/byte packet, or equivalent evidence is available. This evidence boundary must not be hidden by product safety tests or synthetic CI.

The connected environment also does not expose every protected-install artifact/process condition required for the complete real-retail/original-game Level-E acceptance chain.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [L1 writer/failure/width reverse checkpoint](../gdspaces/l1-writer-failure-width-reconciliation-2026-08-28.md)
- [L1 terminal -> L3 completion seam](../gdspaces/l1-terminal-l3-completion-seam-2026-08-28.md)
- [L1 naming integration checkpoint](../gdspaces/l1-naming-full-integration-20260830.md)
- [Runtime -> L1 naming bridge](../gdspaces/dmc3-runtime-l1-naming-bridge-20260830.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Machine-readable status](canonical-status.json)
- [Blockers](blockers.md)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the evidence-gate completion rule.
