# DMC3 HD resource name tables (2026-09-14)

Where the resource names actually live, and the record layout of the cutscene
localisation table.

Follows [the function map](dmc3-function-map-2026-09-14.md), which established
that only 9 of 7,389 functions reference a literal naming a resource family —
and left open *why*.

**Evidence:** [`dmc3-hdc-name-tables.evidence.json`](../../evidence/executable/dmc3-hdc-name-tables.evidence.json)

```bash
dmc-rengine analyze-exe /path/to/dmc3.exe --out report.json   # name_tables section
```

> **On content.** Only each run's layout and one representative name are
> recorded. The tables hold game data, and extracting their contents into this
> repository is prohibited by [`CONTRIBUTING.md`](../../CONTRIBUTING.md).

## Names are fixed-width arrays, not constants

222 runs of fixed-width NUL-padded name fields hold 5,692 entries in read-only
data. Each run's stride is sized to its own longest name, rounded up:

| Base RVA | Stride | Entries | Longest name | Pure array | Sample |
| --- | --- | --- | --- | --- | --- |
| `0x4F22C8` | 24 | 1,747 | 23 | yes | `afs/sound/m03_s00.adx` |
| `0x4FC6B0` | 24 | 1,011 | 22 | no | `id\id300\id358I.pac` |
| `0x4EB4B0` | 32 | 78 | 28 | no | `motion\pl000\pl000_00_0.pac` |
| `0x361560` | 16 | 48 | 12 | yes | `m18_c00.adx` |
| `0x4EC460` | 24 | 43 | 18 | yes | `se\snd_com0a.pac` |

That answers the open question. An indexed array is reached by its **base**, so
only the code that walks it refers to the names at all. Six referrers for 7,118
`.pac` names was never a weak result — it was the correct one.

"Pure array" means nothing but NUL padding follows each terminator. A `no` means
each element is a *record* with a leading name field and further payload, which
is the thread the next section pulls.

### A stride scan must respect string boundaries

The first pass reported 191 runs holding 5,824 entries. Both numbers were wrong,
and the failure mode is worth recording because it is not obvious: a stride can
land *inside* a real name and still read a printable, NUL-terminated tail. Four
bytes into `wandering.pac` sits `ering.pac` — perfectly valid-looking — so a
phantom grid marches on across unrelated tables.

Requiring every element to begin immediately after a terminator gives 222 runs
and 5,692 entries. The *direction* of the change is the signature of the fault:
fewer entries spread over more runs, because grids spanning several real tables
broke into their genuine constituents.

A synthetic fixture with deliberately wandering spacing now guards this. It is
what caught the bug.

## The 352-byte cutscene localisation record

One run resisted the "pure array" reading: 49 records at RVA `0x4ECCB8` with a
352-byte stride, every one carrying payload after its name. It resolves exactly.

```text
+0x000   32 bytes   cutscene archive path
+0x020   40 bytes   message text, japanese
+0x048   40 bytes   message text, english
+0x070   40 bytes   message text, french
+0x098   40 bytes   message text, german
+0x0C0   40 bytes   message text, italian
+0x0E8   40 bytes   message text, spanish
+0x110   40 bytes   message text, chinese
+0x138   40 bytes   message text, chinese (second variant)
                    = 32 + 8 x 40 = 352
```

No residue. The language order is identical across all 49 records, and records
differ **only** in the scene identifier embedded in each of the nine paths —
consistent with one record per cutscene.

This also explains a reading artifact from the scan: the message slots sit 40
bytes apart, so the scanner reports stride-40 runs *inside* this table. Both
descriptions are true at different levels. The scanner recovers the innermost
regular spacing; a composed period like 352 is a higher-order structure found by
looking at what surrounds the run.

The six functions referencing `demo\…\*.pac` literals, identified in the
previous layer, are the consumers of this table.

## Extension census

Counting terminated name literals by trailing extension:

| Extension | Count | | Extension | Count |
| --- | --- | --- | --- | --- |
| `.pac` | 7,118 | | `.fxh` | 108 |
| `.txt` | 920 | | `.bin` | 58 |
| `.adx` | 306 | | `.sfd` | 32 |
| `.hlsl` | 283 | | `.tm2` | 9 |
| `.ogg` | 154 | | `.mod` | 8 |

