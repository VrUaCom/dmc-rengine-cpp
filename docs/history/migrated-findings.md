# Migrated Reverse-Engineering Findings

This document is a migration ledger for findings established before the clean C++ repository. It records knowledge to be re-encoded as evidence packets, fixtures, tests, and typed APIs. It does not mean every finding is already implemented in C++.

## Canonical executable target

**Status:** confirmed research target

Known target identity:

- platform: Windows PE32+;
- architecture: x86-64;
- subsystem: GUI;
- ImageBase: `0x140000000`;
- EntryPoint: `0x14034615C`;
- exported `dmc3_main`: `0x1402C5DF0`;
- canonical Phase 12 SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Repository policy: the executable itself is never committed. The hash and independently derived facts may be stored as evidence metadata.

## Stage resource table

**Status:** confirmed

Recorded facts:

- table file offset: `0x5C30A8`;
- table VA: `0x1405C4AA8`;
- 440 entries;
- logical shape: 110 rows × 4 columns;
- approximately 7118 strings and 7911 pointers were recorded in the broader analysis context.

Column roles:

1. `scr\stXXX.pac`;
2. `room\stXXXcfg.pac`;
3. `room\stXXX_effect.pac`;
4. `se\snd_rXXX.pac`.

Migration target:

- `exe::StageResourceTable` evidence model;
- hash-gated reader/fixture test;
- EXE-backed `StageIdentity`;
- `StageBundle` assembly without embedding source bytes.

## StageSet token classifier

**Status:** confirmed/partial map

Function candidate treated as confirmed for token classification:

- VA `0x140246680`.

Recorded mappings:

- `DUMMY = 1`;
- `STAY = 2`;
- `BREAK = 3`;
- `ORBREAK = 4`;
- `SEAL = 5`;
- `SWITCH = 6`;
- `YURE = ...` (mapping incomplete in current migration record).

Migration target:

- evidence packet with exact artifact hash;
- typed enum with unknown preservation;
- parser tests using synthetic tokens.

## TXT parser helper group

**Status:** confirmed

Recorded addresses:

- `0x140322CA0` — init;
- `0x140322CB0` — clear;
- `0x140322CC0` — advance;
- `0x140322D40` — peek;
- `0x140322D90` — read token;
- `0x140322E20` — read int;
- `0x140322E70` — read float;
- `0x140322F10` — read string;
- `0x140322FB0` — skip whitespace/comments;
- `0x140323050` — parse `#SET`.

Migration target:

- recovered decompilation-unit metadata;
- independent clean C++ TXT lexer/parser;
- behavior tests against synthetic input;
- explicit links between parser behavior and Stage Ops TXT documents.

## Door and Box parsing

**Status:** high-confidence candidates

Recorded candidates:

- `0x1401AB420` — Door Box type 0;
- `0x1401AB600` — BoxLoad;
- `0x1401AB6D0` — next stage/room writer.

These remain high-confidence until fresh evidence packets reproduce the behavior against the canonical executable hash.

## Format findings

### HITS$

**Status:** confirmed

- magic: `HITS$`;
- record marker: `0x18060001`;
- record size: 56 bytes;
- record model: one `u32` plus thirteen floats.

### LIG2

**Status:** confirmed for known sample family

- ten known `stXXXcfg_001.lig` files in the recorded sample set;
- size: 2336 bytes;
- 48 records;
- data begins at `0x20`;
- record size: `0x30`;
- recorded duplicate: `st111 == st434`.

### DCA

**Status:** confirmed for known sample

- magic: `DCA\0`;
- header words: `[0, 12, 0, 0]`;
- header size: `0x10`;
- record size: `0x410`;
- example historical role: `st001cfg_008.ukn`.

### SCM

**Status:** partial/high

Recorded model:

- magic `SCM`;
- version near 1.01 in observed samples;
- object tables, nodes, buffers, and `triCmd` structures;
- null `triCmd` associated with invisible geometry in observed behavior.

### PAC/PNST

**Status:** confirmed basic identity, incomplete full schema

- PAC magic: `PAC\0`;
- PNST combines text-index and binary structure;
- slot count may exceed the number of non-empty entries;
- slot identity must be preserved independently from display names.

### TXT stage layers

**Status:** confirmed usage patterns

Recorded roles include:

- `_004.txt` — main stage layer in observed families;
- `cfg_000.txt` — doors/configuration;
- `_effect_000.txt` — effects;
- tokens/constructs including `DOOR`, `BoxIn`, `NextRoom`, `ORBREAK`, `SEAL`, `SWITCH`, `LIFT`, `SE`, and `DUMMY`.

## Item Editor/runtime findings

**Status:** mixed implemented/confirmed/high; requires evidence repackaging

Recorded implementation baseline:

- standalone ITM Editor;
- Spider Hub node;
- floating Item Library;
- schema v2 with migration/import/export;
- validation;
- guarded EXE patch planning;
- mod build, backup, manifest, report, and README generation.

Recorded runtime work:

- item slots 50–63 used in guarded experiments;
- confirmed historical target: slot 50 at file offset `0x005C4CE8`;
- expected source bytes: `00 01 08 13`;
- six inventory-limit patch sites were recorded;
- compare pattern included `83 F8 cap`;
- 1–127 and 128–255 ranges required different safety handling;
- 256+ was blocked in the historical implementation.

Recorded user tests:

- IDs 1–3: red orbs;
- IDs 9–11: green orbs;
- IDs 12–14: white orbs;
- ID 17: star behavior observed once per new game;
- ID 19: holy water appeared in Mission 2 inventory after executable edits.

Open hypotheses:

- non-orb items may use event-based spawn paths;
- replay reward ID 9 behavior requires proof;
- blue shard to gold-orb conversion;
- gold-orb capacity changes.

## Texture/resource architecture — Phase 16

**Status:** research phase completed; C++ migration pending

Recorded outputs covered:

- PTX bundle/runtime/cache architecture;
- partial parsed entry models;
- `gfxTexture`;
- DDS header validation;
- `LoadedResourceViewV2`;
- texture sample analysis;
- function maps;
- annotations;
- recovered C/C++ seed units.

Seed units are not automatically trusted source. Each requires ABI, lifetime, and behavior validation in the new repository.

## GDSpaces implementation history

Recorded capabilities in the legacy generation included:

- `GDSpacesAPI` and `GDOpenRouter` concepts;
- `GDResourceGraph`;
- `GDStageBundle`;
- read-only `NBZVolumeSource`;
- NBZ/AFS/PAC/PNST navigation;
- profiles: `dmc1-hd`, `dmc2-hd`, `dmc3-hd`, `dmclauncher-hd`, `unknown`;
- game-folder scanner;
- extension/classification work;
- source/container navigation.

Recorded corrections:

- `.index` is metadata/linkage, not automatically a runtime asset;
- missing physical names require synthetic display identities without inventing physical truth;
- DDS inside a container must be exposed as child resources;
- stage and non-stage PACs share generic container handling;
- EXE-backed identity can be more canonical than a fallback container name.

Known architecture debt in the legacy code included direct `ResourceResolver` use in Stage Ops, ModViz, ItemEditor3DScene, and texture code. The clean C++ generation must not reproduce this debt.

## Migration rule

A historical finding becomes implemented Canon in the C++ repository only when it has:

1. a public evidence record or sanitized evidence metadata;
2. a typed representation;
3. a synthetic or locally generated test;
4. an explicit confidence state;
5. architecture-compliant ownership and source flow;
6. status documentation.
