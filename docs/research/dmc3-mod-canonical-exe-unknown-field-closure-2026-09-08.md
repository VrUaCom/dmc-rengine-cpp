# DMC3 HD MOD — canonical EXE unknown-field closure pass (2026-09-08)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

> Historical pass, normalized after the 2026-09-09 closure work. The observations below remain useful, but narrower labels from the original pass are superseded by the project-wide canonical evidence vocabulary. Current aggregate authority: `docs/research/dmc3-mod-public-status-2026-09-09.md` and `data/reverse/dmc3-mod-canonical-exe-unknown-byte-closure-20260909.json`.

## Purpose

This pass attacked the remaining MOD unknown-byte questions directly against the canonical executable. External Blender/importer behavior is secondary corroboration only and is never format authority.

Targets:

- transform `+0x1C`;
- mesh `+0x0C`;
- mesh `+0x38`;
- mesh `+0x4C`;
- `BLENDINDICES.x`.

Canonical status vocabulary used by the current project:

```text
EXE_CONFIRMED
CORPUS_CONFIRMED
EXE_AND_CORPUS_CONFIRMED
STRUCTURAL_CONFIRMED
SEMANTIC_CANDIDATE
PRESERVED_UNDECODED
RESERVED_OBSERVED_ZERO
REJECTED
```

Zero in the corpus is not padding proof. Bounded no-read evidence is not global non-use proof. A same-offset field in another model family does not inherit that family's semantic automatically.

## 1. Transform `+0x1C`

Canonical MOD/EFM transform initializer: `0x1402FA080`.

For each serialized `0x20`-byte transform record the initializer loads:

```text
+0x00..+0x0F -> translation block
+0x10..+0x1F -> rotation block
```

Rotation helper `0x140330450` reads only X/Y/Z from the second block and does not consume its fourth float, serialized transform `+0x1C`. Later CMotion transfer evidence also copies the proven transform components while skipping serialized `+0x1C`.

Current bounded corpus:

```text
MOD  +0x1C == 0.0f  285 / 285 transforms
EFM  +0x1C == 0.0f    5 / 5 transforms
```

Current safe status:

```text
serialized position           STRUCTURAL_CONFIRMED
canonical XYZ transform use   EXE_CONFIRMED
multi-corpus value            CORPUS_CONFIRMED
high-level semantic           PRESERVED_UNDECODED
writer rule                   preserve source float exactly
```

The important correction is negative: `+0x1C` must not be called padding/reserved merely because the canonical XYZ construction path ignores it and current corpus values are zero.

## 2. Mesh `+0x0C`

Material helper `0x1402F9890` consumes the typed material/CLAMP prefix:

```text
+0x02 texture_slot
+0x04 MINU
+0x06 MAXU
+0x08 MINV
+0x0A MAXV
```

and does not read `+0x0C`. The primary MOD/EFM/SCM post-load handlers audited by this pass also do not establish a consumer for that dword.

Physical layout remains:

```text
0x00..0x0B  typed prefix
0x0C..0x0F  raw u32 lane
0x10..      aligned pointer/stream region
```

Current MOD corpus:

```text
mesh +0x0C == 0   180 / 180
```

Current safe status:

```text
physical width/location       STRUCTURAL_CONFIRMED
bounded corpus value          CORPUS_CONFIRMED
semantic                      PRESERVED_UNDECODED
writer rule                   preserve exact source u32
```

The original “reserved/alignment candidate” wording is superseded. Alignment proximity is an observation, not a semantic promotion.

## 3. Mesh `+0x38`

`+0x38` demonstrates why same-offset cross-family reasoning must be evidence-driven.

### MOD

Canonical MOD post-load does not relocate the slot and disables the corresponding runtime auxiliary stream in the audited path.

### EFM

EFM relocates homologous `+0x38` and the bound EFM shader/runtime path identifies that stream as `COLOR0`.

### SCM

SCM also relocates its homologous `+0x38`, where the established SCM adapter uses the family-specific colour/topology stream.

Current MOD corpus:

```text
mesh +0x38 == 0   180 / 180
```

Current safe status for MOD:

