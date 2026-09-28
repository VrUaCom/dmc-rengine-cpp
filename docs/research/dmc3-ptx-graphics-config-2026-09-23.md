# PTX graphics profile and pool lifecycle

Date: 2026-09-23. Branch: **Ада-Астра**.
Canonical executable SHA-256:
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

**Result: static EXE audit confirms the graphics profile backing, the startup
preset, its pool-bound writes, and 14 direct pool lifecycle callsites.** This
narrows the config type and gives the default preset values, while the complete
type and all scene-dependent values remain open.

## Global profile object

`0x140D6D300` is a global pointer slot. Static initializer `0x140025BF0` points
it at `0x140D6D310`, then calls `0x14032D3C0` with count `0x2C`. That helper
multiplies the count by four and zero-fills `0xB0` bytes. The storage therefore
has a confirmed initialized extent of `0xB0`; this is not by itself a complete
class definition. The placement and pool helpers consume these unsigned words:

| Offset | Use established from callers |
|---|---|
| `+0x20`, `+0x22`, `+0x24` | Preset dimensions or profile inputs copied by `0x140337CD0` |
| `+0x3C`, `+0x40`, `+0x46` | Intermediate values read while deriving `+0x4A` |
| `+0x4A` | Derived profile bound |
| `+0x4C` | Pool base; copied into pool `+0xCB08` |
| `+0x4E` | Pool limit; copied into pool `+0xCB10` and used by reservation updates |

The startup routine `0x140337BF0` calls `0x140337CD0` with `ECX=1, EDX=0`,
then calls `0x140332F00` with `CL=1`. Preset-table index 1 points to static record
`0x1405D1AE8`, whose bytes decode to byte values `1, 2, 1, 2`, words `512,
256, 224`, and final byte `2`. `0x140337CD0` copies those preset values into the
global profile object and initializes adjacent flags. This is a canonical
startup preset, not proof that every later scene keeps the same values.

`0x140332F00` writes `+0x4A` as 16-bit wrapped arithmetic:

`factor * word[+0x40] - word[+0x46] - word[+0x3C] + 0x4000`

where the factor is `-2` if byte `+0x06` is nonzero, otherwise `-1`. It then
sets `+0x4C` to `0x1900` and writes `+0x4E = wrap16(0x1900 + word[+0x4A])`.
Thus the base field is established on this startup path; the final limit
depends on intermediate fields also written by profile setup.

## Pool initialization and reconfiguration

Pool initializer `0x140331910` clears `0xCB50` bytes, then copies profile
`+0x4C` to pool `+0xCB08` and `+0x4E` to `+0xCB10`; the current reservation
starts at zero. Four direct initializer callsites were confirmed:
`0x14004F191`, `0x14004F438`, `0x140238376`, and `0x140238498`.

Reservation updater `0x140331D90` writes `wrap32(blocks << 5)`, subtracts it
from profile `+0x4E`, updates pool bounds, then calls `0x140315150` to reset
manager keys and counters. Ten direct updater callsites were recovered in the
canonical executable. Several state-transition paths immediately call
`0x140331710` after updating the reservation. That helper adjusts occupancy
bytes and clears the pool's first `0x2800` bytes; its body contains no backing
resource release call. This establishes a reset of placement bookkeeping, not
that GPU resources referenced by old records were destroyed first.

The direct calls show startup and later state-transition use. They do not
provide a scene-wide trace proving a single initialization/shutdown order. The
remaining resource-owner question is which callers release published GPU
resources before these record and occupancy resets.

## Artifacts and limits

- [Canonical static audit](../../scripts/reverse/audit_ptx_graphics_config.py)
- [Instruction, preset and caller evidence](../../data/reverse/ptx-graphics-config-20260923/evidence.json)
- [Placement and reservation model](../../include/dmc_rengine/reverse/ptx_record_placement.hpp)
- [Pool placement report](dmc3-ptx-placement-2026-09-16.md)

The audit validates instruction boundaries, direct calls, profile bytes, and
selected field writes against the canonical EXE. It does not execute the game,
recover every field in the `0xB0` storage, or infer values for later runtime
profiles. GPU acceptance and pointer ownership remain outside this evidence.
