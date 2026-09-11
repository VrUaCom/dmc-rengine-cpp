# DMC3 MOT — cached compression-3 segment search

Date: 2026-09-11  
Branch: `reverse/mod-completion-20260907`

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- cached segment search: `0x1402E8C80..0x1402E8E10`
- forward segment return: `0x1402E8D52`
- backward segment return: `0x1402E8DF2`
- compression-3 evaluator: `0x1402E9170`, compression-3 path `0x1402E91D8`

This pass promotes only the bounded cached key-selection behavior needed before interpolation. It does not claim complete MOT playback, looping, blending, motion selection, or bit-identical SSE evaluation.

## Recovered input contract

The search operates on the compression-3 key array and a signed cached key index.

For each key, comparison time is:

```text
key_time = key.time_control & 0x7FFF
```

The evaluator subtracts signed `track +0x06` from the external evaluation time before entering the local key-time domain. `select_cached_segment3()` therefore accepts this already-local query time.

The runtime count is sign-extended from `track +0x02`. Parser-level `u16` acceptance therefore does not prove runtime eligibility above `INT16_MAX`; the reconstructed helper rejects those counts.

## Exact bounded walk

For finite local query time, non-empty sorted masked key times, and an initially valid cached index:

```text
i = cached_index

if t >= time[i]:
    while i < count - 1:
        if time[i + 1] > t:
            return segment(i, i + 1), cache = i
        if time[i] == t:
            return single(i), cache = i
        i += 1

    return single(i), cache = count - 1

else:
    while i >= 1:
        previous = i - 1

        if t > time[previous]:
            return segment(previous, i), cache = previous
        if t == time[previous]:
            return single(previous), cache = previous

        i = previous

    return single(0), cache = 0
```

This is intentionally not rewritten as a stateless binary search.

## Duplicate-time consequence

Equal-time runs are cache-dependent in the recovered routine.

For masked key times:

```text
0, 10, 10, 20
```

and query `t = 10`:

- entering from cache `0` reaches the first `10` through the forward walk and can return that key as a single-key result;
- entering from cache `2` sees the second `10` as the current key and the following `20` as greater, so it returns segment `(2, 3)`.

Normalizing duplicate times or replacing the walk with `lower_bound`/`upper_bound` would therefore change observable state selection.

## Canonical implementation

Added to:

```text
include/dmc_rengine/analysis/mot/key_decode.hpp
```

API:

```cpp
std::optional<CachedSegmentSelection> select_cached_segment3(
    const formats::mot::TrackRecord& track,
    float local_time,
    std::int32_t cached_index) noexcept;
```

The implementation uses the existing canonical parsed MOT IR. It does not parse bytes again and does not add a second MOT representation.

It fails closed for:

- non-compression-3 tracks;
- wrong quantization envelope;
- empty tracks;
- runtime-ineligible signed key counts;
- truncated key arrays;
- non-finite query time;
- invalid incoming cache;
- non-monotonic masked key times.

## Regression

`tests/mot_key_decode_tests.cpp` now covers:

1. forward segment selection;
2. backward segment selection;
3. before-first single-key result;
4. after-last single-key result;
5. cache-dependent duplicate-time behavior;
6. invalid/non-finite cache/query rejection;
7. signed-count runtime gate;
8. unsorted masked-key rejection.

## Evidence status

- key time mask `time_control & 0x7FFF`: `EXE_CONFIRMED`;
- cached index ownership at the recovered compression-3 channel path: `EXE_CONFIRMED`;
- forward/backward search direction and return boundaries: `EXE_CONFIRMED`;
- cache update index on the bounded returns: `EXE_CONFIRMED`;
- duplicate-time cache-dependent behavior implied by the recovered branches: `EXE_CONFIRMED` static reconstruction;
- full live-process/retail playback parity for all cache histories: **not yet promoted**;
- whole-animation playback: **not claimed**.

## Next direct reverse gate

The next high-value boundary is no longer the segment search itself. It is the complete normal MOT-to-CMotion channel application path:

```text
channel mask
  -> serialized track ordinal
  -> evaluated scalar
  -> CMotionJoint translation/rotation/scale channel
  -> current animated local matrix
  -> MOD currentWorld
  -> inverseRest * currentWorld
```

The normal channel-bit mapping and the downstream scale matrix path already have bounded EXE evidence. Remaining work is to close scalar evaluation/application and full local-transform composition without guessing the alternate header-flag `0x2` path.
