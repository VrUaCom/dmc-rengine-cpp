# DMC3 HD SO working-family identity — em000 continuation (2026-09-08)

**Branch:** `reverse/so-em000-identity-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Predecessor research branch:** `research/so-cpp20`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose of this continuation

Main already contains the modular SO working-family parsers recovered from `research/so-cpp20`:

```text
formats/so/graph
formats/so/link_table
formats/so/volume_table
analysis/so/... correlations
```

The unresolved issue is now **identity**, not the existence of these three structures.

`SO` remains a DMC Rengine working family name. No original `.so` filename extension or runtime lookup extension has yet been proven.

## em000 mapping

Only these three top-level extractor-labeled BIN payloads belong to the current SO working set:

```text
em000_038.bin -> graph/control payload, 6144 bytes
em000_039.bin -> compact link table,      96 bytes
em000_040.bin -> spatial-volume table,  1840 bytes
```

This does **not** imply:

```text
.bin == SO
```

Other `.bin` extraction labels in em000 include confirmed MOT-family payloads and unrelated effect-pack members. BIN is therefore a generic extraction/fallback suffix, not a semantic format.

## Current structural evidence

### `em000_038.bin`

- type-6 indexed block at file offset 0;
- type-6 bounded header `0x0E`;
- 87 strictly increasing u16 entry offsets;
- type-8 block at `0x0B98`;
- type-8 bounded header `0x08`;
- 75 strictly increasing u16 entry offsets;
- variable-length entries;
- auxiliary regions remain semantically unresolved.

### `em000_039.bin`

```text
96 / 4 = 24 records
```

Record 0 is header-like; the remaining 23 records are available for one-to-one correlation with the volume domain. Field semantics remain raw.

### `em000_040.bin`

```text
1840 / 0x50 = 23 records
```

Observed current corpus:

```text
22 x type 2
 1 x type 4
```

The records are spatially renderable as candidate primitive volumes, but gameplay labels such as hitbox/hurtbox/collision remain forbidden until the consumer switch is recovered.

## MOD correlation

The em000 companion MOD exposes exactly 23 matrix/transform selector indices. This matches:

```text
23 post-prefix link records
23 volume records
23 MOD transform selectors
```

This is strong cross-resource transform-domain evidence, but not proof of the original resource-family name.

## Presentation rule

DMC Rengine may present the three resources as:

```text
SO (working identity)
  graph
  link-table
  volume-table
```

only when accompanied by provenance such as:

```text
identity_status = working/project family
original_extension = unresolved
source_extension = .bin
```

The UI must not silently rename the physical source to an asserted historical `.so` filename.

## Reverse target

The next identity pass must search the canonical executable for the consumers/producers of all three structures and answer:

1. what manager owns the graph/control structure;
2. what code consumes the 4-byte link records;
3. what switch consumes the 0x50 volume `type` field;
4. whether those consumers request an extension/path name;
5. whether the three payloads form one original file family or three companion formats;
6. whether any original symbols/strings supply a stronger name than `SO`.

Only direct runtime/path or producer evidence may replace the working name.

## Architectural rule

Do not create a generic `formats/bin` parser. Classification must occur by structure/context:

```text
.bin source label
  -> MOT structure?       route formats/mot
  -> SO working grammar?  route formats/so/* with working-identity evidence
  -> wrapped DDS?         route texture framing
  -> other?               preserve as unknown
```

This keeps extractor naming separate from file-format identity.
