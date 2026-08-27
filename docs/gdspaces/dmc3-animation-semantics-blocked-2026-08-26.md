# Why animation semantics stop here — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The plan's third step was to reverse the animation payloads rather than only
the registry that types them. It does not work with what is available, and the
reasons are worth more than a guess would have been.

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

Not more reversing. **Data.**

- More than one `.mot` file. There is exactly one in everything supplied:
  `st001.pac` slot 7 slot 0, 63,440 bytes. Its header reads `0x50`, `MOT`,
  two floats of `650.0`, then `u16` pairs that repeat `0x0038` — a stride or a
  count that one sample cannot distinguish from a coincidence.
- A `.mcv`, `.hid` or `.tsc` file, of which the corpus contains none at all.
- The consumer's call site, which would come from a runtime trace rather than
  from static reading: the registry hands out an entry handle, and what reads
  that handle is not reachable by searching for the type array.

## 5. What this changes about the plan

Step 3 is blocked on data, not on effort, and the same is already true of `EFM`
and `SHW`. Three of the four remaining semantic targets need files rather than
hours.
