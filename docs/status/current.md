# Current Project Status

**Snapshot date:** 2026-09-05  
**Canonical implementation base reviewed:** `main@ee08b388cbc5448a0e1a5d02231d9aaf7e01587d`  
**Latest canonical Native Reader promotion:** PR #285 — SCM/MOD Model Family  
**Latest retail evidence promotion:** PR #279 — bound `dmc3-0.nbz` `0x0E` collision census + SCM/SO integration  
**Primary execution program:** proof-gated L2 -> L1 -> L3 vertical acceptance  
**Overall status:** L1/L2/L3 remain incomplete; canonical implementation is advanced, but real protected-process/original-game receipts remain mandatory.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims are bounded to exact artifact/address/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires canonical-EXE reverse plus original-process evidence where runtime identity/consumption is claimed.
- Retail corpus claims require hash-bound corpus receipts.
- DMC Rengine product-safety policies such as atomic/no-replace publication are not presented as Capcom behavior.
- Canonical analysis executable and protected distribution execution authority are separate builds; canonical VAs/RVAs require independent mapping before protected-process use.

See [project roadmap](../roadmap.md) and [proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md).

## Canonical analysis authority

Canonical `dmc3.exe`:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- PE32+ x86-64;
- ImageBase `0x140000000`.

Protected distribution/original execution candidate remains separately identified as SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320. No global build equivalence is claimed.

## Native Reader current state

The canonical resource-level Native Reader is modular (`NativeReaderModuleRegistry`). PAC/PNST remain container parsers; NBZ remains a source/materialization adapter.

Canonical built-in reader modules on current `main`:

- DDS;
- PTX;
- HITS;
- DCA;
- LIG2/LIG;
- Stage TXT;
- SCM;
- MOD;
- PE/EXE.

SCM and MOD are structural/read-only. Their reader promotion does not imply writer authority.

Current reader frontier:

- EFM — reverse evidence exists, but no canonical Native Reader module on current `main`;
- MOT — research/parser work exists outside the canonical reader set;
- SHW — strong EXE + real-payload evidence exists, but no canonical Native Reader module on current `main`;
- MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

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

### Reverse-backed bounded facts

- `DMC3-%d.nbz` bootstrap and first-gap discovery;
- PAC/PNST typed traversal in the recovered post-load path;
- PAC physical slot 0 is traversed;
- materialization-dispatch success gates LoadedResource state1;
- packed-vs-`.lst` representation selection is original-runtime authority; external `.index` is not recovered as runtime materialization authority on this path.

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

### Closed/reverse-backed

- `OpenGameResource` bounded direct caller census;
- six-prefix archive-then-physical request policy for the recovered direct-call surface;
- executable-relative `data\\dmc3\\` root and numbered first-gap discovery;
- successful archive registrations prepend to the mount list;
- clean higher-numbered archive precedence;
- archive `0x0E` / physical `0x0C` normalization;
- terminal archive-wrapper failure distinction;
- type-0 physical final-open/miss bounded contract;
- protected-runtime mapping tooling (#219).

### Retail collision evidence

Bound retail `dmc3-0.nbz`:

```text
files-only          : 4333 keys / 4333 unique / 0 collisions
all central entries : 4334 keys / 4334 unique / 0 collisions
```

Receipt: `data/reverse/dmc3-nbz-archive-key-census-20260903.json`.

This closes collision freedom only for that exact archive. Wider resolver scope still requires per-volume and cross-volume census.

### Remaining L2 frontier

- discovery range vs successful mounted set is reverse-proven, but the product API correction from historical #246 still needs semantic port to current main;
- real protected-process R2B multi-anchor mapping receipt;
- trusted R3 selected-provider/member identity;
- direct-retail original resolver winner receipt;
- final L2 audit.

**L2 COMPLETE = NO.**

## L3 — Original Runtime / Lifecycle

Reverse-backed bounded core includes:

- 363-record LoadedResource registry topology;
- acquisition/materialization success -> state1;
- normal completion `1 -> 2`;
- typed post-load -> optional callback -> state3;
- global cancellation `1|2 -> 4`;
- quiescence requires all records in `{0,3}`;
- distinct ordinary release, cancellation cleanup and forced reset policies;
- central typed dispatcher paths for MOD/EFM/SCM/SHW plus PNST recursion.

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

Immediate order:

1. reconcile roadmap/status to proof-level truth;
2. semantically port discovery-vs-successful-mount topology semantics to current main;
3. close the remaining materialization scheduler terminal dependency when exact canonical EXE bytes are available for a fresh raw pass;
4. execute protected-process R2B mapping;
5. capture trusted R3 selected identity;
6. bind selected identity to exact independently materialized bytes;
7. observe typed post-load/state3 for the same resource;
8. repeat using authored higher-volume overlay;
9. record deterministic consumer effect + rollback;
10. run independent final L1/L2/L3 audits.

## Current evidence-access boundary

The current connected file/library surface contains substantial derived reverse documentation and evidence packets, but a raw canonical `e454...` executable blob was not located during this reconciliation pass. Therefore no new raw-byte claim is promoted here for the still-open materialization scheduler dependency. Existing canonical reverse authority is preserved; the exact unresolved raw targets remain explicit.

## Navigation

- [Project roadmap](../roadmap.md)
- [Proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md)
- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)

No percentage, green synthetic suite, parser success or crash-free launch overrides the gate-based completion rule.
