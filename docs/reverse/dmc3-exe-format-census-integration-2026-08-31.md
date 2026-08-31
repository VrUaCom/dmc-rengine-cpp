# DMC3 EXE format census integration — 2026-08-31

**Status:** integration receipt  
**Canonical EXE:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Purpose:** record where the new format findings were propagated so the project does not keep divergent Android/C++ knowledge.

## Canonical findings propagated

- three-byte registry probe `0x1402DB1F0`: `MOD / EFM / SCM / MRP / SHW`;
- container dispatcher `0x1401B9FA0`: normal handlers for `MOD / EFM / SCM / SHW`, `EFE / EFW` sentinels, exact `PNST` recursion;
- four-byte family mask `0x1402FD650`: `MOD / EFM / SCM / MRP / MCV / SHW` with trailing ASCII space;
- motion/control extension dispatcher `0x1402E01A0`: `MOT / MCV / CAM / HID / CLT / TSC`;
- direct `VAGp` content check `0x140032970`;
- direct `DDS ` checks `0x140049A8E`, `0x14004AD9D`;
- direct exact `TM2\0` check `0x1403365BA`;
- `LIG2` constructor/type-tag write `0x14023ECC9` into object `+0x08`;
- media capability extension checks for `PSS / THP / PAM / XMV / WMV / PMF / AVI / MPG / BIK / MP4`;
- runtime/path references for NBZ, AFS namespaces, PAC, ADX, OGG, SFD, TM2, PTZ, DDS, fonts/icons/saves, PHD/TSB/BD, BIN/TXT and SPUMAPDT.

## Corrected claims

- `HITS` remains a real four-byte `DATA_CONFIRMED` collision payload identity from corpus/parser evidence. It is **not** promoted to an EXE runtime tag by this bounded census.
- `HITS$` remains `REJECTED`.
- `TIM2` is not canonical direct EXE magic on the newly bounded content path; exact direct bytes are `TM2\0`.
- `LIG2` is stronger than a random ASCII occurrence because the executable constructs the tag, but that still does not close the full disk schema.
- runtime/media capability recognition must remain distinct from DMC resource ownership.

## Updated canonical surfaces

### Machine-readable reverse authority

- `data/reverse/dmc3-exe-format-census-20260831.json`
- `data/reverse/dmc3-runtime-type-identification-20260831.json`
- `data/reverse/dmc3-primary-3d-family-20260831.json`

### Canonical format knowledge

- `docs/formats/dmc3-hd-format-purpose-registry.json`
- `docs/formats/dmc3-hd-format-catalog.md`
- `docs/formats/README.md`

### Research narratives

- `docs/research/dmc3-runtime-type-evidence-split-2026-08-31.md`
- `docs/research/dmc3-primary-3d-render-family-reverse-2026-08-31.md`

### Product code

- `src/integration/format_registry.cpp`
  - adds EXE-backed unresolved families as `recognized/read-only`;
  - does not invent parsers or writers;
  - updates HITS/DDS/TM2/LIG2 evidence boundaries.
- `src/gdspaces/classifier.cpp`
  - adds direct content recognition only where the canonical EXE has a direct first-DWORD content check: `TM2\0`, `VAGp`, existing `DDS `;
  - does not collapse three-byte registry recognition into generic file magic.

### Tests

- `tests/runtime_synth_format_registry_tests.cpp`
  - locks presence/read-only status of new EXE-backed format families;
  - checks HITS correction;
  - checks DDS/TM2/VAGp/LIG2 evidence strings;
  - checks GDSpaces direct content classification for `VAGp`, exact `TM2\0`, DDS;
  - rejects `TIM2` bytes as canonical direct magic in the generic classifier.

## Fail-closed boundary

This integration intentionally does **not** create guessed parsers for `MRP`, `MCV`, `EFE`, `EFW`, `C1D`, `CLT`, `HID`, `TSC`, `VAGp` or `TM2` beyond evidence already present.

Recognition may advance ahead of decoding, but schema maturity and write authority must advance only with independent structural/runtime evidence.
