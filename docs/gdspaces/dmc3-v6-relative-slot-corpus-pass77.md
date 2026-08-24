# DMC3 v6 real relative-slot corpus receipt — Pass 77

Status: real-artifact Layer-1 parser/classification evidence; **not** a retail writer/game-consumption receipt.

## Source artifact

The inspected source package is the previously archived project corpus:

- file: `DMC 3 RENGINE (6).zip`
- exact size: `237658858` bytes
- SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

The package was reacquired as raw bytes before this pass. No game payload bytes are committed by this receipt.

## Canonical parser execution

A standalone corpus harness was built from the exact current repository implementations without changing parser semantics:

- `src/formats/relative_slot_container.cpp` — Git blob `c8e966918634563904f79c0006e4a2fc78692c99`
- `src/formats/pac.cpp` — Git blob `37edd98ec7c9430fc0ed1f0522b39e85d7acda9f`
- `src/formats/pnst.cpp` — Git blob `e793ba2dcada4a29c219854766a18ad77754afa9`
- `src/formats/container.cpp` — Git blob `20856c005c463eed11e943d9dc218081670998c8`

The harness did not implement an alternate PAC/PNST parser. It only loaded each candidate byte image, invoked `PacParser::parse()` or `PnstParser::parse()`, and emitted topology fields from the returned `ContainerDocument`.

An independent inventory implementation was also run as a cross-check. Canonical C++ and independent inventory agreed on every candidate's accept/reject result and every compared accepted topology field.

## Corpus result

All ZIP members whose first four bytes were `PAC\0` or `PNST` were inspected.

| Dimension | Result |
|---|---:|
| Magic-prefix candidate instances | 59 |
| Canonical structurally valid instances | 46 |
| Canonical rejected instances | 13 |
| Unique structurally valid payload SHA-256 values | 35 |
| Unique valid PAC payloads | 25 |
| Unique valid binary PNST payloads | 10 |
| Unique rejected PNST-prefixed text candidates | 11 |
| Unique valid payloads with one or more empty slots | 8 |
| Unique valid payloads with duplicate non-zero alias offsets | 0 |
| Unique valid payloads with bytes between offset-table end and first payload | 28 |

The absence of aliases in this representative corpus is an observation, not proof that DMC3 can never contain aliases. Product alias handling remains required and is covered synthetically.

## Real topology bounds observed

### Largest slot table / sparsest representative

`analysis_inputs/stage_drops/st445/st445_005.pac`

- actual binary format: PNST
- SHA-256: `08491f49fc713c73a17e3354b5832e3d4e13128d5d4508067422b239e6528ae9`
- container size: `2065312`
- declared slots: `461`
- populated: `51`
- empty: `410`
- distinct physical spans: `51`
- aliases: `0`
- offset-table end: `1852`
- first payload / protected-prefix length: `1856`
- prefix gap: `4`
- minimum physical span: `1216`
- maximum physical span: `536688`

### Largest accepted relative-slot payload

`analysis_inputs/stage_drops/m20_s00/m20_s00_004.pac`

- actual binary format: PNST
- SHA-256: `63ff1b987a475990a4bdc450a84834c8400c7653fedc144330a3400d818cf863`
- container size: `8616160`
- declared slots: `45`
- populated: `45`
- empty: `0`
- aliases: `0`
- offset-table end: `188`
- first payload / protected-prefix length: `192`
- prefix gap: `4`
- maximum physical span: `3559424`

### Largest observed pre-payload gap

`analysis_inputs/stage_drops/em035/em035_010.pac`

- actual binary format: PAC
- SHA-256: `bea48779ee18b4f8bfa36b1213d9fb95b8b36ce72fe05b72bfc895af02c469e8`
- declared slots: `95`
- populated: `45`
- empty: `50`
- offset-table end: `388`
- first payload / protected-prefix length: `400`
- prefix gap: `12`

This confirms that a size-changing packed writer must preserve the complete pre-payload domain and may only patch the bounded offset-table fields. `RelativeSlotPackedReflowWriter` is designed around exactly that invariant.

## PNST text-index boundary discovered

The real corpus contains text `.index` manifests whose first bytes are the literal line prefix:

```text
PNST\r\n
```

These are not binary relative-slot PNST containers. Interpreting bytes `+0x04..+0x07` as binary slot count yields an invalid product envelope.

All 13 PNST-prefixed text-index instances were rejected by the canonical `PnstParser` with `slot_count_limit`. No text-index candidate was accepted as binary PNST.

This creates an evidence-backed classifier rule:

> `PNST` prefix is a probe candidate. Binary PNST container authority requires successful canonical structural validation.

Pass 77 updates `ResourceClassifier` accordingly. Structurally valid binary PNST under misleading `.pac` paths still outranks extension; structurally invalid PNST-prefixed `.index` text falls back to `format=index`, `container=false`.

## What this receipt closes

This pass provides representative real-byte evidence that the canonical PAC/PNST structural parser handles:

- large slot counts;
- heavily sparse slot tables;
- all-populated slot tables;
- non-zero pre-payload gaps;
- large physical spans;
- binary PNST payloads stored under `.pac` names;
- PNST-prefixed text-index false-positive candidates.

It also directly motivated and validates the Pass-77 classification correction.

## What remains open

This receipt does **not** prove:

- intrinsic child-file extent for arbitrary parser-inferred physical spans;
- a real size-changing child edit followed by `RelativeSlotPackedReflowWriter` on these retail-derived payloads;
- a real retail NBZ repack of `dmc3-0.nbz`;
- successful original-game consumption of authored output;
- raw `.lst` corpus validation.

Those remain mandatory Layer-1 gates. The synthetic A-to-Z composition path is green separately; the next evidence step is to select an exact child whose intrinsic extent is independently authoritative, perform a controlled real-corpus size-changing reflow, and then carry that authored root into the retail-NBZ/game-backed boundary when the exact source artifact is available to the product runner.
