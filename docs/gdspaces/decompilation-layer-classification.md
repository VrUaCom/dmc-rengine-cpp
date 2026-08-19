# GDSpaces decompilation-layer classification

Canonical reconciliation: 2026-08-19.

This document classifies existing GDSpaces/resource-runtime reverse, implementation, and validation work by the already-established decompilation layers. It does not define a new workflow. Its purpose is to prevent Layer 1, Layer 2, Layer 3, and validation progress from being reported as if they were interchangeable.

## Canonical tags

### [L1] Resource Materialization

Physical/container bytes -> bounded parse/read -> decompression/transform -> exact materialized resource bytes -> nested extraction -> editable `WorkingCopy` -> writer/rebuild/repack -> reopen/round-trip.

Layer-1 completion is not satisfied by enumeration, lookup, static reverse, or read-only parsing alone. The current GDSpaces execution priority is Layer 1 and ultimately includes edit/rebuild/repack/round-trip validation.

### [L2] Resource Resolution

Logical request -> candidate/path construction -> normalization -> provider/source/volume selection -> duplicate/ambiguity behavior -> exact `ResourceRef` identity.

Layer 2 answers which resource is selected, not how selected storage becomes editable bytes.

### [L3] Original Runtime / Lifecycle

Original scheduler/FileSlot ownership beyond the minimum byte-read contract -> callbacks -> `LoadedResource` states -> typed post-load -> claims/releases -> cancellation/reset/unload/shutdown.

Recovered original functions/types here belong to the Recovered Game Source Tree/runtime authority, not GDSpaces product ownership.

### [V] Validation / Equivalence

Cross-cutting validation boundary, not a fourth decompilation layer: hash-bound real-corpus receipts, exact-build pairing, original-vs-reconstruction comparison, Windows/Ubuntu product CI, and Level-E behavioral validation.

### [OUTSIDE] Product/tooling metadata

Useful product information that is not established original-game runtime behavior. Example: external `.index` naming manifests.

## Classification matrix

| Area / finding | Layer | Boundary |
|---|---|---|
| NBZ/ZIP EOCD, central/local records, storage spans | L1 | Physical archive representation |
| STORE/raw-DEFLATE materialization | L1 | Stored bytes -> materialized bytes |
| ZipRawSubstream / ZipEntryStream / inflater / compressed seek | L1 | Original evidence supporting byte materialization |
| PAC structure, sparse/empty/duplicate slots | L1 | Container bytes -> member views |
| PNST shared relative-slot structure / nested extraction | L1 | Container bytes -> member views |
| PACK raw binary schema/materialization | L1 | OPEN until raw/schema authority exists |
| Binary AFS backend, if directly evidenced | L1 | Current `.afs/` strings do not prove it |
| `.lst` grammar, sparse slots, synthesized layout/bytes | L1 | Loose representation materialization |
| `.lst` packed-first / fallback / sibling `.pac` precedence | L2 | Representation selection policy |
| ByteProvenance | L1 product support | Byte-domain lineage |
| `WorkingCopy` | L1 product support | Editable materialized domain |
| Writers / serializers / rebuild / repack | L1 | OPEN current-priority work |
| Reopen/reparse/round-trip receipts | L1 + V | Required for Layer-1 completion |
| `DMC3-N.nbz` bootstrap / first-gap / N..0 precedence | L2 | Source/volume selection |
| `%d` non-negative runtime index domain | L2 | Numbered volume identity |
| `OpenGameResource` six-prefix / twelve-attempt plan | L2 | Candidate planning |
| `ResourcePathNormalize` 0x0E archive / 0x0C physical | L2 | Provider-key/path identity |
| Archive normalized lookup / `ResourceKeyIndex` | L2 | Lookup representation |
| Duplicate normalized-key ambiguity | L2 | No invented semantic winner |
| Physical-vs-archive provider selection | L2 | Final byte read crosses into L1 |
| `.afs/`, `Video/`, `afs/sound/`, `GData*.afs/` namespaces | L2 | Logical namespaces, not binary AFS proof |
| `.index` manifests | OUTSIDE | Product/extraction metadata; not original L2 authority |
| ResourceTypeInfo path vs basename/open-surface question | L2 | Still partially unresolved |
| Minimum selected backend/range read into caller buffer | L1 support | Boundary to original I/O runtime |
| FileSlot global pool ownership / request tickets / AsyncIO worker | L3 | Original runtime ownership |
| scheduler helpers / callback lifecycle / `modeFlag` | L3 | `modeFlag` numeric meaning unresolved |
| `LoadedResource` states 0/1/2/3/4 | L3 | Original lifecycle |
| typed post-load / MOD/EFM/SCM/SHW | L3 | Materialized bytes -> game-ready object |
| loader-node claims/releases | L3 | Higher-level ownership |
| scene transition/reset/unload/shutdown | L3 | Original lifecycle |
| Stage catalog selector/descriptor universe | Outside core L1 | Consumer/runtime identity + validation input |
| Product safety bounds/CRC/budgets | Tag protected layer | Product policy unless original behavior is separately evidenced |

