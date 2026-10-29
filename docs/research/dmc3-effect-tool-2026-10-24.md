# The effect tool behind FXBANK (v121)

> Ported 2026-10-29 from the DMC Native Reader (`NR-Luna-v73`, `docs/research/dmc3-effect-tool-v121.md`). Paths below that start with `app/` or `tools/research/` are the Reader's. In Rengine: the FXBANK manifest kinds are in `profiles/dmc3/effect_pack_contract.hpp`; the effect bank readers are under `profiles/dmc3/fx/`.

Question: did DMC3's effects come from a dedicated tool, and what does that
mean for reproducing all of them? Read from the HD Collection's `dmc3.exe`
(6 356 432 bytes, the same file as dmc3-hd-shaders-exe.md) and the corpus
PACs. Only addresses, names and sizes are recorded here.

## The tool's database

The executable still holds the loader for the tool's exported files. One
function, `0x1402EF1E0(manager, category)`, picks a path by category with a
jump table, reads the file through `0x140338250`, and hands it to the parser
`0x1402EEE60`. The manager object is `0x140CAF400`.

| Category | Path | FXBANK kind |
| --- | --- | --- |
| 0 | `/eff/texture/TextureDataNameTbl.bin` | T |
| 1 | `/eff/movie/MovieDataNameTbl.bin` | (none in the banks) |
| 2 | `/eff/curve/CurveDataNameTbl.bin` | C |
| 3 | `/eff/texanime/TexAnimeDataNameTbl.bin` | A |
| 4 | `/eff/effect/EffectDataNameTbl.bin` | E |
| 5 | `/eff/particle/PtclParamFiles.bin` | P |
| 6 | `/eff/generator/GeneratorDataNameTbl.bin` | G |
| 7 | `/eff/value/ValueDataNameTbl.bin` | V |
| 8 | `/eff/model/EfcMdlDataNameTbl.bin` | M |
| 9 | `/demo/DemoDataNameTbl.bin` | the cutscenes |
| 10 | none: the loader returns true at once | |

The kind letters of the bank manifests (effect_bank.h: A, C, E, G, M, P, T,
V) match the categories one for one; only the movie category has no letter.

The tool's categories, its exported files and the release banks share the
same id space. A bank is the release packing of that database: the manifest
lists `<kind> <id>`, and the records are the tool's items with their names
left out.

## Name table layout

From the lookup `0x1402EF3C0(manager, category, id)`, which returns an index
or -1:
- `+0x0000`: u16 count.
- `+0x0002`: 2000 names of 32 bytes each (0xFA00 bytes).
- `+0xFA02`: 2000 u16 ids.

So the tool gave every item (an effect, a particle, a curve and so on) a name
of up to 31 characters and a numeric id. The release banks keep only the ids.

## Two ways to load

- **Loose files (development).** At `0x1402D5174` the manager loads the
  name tables of categories 0..8 when a mode word equals 1, and stops at the
  first that fails. These are the tool's loose exports, read by name.
- **Banks (release).** The FXBANK records in `eff\static_effect.pac`, the
  stage `room\stNNN_effect.pac` files and the character archives.
  Registrars by kind: effect_bank.h.
- **Cutscenes.** At `0x1402D622E` the demo name table (category 9) maps a
  cutscene id to its name, for `/demo/%s` and `/demo/%s/%s`.
- **Cutscene clips.** The kinds the clips name are `Effect`, `EffectI`,
  `ScrEfc`, `MODELCLIP` and `ClipScale`. The classes behind them are
  `CEffectClip`, `CScrEfcClip` and `CLightClip`: the cutscene tool placed
  effects, screen effects and lights as clips on its timeline.

## What the executable does not have

None of its 400 RTTI classes is an editor or tool class. The effect classes
are the runtime:

| Area | Classes |
| --- | --- |
| Effects | `CEffect`, `CEffectBase`, `CEfc`, `CEfcPub`, `CEfcShadow`, `CEfcCartridge` |
| Particles | `CParticle`, `CRealParticle`, `CFakeParticle` |
| Particle shapes | `CPtclSprt00`, `CPtclLine00..02`, `CPtclPoly00..01`, and the `CFPtcl*` twins |
| Other | `CGenerator`, `CScrEfcClip`, `CEffectClip`, `CLightClip` |

The tool ran on Capcom's side; the game only reads what it exported.

## Corpus counts (manifest kinds)

The stage and cutscene PACs at hand hold:

| Kind | Records |
| --- | --- |
| A | 35 |
| E | 88 |
| G | 30 |
| M | 11 |
| P | 45 |
| T | 45 |
| V | 37 |
| C (curves) | 0 |

The curves are therefore in banks not yet seen, such as
`eff\static_effect.pac`.

## For the Reader

- Reproducing every effect needs the data the tool exported, which the banks
  already carry, plus the triggers that start them. The tool itself is not
  needed.
- The open work is in dmc3-effect-roadmap-v74.md and
  dmc3-effect-triggers-recon-v77.md:
  - P classes 0, 2 and 5;
  - the enemy, weapon and Dante triggers;
  - stage `beff` / `bmodel` and `SET LIGHT`;
  - C records, once a bank with them is found;
  - the cutscenes' ScrEfc clips, which the DMC3 shaders now cover
    (SHADERS.md).
- The name tables, if a copy of the game has them as loose files, would give
  every effect id its original name.

Who starts which effect in the code: dmc3-effect-spawn-sites-v122.md.
