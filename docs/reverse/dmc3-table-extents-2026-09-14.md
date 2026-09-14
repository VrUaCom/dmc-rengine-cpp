# DMC3 HD table extents (2026-09-14)

Closing the blind spot left open by
[the name-table work](dmc3-name-tables-2026-09-14.md), and the check that found
it.

**Evidence:** [`dmc3-hdc-table-extents.evidence.json`](../../evidence/executable/dmc3-hdc-table-extents.evidence.json)

```bash
dmc-rengine analyze-exe   /path/to/dmc3.exe --out analysis.json
dmc-rengine map-functions /path/to/dmc3.exe --all --out map.json
python research/sql/import_executable_reverse.py \
    --analysis analysis.json --map map.json --db research-private/reverse.sqlite3
```

> **On content.** Only each run's layout, linkage and one representative name
> are recorded, as before.

## The check

A constant index is folded into its displacement, so every reference from code
arrives as an exact byte offset into the table. The *spacing* of those offsets
is therefore a measurement of the element size — one taken entirely from the
instruction stream, owing nothing to how the bytes were read.

Two measurements of the same quantity can disagree, and where they do, the byte
reading is the one to doubt. Two runs disagreed:

| Run | Declared stride | Reference offsets | Pitch |
| --- | --- | --- | --- |
| `0x506F68` | 16 | 0, 16, … 240 | 16 |
| `0x506F68` | 16 | 256, 264, 272, 280, 288, … 336 | **8** |
| `0x36F288` | 16 | 456, 464, 472 | **8** |

Neither run is indexed at 8 throughout. In both the pitch *changes partway*, and
the point at which it changes is the end of the real table.

This needed no new analysis — only references already recovered — and it is the
check that would have caught the blind spot without waiting for anyone to doubt
the result.

## What was actually wrong

Not a pool that resembles a table. A real table whose extent ran past its own
end.

`0x506F68` holds sixteen rendering parameter names. Immediately after them the
image holds a format string and ten file-extension variants, each padded to an
eight-byte boundary. The stride-16 grid walks off the end of the table and keeps
validating, because every second pool string lands on a 16-byte boundary and the
ones between land at offset 8 — *the same offset every time*.

That is exactly what the pool rejection asks about. It requires an element's
payload to be all zero or text at a consistent offset, and a pool aligned to a
sub-multiple of the stride gives it perfect consistency. The rule was satisfied
by the thing it exists to catch.

## The rule that separates them

Payload *position within the run*, not payload offset within an element.

An element of a name table is a name and NUL padding. Payload can therefore only
appear once the grid has left the table — as an unbroken run at the end. So: a
run is trimmed when its payload-bearing elements form a **strict suffix**
containing at least one text payload.

The suffix requirement is what protects real tables. Payload from the *first*
element is a record whose second name field a constant-stride scan cannot see,
and trimming that would delete a genuine table rather than correct one.

What remains after trimming is not second-guessed. A head too short to be a
table fails the minimum entry count and the run goes entirely, which is the
right answer: three names followed by a pool is evidence of a pool.

## Result

| Run | Entries before | Entries after | Absorbed |
| --- | --- | --- | --- |
| `0x506F68` (`FogColor`) | 22 | 16 | 6 |
| `0x36F288` (`Maguma.ogg`) | 30 | 28 | 2 |
| `0x4EC868` (`message\japanese\msg_jpn.pac`) | 19 | 16 | 3 |

Four runs trimmed of 14 absorbed elements. With the record reconciliation below
applied as well, the scan reports 223 runs holding 5,543 entries — of which 44
stand on their own and 179 are a record's interior.

### The tail is a table too

Trimming does not consume the elements it removes, so the scan reads the region
again on its own terms. The tail taken off `0x506F68` comes back as a run at
`0x507068` with a stride of **8** and 11 entries — and all 11 are named by a
constant index from two functions.

