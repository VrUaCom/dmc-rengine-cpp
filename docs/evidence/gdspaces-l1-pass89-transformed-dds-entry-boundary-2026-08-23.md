# GDSpaces L1 Pass 89 — Transformed DDS entry boundary and promotion reconciliation — 2026-08-23

## Scope

Layer 1 only. This pass independently re-checks the preserved Phase16 texture corpus against the newer PTX/TM2 runtime recovery and deliberately separates three things:

1. the **outer sector envelope**;
2. the **per-entry representation**;
3. **retail provenance / storage-to-runtime materialization**.

Status: **HIGH — transformed-corpus structure confirmed; retail provenance not proven.**

No writer, converter, or retail writeback authority is promoted.

## Revalidation source

The current pass re-analysed the preserved `phase16_texture_sample_analysis.json` per-entry byte heads and DDS header receipts associated with the canonical DMC3 target SHA-256:

`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The raw canonical EXE and a direct retail `DMC3-0.nbz` were **not** materialized in this pass, so claims that require those exact raw inputs remain gated.

## Corpus result

| PAC | DDS | TM2 | Recognized block-table slots | Recognized entries |
| --- | ---: | ---: | ---: | ---: |
| `st001.pac` | 17 | 0 | 1 | 17 |
| `id100.pac` | 21 | 0 | 21 | 21 |
| `pl000.pac` | 4 | 0 | 1 | 4 |
| `em000.pac` | 19 | 0 | 3 | 8 |
| `m09_b01.pac` | 30 | 0 | 0 | 0 |
| **Total** | **91** | **0** | **26** | **50** |

The remaining **41 DDS signatures** belong to opaque/non-block-table texture slots and remain a separate reverse target.

## Outer-envelope finding

For all 26 recognized transformed texture bundles, the preserved corpus uses the same structural geometry already recovered for PTX:

```text
u32 textureCount            @ +0x00
u32 blockCount[count]       @ +0x04
first entry                 @ +0x800
next entry                  += blockCount * 0x800
```

For `st001.pac` slot 1, for example:

- `textureCount = 17`;
- the 17 recorded block counts exactly account for the complete slot;
- computed end equals the physical slot size.

This establishes a strong structural correlation:

> the transformed DDS-bearing representation can preserve the **outer PTX sector envelope** even though its entries are not `TM2\0`.

It does **not** establish that these PAC bytes came directly from pristine retail storage.

## Recovered transformed entry preamble

Across **50/50** recognized entries, DDS begins at `entry + 0x70`. The preceding 0x70-byte descriptor has repeatable relations to the DDS that follows:

| Offset | Observed relation | Coverage |
| ---: | --- | ---: |
| `+0x08` | `0x00020000 | (DDS mipCount << 8)` | 50/50 |
| `+0x0C` | constant `0x0000AAE4`; semantics unknown | 50/50 |
| `+0x10` | `u16 width` = DDS width | 50/50 |
| `+0x12` | `u16 height` = DDS height | 50/50 |
| `+0x14` | constant `1`; semantics unknown | 50/50 |
| `+0x20` | constant `0x40`; semantics unknown | 50/50 |
| `+0x38` | DDS bytes after the 0x80-byte DDS header | 50/50 |
| `+0x44` | `floor(width / 2)` | 50/50 |
| `+0x46` | `floor(height / 2)` | 50/50 |
| `+0x48` | `f32 2.0 / width` | 50/50 |
| `+0x4C` | `f32 2.0 / height` | 50/50 |
| `+0x64` | exact full DDS byte length, including 0x80-byte header | 50/50 |
| `+0x68` | constant `8`; semantics unknown | 50/50 |
| `+0x6C` | constant `0`; semantics unknown | 50/50 |
| `+0x70` | `DDS ` | 50/50 |

All 50 recognized entries in this subset use DXT5, so format-specific generalization beyond this observed set is intentionally withheld.

### Concrete example

The first `st001` entry has:

```text
allocation span = 11 * 0x800 = 22528
DDS offset      = +0x70
DDS dimensions  = 128 x 128
mips            = 8
DDS full bytes  = 22000
descriptor+0x38 = 21872 = 22000 - 0x80
descriptor+0x64 = 22000
descriptor+0x48 = 0.015625 = 2 / 128
descriptor+0x4C = 0.015625 = 2 / 128
```

The descriptor is therefore structured metadata, not arbitrary padding.

## Runtime TM2 versus transformed DDS entry

The current runtime recovery and the preserved transformed corpus describe **different entry wrappers**:

```text
runtime:
PTX outer envelope
  -> TM2\0 entry
     +0x08 DDS-relative pointer/offset
     +0x3C DDS byte size
     +0x58/+0x5A width/height
     -> DDS
