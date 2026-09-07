# Current Project Status

**Snapshot date:** 2026-09-07  
**Canonical implementation base reviewed:** `main@109b63009949be526aa691809d39a27db076e4ff`  
**Latest repository promotion in this snapshot:** PR #325 — manifest-backed discovery breadcrumbs + `BreadcrumbList` metadata  
**Latest canonical reverse/model-family promotion:** PR #323 — EXE-backed MOD mesh preservation + skin ABI closure  
**Latest canonical Native Reader module promotion:** PR #288 — evidence-backed SHW structural reader + cross-registry integration hardening  
**Primary execution program:** proof-gated L2 -> L1 -> L3 vertical acceptance  
**Overall status:** L1/L2/L3 remain incomplete. Canonical implementation and MOD/model-family reverse evidence advanced materially on 2026-09-07, but no product, parser, writer, synthetic CI result or discovery feature overrides the mandatory original-runtime/evidence gates.

## Authority split

- GitHub `main` is canonical implementation truth.
- A pull-request branch is branch truth until promoted.
- Reverse claims are bounded to exact artifact/address/range/scope.
- Synthetic/public CI proves bounded product/tool behavior only.
- Original-game equivalence requires canonical-EXE reverse plus original-process evidence where runtime identity/consumption is claimed.
- Retail corpus claims require hash-bound corpus receipts.
- DMC Rengine product-safety policies such as atomic/no-replace publication are not presented as Capcom behavior.
- Discovery/SEO pages are readability/navigation surfaces, never a second technical authority.

See [project roadmap](../roadmap.md) and [proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md).

## Canonical analysis authority

Canonical `dmc3.exe`:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- PE32+ x86-64;
- ImageBase `0x140000000`.

Protected distribution/original execution candidate remains separately identified as SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320. No global build equivalence is claimed.

## Native Reader current state

The resource-level Native Reader is modular (`NativeReaderModuleRegistry`). PAC/PNST remain container parsers; NBZ remains a source/materialization adapter.

Canonical built-in reader modules on current `main`:

- DDS;
- PTX;
- HITS;
- DCA;
- LIG2/LIG;
- Stage TXT;
- SCM;
- MOD;
- SHW;
- PE/EXE.

Current reader frontier:

- EFM — reverse evidence exists, but no canonical Native Reader module;
- MOT — research/parser work exists outside the canonical reader set;
- MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

### SHW boundary

SHW remains canonical as `formats.shw-structural-v1`, structural/read-only, promoted by PR #288. The slice is bounded to the canonical EXE plus one hash-bound real payload (`slot_0008.shw`, size 9,488, SHA-256 `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e`). No SHW writer, universal revision coverage or original-game authored-SHW acceptance is claimed.

### MOD / model-family promotion stack

The MOD reader remains read-only, but its canonical structural/runtime evidence is substantially stronger than the 2026-09-06 status snapshot. Promoted slices now include:

- PR #305 — canonical hierarchy/local transforms/world propagation consolidation;
- PR #307 — typed mesh texture slot + legacy GS CLAMP state;
- PR #310 — EXE-backed post-load relocation/topology generation reconstruction;
- PR #312 — typed object alpha/control, source flags and bounds;
- PR #313 — EXE-confirmed model texture companion envelope with TM2-backed payload allocation;
- PR #314 — typed document/header texture-domain mirror and raw runtime header state;
- PR #315 — mesh texture binding validation against companion-authoritative runtime domain;
- PR #316 — inverse-rest ownership and `skinMatrix = inverseRestWorld * currentWorld` palette composition;
- PR #317 — runtime model-texture descriptor ABI with GS TEX0/MIPTBP1 decode;
- PR #318 — EXE-backed object runtime flag/render-parameter projection;
- PR #323 — serialized mesh `+0x0C/+0x38/+0x4C` preservation classification plus direct `/4` blend-index and 5+5+5-bit `/31` weight ABI evidence.

These promotions authorize stronger truthful read-only MOD inspection, skeleton/weight visualization and pose-aware analysis. They do **not** establish:

- MOD writer authority;
- byte-identical no-edit MOD rebuild;
- safe mutation ranges for unresolved serialized fields;
- complete TIM2 semantics or production texture replacement authority;
- complete current animation/pose ownership;
- edited MOD acceptance by the original game;
- Capcom offline-tool equivalence.

## L1 — Resource Materialization

### Closed product capabilities

- NBZ classic ZIP indexing/materialization;
- STORE + raw-DEFLATE product path;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + recursive expansion;
- size-changing relative-slot reflow;
- nested root-to-leaf PAC/PNST slot-path authoring;
- immutable NBZ copy rebuild;
- next-contiguous STORE NBZ overlay authoring;
- canonical reopen/rematerialization checks;
- direct-retail acquisition tooling and protected-build preflight.