That is the strongest confirmation available for both readings at once: the
pitch of 8 that contradicted the old stride is the new run's declared stride,
and the contradiction is gone.

Across the image, references the scan could only place *inside* an element fall
from 16 to 7, while total references rise from 202 to 206. Nine mid-element
references became clean element references.

The seven that remain sit at offsets of 13, 16 and 20 under strides of 16 and
24. That is not the sub-multiple pitch of an absorbed pool, and points instead
at genuine field access within a record — a separate question.

## The other extent error: record interiors

A record whose name fields are all one width **contains a constant-stride grid
by construction.** Both scans then describe the same bytes, and both report
them.

The record array at `0x35D640` is the plainest case. Its 264-byte record is
fifteen 16-byte name fields followed by one of 24:

```text
record 0  [16][16][16][16][16][16][16][16][16][16][16][16][16][16][16][   24   ]
          \___________________ reported as a 15-entry table ___________/
record 1  [16][16] ...
```

The stride scan reports those fifteen fields as a table — once per record, 20
times over. Across the image **179 of 223 runs** coincide with a block of
equal-width fields in one of the 25 record layouts, leaving **44 that stand on
their own.**

They are kept and marked rather than deleted; the record is simply the fuller
reading of the same bytes, and counting both as tables counts the bytes twice.

### And a grid does not stop at the block's end

Nothing halts a constant-stride walk at a field of a *different* width whose
name is short enough to terminate inside the walk's stride. So a run started
inside a record keeps going — into the next field, the next record, or out of
the record array altogether. **87 runs overrun their field block, by 270
elements in total.**

The 352-byte localisation record shows it cleanly. Its fields are one of 32
bytes then eight of 40; a run starting at field 2 has seven 40-byte fields ahead
of it, yet eleven separate runs claim eight. The eighth element is the *next
record's* 32-byte field, read as though it were 40.

| Run | Claimed | Field block | Overrun |
| --- | --- | --- | --- |
| `0x4ED6A0` and 10 others | 8 × 40 | fields 2–8 of `0x4ECCB8` | 1 |
| `0x4F0F00` | 16 × 40 | fields 2–8 of `0x4ECCB8` | 9 |
| `0x361560` | 48 × 16 | fields 4–15 of `0x35FE68` | 36 |

The record's widths repeat with a measured period; the grid's evidence is only
that names keep terminating. The record wins, and the run is cut back.

A block shorter than the minimum entry count is **kept**. The minimum exists to
stop a stride *hypothesis* being reported on the strength of a few coincidences,
and a seven-field block that two scans agree on is not a hypothesis. Erasing
those blocks was the first thing this pass got wrong, and the references caught
it — see below.

## The references are the check

Cutting the interiors back left references pointing into bytes no run covered.
The matcher checked only the span with the nearest preceding base, and an
interior sitting inside a record array starts later and now ended sooner, so it
shadowed the record that encloses it: 14 references vanished and the tables
reached fell from 28 to 17.

Considering every span that can contain an address, and preferring the record
reading, restores **all 206 references** and moves 18 of them from a grid
position to a record and a field — references resolved against a record layout
rise from 8 to 26.

That the count comes back *exactly* is the check that the reconciliation removed
duplication rather than evidence. A reconciliation that loses a reference has
deleted a table the code demonstrably reads.

## The fifth pool mode, and the last mid-element reference

Four mid-element references remained after the reconciliation, plus three more
that were not what they looked like. Chasing all seven closed two more things.

### Names that share a length class

Three of the seven pointed at addresses that turned out to be perfectly good
**name starts** — just not at the stride the run claimed:

```text
0x371598  +  0  'FX_FS_DrawToTexBuff End'   23 chars → padded to 24
          + 24  'FX_ShadowPrepare Begin'    22        → 24
           ...                                 16..23 → 24  (eleven more)
          +288  'FX_GradFog End'            14        → 16   ← breaks here
          +304  'FX_HeatHaze Begin'         17        → 24
```

