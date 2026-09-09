# DMC3 HD MOD — preserve-layout writer gate

**Date:** 2026-09-09  
**Working branch:** `reverse/mod-completion-20260907`  
**Evidence class:** `WRITER_GATE`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

MOD authoring now uses a bounded preservation-first writer over the canonical MOD parser and typed IR. This remains intentionally different from a rebuild-from-scratch serializer: the retained retail byte image is still physical layout authority, and unsupported structural or undecoded edits fail closed.

The no-repeat frontier remains in force. New MOD work must advance writer/container/game evidence rather than reopen already-consolidated unknown-byte questions.

## Authority model

The writer requires both:

1. a caller-owned immutable source byte image; and
2. a parsed `formats::mod::Document` whose retained `source_bytes` match that image exactly.

The source is reparsed before writing. Output is reparsed before success. A caller cannot mutate both `Document::source_bytes` and typed IR and then launder that edited image into the preserve-layout baseline.

## Current writable fixed-layout fields

The bounded writer permits only same-cardinality fields with already-promoted serialized spans:

- object bounding center (`f32 x/y/z`);
- object bounding radius (`f32`);
- mesh positions (`float3`);
- mesh normals (`float3`);
- mesh UV values (`int16 u/v`).

The writer still refuses header/cardinality/offset/reflow changes, transform-domain authoring, skin/blend/control authoring, texture/material/source-flag/alpha authoring, generated workspace changes and preservation-only fields such as mesh `+0x0C`, `+0x38`, `+0x4C` and transform `+0x1C`.

## Byte-preservation enforcement

Output begins as a copy of the immutable source. The writer builds explicit authorized changed spans for requested typed edits and then compares the complete output against the source. Any changed byte outside an authorized span fails with `mod.writer.unauthorized-byte-change`.

Successful `WriteReceipt` records source/output SHA-256, byte count, modified byte count, immutable-source binding, unauthorized-byte preservation, output reparse and no-edit byte identity.

## Regression gates

`tests/mod_writer_tests.cpp` covers:

1. exact no-edit parity;
2. source/output SHA equality on no-edit;
3. controlled bounding/position/UV edits;
4. unchanged bytes outside exact edited spans;
5. unsupported source-flag rejection;
6. preservation-only mesh-field rejection;
7. stream-cardinality rejection;
8. transform-edit rejection;
9. `Document::source_bytes` tamper rejection.

`tests/mod_writer_corpus_tests.cpp` and the CLI corpus runner separately cover deterministic recursive multi-file no-edit validation.

## Gate 1 — provenance-bound 38-file retail no-edit parity

Canonical CLI:

```text
dmc-rengine mod-writer-corpus <directory> [receipt.json]
```

A GitHub-Actions-built Linux runner from source commit
`b9a3e91b1e7bd5ef23ff4f2a09f6c215b3936348`
(SHA-256 `d8f4c2afdd05f8238d4d8f4ae9593aa2aefa230a74b96a2d9ddb7387d004d5ae`)
was run over 38 unique provenance-bound retail MODs:

- 35 `em000`;
- 2 `pl000`;
- 1 `id100`.

Result:

```text
MOD files          : 38
passed             : 38
failed             : 0
total source bytes : 882736
modified bytes     : 0
byte-identical     : 38/38
canonical reopen   : 38/38
result             : PASS
```

Raw receipt SHA-256:

```text
f71ea812a1e2af7fbba1f0c16863618da10d50f9b459566e18b9c565822d9dc8
```

Independent source-file rehashing found zero receipt/hash mismatches. Repository evidence stores hashes and metadata only:

- `data/reverse/dmc3-mod-writer-retail-38-attestation-20260909.json`;
- `data/reverse/dmc3-mod-writer-retail-38-source-hashes-20260909.txt`.

## Gate 2 — controlled real-retail fixed-layout edit

The bounded command:

```text
dmc-rengine mod-writer-set-bounding-radius <input.mod> <object-index> <radius> <output.mod> [receipt.json]
```

was executed against provenance-bound `em000_021.mod` with a Linux runner built from source commit
`8ddd5c2acd455b12ed1f67b9af30571d5e9634be`
(runner SHA-256 `a308a428b0d4e6ac54080a6d58cf9e98f2f6e303b7849c22370c7671cd1d7eb8`).

Result:

```text
source size         : 592
source SHA-256      : 03c18bd75452b0419b398b48d7ef436bb4b2c4c797dae865c6823f8225205f74
field               : object[0].bounding_radius
old value           : 0.6208532452583313
new value           : 0.625
serialized span     : [124,128)
changed offsets     : [124,125,126]
modified bytes      : 3
output SHA-256      : d074416967a163bbfb2707310141357b88b315c4359fbef69d7c63d2ce3c7f86
disk reopen         : PASS
```

Independent raw-byte comparison found exactly the same three changed offsets and every other byte identical. Raw controlled-edit receipt SHA-256:

```text
896def2d5f6fc21fb8a0653542dae8e4dea536f138ec6f567c2c81345f3ef761
```

Machine attestation:

- `data/reverse/dmc3-mod-controlled-retail-edit-attestation-20260909.json`.

