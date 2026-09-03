# DMC3 `dmc3-0.nbz` archive normalized-key census — 2026-09-03

**Scope:** one retail NBZ volume.
**Archive:** `dmc3-0.nbz`, SHA-256 `2c2302cef5251d9a2499be728d81427e9689d0b9c3ceaeef10d9786260fd13df`.
**Bound surface:** central-directory CSV, SHA-256 `0616683ed1280e80421b5680725d258fe78e41f939ba994a433eadc0f99650af`.
**Status:** CENSUS CLEAN FOR THIS ARTIFACT / SCOPE-WIDE STATEMENT STILL OPEN.

Receipt: [`data/reverse/dmc3-nbz-archive-key-census-20260903.json`](../../data/reverse/dmc3-nbz-archive-key-census-20260903.json).
Companion reverse: [`l2-archive-index-duplicate-key-reverse-2026-08-26`](../gdspaces/l2-archive-index-duplicate-key-reverse-2026-08-26.md).

## 1. What was measured

The operator supplied a central-directory surface for one retail volume: 4,334
records, of which 4,333 are files and one is a directory entry. The census was
run with:

```bash
dmc-rengine census-archive-keys <central-directory.csv> receipt.json
```

The command normalizes every central name through
`profiles::dmc3::ResourcePathPolicy::archive` — the recovered `0x0E` profile
(strip leading separators, strip trailing separators, lowercase ASCII,
`/` to `\`, collapse repeated `\`). It deliberately does **not** reimplement the
transformation, so the census measures the product normalizer rather than a
second copy of it.

## 2. Result

| Pass | Considered | Rejected | Normalized keys | Unique keys | Collisions |
|---|---:|---:|---:|---:|---:|
| files-only | 4,333 | 0 | 4,333 | 4,333 | 0 |
| all central entries | 4,334 | 0 | 4,334 | 4,334 | 0 |

`normalized_key_count == unique_normalized_key_count` in both passes. This is
the clean result named in section 5 of the companion reverse.

## 3. Consequence

The comparator at `0x1403291D0` adds no tie-break, so comparator-equal entries
would have no recovered deterministic winner, and CRT `qsort`/`bsearch` supply
none either. Because this artifact produces no comparator-equal entries at all,
**that indeterminacy cannot arise for `dmc3-0.nbz`**. Duplicate-winner semantics
are not on the critical path for this volume.

## 4. What this does not establish

- It covers `dmc3-0.nbz` only. Each additional volume in a claimed resolver
  scope needs its own census, and a cross-volume census is required before any
  scope-wide zero-collision statement.
- It says nothing about other editions, regions, or patched installations.
- A clean census is not a resolver-identity receipt. `B-L2-04` still needs the
  real member-winner observation.

## 5. Structural observations from the same surface

These are observations about the bound surface, recorded because they change
product priorities. They are not format claims.

- All 4,333 file members use compression method 8 (raw DEFLATE). **No STORE
  member exists in this volume.** The product STORE path is therefore not
  exercised by this retail data at all, and raw DEFLATE is the only
  materialization route for it.
- Members are flat: every path has exactly one `/`, under a single `GData.afs/`
  directory entry. There are no nested archive directories and no path in the
  surface contains `obj`.
- By uncompressed volume the surface is dominated by one format:

  | Extension | Files | Uncompressed bytes |
  |---|---:|---:|
  | `pac` | 3,725 | 2,251,447,496 |
  | `txt` | 565 | 2,009,453 |
  | `bin` | 24 | 923,056 |
  | `tm2` | 6 | 230,816 |
  | `mod` | 4 | 15,632 |
  | `fon` | 4 | 1,971,728 |
  | `ptx` | 2 | 24,576 |
  | `bd` / `phd` / `tsb` | 3 | 379,232 |

  PAC is 99.75% of the uncompressed bytes. Accuracy of the relative-slot
  container reader is therefore on the path to effectively all game content.

- Every `.pac` member size is a multiple of 8, and 96.3% are a multiple of 16,
  with a sharp fall-off beyond that (32: 24.4%, 64: 13.6%, 128: 5.2%). This is
  consistent with an 8/16-byte alignment convention, which implies inter-slot
  and trailing padding is normal rather than exceptional. That matters for
  `RelativeSlotContainer`, which currently derives a slot's size from the
  distance to the next distinct offset and so absorbs any padding into the
  preceding slot.
- `tm2`, `fon`, `bin`, `bd`, `phd` and `tsb` are present in retail data but were
  absent from `FormatIntegrationRegistry`.
- The 565 `.txt` members are localization message tables
  (`*_msg_<lang>.txt`, at least 8 languages). The existing stage-TXT lexer
  targets StageSet/door tokens, which is a different corpus.

## 6. Measured identity of the `em000` resource

The roadmap names `obj\em000.pac` as the first high-value direct-retail
request. That is a **game request form**, not an archive member path. In this
surface the member is:

```text
central_index  9
path           GData.afs/em000.pac
crc32          464f0991
compressed     1,564,598
uncompressed   2,628,368
```

No member path in the surface contains `obj`, and every member sits directly
under `GData.afs/`. The request form and the member path therefore genuinely
differ, which is direct support for the existing rule that documentation and
tooling must not predeclare a `GData*.afs/...` member path: the mapping from
request to member belongs to the runtime resolver and has to be observed, not
assumed.

This census records the member identity as measured. It does not claim to have
derived the resolver's request-to-member mapping, which remains part of
`B-L2-04`.
