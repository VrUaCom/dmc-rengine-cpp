# DMC Rengine Format Documentation

This directory contains format-specific structural documentation and the canonical DMC3-HD format/purpose inventory.

## Start here

- [DMC3 HD format and resource-purpose catalog](dmc3-hd-format-catalog.md) — canonical human-readable inventory of all currently observed or named DMC3-HD resource families, their purpose, evidence status, product support and remaining reverse boundary.
- [DMC3 HD machine-readable format-purpose registry](dmc3-hd-format-purpose-registry.json) — normalized format-purpose registry separating identity, subsystem purpose, schema maturity and current clean-product support.
- [DMC3 runtime type-evidence split — 2026-08-31](../research/dmc3-runtime-type-evidence-split-2026-08-31.md) — supersedes the global “exactly five tags / fourth byte ignored” interpretation by separating the registry three-byte probe, container dispatcher and four-byte family-mask classifier; adds byte-backed MCV and EFE/EFW dispatcher evidence.
- [DMC3 primary 3D/render ABI reverse — 2026-08-31](../research/dmc3-primary-3d-render-abi-2026-08-31.md) — direct EXE/fixup/shader evidence that MOD, EFM, SCM and SHW are geometry-bearing families with distinct ABI boundaries; MRP remains an unresolved primary render family.
- [DMC3 HD format-purpose closure pass — 2026-08-27](../research/dmc3-format-purpose-closure-pass-2026-08-27.md) — earlier direct canonical-EXE dispatcher/parser evidence; retain as history, with the 2026-08-31 correction passes taking precedence where they explicitly supersede it.
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

The catalog and registry deliberately separate:

- **resource identity** — what family the bytes belong to;
- **resource purpose** — what subsystem uses the family;
- **binary schema maturity** — how much of the byte layout is understood;
- **original-runtime evidence** — what canonical DMC3 code actually recognizes/consumes;
- **DMC Rengine product support** — what the current C++ repository can recognize, parse, edit or export.

A known purpose does not imply a complete schema, and a parser does not imply original-game semantic equivalence.

## Evidence vocabulary

- `GAME_VERIFIED` — observed through the original game in a bounded test.
- `EXE_CONFIRMED` — direct canonical executable evidence.
- `DATA_CONFIRMED` — repeatable binary/corpus structure evidence.
- `HIGH_CONFIDENCE` — convergent evidence, but one promotion boundary remains.
- `CANDIDATE` — plausible and evidence-linked; not canonical semantics.
- `RESEARCH_REQUIRED` — identity or label exists, exact purpose/schema unresolved.
- `CAPABILITY_ONLY` — executable/media support exists; shipped DMC3 presence is not claimed.
- `REJECTED` — superseded or contradicted claim; not current authority.

Historical research remains useful acquisition evidence but does not override stronger current code or later canonical reverse documentation. When a later evidence pass explicitly marks an older statement as superseded, the later site-scoped statement is authoritative.
