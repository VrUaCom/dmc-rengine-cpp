# GDSpaces L1 naming full integration checkpoint — 2026-08-30

Status: **INTEGRATION / VALIDATION — NOT COMPLETE, NOT APPROVED FOR MAIN**

Integration branch: `ada/l1-naming-full-integration-20260830`

Validation PR: `#266 — L1 naming full integration validation`

Base at branch creation: `main@94692e8f9971cf8249b4b16ee88d309de8b49f11`

## Branch policy

This branch is an isolated semantic-integration tree. It must not be merged to
`main` until reverse reconciliation, corpus reconciliation, contradiction audit,
and whole-head Windows + Ubuntu validation are all green.

Historical naming PRs/branches remain evidence checkpoints. Do not delete,
force-rewrite, or mechanically merge them merely to flatten history.

## Canonical authority domains

A materialized child has one physical identity and several independent evidence
namespaces. None may be laundered into another.

1. `ResourceId` — physical/materialized resource identity.
2. `physical_slot_index` — physical slot inside the parsed PAC/PNST.
3. `extracted_ordinal` — ordinal among populated payloads only.
4. `external_index_raw_label` — exact retained `.index` line.
5. `external_index_normalized_name()` — normalized external extraction name.
6. `external_index_folder_marker()` — structural `folder` marker state.
7. `embedded_alias` — embedded slot-0 semantic alias evidence.
8. `enclosing_container_stored_name` — direct name stored by an enclosing
   physical container, currently used for the recovered DMC3 effect convention.
9. `semantic_format` + `semantic_format_evidence` — byte/structure-backed format
   interpretation and its sealed evidence snapshot.
10. `canonical_display_name` — presentation result after reconciliation.
11. `LegacyExtractionNamingPlan::representation` — historical file/directory
    representation derived only from exact external `.index` evidence.
12. `export_safe`, `export_name`, `export_path` — host-export projection, kept
    separate from historical evidence and withheld for unsafe paths.

Compatibility member names `external_index_name` and `external_index_folder`
remain storage/API compatibility only. The canonical semantic terminology is
`external_index_normalized_name()` and `external_index_folder_marker()`; there is
no second authority value.

## Recovered `.index` rule

Canonical mapping is:

```text
.index entry N
    == extracted ordinal N
    == N-th populated payload in physical slot order
```

It is **not**:

```text
.index entry N == physical slot N
```

Empty physical slots do not consume extracted ordinals. `physical_slot_index`
and `extracted_ordinal` remain separate even for dense containers where their
numbers happen to be equal.

`PNST` is retained as external extraction/directive evidence only. It is not a
slot-mapping authority.

## Naming pipeline

The canonical DMC3 L1 entrypoint is `Dmc3NamingPipeline`:

```text
materialized PAC/PNST
    -> optional exact enclosing-container stored-name binding
    -> optional explicit external .index
       OR exact physical/logical companion .index discovery
    -> IndexManifest parsing
    -> extracted ordinal -> populated physical slot binding
    -> embedded alias evidence
    -> semantic format evidence
    -> canonical reconciliation
    -> ResourceNamingIdentity snapshot
    -> legacy extraction / safe export projection
```

The pipeline is transactional. A failed authority/binding step restores the
pre-reconciliation expansion.

No naming/display operation may change `ResourceId`, physical slot, payload
bytes, byte provenance, write target, or container topology.

## Companion `.index` discovery boundary

Accepted candidates are derived only from physical/logical container identity,
for example:

```text
GData.afs/scr/st001.pac
 -> GData.afs/scr/st001.index
 -> GData.afs/scr/st001/st001.index
```

If both exact candidates exist, discovery fails closed as ambiguity.

Rejected/superseded heuristics:

- display name -> fabricated `.pac` -> index lookup;
- first arbitrary `.index` in a folder;
- embedded/semantic alias -> index lookup;
- canonical display suffix -> index lookup.

## Embedded alias and texture representation boundary

For `st001`, the embedded list (`st001.ptx`, `st001.scm`, `st001.sch`) is a
separate semantic alias namespace, not the external `.index` namespace.

Example:

```text
physical slot:                 3
external extraction label:     st001_003.ukn
embedded alias:                st001.sch
semantic bytes/structure:      HITS
canonical display:             st001_003.hits
```

For the texture bundle, retained historical extraction evidence is:

```text
st001_001/
    st001_001.index
    st001_001_000.dds
    ...
```

`st001_001.ptx/` remains a derived UI possibility, not a proven historical
extraction directory name.

## Enclosing-container stored names from PR #254

The useful independent naming contribution from #254 is the effect-container
stored-name convention:

- outer `*_effect.pac` is structurally a two-slot PNST;
- physical slot 0 is an ASCII manifest;
- physical slot 1 is a PNST of effect records;
- each accepted manifest line binds sequentially to one populated record
  payload in physical slot order;
- known observed record kinds are `V`, `E`, `P`, `T`, `A`;
- fixed extents are retained as corpus corroboration where known;
- no original executable text-manifest read site is currently proven.

This is now integrated semantically as a distinct sealed
`EnclosingContainerNameEvidence` domain. Evidence is bound to:

