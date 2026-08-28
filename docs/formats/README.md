# Every file format in DMC3, what it is for, and what we actually know

**Canonical image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
(6,356,432 bytes, base `0x140000000`)

This is the index. Each row says what the format is for, how it is recognized,
and — the part that matters most — **on what evidence**. A tool that cannot
tell a recovered fact from an authoring convention will eventually present one
as the other, so the distinction is carried here and in the code rather than
left to memory.

## How to read the evidence column

| class | means |
|---|---|
| **recovered** | read out of the executable's instructions, with a byte window bound to the image hash above |
| **observed** | read off real files, with a count of how many agreed |
| **named** | the runtime types it by name and this project has no reader for it |

And one distinction that cuts across all of them, established by the
[content tag census](../gdspaces/dmc3-content-tag-census-2026-08-27.md):

> The runtime compares exactly **five** payload tags — `MOD`, `EFM`, `SCM`,
> `SHW`, `MRP` — plus the container magics `PAC` and `PNST`. Every other tag
> this project reads is an **authoring convention**: true about the files,
> false about the game.

---

## Containers

### `PAC` — the stage archive
Holds everything one stage needs. A relative-slot container: magic at `+0`,
signed slot count at `+4`, a table of container-relative `u32` offsets from
`+8`. A zero offset is a slot that carries nothing — sparse by design, not
damaged.

The magic is **three bytes**; the stored fourth is NUL and the runtime never
reads it. Only the pool finalizer at `0x1401B92E0` walks a PAC, and only one
held in a pool slot — **the dispatcher does not descend into a nested PAC**.
*Recovered.* → [`pac-readonly-parser.md`](pac-readonly-parser.md)

### `PNST` — the nested container
Same layout, four-byte magic, and the dispatcher at `0x1401B9FA0` recurses into
it. That recursion is what makes a container inside a container work.
*Recovered.* → [`pnst-readonly-parser.md`](pnst-readonly-parser.md)

**Neither walk ever computes a child's size.** Both form `container + offset`
and pass a bare pointer; the handler finds its own end. So a slot's extent is
this project's decision — "up to the next greater offset in the file" — and not
a recovered rule.

### `NBZ` — the shipped volume
The distribution archive. The game builds its name as `%sDMC3-%d.nbz`
(`0x14036E930`), so a volume family is numbered and a member missing from one
may sit in a sibling. *Structural.*

### `AFS` — the logical namespace
`GData.afs/` is the second of six search prefixes the runtime tries, in order:
`GDataX360.afs/`, `GData.afs/`, `Video/`, `afs/sound/`, `SAVEDATA/`, and the
empty string. Two independent resolvers bound that loop at six.
*Recovered.* → [archive catalogue](../gdspaces/dmc3-archive-catalogue-and-effect-names-2026-08-27.md)

---

## Models and geometry

| format | for | evidence |
|---|---|---|
| `SCM` | scene model — stage geometry | **recovered**, handler `0x1403051B0` |
| `MOD` | object model | **recovered**, handler `0x1402FE3B0` |
| `EFM` | effect model | **recovered** from its handler `0x1402F7A90`; **no corpus file exists**, so the layout has never met real data |
| `SHW` | shadow volume | **recovered** from handler `0x1403204C0`; likewise no corpus |
| `MRP` | unknown | compared by the type probe at `0x1402DB232`, dispatched by nothing, carried by no file |

All four share a relocated document shell: group count at `0x10`, document mode
at `0x11`, document pointer at `0x20`, group table at `0x40`, stride `0x40`.

**A model group numbers its slots by tens.** `st001.pac` slot 5 declares 11
slots and fills 0 and 10; `st114.pac` slot 5 declares 21 and fills 0, 10, 20.
The gaps are reserved identity space between groups, not damage.

---

## Textures

