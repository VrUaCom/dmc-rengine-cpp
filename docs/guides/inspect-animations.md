# How to Inspect DMC3 Animations and MOT Research

Animation work in DMC Rengine crosses model hierarchy, evaluated transforms and the MOT resource family. The safe workflow is to keep model hierarchy, serialized MOT data, evaluated pose and runtime player behavior as separate evidence domains.

## Model-side context

The canonical MOD reader exposes hierarchy, local/world transform relationships, motion-group values and skin-palette relationships strongly enough for pose-aware inspection. This establishes the model-side node domain that animation data targets.

## MOT current canonical path

MOT is already registered in the canonical DMC Rengine `NativeReaderModuleRegistry` through `native_reader_modules::mot()`.

PR #372 consolidates the structural implementation onto one modular parser/IR and removes the old duplicate byte-decoding path. Current canonical evidence includes:

- `MOT\0` marker at `+0x04`;
- aligned header/channel-mask extent;
- nine-bit channel mask;
- record count equal to the channel-mask popcount sum;
- bounded track extents;
- typed compression-2 and compression-3 key payloads;
- monotonic masked key times;
- preservation of unknown-compression track bytes;
- zero-only post-track padding validation;
- three hash-bound real MOT payloads parsing through the modular path.

Canonical-EXE-backed key evaluation additionally recovers:

- nine-channel binding-bit traversal order;
- signed 16-bit track start-time offsets;
- quantization `raw * range / 65535 + min`;
- compression-3 linear vs cubic/Hermite segment algebra;
- incoming/outgoing slope orientation.

The interpolation helper is an algebraic semantic recovery, **not** a bit-identical SSE reimplementation and not a complete animation-player claim.

## Remaining animation boundary

Still open:

- exact segment lookup/cache and duplicate-time behavior;
- flag `0x2` alternate binding path;
- other compression modes with executable + real-corpus agreement;
- looping, blending, motion selection and full transform composition;
- original-game output comparison;
- edited MOT authoring/original-game acceptance.

A useful analysis chain is:

```text
MOD hierarchy / node domain
  -> motion-group relationship
  -> MOT parser/IR
  -> bounded key evaluation
  -> evaluated local transforms
  -> world pose / skin palette analysis
```

## Product-surface distinction

DMC Rengine C++ registry membership and the DMC Native Reader Android application are separate product surfaces. Do not infer that every C++ module is exposed by a particular Android build without checking that application's current baseline.

For exact current authority, use `docs/research/dmc3-model-formats-unified-frontier-2026-09-09.md` and `docs/status/current.md`.
