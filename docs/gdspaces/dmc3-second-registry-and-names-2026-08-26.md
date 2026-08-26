# The second registry, and where a slot name comes from — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
Receipts for every address in `data/reverse/dmc3-type-identification-windows.v1.json`.

Two findings, both from the complaint that an unpacked folder is missing
formats and that its names are invented. Both complaints were right.

## 1. There is a second resource registry

`ResourceTypeContract` recovered a registry that asks the name first and the
payload's own tag second. It looked complete. It is not: `0x1402E01A0` is a
**different** registry with a different table and a different type numbering.

| | first registry | animation registry |
|---|---|---|
| register | `0x1402DB3C0` | `0x1402E01A0` |
| reset | `0x1402DB370` | `0x1402E0150` |
| capacity | 256 | 1024 |
| entry names | `+0x0008` | `+0x8008` |
| type array | `+0x6108` | `+0x18408` |
| extensions | `.ptx` `.clt` `.c1d` | `.mot` `.mcv` `.cam` `.hid` `.clt` `.tsc` |
| content-tag fallback | yes — `MOD` `EFM` `SCM` `MRP` `SHW` | **none** |

The animation registry has no content probe at all. A name it does not
recognize is not registered — there is no byte-level second chance.

Its type numbering is its own: `.mot` 0, `.mcv` 1, `.cam` 2, `.hid` 3,
`.clt` 4, `.tsc` 5. `.clt` appears in both tables and is **5 there and 4
here**, so a type code means nothing without the registry that issued it.

Animation is the largest thing missing from an unpacked stage folder, and it
is missing because it is typed here rather than there.

## 2. Where a slot name comes from, and what that is worth

A relative-slot container stores no names — proven, not assumed: the runtime
reaches those payloads by position and never probes for a signature. So every
name a tool shows for a slot is something that tool decided.

Three kinds of decision now travel with every slot, and they are labelled:

| origin | what it is |
|---|---|
| `parser-placeholder` | `slot_0003`. An index, formatted. Says nothing. |
| `byte-derived-suffix` | the suffix the payload's own bytes declare |
| `container-manifest` | a line from a name list the container itself carries |

`stNNN.pac` slot 0 is a CRLF name list — `st001.ptx\r\nst001.scm\r\nst001.sch`
— and the mapping is line *i* to slot *i + 1*, the manifest occupying slot 0.
That is the same convention the extracted corpus's external `.index` files use,
where a container directive occupies position 0 and names follow. Two
independent conventions agreeing on the mechanism is why the mapping is
recorded.

It is still **attributed, never asserted**. The identity and the display name
are untouched; the manifest name travels beside them with its origin and one
check: does the type the payload independently declares agree with the
extension the line carries?

On the real corpus:

| slot | manifest line | payload says | agrees |
|---|---|---|---|
| 1 | `stNNN.ptx` | texture pack, 17 DDS | yes |
| 2 | `stNNN.scm` | `SCM` tag | yes |
| 3 | `stNNN.sch` | `HITS` | no — recorded as disagreement |

Two of three corroborate on both stages. The third is not forced into
agreement, and an operator can see that it did not.

## 3. A real texture pack was being refused

Attributing st114 turned up a regression this work introduced. `st114.pac`
slot 1 read as `unknown`: `TextureSlotFramingParser` rejected it with
`descriptor-mismatch`, because it required a non-zero descriptor auxiliary
mode to imply DXT5 — a relation confirmed on a corpus that did not contain
this stage. **Seventeen of seventeen** textures in `st114` carry a non-zero
mode on DXT1.

The rule was conflating two different questions: *is this a texture pack* and
*may the writer author it*. Recognition now records the auxiliary pair; the
writer refuses non-zero auxiliary metadata on its own, as it already did. So
`st114`'s pack reads — 17 textures — and still cannot be written, with
`unresolved-auxiliary-metadata` rather than a claim that it is not a pack.

## 4. What is still open

- `MOT` payloads carry their tag at `+4` behind a `u32`, observed once in
  `st001.pac` slot 7. The runtime types motion by extension, never by that
  tag, so it stays an observation.
- The `.index` sidecar itself: the corpus contains 13, this project writes
  none. The attribution above is what a written one would carry.
- Whether the manifest's third line is `.sch` for collision, which the payload
  neither confirms nor contradicts.
