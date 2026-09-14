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

Four runs trimmed of 14 absorbed elements, giving 223 runs holding 5,813
entries.

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

## The index

The reports are now loaded into SQLite
([`research/sql/`](../../research/sql/README.md)), which is where this check
lives as a standing query rather than a one-off:

```sql
SELECT base_rva, element_bytes, entries, interior_references, extent_status
FROM v_exe_table_coverage
WHERE interior_references > 0;
```

`0x506F68` now reads 16 of 16 elements covered, where it read 22 of 22 before.
The caveat from the earlier document — that coverage over a payload-bearing run
is an upper bound until the extent is confirmed independently — stands, and is
worth keeping. It is now discharged for the runs it was written about.

## Open work

- **overlapping runs.** 29 pairs of recovered runs overlap. Some are containment
  rather than conflict: a 264-byte record array at `0x35D640` contains 21 of the
  reported stride-16 grids at exactly its record spacing, which makes them its
  interior field layout rather than 21 independent tables. Others are genuine
  boundary conflicts. Reconciling the two detectors' views is the next extent
  question and needs no new measurement;
- **the 7 remaining interior references** (offsets 13, 16, 20) — field access
  within a record, or a further layout error;
- **the computed indices**, which still need the value tracking that virtual
  dispatch needs;
- **the tables no direct reference reaches**, whose bases arrive through
  registers and writable data.
