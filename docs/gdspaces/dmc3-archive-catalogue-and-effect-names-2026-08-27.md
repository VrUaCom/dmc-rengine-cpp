# The names were not all invented — 2026-08-27

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
(6,356,432 bytes, base `0x140000000`)

Two findings, both about naming, and they are of deliberately different
strength. Saying which is which is most of the value here.

## 1. The search path — recovered

`0x14002E7D0` takes a requested name, walks it backwards to the last `\` or
`/`, keeps the basename, and then loops: format `prefix + basename` into a
`0x400` buffer, try to open it, next prefix. The loop's bound is in the
instruction stream:

```
14002e840  lea r15, [rip + 0x52c6b1]      ; -> 0x14055AEF8
14002e850  mov r8d, edi
14002e863  mov r8, qword ptr [r15 + r8*8]
14002e867  call 0x1403272C0               ; format
14002e889  call 0x1403270B0               ; open
14002e892  inc edi
14002e894  cmp edi, 6                     ; <- the table's length
```

A second routine at `0x14002FD55` loads the same table with the same bound and
an outer pass counter of two. One routine reading six entries could be reading
past a five-entry table; two agreeing is what makes six a fact.

The table:

| # | prefix |
|---|--------|
| 0 | `GDataX360.afs/` |
| 1 | `GData.afs/` |
| 2 | `Video/` |
| 3 | `afs/sound/` |
| 4 | `SAVEDATA/` |
| 5 | *(empty)* |

This project already used `GData.afs/` as the logical root of a mounted volume,
chosen from the corpus before any of this was read. It is in the table, second.
That **corroborates** the choice; it does not establish it, and the difference
matters — a guess that turns out right is still a guess until something
independent agrees with it.

The empty last entry is how a name with no prefix resolves at all.

## 2. The archive catalogue — observed, not recovered

At `0x140553050` there are **4,039 consecutive 8-byte pointers** into the
`.rdata` string pool: 3,398 `.pac`, 424 `.txt`, 154 `.adx`, and the rest. They
are ordered, and a stage contributes four consecutive entries:

```
[276] st001.pac  [277] st001cfg.pac  [278] st001_effect.pac  [279] snd_r001.pac
[380] st114.pac  [381] st114cfg.pac  [382] st114_effect.pac  [383] snd_r114.pac
```

That number is four because an assertion refused three. The first reading of
this, made from the two stages the corpus contains, was that a stage is a
triple — stage, config, effects. Written down as
`(380 - 276) % stride == 0` it failed immediately: 104 is not a multiple of
three. Checking the whole catalogue rather than the two files on hand produced
the fourth member, the sound bank, and the real count: **175 of 182 stage
groups are complete quadruples**, the seven exceptions all in the `st6xx`
range, which carries a stage and a config and neither an effect pack nor a
sound bank.

This is the second time in this project that stating a reading as an assertion
has caught it being wrong within seconds. It is cheaper than being careful.

These are the game's own archive names — 4,039 of them, against a corpus of
six files.

**And no instruction reads it.** A linear sweep with resynchronization covering
**99.87%** of `.text` (877,019 instructions, 3,460,335 of 3,464,704 bytes)
finds:

- no reference to `0x140553050`,
- no 64-bit or 32-bit immediate equal to that address anywhere in the image,
- no qword anywhere in the image pointing at it,
- and no reference to the individual name strings either — `st001.pac` at
  `0x140367B90` and `pl000.pac` at `0x140362E60` both have zero xrefs.

So the catalogue's read site is not reachable by static reference. That is
recorded, not explained away: `ArchiveCatalogContract::catalog_read_site_found`
is `false`, and `catalog_index_is_a_known_identifier` is `false` with it. The
index into this array is **not** asserted to be an archive id, because nothing
observed says it is.

What the catalogue is good for is exactly what its evidence class allows: a
strong source of **candidate** names, to be attributed as such. What it is not
is an authority on order.

This is the third time in this project that a table turns out to be written and
not read back — the two type registries were the first two. The pattern is
worth naming: an image can carry a great deal of true information that its own
code does not consult.

## 3. The effect container carries real names — observed, two files

This one answers the complaint directly. `*_effect.pac` is a `PNST` of exactly
two slots:

- **slot 0** — an ASCII, CRLF-terminated manifest, NUL-padded, ending `# End`.
- **slot 1** — a `PNST` whose child count equals the manifest's line count.

```
st001_effect.pac    9 lines,  9 slots
st114_effect.pac   11 lines, 11 slots
```

Each line is a kind letter and a decimal identifier — `V 922`, `E 1454`,
`T 125`, `A 220`. The letter is not decoration: it predicts the record's
extent, and it does so across two files authored separately.

| kind | records | extent |
|---|---|---|
| `V` | 4 | 368, in both files |
| `E` | 9 | 544, in both files |
| `P` | 2 | 704 |
| `A` | 2 | 336, in both files |
| `T` | 3 | variable — 22,112, 22,112, 43,944 |

Four kinds of five have one extent shared by every record of that kind in two
independent files, and the line count equals the slot count in both. That is
what raises this above a coincidence.

`T` is a texture, and it is the same texture descriptor this project already
reads: its packed dimensions sit at `+0x10` as `(height << 16) | width`, which
is `kDescriptorDimensionsOffset` in the texture slot framing. The two records
read 128×128 and 256×256.

**These are the first slot names in this project that a container actually
stored.** Every other name shown for a slot has been a decision the tool made.
`EffectPackParser` reports them as the manifest's own text, and refuses any
file where the line count and the slot count disagree — because that equality
is the only thing that makes a line a name for a particular slot rather than a
line that happens to sit nearby.

No read site for this manifest has been found in the image either, and
`EffectPackContract::manifest_read_site_found` says so.

## 4. What did not turn up

A corpus-wide sweep of all six `.pac` files and the supplied `.zip`, expanding
every container recursively, finds **exactly one** structural motion payload —
`st001.pac` slot 7 slot 0, the one already read. The earlier claim that the
corpus contains exactly one `.mot` is confirmed rather than merely repeated.

The same sweep leaves 90 leaves, of which 27 are absent slots. After `DDS`,
`SCM`, `MOD`, `HITS`, `MOT`, `LIG2` and the text records, what remains
unidentified is now the effect record bodies themselves — `V`, `E`, `P`, `A` —
whose extents are known and whose contents are not.

## 5. Where it lives

- `include/dmc_rengine/profiles/dmc3/archive_catalog_contract.hpp`
- `include/dmc_rengine/profiles/dmc3/effect_pack_contract.hpp`
- `include/dmc_rengine/formats/effect_pack.hpp`, `src/formats/effect_pack.cpp`
- `tests/effect_pack_structural_tests.cpp`
- `data/reverse/dmc3-archive-catalogue.v1.json` — all 4,039 names and the six
  prefixes, bound to the image hash
- `data/reverse/dmc3-type-identification-windows.v1.json` — six new byte
  windows for the resolvers, the prefix table and the catalogue's ends
