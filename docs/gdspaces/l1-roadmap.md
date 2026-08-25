# GDSpaces Layer 1 Roadmap

**Status:** ACTIVE / NOT COMPLETE  
**Snapshot date:** 2026-08-25  
**Canonical implementation base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`  
**Primary tracking:** issues #100, #182, #209; active product hardening #210 and #212

This document is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**. It replaces percentage-first planning with explicit acceptance gates.

L1 answers one question:

> Can DMC Rengine obtain the exact resource bytes used by DMC3, preserve their identity/provenance, safely edit supported representations, rebuild the required container/archive stack, reopen the result through the canonical resource path, and prove original-game consumption at the claimed scope?

L1 is not closed by a working resolver, structural parsing, synthetic A-to-Z tests, or readable recovered C++ alone.

## 1. Canonical L1 boundary

```text
physical source bytes
  -> exact archive/container identity
  -> member/span acquisition
  -> transform/decompression
  -> materialized bytes + ByteProvenance
  -> nested PAC/PNST/container expansion
  -> exact editable child authority
  -> WorkingCopy / bounded edit
  -> child/container rebuild
  -> NBZ publication
  -> canonical resolver/reopen/rematerialization
  -> byte/evidence receipt
  -> original DMC3 consumption receipt
```

L2 resource resolution and L3 original runtime/lifecycle may support L1, but they do not count as L1 closure unless they directly close one of these materialization gates.

## 2. What is already canonical

Current `main` contains strong bounded authority for:

- NBZ classic ZIP indexing and member enumeration;
- STORE and raw-DEFLATE materialization with CRC/size/product budgets;
- explicit `ByteProvenance` and transformed-vs-materialized coordinate separation;
- PAC and PNST relative-slot parsing with sparse/empty/alias slot identity preservation;
- recursive PAC/PNST expansion;
- same-size layout-preserving PAC/PNST writes;
- size-changing relative-slot reflow and nested reintegration for evidenced writer domains;
- synthetic full nested A-to-Z composition through NBZ repack/reopen;
- transformed DDS-bearing texture framing, bounded size-changing writer and original runtime relocation compatibility for the safe subset;
- recovered numbered-volume bootstrap and higher-volume precedence;
- deterministic next-contiguous STORE NBZ overlay authoring;
- runtime-resolver validation of generated higher-volume overrides;
- original-game preflight authority separation between protected distribution execution image and unpacked analysis image;
- structurally validated PNST classification on current main;
- shared atomic/no-replace publication;
- artifact-bound STORE/raw-DEFLATE retail member observation and acquisition receipts;
- resolver-based retail acquisition CLI outside the protected retail tree;
- immutable-source NBZ copy authoring;
- merged protected-retail product-side closure orchestration (#208).

These are substantial closed slices. They do **not** by themselves imply L1 completion.

## 3. Current critical path

### Gate L1-A — publication integrity

**Status:** CLOSED / MERGED PRODUCT CONTRACT

PR #194 introduced the shared atomic/no-replace publication primitive and migrated the active acquisition/overlay/rebuild paths. Windows and Ubuntu regressions protect non-destructive destination ownership. Real retail acceptance still must prove the generated artifacts were published outside the protected source tree.

### Gate L1-B — artifact-stable retail member acquisition

**Status:** CLOSED / MERGED PRODUCT CONTRACT

PRs #195–#198 bind the indexed NBZ snapshot, selected central entry, materialized bytes and whole archive identity through the artifact-bound observer, cover STORE/raw-DEFLATE, preserve first-gap volume semantics and reject evidence output inside the retail tree.

The acquisition sidecar is the archive/member provenance authority. PR #212 is active hardening that binds this sidecar by path, size and SHA-256 into the higher-level closure receipt so the two receipts cannot be detached after a run.

### Gate L1-C — direct-retail representative member provenance

**Status:** OPEN / EXTERNAL RETAIL RECEIPT REQUIRED

The product command path is implemented. The highest-value first request remains:

```text
obj\em000.pac
```

The canonical resolver must select the actual member/volume. The receipt must bind the protected executable authority, contiguous volume observation, exact archive SHA/size, central entry metadata, materialized SHA/size, transform and ByteProvenance. The current connector still cannot deliver the 960,358,951-byte `dmc3-0.nbz` because it rejects files above 268,435,456 bytes; that is an ingress limitation, not an NBZ limitation.

### Gate L1-D — retail representation classification

**Status:** OPEN / DEPENDS ON L1-C

Compare the direct-retail member with preserved transformed DDS-bearing and runtime-representation evidence.

Possible outcomes must stay distinct:

- exact retail bytes already match the transformed DDS-bearing safe writer profile;
- exact retail bytes use another supported representation;
- preserved transformed corpus is derived from a different representation and cannot be used as retail writer authority.

Do not infer a conversion/writer solely because DDS/TM2/runtime structures are understood.

### Gate L1-E — real-retail bounded edit and bottom-up rebuild

**Status:** PRODUCT WRITERS STRONG / REAL RECEIPT OPEN

Current main has bounded size-changing PAC/PNST reflow. PR #210 adds root-to-leaf slot-path authoring and cryptographically chained bottom-up level receipts. It is branch truth until reviewed and promoted.

A real run may proceed only when the observed retail representation lies inside a proven authoring domain. It must prove the replacement bytes, exact untouched siblings, empty-slot set and alias partition at every affected level. Unsupported representations stop the run rather than forcing a serializer.

### Gate L1-F — next-volume NBZ publication and canonical reopen

**Status:** PRODUCT PATH MERGED / COMPOSITE RECEIPT HARDENING ACTIVE #212 / REAL RECEIPT OPEN

PR #208 composes protected-executable preflight, direct-retail acquisition, PAC/PNST rebuild, next-contiguous STORE overlay authoring, canonical resolver win and exact rematerialization. PR #212 closes the remaining evidence-integrity defect by hashing the artifact-bound acquisition sidecar into the closure receipt.

Product composition is not the real receipt. The representative retail run must still prove the authored volume identity, selected winner, exact rematerialized container and exact target replacement while preserving relevant untouched bytes.

### Gate L1-G — original DMC3 consumption

**Status:** OPEN / FINAL ACCEPTANCE

Run the generated next-volume artifact against the exact protected distribution execution authority.

The receipt must identify:

- executable SHA/size/role;
- retail archive set identity;
- generated volume identity;
- game request/resource identity;
- observed successful load/consumer behavior;
- failure/rollback information if applicable;
- whether the test covered initial load, reload/restart or transition.

Product reopen/reparse is not a substitute for this receipt.

### Gate L1-H — final cross-stack acceptance audit

**Status:** OPEN

Before any `L1 COMPLETE` claim:

- all mandatory gates above are closed or evidence-pruned with written justification;
- closing code/documentation is in canonical `main`;
- exact-head Windows + Ubuntu CI is green;
- real-retail provenance receipt exists;
- representative edit/rebuild/reopen receipt exists;
- original-game consumption receipt exists;
- no unresolved contradiction changes the architecture or representation boundary;
- #100, #182, current status, blockers, risks and machine status agree.

Only then may the project state L1 complete.

## 4. Supporting EXE reverse frontier for GDS

The following EXE-derived boundaries are already strong enough that they should **not** be restarted without contradictory direct evidence:

- DMC3 resource bootstrap / numbered-volume registration;
- basename candidate construction and archive-first/physical-second provider ordering;
- archive normalization/index/qsort/bsearch behavior;
- FileSlot/AsyncIO whole-file materialization spine at the bounded recovered scope;
- ZIP direct-vs-inflated member path and raw-DEFLATE core behavior;
- LoadedResource `0 -> 1 -> 2 -> typed post-load -> 3` spine;
- PAC/PNST recursive typed traversal;
- primary `.lst` packed-first fallback/synthesis structure.

The exact EXE boundaries still relevant to GDS completion are:

1. **type-0 physical provider final-open semantics** after recovered `0x0C` normalization: exact Win32 filename/case/open/failure behavior;
2. **ZIP stream initializer `0x140328540`** complete body/lifetime;
3. **compressed seek/reset `0x140328FE0`** complete reset/reinflate/error behavior;
4. remaining malformed/partial-read/error-code equivalence where a product claim depends on it;
5. dynamic `.lst` allocation/free/error/cycle behavior only where needed to validate actual loose-container use;
6. dynamic original-process lifecycle receipts for representative resource families after L1 closure work reaches game execution.

These are reverse targets, not permission to move original game runtime ownership into GDSpaces.

## 5. Explicit non-blockers / freezes

### Binary AFS

`GDataX360.afs/`, `GData.afs/` and `afs/sound/` are evidenced logical namespaces. They are not evidence for a dedicated binary AFS backend on the current DMC3 HD path. Binary AFS does not block L1 without new direct evidence.

### PACK

Historical GDSpaces PACK parsing is product-history evidence, not original DMC3 runtime authority. PACK does not block L1 absent a direct runtime/materialization dependency.

### Stage Ops / ModViz

Stage assembly, scene semantics, HITS semantics and editor workflows are downstream consumers. They must not create another archive/materialization authority and do not count as L1 progress.

### Capcom offline writer equivalence

DMC Rengine writers are product authoring implementations constrained by observed/read-side behavior and corpus evidence. L1 does not require proving Capcom's external offline tool implementation.

## 6. Work order

```text
1. validate/promote #212 composite receipt binding
2. validate/promote #210 recursive slot-path receipt hardening
3. acquire direct-retail obj\em000.pac request receipt
4. classify the exact retail representation
5. execute bounded real edit + bottom-up rebuild
6. publish next contiguous NBZ + canonical resolver/rematerialization
7. complete #209 original DMC3 consumption + rollback receipt
8. final L1 cross-stack audit and synchronized status
9. only then: L1 COMPLETE
```

Parallel L2/L3 work is allowed when it directly supports this vertical acceptance chain.

## 7. Documentation synchronization rule

Current-state documentation must reference this roadmap rather than maintaining independent L1 checklists. Historical evidence documents remain historical and should not be rewritten to look current.

When the L1 state changes, update together:

- `README.md`;
- `docs/README.md`;
- `docs/roadmap.md`;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json`;
- `docs/gdspaces-contract.md` when an architecture contract changes;
- issues #100 and #182.

The canonical roadmap is gate-based. Percentage estimates may be used only as non-authoritative planning annotations and must never replace the mandatory completion gates.