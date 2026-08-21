# GDSpaces L1 Pass 87 — Original PTX envelope structural recovery — 2026-08-21

## Scope

Layer 1 only. This pass recovers the **read-side structural envelope** used by the original DMC3 PTX/TIM2 runtime path. It is deliberately separate from the transformed DDS-bearing profile covered by Passes 78–85.

This pass does **not** claim a PTX/TIM2 writer. TIM2 picture-header semantics, CLUT/swizzle conversion and the original writer/converter boundary remain open.

## Canonical research context

The canonical Phase16/17 Drive corpus already states:

- runtime-original PTX/TIM2 is distinct from the supplied DDS-bearing PAC representation and from extracted DDS;
- the original PTX path starts entries at `+0x800` and advances by `blockCount * 0x800`;
- the TIM2 parser checks `TM2\0`;
- DDS-from-memory is a separate runtime path;
- DDS → original PTX/TIM2 authoring remains blocked.

Pass 86 additionally proves that the preserved v6 analysis corpus contains no raw binary TIM2 sample, so the envelope ABI must be recovered from executable evidence rather than inferred from the transformed DDS-bearing corpus.

## Probe artifact used for instruction recovery

A preserved generated executable was materialized from the ChatGPT Library:

- file: `dmc3_phase17_reng_probe.exe`
- size: `6,415,872` bytes
- SHA-256: `9a3513db0f7cfeabed38f62836a5a6d55e42741b0965bfa5947d3c7b33532735`
- PE entry RVA: `0x34615C`, matching the canonical DMC3 target entry point;
- first eight section names/layouts match the canonical target family;
- one additional executable section exists: `.reng` at RVA `0xDAC000`, raw offset `0x60E600`;
- `.reng` contains marker `DMC_RENG_PHASE17_PROBE`.

The recovered PTX/TIM2 instructions below are in the original `.text` section, not in `.reng`.

The probe is **corroborating executable evidence**, not a replacement for the canonical unpacked EXE SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`. Promotion to canonical exact-byte authority still requires a byte-window parity check against that original executable when it becomes raw-accessible again.

Probe `.text` SHA-256:

`6b16d64ea8da9e3a1a0afe415f628328a209bba023364ab6f769db538d84c5ce`

## PTX envelope access fragment — VA 0x140336340

The `.text` byte window begins:

```text
48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57
48 81 EC 80 00 00 00
8B 01
48 8D 59 04
48 8D B1 00 08 00 00
...
```

Direct instruction consequences:

```text
mov eax, dword ptr [rcx]       -> textureCount is u32 at envelope +0x00
lea rbx, [rcx+0x04]            -> blockCount table begins at envelope +0x04
lea rsi, [rcx+0x800]           -> first TIM2 entry begins at envelope +0x800
```

In the entry loop:

```text
call 0x1403365B0               -> parse current TIM2 entry
mov ecx, dword ptr [rbx]       -> read current blockCount[i]
add rbx, 4                     -> next u32 block-count cell
shl ecx, 0x0B                  -> blockCount[i] * 0x800
...
advance rsi by that byte count -> next TIM2 entry
```

The same fragment also contains a vectorized branch that sums multiple block-count cells before selecting a requested entry. Both paths agree on `u32` block counts and `<< 11` sector geometry.

### Recovered structural ABI

For an envelope base `B`:

```text
B + 0x000: u32 textureCount
B + 0x004: u32 blockCount[textureCount]
...
B + 0x800: TIM2 entry 0
entry(i).offset = 0x800 + sum(blockCount[0..i-1]) * 0x800
entry(i).span   = blockCount[i] * 0x800 when blockCount[i] != 0
```

The runtime parses the current TIM2 entry **before** reading that entry's `blockCount`. Therefore a zero count on a non-final entry makes the next entry non-progressing/unlocatable, but a zero count on the final entry does not invalidate the already-parsed final entry. Pass 87 models this conservatively as:

```text
non-final blockCount == 0 -> fail closed
final blockCount == 0     -> terminal entry is bounded to supplied resource EOF
```

This terminal rule is a bounded product policy, not a claim that every original PTX uses a zero terminal sentinel.

The bytes between the end of the count table and `+0x800` remain **opaque header bytes**. Pass 87 does not assign them invented semantics or require them to be zero.

## TIM2 entry parse fragment — VA 0x1403365B0

The `.text` byte window begins:

```text
48 89 5C 24 18 57 48 83 EC 20
81 39 54 4D 32 00
48 8B FA
48 8B D9
0F 85 AD 00 00 00
...
8B 69 08
...
48 03 E9
```

Direct consequences:

```text
cmp dword ptr [rcx], 0x00324D54  -> entry +0x00 must be "TM2\0"
mov ebp, dword ptr [rcx+0x08]    -> u32 relative offset at TIM2 +0x08
add rbp, rcx                      -> runtime converts it to an interior pointer
```

This pass intentionally records the `+0x08` field only as a **relative interior-data offset**. Its deeper TIM2 semantic name is not promoted here.

## Independent caller corroboration

At `0x1402FDA66` a runtime object field supplies an envelope base, the code adds `0x800`, calls `0x1403365B0`, and separately reads `dword ptr [envelope+0x00]` for count-dependent sizing. This independently corroborates `textureCount @ +0x00` and first entry `@ +0x800`.

At `0x14030DF90` renderer-side state passes a PTX base from `[rdi+0x40]` into `0x140336340` together with per-record index/parameters, confirming that `0x140336340` operates on a live PTX envelope rather than on an unrelated TIM2 buffer.

## Product parser policy

Pass 87 adds `Dmc3PtxEnvelopeParser` with these rules:

- minimum envelope header: `0x800` bytes;
- `textureCount` read from `+0x00`;
- `blockCount[]` read as contiguous little-endian `u32` from `+0x04`;
- table must remain inside the first `0x800` bytes;
- each bounded entry starts at the EXE-derived sector offset;
- each entry must begin `TM2\0`;
- entry `+0x08` relative offset must stay within that bounded entry span;
- opaque header bytes and trailing bytes are reported, not normalized;
- non-final zero block counts fail closed;
- final zero block count is represented explicitly as `terminal_span_to_eof` and bounded to supplied resource EOF;
- no writer or TIM2 conversion API is exposed.

## What this pass proves

- exact PTX envelope count offset/width;
- exact block-count table offset/width;
- exact first-entry offset;
- exact sector multiplier;
- exact TIM2 magic check;
- exact TIM2 `+0x08` relative-pointer use;
- a bounded read-side materialization contract suitable for GDSpaces.

## What remains open

- byte-window parity of these probe `.text` ranges against the canonical unpacked EXE;
- real retail PTX sample receipt from `dmc3-0.nbz`;
- semantics of the remaining PTX header region;
- exact TIM2 picture header fields beyond the minimal recovered fields;
- CLUT ownership and conversion;
- swizzle/unswizzle rules;
- original PTX/TIM2 writer/converter;
- original-game consumption of a rebuilt texture resource.

Layer 1 remains **NOT COMPLETE**.