- exact enclosing `ResourceId`;
- SHA-256 of the enclosing bytes;
- exact target child `ResourceId`;
- SHA-256 of the target bytes;
- physical target slot;
- exact manifest line/source line.

A stale target edit, wrong parent identity, byte-range mismatch, incomplete
record coverage, or conflicting existing authority fails closed.

This evidence is **not** external `.index`, **not** an embedded alias, and never
participates in companion `.index` discovery.

## PR / Pass reconciliation matrix

| PR | Integration status | Semantic disposition |
|---|---|---|
| #251 | already integrated in ancestry | Historical checkpoint. Original direct physical-position interpretation is superseded by #261; retain only evidence/history, not that mapping rule. |
| #252 | already integrated | Nested DDS `.index` / retained expanded texture representation remains active. |
| #253 | already integrated | Persistent sealed naming evidence remains active. |
| #256 | already integrated | Embedded slot-0 name-list / aliases remain active as a distinct evidence domain. |
| #259 | already integrated | Semantic canonicalization is retained, but embedded name-table evidence is not allowed to impersonate external `.index` authority. |
| #260 | already integrated | Sealed semantic naming authority remains active. |
| #261 | already integrated; canonical correction | `extracted ordinal -> N-th populated physical payload` is the canonical `.index` mapping rule. |
| #262 | already integrated | Unified identity, legacy replay, exact companion discovery and one DMC3 naming pipeline remain the canonical orchestration. |
| #254 external-index naming path | **superseded / rejected** | Its direct `.index`-row -> physical-slot binding and warning-only directive mismatch conflict with #261/#262 fail-closed authority rules. Do not merge. |
| #254 old ContainerExpander naming mutations | **superseded / rejected** | Direct naming/display mutation in physical expansion is replaced by sealed evidence + reconciliation. |
| #254 effect/container stored names | **semantic merge integrated** | Reimplemented as a separate enclosing-container evidence domain with authority/target byte seals and canonical pipeline integration. |
| #254 runtime semantic names | docs/reverse evidence only | Runtime/resource semantic names are not external extraction names and are not promoted to L1 `.index` authority. |
| #254 slot-0 reverse | research/evidence only | Runtime reaches/uses physical data, but no new direct proof turns slot-0 text into a general runtime naming manifest. |
| #254 index probing | superseded where heuristic | Only #262 exact physical/logical companion candidates survive. |
| #254 runtime-vs-extraction boundary | retained documentation boundary | `.index` remains extraction/naming evidence; it is not declared a DMC3 runtime manifest without new direct evidence. |

## Duplicate / obsolete implementation audit

The following must not re-enter the canonical tree:

- `.index` positional mapping by physical slot number;
- warning-and-continue authority mismatch behavior;
- direct physical `ContainerExpander` rename logic;
- fabricated filename probing for companions;
- arbitrary first-sidecar selection;
- semantic aliases used as physical/extraction lookup keys;
- presentation/display names used as write targets or physical IDs;
- silent host-path sanitization that changes historical evidence.

## Safe export boundary

Historical extraction evidence and host export are deliberately separate.

For a safe relative external name, `LegacyExtractionNamingPlanner` exposes:

```text
extraction_name = historical normalized external name
export_safe      = true
export_name      = safe leaf
export_path      = safe normalized relative path
```

For absolute, drive-like, empty-component, `.` or `..` paths, historical
`extraction_name` is preserved but:

```text
export_safe = false
export_name = null
export_path = null
```

No fallback/fabricated replacement name is created.

## Regression coverage added in this integration pass

`effect_stored_name_evidence_tests` covers:

- structural two-level effect PNST parsing;
- exact enclosing -> physical records child identity binding;
- direct stored names retained separately from external `.index` names;
- populated extracted ordinals remain independent of stored names;
- stale target-byte evidence rejection;
- wrong-parent rejection;
- coexistence of external extraction name and enclosing stored name;
- safe export projection;
- unsafe historical path retained as evidence but refused as export authority.

Existing naming tests continue to cover external `.index`, sparse ordinal
binding, embedded aliases, ResourceNamingIdentity, companion discovery, legacy
replay and DMC3 pipeline behavior.

## CI / promotion gates

Do **not** mark naming COMPLETE merely because the branch compiles.

Required before promotion review:

- [x] stacked #251-#262 ancestry present in the isolated integration branch;
- [x] #254 naming contributions classified semantically rather than merged;
- [x] obsolete direct physical-slot `.index` authority rejected;
- [x] effect/container stored names integrated as a separate sealed domain;
- [x] explicit semantic-format evidence retained in unified identity;
- [x] historical extraction and safe host export separated;
- [x] compatibility tests updated for versioned injective `ResourceId::canonical()` (`rid2`) rather than rolling back identity hardening;
- [ ] latest whole-head Ubuntu CI green after all integration edits;
- [ ] latest whole-head Windows CI green after all integration edits;
- [ ] real retained effect corpus replay/reconciliation recorded on this exact integration head;
- [ ] final contradiction audit across code + naming docs on this exact integration head;
- [ ] separate promotion review.

Until all unchecked gates are satisfied, status remains **INTEGRATION / NOT
COMPLETE** and PR #266 remains validation-only.
