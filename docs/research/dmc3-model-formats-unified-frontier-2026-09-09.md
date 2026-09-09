# Unified MOD, SCM and MOT research frontier

Working branch: `reverse/mod-completion-20260907`  
Review surface: PR #372

This is the canonical consolidation checkpoint for the DMC3-HD model-format family. It replaces the practice of continuing separate MOD, SCM and MOT reverse passes when the same evidence or implementation has already been recovered elsewhere.

It is **not** a declaration that MOD, SCM, MOT or the original game authoring pipeline are fully recovered.

## Consolidation decisions

| Source | Disposition |
|---|---|
| Existing `reverse/mod-completion-20260907` | Canonical working branch retained |
| Historical remote `scm` | Unique writer/edit/corpus/research material integrated; stale alternate implementation not installed in parallel |
| Historical `reverse/mot-em000-20260908` | Modular MOT parser/IR and unique corpus/research evidence integrated |
| Previous legacy MOT summary decoder | Byte-decoding duplication removed; summary now projects the modular parser |
| Historical model-family / skin / transform work | Unique evidence retained where still applicable; superseded code not reinstated |
| Unpublished alternate SCM writer snapshot | Not installed beside the selected writer; corpus result independently repeated with selected canonical code; only genuinely unique evidence may survive |

No new model-format branch is authorized by this consolidation unless a future task has a genuinely independent technical lifecycle that cannot safely live on the canonical branch.

Historical refs may remain temporarily for auditability while PR #372 is open. After successful promotion, superseded remote SCM/MOT working refs should be reviewed for deletion rather than treated as parallel active branches.

## Current evidence summary

### MOD

Already-established prerequisites include:

- provenance-bound 38-file retail no-edit parser/writer/reopen parity;
- one tightly controlled real-retail `object.bounding_radius` edit with exact changed-byte evidence;
- fail-closed MOD writer receipt -> `AuthoredChildImage` trust bridge.

The current branch advances container evidence:

- **real retail PNST reintegration** of an authored same-size MOD child;
- physical slot identity resolved by canonical parser/hash binding rather than filename ordinal;
- rebuilt parent retains exact size and slot table;
- only the three expected authored child bytes change in the complete retail parent image;
- canonical parent reopen/re-expand returns the exact authored MOD bytes.

The current branch also proves a **synthetic** MOD -> container -> NBZ overlay -> reopen chain using the existing NBZ writer/source implementation.

Still open:

- provenance-bound retail NBZ overlay acceptance for the edited MOD chain;
- original `dmc3.exe` no-edit and controlled-edit acceptance;
- retail PAC0 family coverage only if needed for independent container-family evidence;
- broader MOD authoring domains such as transform/skin/material changes.

### SCM

The selected canonical SCM writer/edit stack now has:

- `preserve_layout` same-layout authoring;
- deterministic `canonical_rebuild` layout planning;
- typed geometry, UV, texture-index, alpha/filter, GS CLAMP and node-transform editing;
- mandatory canonical output reparse;
- source-bound mutation authority guards;
- fail-closed rejection of non-zero unmodeled source bytes during layout-changing reflow;
- bounded SCM/texture-companion coherence through the existing texture framing/reflow infrastructure.

Provenance-bound consolidated no-edit corpus:

```text
paths                           78
unique SHA-256 inputs           68
parse                           78/78 PASS
preserve-layout exact parity    78/78 PASS
canonical rebuild + reparse     78/78 PASS
canonical exact no-edit parity  78/78 PASS
```

Receipt: `data/reverse/dmc3-scm-consolidated-corpus-20260909.json`.

This closes the no-edit 68-unique SCM writer corpus gate. Do **not** repeat it unless a new corpus class or contradiction appears.

Still open:

- provenance-bound real same-layout semantic edits across representative SCM domains;
- provenance-bound retail texture rewrite through `ScmResourceBundleWriter`;
- real-retail size-changing canonical rebuild;
- SCM PAC/PNST/NBZ reintegration;
- original `dmc3.exe` acceptance.

### MOT

The modular MOT parser/IR is the canonical structural path. Current evidence includes:

- `MOT\0` marker and aligned header/channel-mask contract;
- nine-bit channel mask and record-count/popcount relationship;
- compression-2 and compression-3 typed key records;
- signed track start-time offset recovery;
- quantization recovery;
- compression-3 linear/Hermite segment algebra and slope orientation from canonical EXE analysis;
- normal nine-channel binding-bit traversal order;
- three hash-bound real MOT payloads parsing successfully through the modular parser.

The current interpolation helper is an algebraic semantic recovery, **not** a claim of bit-identical SSE implementation or complete animation-player parity.

Still open:

- exact segment lookup/cache and duplicate-time behaviour;
- consumers of remaining raw header fields and flag `0x2` alternate binding path;
- other compression modes with executable + real-corpus agreement;
- looping, blending, motion-group selection and full transform composition;
- original-game output comparison.

## No-repeat rules

A new reverse pass for these formats must contribute at least one genuinely new evidence class, for example:

- new executable consumer/producer;
- new corpus class;
- contradiction to current evidence;
- writer/authoring gate;
- container/NBZ reintegration gate;
- original-game acceptance;
- runtime behaviour that resolves a currently open semantic boundary.

Repeating the same structural parse, zero census, branch archaeology or already-passed no-edit corpus is not progress.

## Evidence authority boundary

Keep the following levels distinct:

```text
EXE-confirmed runtime semantics
!= corpus structural confirmation
!= synthetic writer acceptance
!= provenance-bound retail writer/container acceptance
!= original dmc3.exe acceptance
```

No layer may be promoted merely because a weaker layer passed.

## Promotion gate for PR #372

The consolidated stack may leave draft only when the exact final head has:

- Discovery Site success;
- Ubuntu Build + full CTest success;
- Windows Build + full CTest success;
- `behind main == 0` immediately before promotion/merge;
- no unresolved semantic audit finding that weakens preservation or evidence boundaries.

After merge, review superseded remote SCM/MOT branch refs and remove only those proven fully subsumed by the canonical history. Keep any branch only when it still contains unique unreconciled evidence.

## Next frontier after consolidation

Highest-value next work, in order:

1. provenance-bound **retail NBZ overlay/reopen** using the already validated retail MOD-in-PNST output and existing `build-dmc3-overlay` path;
2. provenance-bound real SCM same-layout edit receipts, then SCM container reintegration;
3. MOT segment-selection/runtime parity work rather than another structural parser pass;
4. original `dmc3.exe` acceptance for tightly bounded authored resource chains.

Full MOD/SCM/MOT authoring authority remains false until those stronger gates are independently satisfied.