```text
shared physical slot          STRUCTURAL_CONFIRMED
MOD-specific inactivity       EXE_CONFIRMED
bounded MOD zero population   CORPUS_CONFIRMED
combined MOD role boundary    EXE_AND_CORPUS_CONFIRMED
writer rule                   preserve exact source slot
```

Do **not** rename MOD `+0x38` to `COLOR0`: EFM owns that semantic in its own adapter.

## 4. Mesh `+0x4C`

Canonical MOD post-load generates topology count at neighboring `+0x48`, while the audited load/build path does not establish a transfer or interpretation for `+0x4C`. The `+0x48` behavior is the positive control showing that the provenance-confirmed source record itself is being followed.

Current corpus:

```text
MOD  mesh +0x4C == 0   180 / 180
EFM  mesh +0x4C == 0     2 / 2
```

Current safe status:

```text
physical width/location       STRUCTURAL_CONFIRMED
bounded corpus value          CORPUS_CONFIRMED
semantic                      PRESERVED_UNDECODED
writer rule                   preserve exact source u32
```

The earlier “trailing reserved/preservation candidate” label is superseded. The field remains a preservation field until a stronger semantic consumer or ABI rule is proven.

## 5. `BLENDINDICES.x`

The 2026-09-08 pass established that embedded MOD HLSL source uses `matIndex.y/z/w` and found no `matIndex.x` reference. The later 2026-09-09 pass closed the important remaining escape route:

- all eight compiled DXBC input signatures carrying `BLENDINDICES` have `Mask = 0xF` and `ReadWriteMask = 0xE`;
- therefore compiled shaders read Y/Z/W but not X;
- provenance-confirmed CPU consumers likewise do not establish lane-X use;
- canonical serialized/runtime pointer propagation for the stream is proven.

Current corpus:

```text
BLENDINDICES.x == 0   20,976 / 20,976 vertices
```

Current safe status:

```text
serialized/runtime stream     STRUCTURAL_CONFIRMED + EXE_CONFIRMED
canonical lane-X non-use      EXE_CONFIRMED
bounded zero population       CORPUS_CONFIRMED
serialized semantic           PRESERVED_UNDECODED
writer rule                   preserve raw X lane exactly
```

Canonical runtime non-use does **not** authorize calling the serialized byte padding or synthesizing zero in a writer. ABI/source preservation is a separate contract.

## 6. Updated unresolved-field table

| Field | Current status | Evidence-safe interpretation |
|---|---|---|
| transform `+0x1C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | outside proven XYZ transform semantic; preserve source float |
| mesh `+0x0C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | raw source-preserved u32 before pointer region |
| mesh `+0x38` | `EXE_AND_CORPUS_CONFIRMED` for MOD-specific runtime role | family-specific auxiliary slot; inactive in canonical MOD path; not EFM `COLOR0` by inheritance |
| mesh `+0x4C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | trailing raw source-preserved u32 |
| `BLENDINDICES.x` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | canonical runtime does not read X; serialized byte still preserved |

## 7. What changed after this 2026-09-08 pass

Several items that were genuinely open at capture time were resolved or narrowed on 2026-09-09:

1. `BLENDINDICES.x` canonical runtime non-use was closed with compiled DXBC/HLSL/CPU evidence.
2. Header `+0x14` universal decimal identity was `REJECTED` by broader `pl000`/`id100` evidence; the safe contract is raw `runtime_metadata_u32` / `PRESERVED_UNDECODED`.
3. Source flag `0x00100000` received a closed technical renderer semantic: GS `TEST_1.AREF 0↔16` plus `ZBUF_1.ZMSK 1↔0` in the proven path.
4. Source flag `0x00200000` was separated from unrelated bit-21 domains; runtime carry/restoration is confirmed while its distinct terminal semantic remains open.
5. Corpus coverage expanded to 38 unique MOD / 166 objects / 180 meshes / 20,976 vertices / 285 transforms.

The still-open direct evidence frontier is therefore narrower: provenance-complete whole-EXE closure for mesh `+0x0C/+0x4C`, retained-pointer consumers for secondary serialized regions, terminal semantics for source bit `0x00200000` if one exists, and writer/original-game acceptance gates.

External importers remain corroboration only. Canonical executable and retail corpus evidence remain authoritative.
