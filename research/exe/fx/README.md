# Effect-system reverse tools (`fx`)

Tools behind the effect notes of 2026-10-01:

* `docs/research/dmc3-effect-runtime-2026-10-01.md`
* `docs/research/dmc3-particle-p-records-2026-10-01.md`
* `docs/research/dmc3-generator-g-records-2026-10-01.md`
* `docs/research/dmc3-effect-triggers-2026-10-01.md`
* `docs/research/dmc3-em000-attack-effects-2026-10-01.md`
* `docs/research/dmc3-reader-cpp20-port-2026-10-01.md` (C++20 code of these reverses)

Nothing of the game is committed here. Every tool reads a local copy of
`dmc3.exe` (canonical SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`; `pe.py`
prints a note for any other image) and, where needed, records dumped from your
own PACs into a local directory. Do not commit their output.

```sh
pip install capstone unicorn
export DMC3_EXE=/path/to/dmc3.exe
```

## Static analysis (this directory, `python3 tool.py dmc3.exe ...`)

| Tool | What it prints |
| --- | --- |
| `pe.py` | PE64 reader: sections, VA <-> file offset, `.pdata` function bounds |
| `annot.py START END` | disassembly with RIP-relative constants resolved (floats, strings) |
| `xdis.py START END` | plain disassembly with file offsets |
| `xref.py VA [--disp D]` | rel32 call / jmp sites and 8-byte pointers to VA; with `--disp`, every memory operand using displacement D |
| `leaxref.py VA...` | RIP-relative `lea` / `mov` that take the address VA |
| `ripref.py VA...` | any RIP-relative operand that reads VA |
| `vtables.py CLASS [--slots N]` | vtables of a class by RTTI name (one per base subobject) and their slots |
| `rtti.py VA...` | class name and slot index of a vtable or a vtable slot |
| `resolve.py` | helper: constant ecx / edx / r9 before a call, following `lea ecx, [rdx - N]` |
| `spawn_sites.py` | every call of the spawn API with (kind, id, mode) and the owning class (translation unit by own vtable functions) as JSON |
| `vcall.py DISP SUB` | virtual calls `[reg + DISP]` preceded by a `+SUB` subobject reference, with the constant edx (enemy events: `vcall.py 0x80 0x110`) |
| `vcall3.py CODE...` | `call [reg + 0x80]` with one of the given constant codes in edx (callers of the enemy event handler through other pointers) |
| `comcmd.py LO HI` | per function in [LO, HI): `play(cmd)` (interface `+0x20`), `play2` (`+0x28`), frame gates (`comiss` after `+0x40`), event codes (`+0x80`): the enemy AI command layer |
| `linsum.py LO HI` | per function: spawns (`K#id`), float compares, script plays (`0x14005A1F0`) in code order |

## Emulator (`emu/`)

`emu.py` maps the image into Unicorn, maps pages lazily, stubs functions by
address and returns to the caller. `imports.py` installs CRT / math imports.
`init_funcs.txt` lists the static initializers to run first; regenerate it
with `make_init_funcs.py` (6260 frameless leaf entries of the initializer
table `0x14034F7F0..0x14035D2B0` that load an XMM constant). Without them the
w-mask at `0x1405D9F30` is zero and every particle matrix gets w = 2.

| Script | Produces |
| --- | --- |
| `evt.py [codes]` | runs the enemy event handler `0x1401C3130` per code on a fabricated actor; prints / writes `evt.json` (kind, id, matrix, parent entry and mode) |
| `ptcl.py`, `ptcl2.py`, `dbg2.py` | P emitter harness (`Sys`): spawn, update, draw packets for any record |
| `gt.py`, `gtall.py`, `gt_poly.py` | synthetic P truth (Sprt00 / Poly00 / Line01, two layers, every ease) |
| `geninc.py`, `geninc_poly.py OUT` | C++ include of that truth (the Reader's `particle_truth.inc`) |
| `realdump.py` | per real P record: spawn state + 12 updates (`$FX_RECORDS` -> `$FX_OUT`) |
| `gen.py`, `gen_gt.py` | generator harness (`GSys`) and per real G record spawn events with an injected LCG |
| `gen_synth.py OUT` | synthetic G truth (the Reader's `generator_truth.inc`, reproduced byte for byte) |
| `comemu.py [VTABLE LO HI]` | runs AI command pairs (update / start) of a CCom vtable with a stub enemy interface; plays, frames and event codes (`FX_LENGTHS`, `FX_TABLES`) |
| `action_lengths.py PAC SCRIPT_SLOT MOT_SLOT` | MOT and frame count of every script action (local PAC only) |
| `clip_t.py` | C-clip B-spline samples |
| `records.py`, `dump_bank_records.py PAC SLOT DIR` | FXBANK parsing; dump every record of a bank into DIR (local only) |

FXBANK slots of the corpus archives: em000 / em006 / em007 41, em028 9, em034
28, plwp_sword / plwp_2sword 2; an `st*_effect.pac` is a bank itself (slot
-1).

## Reproduce the event table

```sh
cd research/exe/fx/emu
python3 evt.py            # all codes 0..0x2C, 0x66..0x6A, 0xC8..0xCE
```

The run uses no game data: the actor, its part matrices and the game manager
(`0x140C90E28`, empty, which selects the no-player branch) are fabricated.