Four families were not in the project's list: `.adx`, `.ogg`, `.sfd`, `.fxh`,
plus `.tm2`. They are now in the classifier.

`.sfd` earns a note. Sofdec video names appear alongside the `FullMotionVideo`
classes recovered from RTTI and the Media Foundation imports — **three
independent parts of the image agreeing on a video subsystem**, which is the
strongest form of corroboration this method produces. `.tm2` is consistent with
the title's PlayStation 2 lineage.

Counting a name's extension establishes what the *name* says. It does not
establish that the image implements a reader for that family.

## An existing boundary, corroborated

The largest name array holds entries under an `afs/sound/` prefix with **forward
slashes**, while resource paths elsewhere in the same image use backslashes.

The project's architecture already states that `.afs/` strings are evidence of a
logical namespace rather than of a binary AFS backend. This corroborates it from
a new direction: the namespace appears as stored table text, and nothing here
shows a container implementation behind it.

## Records with fields of differing widths

The constant-stride scan cannot see the localisation table at all. Its leading
field is 32 bytes while its message fields are 40, so the gap sequence is
`32, 40, 40, 40, 40, 40, 40, 40` — and every single-stride hypothesis breaks at
each record boundary.

Looking for a period in the **gap sequence** instead finds records whole. That
recovers 25 layouts holding 5,076 name fields, with 2, 4, 9, 12, 16 or 32 fields
per record.

| Base RVA | Record | Fields | Records | Field widths | Extensions |
| --- | --- | --- | --- | --- | --- |
| `0x5038E8` | 80 | 4 | 119 | 16, 16, 24, 24 | all `pac` |
| `0x502530` | 80 | 4 | 56 | 24, 24, 16, 16 | all `pac` |
| `0x36B308` | 264 | 16 | 51 | 24 then 15×16 | all `pac` |
| `0x4ECCB8` | 352 | 9 | 49 | 32 then 8×40 | `pac`, 8×`txt` |
| `0x35FE68` | 264 | 16 | 23 | 16,16,16,24 then 12×16 | `adx`,`pac`,`pac`,`pac` ×4 |
| `0x35F448` | 200 | 12 | 13 | 11×16 then 24 | `pac`×2, `afs`, `pac`×9 |

A layout is accepted only when every field of every record holds a name
terminated inside that field's width — a run of 51 records validates its pattern
816 times.

### The detector re-derives the localisation table on its own

Row four above is the 352-byte record recovered by hand earlier in this document:
same base, same nine fields, same widths, same extension sequence, same 49
records. An automatic pass arriving independently at the identical field table is
confirmation of both the layout and the detector.

### Two ways the search can mislead, and the checks for them

**A multiple of the real period.** Maximising coverage can settle on `2p` or `4p`
where `p` fits equally well. The detector reduces to the smallest divisor at
which both widths *and* extensions repeat.

The run at `0x35FE68` shows why widths must be part of that test. Its extensions
repeat every four fields — `adx, pac, pac, pac` — but its widths do not: field 3
is 24 bytes wide in all 23 records while the corresponding field of each later
group is 16. It is a genuine 16-field record, and reducing on the extension
pattern alone would have misreported it as four fields.

**Alignment-padded pools wearing record clothing.** Two runs holding
parameter-style names (`FogColor`, `AlphaBlendMode`, `ShadowDarkness`,
`SPEED_CHG`, `GRAVITY`, `UVScroll`, `CnsTrack`) appeared to carry payload after
their terminators, which reads as a record with a leading name field. They are
nothing of the kind: the payload decodes as *further names*, and the offset where
it begins differs between elements.

That is the signature of a pool padded to an alignment boundary, whose names fall
on a regular spacing only where consecutive lengths happen to allow it. A record
table shows the opposite — payload at the same offset in every element. The
scanner now measures both the payload's text-likeness and the consistency of its
offset, and reports both rather than issuing a verdict.

## Open work

- **non-name fields.** Every layout recovered so far is made entirely of name
  fields. A record mixing names with numeric fields would not be found by gap
  periodicity, because the numbers leave no candidate name to measure from;
- **table base to function.** Six functions index the `.pac` array; recovering
  how they compute the index would tie a name to a call site;
- **the `id\idNNNN\` numbering.** The two largest arrays are dominated by a
  structured identifier scheme whose meaning is not established here.
