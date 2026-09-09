# MOT key decoding and normal MOD binding

Status: EXE_CONFIRMED bounded static recovery; not full playback parity.

Canonical `dmc3.exe`, 6,356,432 bytes, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Addresses below are virtual addresses for image base `0x140000000`.
Reproduce with `objdump -d -Mintel --start-address=... --stop-address=... dmc3.exe`.
No executable or retail payload bytes are distributed here.

## Track decoding

`0x1402E9AA0` attaches track records by ordinal, walking their u16 spans.
Compression 2 has a 0x10-byte prefix; compression 3 has a 0x20-byte prefix.
`0x1402E9650` decodes the selected key into a sample:

| Sample | Recovered expression |
|---|---|
| Time | `(key.time_control & 0x7fff) + signed_i16(track + 6)` |
| Mode | bit 15 clear: 1; set: 2 |
| Value | `float(key.value) * f32(track+0x0c) / 65535 + f32(track+0x08)` |
| Incoming slope, compression 3 | `float(key+4) * f32(track+0x14) / 65535 + f32(track+0x10)` |
| Outgoing slope, compression 3 | `float(key+6) * f32(track+0x1c) / 65535 + f32(track+0x18)` |

The signed offset is **track +6**, not key +6. Compression 2 returns zero
slopes. The PE-mapped float at `0x140507398` is exactly 65535.0.
Compression 0/1 decoder cases use stride 8/16, value f32 at key+4;
type 1 also copies f32 key+8/+12. They remain outside the quantized analysis
API and the currently validated parser grammar. Attach-table entries for
types 4–7 are insufficient to establish usable decoding semantics.

## Compression-3 interpolation

The evaluator starts at `0x1402E9170`; compression-3 handling is at
`0x1402E91D8`. It first subtracts signed track+6 from evaluation time.
At `0x1402E9261`, the sign of the left key's raw time selects interpolation:
bit 15 clear uses linear interpolation; set calls `0x1402E9880`.
The cubic helper ends at `0x1402E994F`. Its constants at `0x14035D56C`
and `0x1404C6054` are 1.0 and 3.0, independently read from the PE.

For `d=t1-t0`, `u=(t-t0)/d`, the algebraic result is:

```
(2*u^3 - 3*u^2 + 1)*v0 + (3*u^2 - 2*u^3)*v1
+ (u^3 - 2*u^2 + u)*d*outgoing0 + (u^3 - u^2)*d*incoming1
```

The analysis helper implements this formula for an already selected,
positive-length compression-3 segment. It rejects out-of-segment and nonfinite
query times. It is not a bit-identical reproduction of SSE operation order.
Segment lookup/cache, repeated times, looping and endpoint selection remain
unrecovered here; no whole-animation evaluator is claimed.

## Normal binding to MOD nodes

`0x14030F850` binds model rest transforms. The source advances by 0x20 bytes:
translation source+0/+4/+8 feeds joint+0x124/+0x144/+0x164;
rotation source+0x10/+0x14/+0x18 feeds +0x184/+0x1A4/+0x1C4.
The last three channel defaults at +0x1E4/+0x204/+0x224 are 1.0.

The normal MOT binding path at `0x140310A61` reads masks from MOT+0x1E
in joint-array order, bounded by the model node count. It does not establish
that arbitrary mismatched MOT/model domain counts are safe.

| Mask bit | Joint channel base | Proven rest binding |
|---|---|---|
| 0x040 | +0x120 | Translation X |
| 0x080 | +0x140 | Translation Y |
| 0x100 | +0x160 | Translation Z |
| 0x008 | +0x180 | Rotation X |
| 0x010 | +0x1A0 | Rotation Y |
| 0x020 | +0x1C0 | Rotation Z |
| 0x001 | +0x1E0 | Unit default; scale interpretation pending downstream proof |
| 0x002 | +0x200 | Unit default; scale interpretation pending downstream proof |
| 0x004 | +0x220 | Unit default; scale interpretation pending downstream proof |

Track traversal follows table order, **not ascending numeric mask bits**.
Group-excluded nodes still consume the corresponding track ordinals.
Header flag 0x2 takes an alternate path at `0x140310CBF`, outside this proof.
Header +0x0C remains raw in the parser: copying it into motion state does
not alone prove duration units or playback range semantics.

## Implementation and validation

`include/dmc_rengine/analysis/mot/key_decode.hpp` consumes the canonical parsed
IR without adding a second byte parser. Tests in `tests/mot_key_decode_tests.cpp`
cover signed offsets, quantization endpoints, slope orientation, unequal
slopes at the cubic midpoint, linear selection and rejected boundaries.
Built and executed with GCC 13, C++20, `-Wall -Wextra -Werror`.

The three-file structural receipt is not playback validation. The modular
parser permits equal low-bit times; its zero-tail check is less restrictive
than the compatibility facade's alignment/strict-time policy. Neither this
receipt nor this helper proves all MOT variants or full MOD/SCM closure.
