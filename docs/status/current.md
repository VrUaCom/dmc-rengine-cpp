# Current Project Status

**Snapshot date:** 2026-08-28  
**Canonical implementation base:** `main@94692e8f9971cf8249b4b16ee88d309de8b49f11`  
**Primary execution program:** GDSpaces Layer 1 original-materialization reverse + real acceptance  
**Overall status:** **L1 INCOMPLETE / NOT 100%; L2 INCOMPLETE; L3 INCOMPLETE.** Product capabilities are advanced, but no layer may be promoted to complete from synthetic/product implementation alone.

## Authority split

- GitHub `main` is canonical implemented/product truth.
- Canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.
- Reverse claims are artifact/range/scope bound.
- Synthetic/public CI proves bounded product behavior only.
- Original-game equivalence requires original-process evidence.
- Product safety/hardening must not be mislabeled as recovered original behavior.
- Unsafe original implementation behavior must not be copied into GDSpaces merely for literal parity.

## GDSpaces layer model

- **L1 — Resource Materialization:** selected-resource size/capacity, allocation, exact bytes, transfer/decompression, packed/loose representation construction, nested expansion, authoring/rebuild/repack and exact reopen/rematerialization.
- **L2 — Resource Resolution:** logical request, candidates, normalization, provider/volume/source/member selection and exact selected identity.
- **L3 — Original Runtime/Lifecycle:** request/queue/callback ownership, LoadedResource state publication, typed post-load, ready visibility, claims/cache, cancellation/reset/release/shutdown.
- **L1/L3 seam:** terminal L1 byte/result state gates normal L3 lifecycle publication; upstream queue/writer booleans are not automatically terminal-byte receipts. The canonical static normal-path seam is now bounded and documented; dynamic current-slot cancellation/concurrency remains L3 breadth.
- Validation is cross-cutting.

## L1 current state — INCOMPLETE

### Product capabilities already integrated

Current `main` includes:

