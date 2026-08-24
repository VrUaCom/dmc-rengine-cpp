# GDSpaces Layer 1 Roadmap

**Status:** ACTIVE / NOT COMPLETE  
**Snapshot date:** 2026-08-24  
**Canonical implementation base:** `main@c4920c8602dd7492b6c89e9fc8ecf8a6d8397ee0`  
**Primary tracking:** issues #100, #182; active acquisition seam #191

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
- structurally validated PNST classification on current main.

These are substantial closed slices. They do **not** by themselves imply L1 completion.

## 3. Current critical path

### Gate L1-A — publication integrity

**Status:** OPEN / CODE CORRECTION REQUIRED

The project currently has inconsistent output publication semantics. The retail NBZ repacker uses a no-replace staging/publication pattern, while CLI artifact seams still contain `exists() -> ofstream` publication paths.

Required closure:

- one shared atomic/no-replace publication primitive;
- use it for overlay artifacts, retail-member acquisition outputs and evidence receipts;
- no destination replacement race;
- Windows and Ubuntu regressions including concurrent destination creation;
- generated output must remain outside measured retail source trees unless an explicit separate export contract says otherwise.

No CLI may claim `no-clobber` until this gate is closed.

### Gate L1-B — artifact-stable retail member acquisition

**Status:** OPEN / ACTIVE #191

`NbzZipSource` currently builds its index from one file open and reopens the archive during member materialization. A provenance-grade receipt therefore needs an explicit artifact-stability contract so index metadata, selected member bytes and archive identity cannot silently refer to different physical snapshots.

Required closure:

```text
observed retail volume set
  -> exact selected volume identity
  -> exact central/member identity
  -> exact materialized bytes
  -> archive SHA/size
```

all bound to one stable observation or an explicit before/after revalidation that proves no mutation occurred.

The active acquisition command must also:

- use canonical `RuntimeResourceResolver` selection;
- record the winning member path rather than pre-guessing it;
- fail closed on archive/source instability;
- reject publication inside the measured retail game tree;
- use atomic no-replace output publication.

### Gate L1-C — direct-retail representative member provenance

**Status:** WAITING ON L1-B

Highest-value first target remains the game request:

```text
obj\em000.pac
```

The request must be passed to the canonical resolver. The archive member is whatever the resolver actually selects from the recovered basename candidate sequence; documentation must not predeclare `GData.afs/obj/em000.pac` or any other unobserved winner.

Receipt must include:

- protected distribution executable identity/preflight;
- contiguous `DMC3-N.nbz` set observation;
- selected volume index/path/SHA/size;
- exact central-entry index/name/flags/method/CRC/sizes;
- exact materialized member SHA/size;
- ByteProvenance transform;
- source-chain identity.

### Gate L1-D — retail representation classification

**Status:** OPEN / DEPENDS ON L1-C

Compare the direct-retail member with preserved transformed DDS-bearing and runtime-representation evidence.

Possible outcomes must stay distinct:

- exact retail bytes already match the transformed DDS-bearing safe writer profile;
- exact retail bytes use another supported representation;
- preserved transformed corpus is derived from a different representation and cannot be used as retail writer authority.

Do not infer a conversion/writer solely because DDS/TM2/runtime structures are understood.

### Gate L1-E — real-retail bounded edit and bottom-up rebuild

**Status:** OPEN / DEPENDS ON L1-D

Only if the observed retail representation lies inside a proven authoring domain:

```text
retail member
  -> exact editable child
  -> bounded edit
  -> child writer
  -> PAC/PNST bottom-up rebuild
  -> validation of edited child
  -> byte-exact untouched sibling checks
```

If the representation is outside current writer authority, stop and create a new evidence/reverse gate rather than forcing the existing writer.

### Gate L1-F — next-volume NBZ publication and canonical reopen

**Status:** PRODUCT PATH STRONG / REAL RECEIPT OPEN

Publish the rebuilt resource through the next contiguous numbered volume without modifying retail archives.

Required receipt:

```text
rebuilt resource
  -> generated DMC3-N.nbz
  -> atomic no-replace publication
  -> NbzZipSource reopen
  -> VolumeBootstrapPolicy
  -> RuntimeResourceResolver
  -> higher-volume selection
  -> exact rematerialized edited bytes
```

Untouched members/siblings relevant to the edit must retain byte-exact validation at the claimed scope.

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

Execute in this order unless direct evidence creates a stronger dependency:

```text
1. shared atomic/no-replace publication
2. artifact-stable retail acquisition contract
3. correct and promote #191
4. acquire direct-retail obj\em000.pac request receipt
5. classify exact retail representation
6. bounded real edit + bottom-up PAC/PNST rebuild
7. next-volume NBZ publication + canonical resolver/reopen
8. original DMC3 consumption receipt
9. final L1 cross-stack audit
10. only then: L1 COMPLETE
```

Parallel reverse work is allowed only when it directly supports one of these gates or does not displace the critical path.

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