### `PTX` — the texture pack
A count, a sector-span table, then `0x70`-byte descriptors each introducing a
DDS image. Packed dimensions sit at `+0x10` as `(height << 16) | width`.
`st001.pac` slot 1 declares 17 textures. *Structural, corpus-verified.*

### `DDS` — the image itself
The runtime's own FourCC chain at `0x14004A946` accepts **ten** compressed
formats and four aliases, each mapped to a DXGI code:

| FourCC | code | | FourCC | code |
|---|---|---|---|---|
| `DXT1` | 71 | | `ATI2` | 83 |
| `DXT3` | 74 | | `BC5S` | 84 |
| `DXT5` | 77 | | `RGBG` | 68 |
| `ATI1` | 80 | | `GRGB` | 69 |
| `BC4S` | 81 | | `YUY2` | 107 |

Aliases: `DXT2`→`DXT3`, `DXT4`→`DXT5`, `BC4U`→`ATI1`, `BC5U`→`ATI2`.
*Recovered* — the corpus shows two of these; the game reads all of them.

### `TM2` — PlayStation 2 texture
Read at `0x1403365B0`. Magic compared as **one dword including the NUL**, so
`TM2x` is not a TM2 — the opposite of `PAC`. `+0x08` is a base-relative data
offset. On a mismatch the reader reports fallback dimensions rather than an
error, so the runtime never announces "this is not a TM2".
*Recognized by magic; no corpus file.*

### `CLT` — palette
Typed by **both** registries with **different codes** — 5 in the first, 4 in
the animation one. A type code means nothing without the registry that issued
it. *Named; no reader.*

---

## Animation — six kinds, one registry

A second registry at `0x1402E01A0` types these **by name and never by bytes**.
An unmatched name is not registered at all.

| ext | code | for | this project |
|---|---|---|---|
| `.mot` | 0 | motion | **read** — see below |
| `.mcv` | 1 | curve | named only |
| `.cam` | 2 | camera | named only |
| `.hid` | 3 | hide/visibility | named only |
| `.clt` | 4 | palette | named only |
| `.tsc` | 5 | unknown | named only |

**The comparison is `strstr`**, resolved through the import slot `0x14034F3D0`.
The extension is looked for *anywhere in the name*, which is why the game reads
`pl000.mot1` … `pl000.mot6` as motions while carrying no numbered literal
anywhere. A tail match refuses exactly the names the game accepts. Case is
enumerated in pairs, not folded, so `.Mot` is not a motion.

### `MOT` — the motion payload, read
```
+0x00 u32   data offset (0x50)      +0x0C f32 duration
+0x04 "MOT\0"  <- compared nowhere  +0x14 f32 duration mirror
+0x50 u32   track count
      track: u16 size = 32 + 8*keys | u16 keys | u32 kind | 24 B floats | keys
      key:   s16 time, then 3 x s16
      terminator: zero dword, exactly at end of file
```
The single 63,440-byte payload in `st001.pac` slot 7 declares **69 tracks**, so
every per-track claim is tested 69 times by one file: the size identity holds
69/69, stamps increase 69/69, and every track's measured span equals the
`650.0f` the header states twice. *Structural, 69-fold corroborated.*
→ [`dmc3-mot-recovery-2026-08-26.md`](../gdspaces/dmc3-mot-recovery-2026-08-26.md)

---

## Where animation names come from

The registry types by name; a container stores no names. The gap is filled by a
**text script**. The registrar keys on `"%s/%s"`; the group half is formatted
`"demo/%s"`; the name half is a token on a script line.

Three commands reach the registrar: `Motion` (`0x1402D5854`), `Camera`
(`0x1402D50C4`), `Hide` (`0x1402D5414`) — three of the six type codes, matching
exactly. So a motion is `demo/<demo name>/<file>`, and **an animation's name is
neither invented by this tool nor stored in the container**. It is written in a
script the container never sees, which is why an unpacked folder can never
carry it.

