# The records an effect manifest does not name

**Corpus:** the complete `em000` extraction supplied 2026-09-08.
**Source archive SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`
**Pack:** `em000_041.pnst` — 173 manifest lines, 183 populated records.

## What was wrong

`EffectPackParser` required the manifest's line count to equal the container's
populated record count, and refused the pack outright when they differed:

```cpp
if (document.manifest_line_count != document.populated_record_count) {
    → EffectPackParseError::line_count_mismatch
}
```

That equality held for the two packs the reader was recovered from —
`st001_effect.pac` and `st114_effect.pac` — and is not a property of the
format. em000's pack has ten more records than lines, so the whole pack was
refused, and **all 173 named records reached the browser without names** over
ten the manifest never claimed. It is the largest single reason an effect
archive listed as placeholders.

The reader had no test. That is how the rule survived a corpus that
contradicts it.

## The ten

They are byte-identical. Every one is sixteen bytes: a `0x31` word followed by
twelve zeros. Each sits in the slot immediately after a record the manifest
calls `M`.

Setting exactly those aside, **manifest line k names record k for all 173** —
kind and identifier, in order, no exception. That is the whole rule.

The reference extraction names each companion after the record before it
(`..._165_M17.effect-m-companion`), but the bytes carry no identifier: all ten
are the same sixteen bytes. The naming is that tool's attribution, not
something the payload says. So the reader recognizes a companion by its
constant and records the adjacency to `M` as observed rather than required — a
companion that followed another kind would still be a companion.

A record left over after the companions are set aside is still a refusal. The
reader may not invent a name for a payload the manifest does not claim.

## Two kind readings the corpus moved

| kind | was | is | why |
|------|-----|-----|-----|
| `G` | absent | 96, fixed | 12 records; the earlier two packs contained none |
| `M` | absent | variable | 13 records, all reading as MOD payloads |
| `P` | 704, fixed | variable | 34 records across **four** extents: 336, 528, 704, 896 |

`P` is the one that was wrong rather than missing. Its fixed 704 was recovered
from two records; em000 holds thirty-four and they disagree. The four observed
extents are kept in the contract as a record of what exists, not as a
constraint — a fifth would be another payload, not a malformed one.

This is the third time in this corpus that a property of one sample was
written down as a property of the format: the MOT reader read one track kind
as the only kind, the texture reader read a DXT5 coincidence as a rule, and
this reader read one pack's line arithmetic as the format's.

## What this does not establish

No original executable read site for the manifest text has been found. The
contract has said so since #254 and still does. What is recovered here is the
pack's own arithmetic, not the runtime's use of it.

The companion's sixteen bytes are recognized and not interpreted. `0x31` is
recorded as the value it is; nothing here claims to know what it means.
