# DMC3 Native Reader reverses ported to C++20 (2026-10-01)

The reverse-engineered runtime logic of DMC-Native-Reader (branch
`NR-Luna-v73`) is kept here as engine-independent C++20 code: no Reader
`Session`, no Android, no renderer. Every module is a pure function of parsed
records, matrices and ages in ticks, built into `dmc_rengine_core` and covered
by simple tests. Constants and addresses refer to dmc3.exe (SHA-256
`e454272e…d082`); the evidence is in the notes listed per module.

| Header (`include/dmc_rengine/profiles/dmc3/…`) | Namespace | Content | Evidence note |
| --- | --- | --- | --- |
| `fx/fx_types.hpp` | `fx` | row-vector Vec2/Vec3/Matrix4, Rz*Ry*Rx local matrix | — |
| `fx/particle.hpp` | `fx::particle` | P records (Sprt00, Poly00, Line01): parse, spawn, update, draw packets | `dmc3-particle-p-records-2026-10-01.md` |
| `fx/generator.hpp` | `fx::generator` | G records: spawn schedule, LCG, children | `dmc3-generator-g-records-2026-10-01.md` |
| `fx/effect_bank.hpp` | `fx::effect_bank` | FXBANK / PNST bank: ids, E/V/P/G records | `dmc3-effect-runtime-2026-10-01.md` |
| `fx/effect_runtime.hpp` | `fx::runtime` | spawn API kinds, parent modes 0..3, CA0 lift, V clock, E lifetime | `dmc3-effect-runtime-2026-10-01.md` |
| `fx/enemy_events.hpp` | `fx::enemy` | enemy event handler 0x1401C3130 table, em000 death schedule, AI command tables, em000 attack events | `dmc3-effect-triggers-2026-10-01.md`, `dmc3-em000-attack-effects-2026-10-01.md` |
| `motion_script.hpp` | `motion` | enemy motion script (em slot N-3) | `dmc3-native-reader-code-facts-2026-10-01.md` |
| `cloth_chain.hpp` | — | CLT cloth chains: parse and simulate | same |
| `uv_scroll.hpp` | — | TSC texture scroll (types 4, 5, 10) | same |
| `collision_shapes.hpp` | `collision` | character attack/chain capsule tables | same |
| `environment_collision.hpp` | `environment_collision` | stage HITS: parse, segment raycast, sphere slide, floor | same |
| `attachment_tables.hpp` | — | weapon / companion / Lady component attachment tables | same |
| `em034_shells.hpp` | `em034` | Lady shells: Shl02 homing missile spawn + flight, Shl03 rocket, Shl00/Shl05 straight shots, Shl04 grenade with bounce, retire ages | Reader `dmc3-shell-effect-runtime-exe-v73.md` |
| `stage_layout.hpp` | `stage_layout` | stage `# GAME` text: SET blocks, model placement, uv scroll, `eff`/`epos`, `cam_init` | `dmc3-native-reader-code-facts-2026-10-01.md` |

Tests: `dmc3_fx_particle` and `dmc3_fx_generator` replay the emulator truth
(`*_truth.inc`, synthetic records only, no game data); `dmc3_fx_enemy_events`,
`dmc3_reader_ports` and `dmc3_em034_stage` cover the tables, parsers and
flight math with fabricated inputs.

Standalone approximations kept from the Reader (marked in the headers):
Shl02 holds its direction after the first retarget (the EXE steers toward the
player); without a stage raycast the grenade bounces on the plane y = 0.
