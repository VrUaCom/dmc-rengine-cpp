# DMC3 HD MOT -> MOD runtime channel path

Date: 2026-09-11

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

This pass composes already recovered canonical-executable facts into the first bounded end-to-end scalar path from serialized MOT track data to a semantically identified MOD/CMotion joint channel.

It does **not** claim a complete animation player, bit-identical SSE evaluation, edited MOT writer authority, or final animated-local matrix construction.

## 1. Normal channel binding

The normal MOT binding path at `0x140310A61` reads the MOT channel-mask table in joint-array order. The recovered traversal is:

| Mask bit | CMotionJoint channel base | Semantic |
|---|---:|---|
| `0x040` | `+0x120` | Translation X |
| `0x080` | `+0x140` | Translation Y |
| `0x100` | `+0x160` | Translation Z |
| `0x008` | `+0x180` | Rotation X |
| `0x010` | `+0x1A0` | Rotation Y |
| `0x020` | `+0x1C0` | Rotation Z |
| `0x001` | `+0x1E0` | Scale X |
| `0x002` | `+0x200` | Scale Y |
| `0x004` | `+0x220` | Scale Z |

Track traversal follows this table order. A channel consumes one serialized track ordinal when its mask bit is set.

The new `analysis/mot/channel_binding.hpp` codifies that projection as `project_normal_binding(...)` and fails closed unless the MOT channel-domain cardinality matches the supplied model-node cardinality. The executable path is bounded by model node count, but arbitrary mismatched domains are not promoted as safe.

Header flag `0x2` uses the alternate path at `0x140310CBF` and is intentionally excluded.

## 2. Compression-3 scalar evaluation

The recovered compression-3 chain is now composed without a second parser:

```text
serialized TrackRecord
  -> signed track+0x06 time offset
  -> cached segment search
  -> decoded key values/slopes
  -> linear or Hermite interpolation
  -> scalar channel value
```

Executable anchors:

- cached segment search: `0x1402E8C80..0x1402E8E10`;
- key decode: `0x1402E9650`;
- evaluator: `0x1402E9170`;
- compression-3 branch: `0x1402E91D8`;
- left-key bit-15 interpolation selection: `0x1402E9261`;
- cubic helper: `0x1402E9880..0x1402E994F`.

The new `analysis/mot/track_evaluation.hpp` exposes `evaluate_compression3_track(...)`.

For global evaluation time `T`:

```text
localTime = T - signed_i16(track.start_time_raw)
selection = cachedSearch(localTime, cachedIndex)
```

A single-key selection returns the decoded key value. A two-key selection uses the already recovered linear/Hermite algebra. The returned cache index is the index selected by the recovered cached-search loop.

The helper rejects non-finite query times, unsupported compression, malformed/signed-ineligible key counts, invalid cache state and unsorted masked key times rather than inventing recovery behavior.

## 3. MOD motion-group bridge

MOD motion-group ownership is already EXE-confirmed through `0x14030F850`: the serialized third node-domain table is copied to `CMotionJoint +0xF8`.

Matching against the requested group occurs at multiple runtime sites, including:

- `0x14030E658..0x14030E662`;
- `0x14030ED98..0x14030ED9F`;
- `0x14030F378..0x14030F382`;
- `0x1403101B8..0x1403101C2`;
- `0x140310348..0x140310353`;
- `0x140310A90..0x140310A9B`;
- `0x1403112B8..0x1403112C1`;
- `0x140311408..0x140311411`.

`analysis/mod/mot_pose.hpp` now composes the model binding, MOT channel projection and compression-3 scalar evaluator.

The key ownership rule is preserved:

```text
track ordinal traversal is determined by the MOT masks
motion-group filtering affects which joint channels are applied/evaluated
```

Thus an excluded joint does not collapse or renumber later track ordinals.

The helper intentionally leaves excluded-track cache entries unchanged. It models the selected-group scalar application boundary, not the complete CMotion scheduler/cache lifecycle.

## 4. What is now closed

At the bounded supported path:

```text
MOT mask bit
  -> exact track ordinal
  -> exact joint/node index
  -> exact T/R/S semantic channel
  -> exact CMotionJoint channel base
  -> compression-3 cached key selection
  -> decoded scalar value
  -> selected MOD motion group
```

Evidence status:

- nine-channel mask traversal: `EXE_CONFIRMED`;
- T/R/S channel semantics: `EXE_CONFIRMED`;
- CMotionJoint channel base offsets: `EXE_CONFIRMED`;
- track ordinal consumption in normal path: `EXE_CONFIRMED`;
- signed track time offset: `EXE_CONFIRMED`;
- compression-3 cached segment search: `EXE_CONFIRMED` static recovery;
- quantized key decode: `EXE_CONFIRMED`;
- linear/Hermite choice and algebra: `EXE_CONFIRMED` semantic recovery;
- MOD motion-group selector/filter relationship: `EXE_CONFIRMED`;
- composed C++ scalar-to-semantic-channel path: direct composition of the above recovered contracts;
- bit-identical SSE parity: `OPEN`;
- original-process differential output: `OPEN`.

## 5. Remaining animation frontier

The next direct-reverse gates are now narrower:

1. compression-2 segment selection/evaluation parity;
2. alternate header-flag `0x2` binding at `0x140310CBF`;
3. exact CMotion channel-state ownership around the recovered channel bases;
4. T/R/S -> animated local matrix construction, including scale and exceptional factor branches around the normal matrix path `0x14030E9B0`;
5. animated local -> MOD `currentWorld` is already recovered once a trustworthy local matrix exists;
6. `inverseRestWorld * currentWorld` skin palette is already recovered;
7. looping, blending, motion selection and scheduler/cache lifecycle;
8. original-game differential validation.

The current truthful end-to-end frontier is therefore:

```text
serialized MOT compression-3
  -> scalar semantic joint channel        CLOSED, bounded
  -> animated local matrix                OPEN
  -> currentWorld                         CLOSED once local matrix is supplied
  -> inverseRest * currentWorld           CLOSED
  -> skin palette                         CLOSED
```

## 6. Regression surface

`tests/mot_key_decode_tests.cpp` now covers:

- cached forward/backward segment search;
- endpoint behavior;
- cache-dependent duplicate-time keys;
- invalid cache/non-finite/signed-count gates;
- normal mask -> track -> T/R/S channel projection;
- compression-3 signed-offset scalar evaluation;
- MOD motion-group filtering without renumbering track ordinals.

CI success is a compile/regression gate only. It is not original-game runtime acceptance.
