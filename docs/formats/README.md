# DMC Rengine Format Documentation

This directory contains format-specific structural documentation and the canonical DMC3-HD format/purpose inventory.

## Start here

- [Public DMC3 HD file-format and archive index](public-index.md) — query-driven entry point for SCM, MOD, SHW, HITS, PAC, PNST, NBZ, DDS/PTX and the canonical evidence/status sources behind each family.
- [DMC3 HD format and resource-purpose catalog](dmc3-hd-format-catalog.md) — canonical human-readable inventory of all currently observed or named DMC3-HD resource families, their purpose, evidence status, product support and remaining reverse boundary. For `MOD/EFM/SCM/MRP/SHW`, later direct reverse records supersede older broad 3D/render wording where more precise evidence exists.
- [DMC3 HD machine-readable format-purpose registry](dmc3-hd-format-purpose-registry.json) — normalized registry separating identity, subsystem purpose, schema maturity and current clean-product support; use [Current Project Status](../status/current.md) for live promoted reader/integration state.
- [DMC3 HD format presence census](dmc3-hd-format-presence-census.md) — separates formats with real bound payload bytes from EXE-only identities/references, top-level retail members, nested/corpus payloads, generic `.bin`/`.ukn` carriers and project working names such as `SO`.
- [DMC3 HD machine-readable format presence census](dmc3-hd-format-presence-census.json) — machine-readable companion for physical/corpus presence status; presence is intentionally independent from purpose and schema maturity.
- [SCM structural specification](scm.md) — canonical C++20/parser-facing SCM layout and terminal serialized/runtime map: header, objects, fixed 0x50 mesh ABI, streams, hierarchy, transforms, render state, texture ownership and preservation-only fields.
- [DMC3 SCM reverse completion audit — 2026-09-13](../research/dmc3-scm-reverse-completion-2026-09-13.md) — terminal reverse audit: all current SCM serialized/runtime domains are typed/structural, preservation-only, or rejected; writer/integration/original-game acceptance remains a separate track.
- [DMC3 SCM deep reverse — 2026-09-02](../research/dmc3-scm-deep-reverse-2026-09-02.md) — historical canonical EXE + 68-file corpus reconciliation, including the fixed-stride/continuation resolution; later completion records supersede its old open-frontier list.
- [DMC3 SCM runtime object flags — 2026-09-03](../research/dmc3-scm-runtime-object-flags-2026-09-03.md) — direct canonical-EXE mapping for serialized object `+0x01/+0x10` into runtime object state; later render/flag evidence further closes alpha, GS TEX1 and GS TEST/ZBUF technical semantics.
- [DMC3 runtime type-evidence split — 2026-08-31](../research/dmc3-runtime-type-evidence-split-2026-08-31.md) — canonical correction separating the three-byte registry probe, PAC/PNST container dispatcher and four-byte family-mask classifier; supersedes global “exactly five tags / byte 3 never matters” claims.
- [DMC3 primary 3D / render family reverse — 2026-08-31](../research/dmc3-primary-3d-render-family-reverse-2026-08-31.md) — evidence-backed `MOD/EFM/SCM/MRP/SHW` classification, including EFM model/mesh proof, SHW topology ownership and the remaining MRP boundary.
- [DMC3 real MOD / SHW payload binding — 2026-09-01](../research/dmc3-real-mod-shw-payload-binding-2026-09-01.md) — hash-bound payload proof for the MOD five-stream layout and self-contained SHW shadow hulls; supersedes the earlier external-vertex-pool interpretation.
- [DMC3 family-mask `object+0xE0` consumer census — 2026-08-31](../research/dmc3-family-mask-object-e0-consumer-census-20260831.md) — whole-`.text` bounded negative proof: 10 direct discriminator sites specialize only `MOD/EFM/SCM`; no direct `MRP/MCV/SHW` branch was recovered.
- [DMC3 HD format-purpose closure pass — 2026-08-27](../research/dmc3-format-purpose-closure-pass-2026-08-27.md) — earlier canonical-EXE dispatcher/parser closure pass; retain as historical evidence, subject to later corrections.
- [HITS collision resource](hits.md) — current collision-grid/triangle layout; supersedes the obsolete `HITS$` scanner interpretation.
- [PAC read-only structural parser](pac-readonly-parser.md)
- [PNST read-only structural parser](pnst-readonly-parser.md)
- [Generic container foundation](container-foundation.md)
- [Synthetic slot container](synthetic-slot-container.md) — test-only format; never game evidence.

## Authority rule

A filename or extension is never sufficient format authority.

Use this order when evidence is available:

```text
validated magic / structural grammar
 -> original EXE consumer or typed post-load path
 -> hash-bound corpus structure
 -> index/path label
 -> fallback/unknown
```

When the executable exposes multiple type-identification sites, keep their scopes separate rather than merging them into one global detector. A registry content probe, a container child dispatcher and a higher-level family classifier may recognize overlapping families while using different byte widths and semantics.

The catalog, purpose registry and presence census deliberately separate:

- **resource identity** — what family the bytes belong to;
- **resource purpose** — what subsystem uses the family;
- **binary schema maturity** — how much of the byte layout is understood;
- **original-runtime evidence** — what canonical DMC3 code actually recognizes/consumes;
- **physical/corpus presence** — whether real bytes are currently bound to a retail/corpus source, as opposed to EXE-only recognition/reference;
- **DMC Rengine product support** — what the current C++ repository can recognize, parse, edit or export.

A known purpose does not imply a complete schema, a parser does not imply original-game semantic equivalence, and an EXE-recognized family does not imply that a payload file has already been found in the current corpus.

## Evidence vocabulary

- `GAME_VERIFIED` — observed through the original game in a bounded test.
- `EXE_CONFIRMED` — direct canonical executable evidence.
- `DATA_CONFIRMED` — repeatable binary/corpus structure evidence.
- `HIGH_CONFIDENCE` — convergent evidence, but one promotion boundary remains.
- `CANDIDATE` — plausible and evidence-linked; not canonical semantics.
- `RESEARCH_REQUIRED` — identity or label exists, exact purpose/schema unresolved.
- `CAPABILITY_ONLY` — executable/media support exists; shipped DMC3 presence is not claimed.
- `REJECTED` — superseded or contradicted claim; not current authority.

For terminal field-level MOD/SCM reverse work, the stricter canonical vocabulary also includes `CORPUS_CONFIRMED`, `EXE_AND_CORPUS_CONFIRMED`, `STRUCTURAL_CONFIRMED`, `PRESERVED_UNDECODED` and `RESERVED_OBSERVED_ZERO`. A preservation-only terminal state is not equivalent to `RESEARCH_REQUIRED`.

Historical research remains useful acquisition evidence but does not override stronger current implementation or later canonical reverse documentation.