```

versus:

```text
preserved transformed corpus:
same 0x800-sector outer envelope
  -> 0x70-byte non-TM2 descriptor
     +0x10/+0x12 width/height
     +0x38 DDS payload bytes
     +0x64 full DDS bytes
     -> DDS at +0x70
```

The strongest current hypothesis is therefore narrower than the old “whole format changes” model:

> the conversion/materialization boundary may replace or reinterpret the **per-entry wrapper while preserving outer envelope geometry**.

This is **HIGH-confidence structural correlation**, not yet a proven historical transformation algorithm. Direction, producer, timing, and retail provenance remain open.

## Historical extracted-DDS control

The old extracted `st001_001_###.dds` files are not raw byte slices of the preserved PAC entries:

- exact substring matches: **0/17**;
- 16/17 extracted images have doubled width and height;
- raw entries are 17/17 DXT5;
- extracted files are 13 DXT1 + 4 DXT5.

Therefore those extracted DDS files must not be used as writer authority for the raw entry representation.

## Reconciliation defect: PR #172 versus PR #174

A promotion blocker is now explicit.

PR #172's `OriginalPtxEnvelopeGeometryValidator` preserves **zero block-count advances** and states that `blockCount * 0x800` is a runtime cursor advance, not an intrinsic entry length.

PR #174's `Dmc3PtxEnvelopeParser` instead:

- rejects a non-final zero `blockCount`;
- treats nonzero `blockCount * 0x800` as the entry `span_size`;
- uses that span as the hard bound for TM2/DDS validation.

These contracts are not equivalent.

The original runtime evidence says the current entry is parsed before its `blockCount` advance is consumed, so the #174 fail-closed policy cannot be promoted as original runtime semantics without additional evidence.

**Required reconciliation:** split **cursor advance geometry** from **intrinsic payload extent / validation bound**. Do not infer one from the other unless independently proven.

## Storage/materialization boundary still open

The canonical resource-I/O evidence already contains an async whole-file loader:

```text
selector      0x140033480
open          0x1400333F0
chunk submit  0x140033500
completion    0x1400335A0
chunk unit    0x800
```

and the texture subsystem contains:

```text
CPtxManager load A  0x140314E00
CPtxManager load B  0x140314FA0
PTX parse A          0x140336BB0
PTX parse B          0x140336A70
TM2 entry parser     0x1403365B0
```

The identical 0x800 unit is a useful xref/data-flow target, **not proof that these functions are directly connected**.

The next reverse pass must establish the actual buffer provenance:

```text
VFS / PAC-or-direct payload
  -> allocation / whole-file read ?
  -> LoadedResourceView payload
  -> CPtxManager
  -> PTX/TM2 parser
```

or reject that candidate and identify the real producer.

## Promotion boundary

Do **not** promote Layer 1 texture writeback until all of the following are closed:

1. reconcile #172/#174 zero-block and span semantics;
2. acquire a direct retail PTX receipt from `DMC3-0.nbz`;
3. prove the storage-to-runtime producer/materializer chain;
4. classify the remaining 41 opaque DDS-bearing cases;
5. recover enough entry semantics for a safe writer;
6. demonstrate byte-preserving round-trip where expected;
7. demonstrate original-game consumption of a minimally rebuilt texture resource.

## Next reverse targets

1. Trace callers and payload provenance around `0x140314E00` / `0x140314FA0`.
2. Test the `0x140033480` whole-file-loader candidate with xrefs and pointer lineage rather than by matching sector size.
3. Recover the producer/allocation/copy ownership of the runtime `TM2\0` wrapper.
4. Reverse the 41 DDS signatures outside the recognized outer block-table subset.
5. Acquire direct retail `DMC3-0.nbz` bytes and compare resource-by-resource against the preserved transformed corpus.

Layer 1 remains **NOT COMPLETE**.
