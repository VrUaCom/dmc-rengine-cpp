# MOD, and why it is not SCM — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
**Routine:** `0x1402FE3B0`, 746 bytes, receipt in
`data/reverse/dmc3-type-identification-windows.v1.json`

**Status:** RECOVERED FROM INSTRUCTIONS / VERIFIED AGAINST ONE REAL MODEL

The second of the four type handlers. Like `SCM`'s, it is the relocation pass:
it turns stored offsets into pointers and rebuilds a strip index list, naming
every field it touches on the way.

## 1. What the two formats share

Exactly one thing, and both routines read it identically:

```text
+0x10  uint8   group count
+0x11  uint8   document mode        <- MOD branches on it, SCM never reads it
+0x20  qword   document pointer
+0x40  group[] stride 0x40, each: uint8 batch count, qword first batch
```

That shell is now one implementation, `RelocatedModelShell`, used by both
readers — the same arrangement PAC and PNST already have.

## 2. What they do not share

Below the group they agree about nothing.

| | SCM | MOD |
|---|---|---|
| batch addressing | chained through `+0x28` | indexed, `first + n × 0x50` |
| `+0x28` | step to the next batch | a relocated array pointer |
| `+0x30` | — | a relocated array pointer |
| `+0x38` | index array | — |
| index element | 4 bytes | 2 bytes |
| break flag | byte 3, mask `2` | high bit `0x8000` |
| flag after rebuild | left alone | cleared, `&= 0x7FFF` |
| strip length | written only when the buffer is marked | always written |

The first row is the dangerous one. Reading a model the scene model's way
takes an **array pointer** for a **stride**, which produces a batch walk that
lands somewhere plausible and wrong. A shared reader for "the two model
formats" would have been the natural thing to write and would have been wrong
about every object in the game.

## 3. A batch is a vertex stream

The rebuild writes loop counters — `k`, `k-1`, `k-2` — into the strip buffer.
So a triangle refers to positions *within the batch*, and the array at `+0x30`
is not a vertex index at all. It is a per-vertex control word whose high bit
breaks the strip run, and the routine clears that bit once the list is built.

A loaded document therefore no longer carries the flags a stored one does.
That matters for anyone comparing a memory dump against a file.

## 4. The corpus

`id100_001_red_orb_counter.mod`, 2,304 bytes — the HUD red-orb counter.

| | |
|---|---|
| groups · batches | 7 · 7 |
| vertices | 28 (four per batch) |
| break-flagged vertices | 14 (two per batch) |
| strip buffers carrying `0x1212` | 7 / 7 |
| document mode | 2 |

Every batch is one quad. The positions read as screen-space rectangles at a
constant depth — six digit cells 14.39 units apart at z = 3.7, and one larger
cell at z = 12 — which is what a red-orb counter should look like and is why
the layout is believable as well as arithmetically sound.

That is one file. The stage corpus contains scene models only, so this is
verified rather than corroborated, and it is recorded as such.

## 5. What is not claimed

- what the 4-byte attribute and secondary words hold;
- what the low bits of the control word hold besides the break flag;
- what `document_mode == 1` guards — the routine tests bytes 1 and 2 of the
  secondary array under it, and what that decides is not recovered;
- anything about writing. MOD stays read-only.