The full command vocabulary is recovered; the grammar is not, because no script
file exists in any corpus.
→ [`dmc3-where-animation-names-come-from-2026-08-27.md`](../gdspaces/dmc3-where-animation-names-come-from-2026-08-27.md)

---

## Text and authoring records

### `TXT` — stage authoring text
Validated whole-record as Shift-JIS. Several distinct uses share the extension:

- **the slot-0 name manifest** — `st001.ptx\r\nst001.scm\r\nst001.sch\r\n`,
  line *i* naming slot *i+1*. All three lines corroborate: `.ptx` is the
  texture pack, `.scm` the model, and **`.sch` is the `HITS` record** — one
  thing under two names, in both stage files.
- **`# GAME`** scene blocks and **`# DOOR`** tables in the `cfg` container.
- **the effect manifest** (below).

### `C1D` — ClothSim1D
A text format that **names itself**: the parser at `0x1402C8D2D` compares the
file's first token for equality against `ClothSim1D` and bails when it differs.
The only self-identifying text format found in this game.

Vocabulary: `Gravity SpringForce Damping MaxSpeed FloorLevel Cut End ClothNo
Wind WindLocal WindParent Stiffness WindType LimitLength Bone NX NY NZ
ClothNum` — cloth simulation. *Identified; grammar unrecovered, no corpus.*

### The effect container — the one place names are stored
`*_effect.pac` is a `PNST` of exactly two slots: an ASCII manifest, and a
`PNST` whose child count **equals the manifest's line count** — 9 and 9 in
st001, 11 and 11 in st114. Each line is a kind letter and an identifier, and
the letter predicts the record's extent across both files:

| kind | records | extent |
|---|---|---|
| `V` | 4 | 368 |
| `E` | 9 | 544 |
| `P` | 2 | 704 |
| `A` | 2 | 336 |
| `T` | 3 | variable — a texture, dimensions at `+0x10` |

**These are the only slot names in this project that a container actually
stored.** Everywhere else a name is a decision the tool made. *Observed, two
independent files.*

---

## Records read but never recognized by the game

Each of these is read here and compared **nowhere** in the executable. They are
authoring conventions, and the tool says so rather than presenting them as the
game's own identification.

| tag | for | this project |
|---|---|---|
| `HITS` | collision / spatial | structural parser + writer, corpus-derived |
| `LIG2` | light rig | structural; the one tag a constructor *stores* (`0x14023ECC9`) and still never compares |
| `DCA` | area records | structural |
| `SEF` | sound effect binding | recognized |
| `CAM` | camera record | recognized |
| `EVE` | event | recognized |
| `POS` | positions | recognized |
| `ITM` | items | recognized |
| `STE` | unknown | recognized |
| `EST` | unknown | recognized |

`HITS` **cannot** be recovered from the executable — not for want of reversing,
but because the executable does not contain the thing being looked for.

---

## Naming: five different things, and only one the game reads

These are routinely conflated, and each is a different claim:

```
runtime semantic name   ≠   .lst loose member name
                        ≠   slot 0 build manifest
                        ≠   physical PAC slot
                        ≠   detected binary format
```

### `LST` — the only container-side name the game itself reads
The runtime's loose alternative to a packed container. The representation
selector at `0x1401B79E0` prefers the exact packed `.pac` and rewrites the
extension to lowercase `lst` **only when the packed one is absent**, so it is a
fallback and never an override. A `dummy` token occupies a declared slot and
carries no payload. *Recovered.*
→ [`dmc3-loose-container-list.md`](../gdspaces/dmc3-loose-container-list.md)

### Slot 0's manifest — build metadata, proven
`st001.pac` slot 0 carries `st001.ptx / st001.scm / st001.sch`. The runtime
**reaches** slot 0 — the walk starts there — hands it to the dispatcher, and
the dispatcher recognizes nothing in it and returns.

