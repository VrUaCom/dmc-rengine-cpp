# Readers that existed and could not be reached

**Corpus:** the complete `em000` extraction supplied 2026-09-08.
**Source archive SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`
**Payloads:** 302 game records plus two documentation files.

Scoring the classifier against that extraction moved from **130 to 139**
correctly named payloads. Not one of the nine came from a new reader. Every
one came from a reader this repository already had, that nothing called.

| payload | reader | state before |
|---------|--------|--------------|
| `em000_000.ptx` | `TextureSlotFramingParser` (bundle) | refused the file outright |
| 8 × wrapped DDS | `TextureSlotFramingParser` (wrapped) | accepted them, unreachable |
| `em000_038` | `so::graph` | accepted it, unreachable |
| `em000_039` | `so::link_table` | accepted 305 of 306 |
| `em000_040` | `so::volume_table` | accepted 28 of 306 |

## A reader that recognizes everything tells a classifier nothing

Two of the three SO readers claimed recognition on a length check:

```cpp
if (bytes.empty() || (bytes.size() % record_size) != 0U) return result;
result.recognized = true;   // and nothing else was ever checked
```

Their only error path was truncation, which cannot fire once the length
divides. So `ok()` meant "the length divides by four" and "the length divides
by 0x50" — 305 and 28 of the corpus respectively, motions, models, a scroll
table and a JSON file among them. Those readers could not have been wired into
classification, and the reason they were never wired is that doing so would
have been visibly wrong.

What the corpus shows the records actually agreeing on:

**Volume table** — 23 records of 0x50. Each carries a kind (2 for 22 of them,
4 for one), twelve reserved bytes that are zero in every record, and a first
vector whose w is exactly 1: a position in homogeneous coordinates. Kind 2
places a centre in `vector0` and a radius in `vector1.x`; kind 4 places two
points in `vector0`/`vector1` and a radius in `vector2.x`. Against the corpus
**any one of those three invariants** cuts the 28 matches to one.

**Link table** — a leading word and one four-byte record per volume record, 23
of each. The fourth byte of every record is reserved and zero. Neither
invariant is sufficient alone: without the leading word one `effect-v` record
survives, without the reserved byte the ten `effect-M` companions do. Together
they leave one payload of 306.

The record's third byte is the node a volume hangs off, **not** the record's
own ordinal. It equals the ordinal for most records, repeats where two volumes
share a node, and is zero for the four bound to the root. Reading it as a
self-index makes those four look corrupt; `analysis::so::mod_binding` already
binds it to the MOD transform domain, which is the same reading arrived at
independently.

**Graph** — already strict, and already right: a type-6 block whose
entry-offset table closes exactly on its first entry, and a boundary word
pointing at a type-8 companion with the same property. 1 of 306.

## A bound taken from one corpus, written down as the format's

The texture reader refused `em000.pac` slot 0 — the enemy's own texture
bundle, the first row of the first screen a person sees — with:

```
descriptor auxiliary pair lies outside the corpus-confirmed bounded relation
```

The pack is four descriptors over 487,424 bytes, sector spans 43, 86, 86, 22,
which sum with the 0x800 header to the file's exact length. Their auxiliary
pairs:

| # | compression | aux mode | aux value |
|---|-------------|----------|-----------|
| 0 | DXT1 | 2 | `0x1D308000` |
| 1 | DXT5 | 2 | `0x1D308000` |
| 2 | DXT5 | 0 | `0` |
| 3 | DXT5 | 1 | `0x9D308000` |

The rule required a non-zero mode to come with DXT5. Descriptor 0 is DXT1 with
mode 2, so one descriptor of four refused the whole bundle. The rule was true
of every descriptor the earlier corpus contained and is not true of the format
— the same mistake the MOT reader made with its single track kind, in a
different file.

What the pack does show is kept: the mode and the value are zero together or
non-zero together, and the mode stays within 0–2. Worth recording for the next
pass: the three non-zero values are the same 31 bits, `0x1D308000`, with the
top bit set on the one whose mode is 1 — and the two that share a value also
share their dimensions while differing in compression, so whatever the field
carries, it is not a function of the compression.

## What is left

163 of the remaining misses are effect-pack records (`effect-e`, `effect-v`,
`effect-p`, `effect-g`, `effect-a`, and the ten `effect-M` companions). They
have no standalone identity: the pack's manifest names them, so they are a
naming problem for the container expander and not a classification problem.
`EffectPackParser` already reads that manifest.

The remaining two are `FORMAT_MAP.csv` and a README from the extraction
itself, which are not game data.
