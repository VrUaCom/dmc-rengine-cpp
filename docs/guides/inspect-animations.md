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

Canonical-EXE-backed evaluation additionally recovers:

- nine-channel binding-bit traversal order;
- exact Translation/Rotation/Scale channel semantics and CMotionJoint channel-base offsets;
- signed 16-bit track start-time offsets;
- quantization `raw * range / 65535 + min`;
- compression-3 cached forward/backward segment search;
- endpoint and cache-dependent duplicate-time behavior;
- compression-3 linear vs cubic/Hermite segment algebra;
- incoming/outgoing slope orientation;
- the MOD motion-group selector relationship used to decide which joint channels are applied.

The current analysis path can therefore compose, for the supported normal compression-3 route:

```text
MOT mask
  -> serialized track ordinal
  -> MOD/CMotion joint
  -> semantic T/R/S channel
  -> cached key selection
  -> decoded/interpolated scalar
  -> selected motion group
```

This is a bounded semantic recovery, **not** a bit-identical SSE reimplementation and not a complete animation-player claim.

See `docs/research/dmc3-mot-runtime-channel-path-2026-09-11.md` for the exact current boundary.

## Remaining animation boundary

Still open:

- compression-2 segment selection/evaluation parity;
- flag `0x2` alternate binding path;
- exact mutable CMotion channel-state ownership where required;
- T/R/S channel state -> animated local matrix construction, including scale and exceptional factor branches;
- other compression modes with executable + real-corpus agreement;
- looping, blending, motion selection and scheduler/cache lifecycle;
- bit-identical SSE parity where required;
- original-game output comparison;
- edited MOT authoring/original-game acceptance.

The useful analysis chain is now:

```text
MOD hierarchy / node domain
  -> motion-group relationship
  -> MOT parser/IR
  -> normal channel binding
  -> bounded compression-3 scalar evaluation
  -> animated local matrix             # next major reverse gate
  -> world pose / skin palette analysis
```

Once a trustworthy animated-local matrix is available, the later MOD chain is already recovered:

```text
animatedLocal
  -> currentWorld
  -> inverseRestWorld * currentWorld
  -> skin palette
```

## Product-surface distinction

DMC Rengine C++ registry membership and the DMC Native Reader Android application are separate product surfaces. Do not infer that every C++ module is exposed by a particular Android build without checking that application's current baseline.

For exact current authority, use the dated research receipts and `docs/status/current.md` rather than older roadmap wording.
