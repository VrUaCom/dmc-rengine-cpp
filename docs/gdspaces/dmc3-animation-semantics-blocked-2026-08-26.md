# Why animation semantics stop here — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

> **Superseded in part, 2026-08-26 (same day).** §4 and §5 below said the
> animation payloads were blocked on data. That was wrong for `MOT`: the one
> `.mot` file contains 69 tracks, and its structure is now recovered and
> asserted — see `dmc3-mot-recovery-2026-08-26.md`. §§1–3 stand unchanged, and
> the block still holds for `MCV`, `HID`, `TSC`, `EFM` and `SHW`. The
> corrections are marked inline.

The plan's third step was to reverse the animation payloads rather than only
the registry that types them. It works only in part with what is available, and
the reasons are worth more than a guess would have been.

## 1. The payload's tag is inert

A `.mot` record carries `MOT` at `+4`, behind a `u32`. A whole-image search of
`.text` for that four-byte value finds **nothing** — not a comparison, not a
constructor storing it as an object type field the way `LIG2` appears once at
`0x14023ECCC`.

The runtime knows a motion by its **name** and by nothing else. The tag in the
file is for whoever made the file.

## 2. Both registries write a type they never read

This is a correction to how this project has been describing them.

A whole-image search finds the first registry's type array at `+0x6108` and the
animation registry's at `+0x18408` **written and never read back through those
offsets**. Dispatch happens at registration — the registrar calls the handler
directly — and the second dispatcher at `0x1401B9FA0` re-probes the payload's
tag rather than consulting anything stored.

So a stored type code is recorded state for a later query, not the key that
selects a reader. Both contracts now say so, and a `static_assert` holds it.

## 3. The tokenizer addresses lead somewhere real but unusable

The corrected function entries in `0x140322Cxx` are not a tokenizer. The
largest, `0x140322CD0`, is a lookup by `u16` key over a table with a `u16`
count at `+0` and `0x18`-byte entries from `+8`, returning 3 on a miss;
`0x140323030` iterates the same table. Thin wrappers at `0x1402C64B0`,
`0x1402C64C0` and `0x1402C64D0` bind all three to one singleton at
`0x140CF0AA0`.

That is structure with no semantics and no corpus to check it against, which is
where descending further stops being reverse engineering and starts being
invention.

## 4. What would unblock it

**Corrected.** This section claimed `MOT` needed a second file. It did not.

The original argument was that one `.mot` cannot distinguish a stride from a
coincidence. That holds for the `u16` table at `+0x18` — and it still does, so
that table remains unread. It does **not** hold for the payload's body: the
single file at `st001.pac` slot 7 slot 0, 63,440 bytes, declares **69 tracks**,
which makes every per-track claim testable 69 times inside one sample. The size
identity, the increasing stamps and the 650-frame span all hold 69 of 69, and
the size chain closes on a terminator exactly at the end of the file. See
`dmc3-mot-recovery-2026-08-26.md`.

What is still genuinely blocked on data:

- A `.mcv`, `.hid` or `.tsc` file, of which the corpus contains none at all.
- One `EFM` and one `SHW` payload, likewise absent.
- The consumer's call site, which would come from a runtime trace rather than
  from static reading: the registry hands out an entry handle, and what reads
  that handle is not reachable by searching for the type array. This is what
  would give the track kind, the 24 header floats and the three `s16` per key a
  meaning; without it those stay extents.

## 5. What this changes about the plan

**Corrected.** Step 3 is done for `MOT` and blocked for the rest. `MCV`, `HID`,
`TSC`, `EFM` and `SHW` need files rather than hours; the semantics of what a
`MOT` track *drives* need a trace rather than a file.

The general lesson, which the first version of this note missed: before
declaring a format blocked on data, count the repeating structures inside the
sample already in hand. A file with 69 of something is a corpus.