- NBZ classic ZIP indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance with provenance delayed until successful member read/validation (#250);
- artifact-bound archive/member observation;
- PAC/PNST parse/expansion and sparse/alias handling;
- same-size, size-changing and nested relative-slot authoring;
- NBZ rebuild / next-volume overlay publication / reopen/rematerialization;
- protected build preflight and product closure tooling;
- runtime-synth `.lst` writer corrected to original direct `0x800` transfer extents vs recursively synthesized `0x40` complete-image extents, with original zero-filled synthesized padding (#255);
- Windows + Ubuntu CI on promoted product paths.

### Fresh original-runtime reverse now confirmed

The 2026-08-27/28 canonical-EXE pass materially changes the old “internal path closed” status.

Confirmed/corrected:

- `0x14002F9F0 -> 0x140048E20` cached-size semantics;
- physical cached size from low-32-bit `GetFileSize`, NBZ from central uncompressed size;
- whole-file direct transfer extent path through `0x1400333C0 -> 0x1402EF620`;
- direct child transfer granularity `0x800` vs recursively synthesized `0x40` structural complete-image extent;
- lower short-read/EOF composition and callback behavior;
- original zero initialization of synthesized output;
- `0x1402EF4D0` is queue admission, while `0x1402EF790` consumes materialization jobs;
- `0x1401B85C0` ignores direct child enqueue and recursive-writer return values;
- `0x1401B8CA0` has branch-dependent boolean semantics;
- `0x1401B84E0` ignores type-3 completion enqueue failure from `0x1402EF580`;
- original planner/chunk arithmetic is 32-bit and wrap-prone;
- `.lst` scan/token ceilings are bounds, not clean original error enums;
- accepted type-2 materialization jobs and the normal type-3 `0x1401B8DC0` callback share one per-lane FIFO;
- whole-file status `2` keeps the current type-2 job pending, status `4` retries that same current job without FIFO retirement, and status `3` retires it and permits the later callback to become current;
- because status `3` lacks an independent actual-bytes == planned-bytes check, an original short-success transfer can permit normal `state1 -> state2` completion;
- cancellation `0x1401B8430 -> 0x1402EF460` suppresses queued normal work and publishes state `4`; exact concurrent/current-slot races remain L3 dynamic scope.

Canonical detail:

- `../gdspaces/l1-writer-failure-width-reconciliation-2026-08-28.md`
- `../gdspaces/l1-terminal-l3-completion-seam-2026-08-28.md`
- `../gdspaces/dmc3-loose-container-list.md`
- `../gdspaces/l1-roadmap.md`
- `../../data/reverse/dmc3-l1-writer-failure-width-2026-08-28.v1.json`
- `../../data/reverse/dmc3-l1-terminal-l3-completion-seam-2026-08-28.v1.json`

### Static L1/L3 seam — bounded closed

At the canonical static normal-path scope, the old L1 question “what allows or suppresses normal completion after materialization?” is no longer open.

For **admitted** jobs:

```text
L1 type-2 byte job current
 -> status 2: remain pending
 -> status 4: reset local phase and retry same job
 -> status 3: close/clear/retire type-2 and advance FIFO
 ===== L1 native byte/result terminal =====
 -> later admitted type-3 callback becomes current
 -> 0x1401B8DC0 publishes LoadedResource state 1 -> 2
 ===== L3 lifecycle =====
```

This does not repair the original upstream failure-swallowing defects: outer writer/setup `true` still does not prove every expected type-2/type-3 job was admitted. It also does not close dynamic current-slot cancellation races or broader L3 lifecycle behavior.

### L1 reverse still open

- exact recursive `.lst` cycle/depth and allocation/free lifetime behavior;
- residual allocator/backend failure branches;
- final contradiction sweep across the now-recovered L1 byte/materialization path and bounded seam;
- representative real `.lst` corpus receipt if real loose-list equivalence is claimed.

Dynamic current-slot cancellation/concurrency and broader transition/reset/shutdown behavior remain **L3**, not an L1 reverse blocker unless a concrete L1 acceptance receipt activates them.

### L1 real acceptance still open

Required vertical chain remains:

```text
real retail selected identity/provenance
 -> exact representation classification
 -> supported bounded real edit/rebuild
 -> next-volume publication
 -> canonical reopen/rematerialization
 -> original DMC3 consumer-visible effect
 -> rollback / retail immutability
 -> final L1 audit
```

Issue #209 remains the original-game consumption gate.

Therefore **L1 is not COMPLETE and is not 100%** even though many product paths are implemented and CI-green.

## Product safety vs original behavior

Current evidence requires an explicit separation:

| Recovered original behavior | Product stance |
| --- | --- |
| 32-bit wrap can produce negative/zero extents | checked overflow / fail closed |
| loose writer can swallow child enqueue failure | successful product receipt must preserve explicit failure |
| completion enqueue can be ignored by original setup | no authority laundering into product success |
| malformed scan/token bounds lack clean error status | explicit fail-closed product diagnostics |
| short status-3 transfer can permit original normal completion | exact declared/observed validation where product receipt claims exact bytes |

## L2 current frontier

**Status: INCOMPLETE.**

Static resolver/provider work and bounded runtime-mapping tooling are advanced. Real retail collision/member evidence, protected-process mapping and trusted original selected-provider identity remain separate evidence gates. L2 work may support the L1 vertical proof but must not replace L1 byte/materialization closure.

Canonical analysis VAs/RVAs must not be applied to a different protected distribution build without independent mapping evidence.

## L3 current frontier

**Status: INCOMPLETE.**

The static LoadedResource / typed-ready / release spine is strong, but dynamic lifecycle breadth and original-process receipts remain open.

The static normal L1-terminal -> L3 completion seam is now bounded: same-lane FIFO ordering, status `2/4/3` behavior, normal callback eligibility, short-success consequence and queued cancellation suppression are recorded. Dynamic current-slot cancellation/concurrency, transitions, reset and shutdown remain L3 work.

## Current critical path

```text
1. finish residual recursive .lst / allocator/backend failure branches
2. final L1 original-runtime contradiction sweep
3. obtain representative real-retail acquisition/provenance
4. classify exact representation
5. perform supported bounded real edit/rebuild/rematerialization
6. execute original-game consumption + rollback (#209)
7. final L1 audit
8. only then mark L1 COMPLETE / 100%
```

L2/L3 broad programs continue independently when they directly support these gates or their own closure requirements.

## Environment boundary

The canonical analysis executable is available for direct reverse and has been revalidated. The connected environment still does not expose every protected-install artifact/process condition needed for the final real-retail/original-game acceptance chain.

External evidence limits do not justify promoting a layer to complete.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [L1 writer/failure/width reverse checkpoint](../gdspaces/l1-writer-failure-width-reconciliation-2026-08-28.md)
- [L1 terminal → L3 completion seam](../gdspaces/l1-terminal-l3-completion-seam-2026-08-28.md)
- [DMC3 loose-container reconstruction](../gdspaces/dmc3-loose-container-list.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Machine-readable status](canonical-status.json)
- [Blockers](blockers.md)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the evidence-gate completion rule.
