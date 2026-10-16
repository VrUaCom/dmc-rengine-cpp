# BD, SO and malformed PNST reverse pass

## Scope

This pass reads the immutable ASCII junction `F:\Reverse-Core-Corpus\stage_source_ascii` and writes only evidence. It covers all 900 fixed-size 96-byte `.bd` payloads, all 430 `.so` payloads, and the four PNST containers rejected by the native parser.

## 96-byte BD records

The 900 records share a stable 24-word envelope. Empirical invariants across every record:

- `u32@0` is zero in 900/900 records.
- `u32@8`, `u32@12`, `u32@40`, `u32@44`, `u32@68`, `u32@76` are zero in at least 97.6% of records.
- `u32@28`, `u32@32`, `u32@36`, `u32@52`, and `u32@88` vary as small integer or ID-like fields.
- `f32@56` and `f32@60` are finite in every record and are exactly `1.0` in 749/900 and 767/900 records respectively.
- `u32@48` and `u32@64` are mixed bitfield/color-like domains; treating them as floats is not stable.
- `u32@92` is zero in 898/900 records.

These are field-boundary observations, not semantic names. The corpus contains both `static_effect` and stage/enemy effect records, so a single semantic schema would be unsafe without the EXE reader or a round-trip writer.

## SO families

All 430 `.so` files were tested against the recovered SO invariants:

| Class | Count | Evidence |
|---|---:|---|
| 23?16-byte effect payload | 366 | exact size 368 and repeated 16-byte row shape; concentrated in `st*_effect` |
| compact SO link table | 21 | leading `u32=6`, 4-byte rows, fourth byte reserved zero |
| larger opaque SO payload | 43 | first words form a type-6-like header, but the existing graph parser rejects non-monotonic 16-bit table values |

The 43 large payloads cluster beside `.mod`, `.pac`, `.ukn`, `.efm`, `.scm` and `.shw` resources under `em*`, which is evidence of an object or collision companion route, not proof of a field meaning.

## Four PNST anomalies

The four rejected files are deterministic and identical in shape:

| File | Size | Slots | In-bounds offsets | `offset == size` | Out of bounds | Max offset / size |
|---|---:|---:|---:|---:|---:|---:|
| `m11_b00_012.pac` | 203,064 | 48 | 19 | 1 | 28 | 2.984596 |
| `m12_b02_012.pac` | 401,912 | 177 | 71 | 1 | 105 | 2.986684 |
| `m17_c00_012.pac` | 159,312 | 54 | 22 | 1 | 31 | 2.986643 |
| `m99_s00_012.pac` | 485,456 | 240 | 91 | 1 | 148 | 2.983916 |

Every offset is nonzero and unique. The maximum is just under three times the physical file size, and the offsets distribute across three size bands. The companion `_011.txt` manifests contain four `# End` markers, consistent with three repeated sections plus the final terminator. This supports a **truncated or multi-volume extraction image**, not a new valid PNST offset convention: the public `pac-linux` extractor uses offsets directly against the supplied archive span, and the historical QuickBMS discussion likewise describes the first table as direct offsets.

The native parser is therefore correct to reject these four physical byte images. They should not be force-opened by taking offsets modulo the file size; that would silently bind slots to unrelated bytes.

## External repository checks

- `reflake/pac-linux` is a public recursive DMC3 PAC/PNST extractor/repacker. Its extractor reads the 32-bit offset table directly and computes each child span from the next nonzero offset. [Repository](https://github.com/reflake/pac-linux)
- A ZenHAX DMC3 HD thread independently documents the `PAC\0` header, entry count, direct offsets and `dbsT` sound chunks, while explicitly calling the posted script an idea rather than a complete parser. [Discussion](https://www.zenhax.com/viewtopic.php@t=15769.html)

## Reproduction

```powershell
python F:\Reverse-Core-Corpus\dmc-rengine-cpp\tools\reverse\bd_so_pnst_census.py F:\Reverse-Core-Corpus\stage_source_ascii --out F:\Reverse-Core-Corpus\dmc3-stage-reverse\evidence\bd_so_pnst_census.json
```
