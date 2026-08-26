# DMC3 stage container corpus — slot roles and the name manifest — 2026-08-26

**Corpus:** `st001.pac`, `st001cfg.pac`, `st001_effect.pac` and the matching
`st114` set, supplied from a retail installation.

**Status:** OBSERVED / TWO INDEPENDENT STAGES AGREE / NOT PROMOTED

Two stages agreeing is enough to record a structure and not enough to promote
it to recovered authority. Everything below is stated as observation.

## 1. Slot payloads carry a four-byte type tag

Every payload in these containers begins with its own tag. The classifier now
reads them, which is separately committed.

Observed: `LIG2`, `SEF`, `CAM`, `EVE`, `POS`, `ITM`, `STE`, `EST`, alongside the
already-known `PAC`, `PNST`, `SCM `, `HITS`, `DCA`.

The executable does **not** carry a table of these tags. A whole-image search of
`.text`, `.rdata` and `.data` finds only `LIG2`, once, at `0x14023ECCC` — and
there it is a constructor storing `0x3247494C` into `[rcx+0x08]` of an object
whose vtable is `0x1404E3128`. So the tag is an object's own type field, and the
readers are not selected from a literal tag table. How they *are* selected is
open.

## 2. Slot roles are positional and stable across stages

`stNNNcfg.pac`

| slot | st001 | st114 |
|---|---|---|
| 0 | text | text |
| 1 | `LIG2` | `LIG2` |
| 2 | `PAC` (holds `SEF`) | `PAC` |
| 3 | `CAM` | `CAM` |
| 4 | `EVE` | `EVE` |
| 5 | `POS` | `POS` |
| 6 | `ITM` | `ITM` |
| 7 | `STE` | `STE` |
| 8 | `DCA` | `DCA` |
| 9 | `EST` | absent |

`stNNN.pac`

| slot | st001 | st114 |
|---|---|---|
| 0 | name manifest | name manifest |
| 1 | untagged, leading `0x11` | untagged, leading `0x11` |
| 2 | `SCM ` | `SCM ` |
| 3 | `HITS` | `HITS` |
| 4 | `# GAME` text | `# GAME` text |
| 5 | `PNST` | `PNST` |
| 6 | `HITS` | `HITS` |
| 7 | `PAC` | absent |

`stNNN_effect.pac` is slot 0 a version/id list, slot 1 a `PNST`.

## 3. Slot 0 of `stNNN.pac` is a name manifest

```text
st001.ptx\r\nst001.scm\r\nst001.sch\r\n
st114.ptx\r\nst114.scm\r\nst114.sch\r\n
```

This is the first evidence in this project of original names living *inside* a
relative-slot container rather than being absent from it.

The apparent mapping is manifest line *i* to slot *i+1*, and one line of it is
self-checking: `st001.scm` lands on the slot whose payload tag is `SCM `. The
`.ptx` line lands on the untagged `0x11` record and `.sch` on a `HITS` payload,
neither of which contradicts the mapping but neither of which confirms it the
way `SCM ` does.

That is why this is not wired into naming yet. A manifest name asserted against
a payload it does not describe would be worse than the parser's honest
`slot_0003`: it would look like recovered truth.

## 4. Sparse slots are not damage

`st001.pac` slot 5 is a `PNST` declaring 11 slots with offsets 1–9 literally
zero; `st114.pac` slot 5 declares 21 with 18 zero. A slot index is an identity,
not a position in a packed list.

## 5. What would close this

- more stages, to turn a two-sample agreement into a role table;
- an independent unpacker's mapping to compare against, which would raise the
  manifest-to-slot association from plausible to corroborated;
- the reader selection path in the executable, since the tag table is not there.