Every one of these is a string packed to an **eight-byte boundary**. Lengths of
16 to 23 all round up to 24, so thirteen of them in a row produce a flawless
stride-24 run whose every element is a name with nothing but NUL padding after
it. No payload test can see a thing — until a 14-character name rounds up to 16
instead, and the string after it begins inside the element and runs out through
its far edge.

And that is exactly the case the text-payload test was throwing away: it
required the payload's own name to **terminate inside the element**. A field of
a record always does. A pool string crossing the edge never does — which makes
it the strongest pool evidence there is, and it was the one case excluded.

Accepting it takes trimmed runs from 4 to **73**, and absorbed elements from 14
to **83**.

### A reference into padding is an empty string

The remaining four were one address, reached by four different functions: 213
bytes into record 15 of the layout at `0x36B308`. Not a field offset, not a name
start — the second NUL in a field's padding.

```asm
lea  rax,[rip+0x34313]        # 0x36c355  ->  ""
mov  r8,rax
lea  rdx,[rip+0x1d07ea]       # 0x508848  ->  "%s%s%s%s"
call 0x2ff80
```

It is an **empty string literal**. The linker folds `""` into any NUL byte in
the image, and a name table's padding is nothing but NUL bytes, so an empty
string routinely arrives wearing a table's coordinates.

Rejecting an address whose byte is NUL takes mid-element references from 4 to
**0**. Every reference into a recovered table now lands on an element or a
field.

## What a constant index actually proves

Trimming `0x506C38` dropped it below the minimum and released its eight
references — which the code demonstrably makes. Before accepting that, the
instructions:

```asm
2d8b50:  lea rdx,[rip+0x22e0e1]   # 0x506c38
2d8b7a:  lea rdx,[rip+0x22e0cf]   # 0x506c50
2d8b9c:  lea rdx,[rip+0x22e0c5]   # 0x506c68
 ... five more, at five more addresses, interleaved with other work
```

**Eight separate loads at eight different instructions.** The base is never held
and indexed. That is eight string literals that happen to sit 24 bytes apart,
not a table of eight — and the reclassification is right.

This is a caveat on the earlier reading. A reference whose offset is a whole
multiple of the stride was called a constant index folded into the displacement.
It is *equally* the shape of code loading one literal at that offset, and the
instruction cannot tell the two apart. Coverage by constant index is evidence
that code reaches those bytes — not that it indexes them. Separating the two
needs the base followed through a register, which is the same value tracking
virtual dispatch needs.

No reference was lost to any of this: `string_references` rose by exactly the 13
the tables gave up.

## The index

The reports are now loaded into SQLite
([`research/sql/`](../../research/sql/README.md)), which is where this check
lives as a standing query rather than a one-off:

```sql
SELECT base_rva, element_bytes, entries, interior_references, extent_status
FROM v_exe_table_coverage
WHERE interior_references > 0;

-- Tables that stand on their own, with each record's interior excluded.
SELECT COUNT(*) FROM v_exe_independent_table;
```

`0x506F68` now reads 16 of 16 elements covered, where it read 22 of 22 before.
The caveat from the earlier document — that coverage over a payload-bearing run
is an upper bound until the extent is confirmed independently — stands, and is
worth keeping. It is now discharged for the runs it was written about.

## Open work

- ~~**which runs are indexed and which are pools of literals.**~~ Answered: none
  of them are indexed. See
  [indexed access](dmc3-indexed-access-2026-09-14.md);
- **runs that overlap without being interiors.** Reconciliation settles a run
  that matches a field offset and width. A run inside a record's extent matching
  neither is still unexplained, and may mean the *record's* extent is the
  over-stated one;
- **the computed indices**, which still need the value tracking that virtual
  dispatch needs;
- **the tables no direct reference reaches**, whose bases arrive through
  registers and writable data.