`.sch` occurs **zero** times in the executable in any case; `.scm` occurs zero
times as a string, `SCM` being only ever the three-byte magic; and none of the
three names is constructible. Only `.ptx` resolves anywhere, which is why the
manifest looked authoritative — one line of three is not a naming system.

Still read and shown here, attributed as the container's own text. The proof
changes its authority, not its accuracy.
→ [`dmc3-slot-zero-manifest-authority-2026-08-28.md`](../gdspaces/dmc3-slot-zero-manifest-authority-2026-08-28.md)

### Where the index is, in every kind of archive

A census of **every** text slot in **every** container of the corpus, at every
nesting depth, finds 26 candidates and **four** real indexes. Both dialects sit
in slot 0:

| dialect | containers | lines | names |
|---|---|---|---|
| filename list | `st001.pac`, `st114.pac` | 3 filenames | **its own** slots 1..N |
| kind + identifier | `st001_effect.pac`, `st114_effect.pac` | `V 922`, `E 1454`, … `# End` | the slots of the **sibling** container in slot 1 |

That last column is the distinction worth keeping: a stage index names its own
siblings; an effect index names the children of the slot beside it. A probe
that answered "index found" without saying which would apply every name one
level off.

Two things are **not** indexes and used to look like them:

- **tagged binary records** whose first bytes are printable — `CAM`, `EVE`,
  `POS`, `ITM`, `STE`, `DCA`, `HITS`. Read as text they give one line, and the
  line count is what gives them away.
- **text that names nothing** — `# END`, `# GAME`, `# DOOR 0`. These are scene
  and config blocks. They are text; they are not indexes.

`ContainerIndexProbe` is the single entry point that answers for all of them.
It exists because the two dialects were being detected in two unrelated places
under two sets of rules, so nothing could answer "does this archive have an
index" without knowing in advance which kind to look for.

### `INDEX` — ours, not the game's
Extraction and tooling metadata. The literal `.index` occurs **zero** times in
the executable in any case and no format string can construct it, so the
original game cannot name such a file at all. This project *writes* one as an
unpacking sidecar and parses none — reading a sidecar back as naming authority
would be reading back our own decision as evidence.

### `SCH` — an authoring extension, not a runtime one
The name a stage manifest gives the slot holding a `HITS` record, in both
corpus files. It has no entry in either type registry, so a name ending `.sch`
types nothing.

---

## The archive name catalogue

`0x140553050` holds **4,039 consecutive pointers** naming every archive the
game ships — 3,398 `.pac` among them. A stage contributes four consecutive
entries: the stage, its config, its effects and its sound bank. 175 of 182
groups are complete quadruples; the seven exceptions are all `st6xx`, which
carry a stage and a config and neither of the other two.

**And no instruction reads it.** A sweep covering 99.87% of `.text` finds no
reference to the array's base, no immediate equal to that address anywhere, no
pointer to it, and no reference to the individual strings. So it is a strong
source of **candidate** names and not an authority on order, and the contract
says so with `catalog_read_site_found = false`.

---

## Current integration state

35 formats registered: **18 structural, 17 recognized, 0 semantic**.

The ladder is recognized → structural → semantic → editable → exportable.
Nothing has reached semantic, and the honest reason is usually the same one:
a layout can be recovered from a handler, but what a field *means* needs either
a corpus to check a reading against or a runtime trace.

### Blocked on data, not on effort

`MCV`, `CAM`, `HID`, `CLT`, `TSC`, `EFM`, `SHW`, `MRP`, `C1D`, `TM2` — no file
of any of these exists in any supplied corpus. One real payload of each is what
moves it, and no amount of further reading substitutes.

---

## Source of record

Every address above is bound to the image hash at the top of this file by a
byte window in
[`data/reverse/dmc3-type-identification-windows.v1.json`](../../data/reverse/dmc3-type-identification-windows.v1.json).

An address without an image identity is not evidence. It is a number.
