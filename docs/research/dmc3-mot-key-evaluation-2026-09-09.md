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
The helper does not implement segment lookup/cache, looping or blending;
no whole-animation evaluator is claimed. Static segment-search behaviour
is recorded below for subsequent implementation and differential validation.

## Compression-3 cached segment search

`0x1402E8C80..0x1402E8E10` uses the key array at channel+0x10 and a signed
cached index at channel+0x18. It masks every key time with 0x7FFF. For finite
local query time, nonempty sorted keys and an initially valid cached index:

```
i = cached_index
if t >= time[i]:
    while i < count-1:
        if time[i+1] > t: return segment(i, i+1), cache=i
        if time[i] == t: return single(i), cache=i
        i += 1
    return single(i), cache=count-1
else:
    while i >= 1:
        previous = i-1
        if t > time[previous]: return segment(previous, i), cache=previous
        if t == time[previous]: return single(previous), cache=previous
        i = previous
    return single(0), cache=0
```

Forward segment return is `0x1402E8D52`, backward segment return is
`0x1402E8DF2`. Before-first/after-last queries return the first/last key
without a second key. Equal-time runs can select different duplicate keys
depending on the incoming cache; a stateless binary search is not proven
equivalent. The count is sign-extended from track+2 at `0x1402E8CCA`, whereas
the raw parser stores u16. Runtime eligibility above 32767 keys therefore
cannot be inferred from parse success. Invalid cache and nonfinite queries
are outside the finite, well-formed reconstruction above. These branches
need execution-based comparison before adding a complete player.

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
| 0x001 | +0x1E0 | Scale X |
| 0x002 | +0x200 | Scale Y |
| 0x004 | +0x220 | Scale Z |

The downstream proof is the normal matrix path `0x14030E9B0` called at
`0x14030E7CB`. Instructions `0x14030EA09`, `0x14030EA28` and `0x14030EA1B`
multiply external axis factors by joint+0x1E0/+0x200/+0x220 respectively.
The resulting XYZ factors feed `0x14032ED30` at `0x14030EB7D` on an
identity-initialized matrix. That helper broadcasts each scalar and multiplies
the matrix vectors at +0/+0x10/+0x20, leaving +0x30 unchanged
(`0x14032ED30..0x14032ED5B`). The scale matrix is then composed into joint+0x110
via calls at `0x14030EC06` and `0x14030EC1A`. This establishes scale semantics,
not merely neutral defaults. Exceptional tiny/near-unit factors, external
factors and full hierarchy composition remain outside the helper API.

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
