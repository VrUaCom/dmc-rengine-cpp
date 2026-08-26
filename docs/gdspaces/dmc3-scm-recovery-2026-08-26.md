# SCM, from the handler the type probe names — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
**Routine:** `0x1403051B0`, 647 bytes, receipt in
`data/reverse/dmc3-type-identification-windows.v1.json`

**Status:** RECOVERED FROM INSTRUCTIONS / VERIFIED AGAINST TWO STAGES

The type probe recovered which handler the runtime calls for an `SCM` payload.
This is that handler read out. It relocates every stored offset into a pointer
and rebuilds a triangle-strip index list in place — and in doing so it names
every field it touches.

## 1. The layout it walks

```text
document
  +0x10  uint8   group count          <- a byte, not a dword
  +0x20  qword   document pointer     (relocated against the document)
  +0x40  group[] stride 0x40

group
  +0x00  uint8   batch count          <- also a byte
  +0x08  qword   first batch          (relocated against the document)

batch, stride 0x50
  +0x00  uint16  index count
  +0x10  qword   positions   12 bytes each   (relocated against the document)
  +0x18  qword   normals     12 bytes each   (relocated against the document)
  +0x20  qword   attributes   4 bytes each   (relocated against the document)
  +0x28  qword   stride to the next batch
  +0x38  qword   indices      4 bytes each   (relocated against the document)
  +0x40  qword   strip buffer                (relocated against the BATCH)
  +0x48  int32   strip length, written by the runtime
```

Two things here are easy to get wrong and the routine settles both.

**The counts are bytes.** `movzx eax, byte ptr [rax+0x10]`. The other three
bytes of that dword are different fields; a reader that took the whole word
would invent millions of groups out of them.

**`+0x40` uses a different base.** Every other offset is relocated against the
document; the strip buffer is relocated against its own batch. Mixing the two
is the one mistake this structure invites.

## 2. The arrays are grouped by kind, not by batch

Within a group, the file stores every batch's positions, then every batch's
normals, then attributes, then indices — each array padded to 16 bytes.

A reader that assumed each batch owned a contiguous run of its own four arrays
computes the right offsets for a group with one batch, and only for that. Most
groups have one batch. That is what makes this the kind of error that survives
testing and fails on someone's real file.

The packing is arithmetic, so the parser checks it: every recorded offset must
be exactly where the group's packing puts it. On the corpus that reproduces
44 of 44 groups in `st001` and 41 of 41 in `st114`.

## 3. The strip rebuild

For each index `k` from 2 upward, the routine reads byte 3 of the index word
and tests mask `2`. A flagged index breaks the run; an unflagged one extends
it, emitting `k` — and, when a run starts, the two indices before it as well.
The rebuilt list goes into the strip buffer, and the length at `+0x48` is
written only when that buffer opens with `0x1212`.

Every batch in both stages carries the marker: 77 of 77 and 72 of 72.

The list is scratch, regenerated at load. The product records the marker and
the stored length and does not regenerate it, because a rebuilt list is the
runtime's working state rather than the file's content.

## 4. What the corpus says

| | st001 | st114 |
|---|---|---|
| document | 887,760 B | 1,038,816 B |
| groups | 44 | 41 |
| batches | 77 | 72 |
| indices | 23,049 | 27,057 |
| skip-flagged | 8,788 | 10,809 |
| offsets in range | all | all |
| strip markers present | 77/77 | 72/72 |
| packing verified | 44/44 | 41/41 |

The first positions of `st001` group 0 read as `(-44.06, 130.28, 3.08)`,
`(-44.06, -122.89, 3.08)` — plausible stage geometry, though the parser makes
no claim about units or coordinate handedness.

## 5. What is not claimed

- what the 4-byte attribute word holds;
- what the index word holds besides the skip flag in byte 3;
- what the document pointer at `+0x20` points *at*, beyond being in range;
- the remaining fields of the `0x40`-byte group and the `0x50`-byte batch;
- anything about writing. SCM stays read-only.
