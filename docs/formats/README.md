# DMC Rengine Format Documentation

This directory contains format-specific structural documentation and the canonical DMC3-HD format/purpose inventory.

## Start here

- [DMC3 HD format and resource-purpose catalog](dmc3-hd-format-catalog.md) — canonical inventory of all currently observed or named DMC3-HD resource families, their purpose, evidence status, product support and remaining reverse boundary.
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

The catalog deliberately separates:

- **resource purpose** — what the family is used for;
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
- `REJECTED` — superseded or contradicted claim; not current authority.

Historical research remains useful acquisition evidence but does not override stronger current `main` code or later canonical reverse documentation.