### Mandatory remaining L1 receipts

1. real protected-install resolver-selected provenance;
2. exact selected retail representation classification;
3. one real supported edit/rebuild/rematerialization receipt;
4. original runtime selects the authored higher-numbered overlay;
5. deterministic consumer-visible effect attributable to authored bytes;
6. rollback proving original retail immutability;
7. final L1 audit.

**L1 COMPLETE = NO.**

## L2 — Resource Resolution

Reverse-backed/canonical work includes the six-prefix bounded direct-call policy, numbered-volume bootstrap/first-gap discovery, successful-registration prepend ordering, clean higher-volume precedence, archive/physical normalization, type-0 physical final-open behavior, and the PR #287 correction that discovery/registration attempts are not the same thing as successful linked mount topology.

Bound retail `dmc3-0.nbz` still has the exact one-volume collision receipt:

```text
files-only          : 4333 keys / 4333 unique / 0 collisions
all central entries : 4334 keys / 4334 unique / 0 collisions
```

Remaining L2 frontier:

- per-volume + cross-volume collision census for any wider resolver scope;
- real protected-process R2B multi-anchor mapping receipt;
- trusted R3 selected-provider/member identity;
- direct-retail original resolver winner receipt;
- final L2 audit.

**L2 COMPLETE = NO.**

## L3 — Original Runtime / Lifecycle

Reverse-backed bounded core still includes:

- 363-record LoadedResource registry topology;
- acquisition/materialization success -> state1;
- normal completion `1 -> 2`;
- typed post-load -> optional callback -> state3;
- global cancellation `1|2 -> 4`;
- quiescence requires all records in `{0,3}`;
- distinct ordinary release, cancellation cleanup and forced reset policies;
- central typed dispatcher paths for MOD/EFM/SCM/SHW plus PNST recursion.

The new MOD post-load/runtime evidence strengthens one family-specific part of this spine but does not close global lifecycle proof.

Remaining L3 work:

- promote/reconcile final R1 contradiction-gated writer census onto current main;
- close R2 family/backing ownership;
- close the exact materialization scheduler terminal condition preventing failed/incomplete transport from reaching normal state2 publication;
- capture V1–V7 original-process lifecycle receipts;
- final L3 audit.

**L3 COMPLETE = NO.**

## Current P0 proof track

```text
OpenGameResource(request)
 -> discovered volumes
 -> successful mount topology
 -> selected provider/volume/member
 -> exact materialized bytes
 -> PAC/PNST expansion where applicable
 -> typed post-load
 -> LoadedResource state3
 -> deterministic consumer-visible effect
```

Then repeat the same lineage with an authored next-volume NBZ and rollback.

Immediate proof order remains:

1. ✅ roadmap/status reconciled to proof-level truth;
2. ✅ successful-mount topology correction promoted by PR #287;
3. ✅ SHW structural Native Reader and cross-registry routing/validation hardening promoted by PR #288;
4. ⚠️ canonical EXE bytes are now available to recent reverse work, but the still-open materialization-scheduler terminal dependency requires its own fresh bounded raw pass and must not be inferred from MOD evidence;
5. ❌ execute protected-process R2B mapping;
6. ❌ capture trusted R3 selected identity;
7. ❌ bind selected identity to exact independently materialized bytes;
8. ❌ observe typed post-load/state3 for the same resource;
9. ❌ repeat using authored higher-volume overlay;
10. ❌ record deterministic consumer effect + rollback;
11. ❌ run independent final L1/L2/L3 audits.

## Current evidence-access boundary

The old 2026-09-05 statement that raw canonical `e454...` bytes were unavailable is no longer current. The exact canonical executable has been supplied and used for the promoted 2026-09-06/07 MOD/model-family reverse slices listed above.

That availability is **not** a blanket proof upgrade. Each unrelated GDSpaces/L3 raw target must still receive its own fresh address/range/scope-bounded analysis before any claim changes, and protected-process/original-consumption gates remain mandatory where runtime identity or acceptance is claimed.

## Discovery/publication state

The repository now contains the controlled discovery-site builder, unique public entry pages, canonical/OG metadata, share metadata, citation metadata and manifest-backed breadcrumbs. GitHub Pages deployment remains fail-closed and is still a repository-settings gate; discovery work does not change technical completion state.

## Navigation

- [Project roadmap](../roadmap.md)
- [Proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md)
- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)

No percentage, green synthetic suite, parser success, discovery metadata or crash-free launch overrides the gate-based completion rule.