This proves one tightly scoped retail mutation, not arbitrary MOD authoring.

## Gate 3 — writer result to generic container architecture

PR #369 adds `ModAuthoredChildBridge`, a fail-closed format-specific trust boundary into the existing generic `AuthoredChildImage` / `NestedRelativeSlotReintegrator` path.

The bridge independently validates source bytes, `ResourceId.size`, canonical source parse, `WriteResult`, receipt validity, source/output hashes, same-size output and canonical output reparse. It preserves the exact child `ResourceId` rather than inferring physical slot identity from naming.

`tests/mod_authored_child_bridge_tests.cpp` proves a controlled same-size MOD edit can travel through this bridge into a synthetic PAC, then reparse/re-expand and recover exact authored child bytes. Tampered writer output and container-marked child laundering are rejected.

This synthetic regression proves architecture and trust-boundary behavior; it is no longer the highest container evidence gate.

## Gate 4 — provenance-bound real retail PNST reintegration

A retained source package:

```text
DMC_Rengine_Item_Editor_Phase4_source.zip
SHA-256: 400954e637342f8036879120d1a3845f3d74077a9844fcfad643421180602d57
```

contains the raw parent and extracted child pair:

```text
analysis_inputs/stage_drops/m20_s00/m20_s00_012.pac
analysis_inputs/stage_drops/m20_s00/m20_s00_012/m20_s00_012_023.mod
```

Canonical `list-container` detected the raw `.pac` bytes as **PNST**, not PAC0:

```text
parent source bytes  : 346272
parent source SHA    : a09898bbf73d944f9f52a1de13a3bce0eebb90726947248bf35085f52df15be8
slots                : 33
fully expanded       : yes
```

The source child SHA
`096f2e8b81dd55a450b15defeb52345626156010c8fb70f3ab5f540d52b447ce`
was located by canonical expansion at physical slot 23, offset 129280, size 1888. The suffix `_023` was not used as authority; exact expanded bytes + SHA bound the physical slot.

A controlled edit changed only:

```text
object[0].bounding_radius: 4.321839332580566 -> 4.5
local changed offsets: [124,125,126]
authored child SHA-256: 94bcb6189435ca283c58be698ba1f4caa68663af46f6dc42c96c89db5c003f33
```

The existing `rebuild-relative-slot` command replaced physical slot 23 and returned `VERIFIED`:

```text
rebuilt parent bytes : 346272
rebuilt parent SHA   : 78c002cc62dd235a7f95f16b3e53fd1588d8f4e8b917400e8f4471eff48d9e8e
slot 23 offset       : 129280
slot 23 size         : 1888
reopened child SHA   : 94bcb6189435ca283c58be698ba1f4caa68663af46f6dc42c96c89db5c003f33
reopened radius      : 4.5
```

An independent full-parent raw byte diff found only:

```text
[129404,129405,129406]
```

which equals:

```text
physical slot offset 129280 + local child changes [124,125,126]
```

Therefore the PNST header/table, parent size, physical slot identity/offset/size and all other slot bytes remained unchanged. The reopened extracted child is byte-for-byte identical to the authored MOD.

Machine attestation:

- `data/reverse/dmc3-mod-retail-pnst-reintegration-attestation-20260909.json`.

Detailed container evidence:

- `docs/research/dmc3-mod-pac-reintegration-gate-2026-09-09.md`;
- `data/reverse/dmc3-mod-pac-reintegration-gate-20260909.json`.

## Current evidence status

Established:

- 38/38 provenance-bound retail no-edit byte parity and canonical reopen;
- source/output preservation enforced independently of mutable typed IR;
- bounded synthetic fixed-layout authoring regressions;
- one provenance-bound real-retail exact-span MOD edit;
- fail-closed MOD writer-result -> generic authored-child trust bridge;
- synthetic PAC reintegration through existing Layer-1 infrastructure;
- **provenance-bound real retail PNST reintegration**, with physical slot identity preserved and full-parent bytes unchanged outside the controlled child edit.

Still not established:

- canonical layout planning/rebuild from typed IR alone;
- arbitrary MOD editing;
- transform, skin, material/source-flag or texture-domain authoring;
- texture-companion writer/coherence authority;
- PAC0-family retail MOD-child reintegration coverage;
- NBZ overlay/reopen acceptance for the edited retail root resource;
- original `dmc3.exe` no-edit or edited acceptance;
- Capcom authoring-tool equivalence;
- a `100% MOD writer` claim.

## Next evidence frontier

The next non-repeating MOD authoring gates are now:

1. feed the verified rebuilt retail root resource into the existing NBZ overlay path and require canonical NBZ reopen/rematerialization;
2. run original `dmc3.exe` acceptance first on an unchanged/no-edit chain and then on the tightly controlled edited chain;
3. optionally add a provenance-bound PAC0-parent MOD-child experiment for container-family coverage if that coverage becomes necessary.

Do not return to hierarchy, skin packing, header `+0x14`, bit21 or zero-field reverse work unless a genuinely new consumer, producer, corpus contradiction or game experiment appears.
