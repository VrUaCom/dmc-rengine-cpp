# Reading a claim the project already had, and finding my own note short

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-bootstrap-confirmation`.

> **Corrected the same day** by
> `docs/reverse/dmc3-trivial-function-caveat-2026-09-18.md`. This note says the
> size dispatcher's target lives outside the file layer as though that were an
> architectural fact. `0x0629C0` is two bytes long and is also a vtable slot in
> 17 unrelated classes, so its reference set says nothing about where it belongs.

I set out to read who registers the mounts and in what order. I read it, and
then found the answer already in the repository — recorded on 2026-09-05, in
more detail than I had reached. The useful part of this pass is therefore not a
finding. It is a caveat discharged and three errors in my own note from earlier
today.

## What I read

`0x02E930`, 261 bytes, is the whole of it:

- `GetModuleFileNameA(NULL, buf, 0x104)`, then `strrchr(buf, '\')` and a zero
  written at the result — the directory the executable lives in;
- the twelve bytes at `0x36E920`, `\data\dmc3\` with its terminator, appended as
  one eight-byte store and one four-byte store;
- `0x326D20(buf, 0x0C)` — the one and only directory mount in the image;
- then, with `i` from zero: format `"%sDMC3-%d.nbz"` from `0x36E930` with that
  directory and `i`, probe it with the `FindFirstFileA` wrapper `0x327720`, and
  on a hit call `0x326DA0` to mount it as an archive. The loop stops at the
  first index that does not exist.

## All of which the project already had

`docs/reverse/dmc3-resource-vertical-proof-reconciliation-2026-09-05.md` §1A
records the derivation of `<exe-dir>\data\dmc3\`, the registration with flags
`0x0C`, the numbered probe from zero and the first-gap termination. §1B records
that both registrations prepend to `0xCF3180` and gives the resulting order
`DMC3-(N-1) → … → DMC3-0 → physical root`. `docs/roadmap.md` marks the
`DMC3-%d.nbz` bootstrap and first-gap discovery as having canonical static
evidence, and `docs/status/canonical-status.json` names the function by address.

That document is also more careful than I would have been on one point: a
discovered archive whose registration fails is simply absent from the topology,
so discovery does not prove mounting, and an absent volume is not a clean lookup
miss.

## What this pass is actually worth: a caveat discharged

The 2026-09-05 document states plainly that it "does **not** claim a new raw-byte
disassembly of the unresolved scheduler cluster because a raw canonical
executable blob was not available in the connected file surface during this
pass." Its sections A, B and D were reconciliation of earlier authority rather
than bytes read that day.

They are now read. The derivation, the two literals and their storage, the flag
word `0x0C`, the probe loop and its termination, the prepend in both
registrations and the order that follows from it — all confirmed instruction by
instruction against the canonical image. Nothing in §1A, §1B or §1D contradicts
the bytes.

## Three errors in my own note from this morning

`docs/reverse/dmc3-mount-registration-2026-09-18.md`, committed earlier today,
is wrong in three ways, and the first is the one that matters.

**It presented as newly read what the project already had.** That note says of
the normaliser "all of its bits are now read" and lays out the bit map and the
`0x0E` against `0x0C` asymmetry as though establishing them. §1D of the
2026-09-05 document contains the same bit map and states the same two flag
words with the same meanings. I did read them from the bytes, and reading them
independently has value — but presenting them without the citation claims
origination that is not mine. The rule I have applied to the ZIP note and the
file-access note applies here and I did not apply it.

**Its bit map is incomplete.** The table ends at the unconditional `/` to `\`
conversion. There is a second unconditional pass immediately after it, at
`0x327260` through `0x327295`, which copies the string forward and on each `\`
skips every `\` that follows — a repeated-separator collapse. The 2026-09-05
document has it ("always collapse repeated `\`") and I did not. So the
normalisation is: optional case fold, optional leading and trailing strip,
then `/` to `\`, then collapse runs of `\`.

That omission is not cosmetic. A query spelled `data\\thing` and an indexed name
spelled `data\thing` are the same key only because of the pass I left out, and a
port built from my table alone would have produced an index that misses on
exactly the inputs the collapse exists to handle.

**"Two functions do the dispatching" is three.** The mount-list note counted the
read dispatcher at `0x3275C0` and the seek dispatcher at `0x3275E0`. A byte
scan for the twelve-byte prologue those two share finds a third at `0x3272A0`,
for size, and it is the odd one: read and seek both stay inside the file layer,
while its archive branch tail-jumps to `0x0629C0`, outside the layer entirely.
The claim about the shape was right; the census attached to it was not, and it
was a census I stated without running one.

## What is still unread

- `0x0629C0`, the size implementation for an archive entry, and why it lives
  outside the file layer when read and seek do not.
- The inflater at `0x328820`, unchanged.
- `0x328210`, the archive teardown.
- Whether any of §1C, §1E or §1F of the 2026-09-05 document — the request
  construction, the two selection gates and the collision receipt — hold against
  the bytes. This pass covered A, B and D only.
