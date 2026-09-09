# Unified MOD, SCM and MOT research frontier

Working branch: `reverse/mod-completion-20260907`.

This extends the existing MOD consolidation ledger to SCM and MOT. It is a
research checkpoint, not a declaration that the game or its formats are fully
recovered. Existing MOD writer/PAC changes on GitHub through
`dbc87b27a94364cad4451028c654826ad3641ea7` were reconciled before publication;
the imported snapshot's complete tree matched `6ffb1d0a6b6cfe27c5216db1f595d886dd0467ef`.

## Consolidation decisions

| Source | Disposition |
|---|---|
| Remote `scm`, `e52a3a6bca3305e0833bf33ef5ec92f59e7a4db3` | Writer, edit API, corpus command and unique SCM evidence integrated; newer canonical runtime code retained |
| Remote `reverse/mot-em000-20260908`, `122ec051fb9c727ba2db91ff4945e15e197cd399` | Modular MOT parser/IR and corpus research integrated |
| Existing MOT summary parser | Replaced byte decoding with projection of modular parser; historical stricter acceptance policy retained |
| Historical model-family, skin and transform branches | Unique research salvaged; obsolete implementation not reinstated; see existing MOD branch ledger |
| Local unpublished `scm`, `d94838dbe9cd9a121a18d06345f8fb08487f9bf7` | Alternate writer not installed alongside selected writer; older 68-file result independently repeated with selected code; remaining runtime-census claims require provenance reconciliation |

No historical remote branch deletion is recorded at this checkpoint. Keeping
an archival ref does not designate it as an active research branch. The local
unpublished SCM variant must not be deleted while its remaining evidence is
under review. Runtime-wave branches with unrelated code are not wholesale
merged by this checkpoint.

## Fresh validation

All 202 non-CLI core translation units compile with GCC 13/C++20. Sixteen
focused test executables pass: SCM, SCM writer, SCM transform inverse, MOT,
MOT structural, MOT key decoding, MOD, MOD skin, MOD transform domain, model
family, runtime registry, resource analyzer structural formats, classifier,
MOD writer, MOD writer corpus and MOD authored-child bridge.

The selected SCM corpus command reads all 78 available paths, representing
68 distinct SHA-256 payloads and 10 duplicate paths. Both writer modes produce
byte-identical output for every input, and canonical output reparses.
Receipt: `data/reverse/dmc3-scm-consolidated-corpus-20260909.json`.
This is unchanged-payload parity, not arbitrary scene-edit or in-game parity.
Full CMake/Windows CI remains separate from this local compilation.

The complete CLI also compiles and links locally (15 translation units).
The normal low mask triplet is now proven as scale XYZ through the matrix
consumer at `0x14030E9B0` and basis-scaling helper `0x14032ED30`.

Fresh MOT static recovery is documented in
[the key-evaluation report](dmc3-mot-key-evaluation-2026-09-09.md).
Three real MOT payloads also pass the modular structural parser; they do not
provide whole-animation playback parity.

## Next unresolved boundaries

| Boundary | Evidence needed |
|---|---|
| MOT segment lookup | Compression-3 static search now recorded; implement and compare cache-dependent duplicate-time behaviour and companion routines |
| MOT header semantics | Consumers of raw +0x08/+0x0C/+0x10/+0x14/+0x18/+0x1A, including flag 0x2 alternate binding |
| Other MOT compression modes | Executable decoder/evaluator agreement plus real payloads |
| Animation runtime | Looping, blending, group selection, transform composition and comparisons with game output |
| SCM remaining fields | Reconcile unpublished census against canonical pointer lineage; preserve unknown bytes meanwhile |
| MOD remaining fields | Follow existing no-repeat frontier for bit21 and secondary unknowns; absence of direct reads does not prove unused |
| Authoring closure | Edited models/scenes, relocation and full game consumption beyond current no-op and bounded-edit gates |

Confirmed structural facts, static semantic recovery, corpus observations and
runtime parity remain distinct evidence levels throughout this branch.