## Cross-boundary rules

- `.lst` is intentionally mixed: synthesized bytes are L1; packed/list representation choice is L2.
- FileBackend/FileSlot is a boundary: enough range-read behavior to explain selected bytes can support L1; original pool/scheduler/callback ownership is L3.
- Physical provider selection is L2; reading/materializing the selected file is L1.
- PAC/PNST parsing needed to expose member bytes is L1; original typed consumer construction after those bytes exist is L3.
- Product hardening never becomes original-equivalence evidence merely because it protects L1 or L2 code.

## Recent reverse-pass classification

- Pass 45 — resolver ownership / archive-index provenance / physical-provider review: **L2**, with an L3 lifetime edge.
- Pass 46 — `.lst` grammar/materializer: **L1 + L2**.
- Pass 47 — full-path/exact-path interpretation: **L2**, later corrected.
- Pass 48 — `0x1402EF4D0` scheduling/materialization-wrapper investigation: primary **L3**, unresolved possible L2 ingress.
- Pass 49 — exact-path vs `OpenGameResource` correction: **L2 + L3** boundary.
- Pass 50 — inherited `modeFlag` classification: **L3**.
- Pass 51 — physical 0x0C vs archive 0x0E case normalization: **L2**.
- Pass 52 — distribution media case-consistency corpus check: **V supporting L2**.
- Pass 53 — current-main `.lst` integration-seam audit: **L1 + L2** product integration boundary.

## Canonical current PR/implementation classification

- #101 ByteProvenance: **L1**.
- #102 PAC structural decoder: **L1**.
- #103 PAC real-corpus receipt: **V supporting L1**.
- #104 PNST/shared relative-slot core: **L1**.
- #105 provenance fail-closed hardening: **L1 product support**.
- #109 DMC3 PAC/PNST parser registry: **L1 parser ingress**.
- #112 recursive PAC/PNST expansion: **L1**.
- #115 RawDeflate: **L1**.
- #116 recursive parse reuse hardening: **L1 product hardening**, not original L3 cache semantics.
- #118 NBZ source: **L1**.
- #120 path normalization: **L2**.
- #122 candidate plan: **L2**.
- #124 candidate-plan hardening: **L2 product hardening**.
- #125 NBZ budgets/bounds hardening: **L1 product hardening**.
- #129 numbered-volume bootstrap: **L2**.
- #133 ResourceKeyIndex ownership: **L2**.
- #135 signed `%d` volume-domain correction: **L2**.
- #136 ordered runtime resolver: **L2**.
- #137 `.lst` loose-container synthesis: **L1 + L2**.

## Current priority accounting

For the present GDSpaces assignment, primary execution accounting is **Layer 1**. Layer 2 and Layer 3 findings remain valid and must be preserved, but they do not advance Layer-1 completion unless they directly unblock a Layer-1 materialization/rebuild/round-trip requirement.

Current Layer-1 open gates include:

- exact clean PNST real-corpus execution receipt;
- representative real `.lst` receipt;
- PACK raw/schema acquisition if PACK remains in supported materialization scope;
- production selected source/member -> exact materialized bytes with provenance;
- editable `WorkingCopy` -> supported writer/serializer;
- topology/identity-preserving rebuild/repack output;
- reopen/reparse and deterministic round-trip comparison;
- representative legal real-corpus receipt from NBZ member through nested container to edited/rebuilt output;
- explicit product policy for NBZ write/repack versus mod-overlay output before Layer-1 closure is claimed.

Any older status wording that counts resolver/lifecycle progress as Layer-1 completion is superseded by this classification. Those findings remain valid under L2/L3 unless separately corrected by evidence.
