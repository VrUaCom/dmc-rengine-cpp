# Resource structure view

`integration::read_structure` (`include/dmc_rengine/integration/resource_structure.hpp`)
lists what a recovered reader takes out of one payload, as titled sections of
label / value rows. It is the one place GDS clients (the `structure` CLI
command, Pocket GDS) get that listing from, so a reverse lands once and every
client shows it.

| Format | Reader | What the rows show |
| --- | --- | --- |
| `clt` | `motion::parse_clt` (parser 0x1402CA345 / 0x1402CA42A) | per chain: gravity, spring, speed, stiffness, damping, wind, floor, length limit, bones as `node axis` |
| `tsc` | `motion::parse_tsc` (0x14030C1C0) | per scroll record up to `$`: type, texture, joint, directions, rates and times |
| `evt` | `formats::evt::Parser` | header, stream starts, every command with its opcode name and arguments |
| `hits` | `environment_collision` (cell walk 0x14005E880) | bounds, grid, cell references, record kinds by flags with floor / wall / ceiling counts |
| `pnst` (effect bank) | `fx::effect_bank` (loader 0x1402C04C0) + the E / P / G / V / A views | record counts per kind with their registrars, then each record: T texture format and size, A frames, E texture / animation, P class / life / blend / layers, G motion and spawns, V children, C clip points, M model |
| `collision-shapes`, `so-volume` | `collision::parse_shapes` (ICollisionHandle 0x1404C65A0) | each 80-byte record: sphere, box or capsule with its values |
| `motion-script` | `motion::MotionScriptFile` (bind 0x1400594B0, interpreter 0x140058FE0) | banks, every action: played motion, ops, waits, last frame, loop / hand-over, weapon states, MOT ids from table B |
| `pac` (character) | attachment tables, `player_attachment`, `em000_family` | slot roles of `pl000..pl003`, `plwp_*`, `em028`, `em000`, and the cloth / TSC sources of the other enemies |

A plain PNST container is not an effect bank and is declined with a reason; a
PAC with no recovered contract is declined the same way.

## Classification of nameless tables

The runtime fetches a character's shape table and motion script by slot
index, never by name or content tag. The classifier therefore probes them only
for a slot with no name of its own (no extension, or `.bin`), after every
tagged and structural format: an 80-byte-record table with a box record is
`collision-shapes` (spheres and segments only stay `so-volume`, the same
payload as em000 slot 40), and u16 bank tables closed by 0xFFFF whose first
action plays a motion are `motion-script`. Both have read-only Native Reader
modules (`profiles.dmc3.collision-shapes-v1`, `profiles.dmc3.motion-script-v1`)
that report the structure summary.

## CLI

```
dmc-rengine structure <file> [--slot N] [--format F]
```

`--slot` reads one slot of a PAC / PNST and classifies it by its bytes;
`--format` overrides the classifier. Tests: `tests/resource_structure_tests.cpp`.
