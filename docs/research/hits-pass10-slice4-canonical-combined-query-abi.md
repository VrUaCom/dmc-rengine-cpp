# HITS Pass 10 — Slice 4 — Canonical Combined-Query ABI Reacquisition

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED / REVIEWED — CI VALIDATION**

## Purpose

Slice 4 closes the stale Pass-10 assumption that `0x14005E7A0` contains an unresolved metric-based static-vs-dynamic winner selector. Canonical instruction bytes were reacquired from the preserved project corpus, the complete wrapper ABI was reconstructed, and all downstream passes were followed before promotion.

## Canonical-byte provenance

The project Library contains `dmc3_phase17_reng_probe.exe` and the Phase-17 architecture receipt. Phase 17 records that the probe was cloned from canonical `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, with original raw section bytes preserved byte-identically over file range `0x400..0x60E600`; only the new `.reng` extension area and required PE metadata were changed.

All Slice-4 function bodies map inside that preserved original raw range. A second independent project derivative, `dmc3_phase18_red_orb_x2_hook.exe`, is also byte-identical over all promoted Slice-4 body windows. The second derivative is a consistency check; the Phase-17 verifier is the provenance authority.

No proprietary instruction bytes are committed. GitHub preserves VA ranges, body hashes, ABI and reconstructed behavior only.

## Canonical body receipts

| Role | VA range | Size | SHA-256 of body |
|---|---:|---:|---|
| combined query wrapper | `0x14005E7A0..0x14005E880` | `224` | `3716472a87c7edd9ea27b800e165de7fee8254c8b928c3a41e431b0f350b8a6f` |
| static HITS pass | `0x14005E880..0x14005EB95` | `789` | `b3fdeac674795752492e1eca9e7a9d21837552aa6cf90f0a02a14e3546136e8a` |
| dynamic category query | `0x14005BCF0..0x14005C0D6` | `998` | `e7e1c1a56425e0a3c5ef5a5a4fff9105d5e073effeae6fcb268829ec3b451d02` |

Phase-12 function-map metadata independently preserves `0x14005E7A0` as a 224-byte function with 51 direct callers.

## `0x14005E7A0` six-argument ABI

Windows x64 argument mapping reconstructed from the complete body and the 51 caller windows:

1. `RCX` — HITS/runtime wrapper/context;
2. `RDX` — reference/start 16-byte point;
3. `R8` — requested/working 16-byte point;
4. `R9` — corrected/output 16-byte point;
5. stack arg 5 — optional `0x38` common hit-metadata output;
6. stack arg 6 — raw reject mask.

`R8` and `R9` frequently alias in callers, proving in-place use. Other callers use distinct working/output buffers. Return value is boolean success in `AL`.

## Ordered query topology

`0x14005E7A0` performs exactly three passes:

1. static HITS helper `0x14005E880`;
2. dynamic helper `0x14005BCF0`, category `0x0E`;
3. dynamic helper `0x14005BCF0`, category `0x11`.

The same raw reject mask is forwarded to all paths.

After the static pass, the wrapper constructs one baseline: static corrected output if static succeeded, otherwise caller working point `arg3`. Both dynamic calls receive that same baseline pointer.

### Review correction — no cross-category progressive working point

The initial Slice-4 descriptor incorrectly described the category `0x0E` and `0x11` calls as progressively chaining a caller-side working point. Second review rejected this: `0x14005BCF0` copies its `R8` baseline into an internal local working point and does not write back through `R8`.

Therefore category `0x0E` cannot alter the baseline seen by category `0x11`.

The actual wrapper-level output precedence is:

`static HITS < dynamic category 0x0E < dynamic category 0x11`

where a later successful pass overwrites the caller-visible output written by an earlier successful pass. This is an ordered **last-successful-writer** policy, not a metric comparison or equality/tie-break in `E7A0`.

If all three passes fail, `E7A0` copies `arg3` to `arg4` and returns false.

## Static pass `0x14005E880`

Relevant combined-query behavior:

- derives segment/direction from the reference and working points;
- resolves broadphase cells and raw HITS records through the established HITS runtime helpers;
- applies the forwarded reject mask to raw HITS flag bytes;
- maintains an internal working point while accepted static contacts are processed;
- writes accepted/corrected 16-byte output through combined-query `arg4`;
- remembers the corresponding accepted raw HITS record;
- if `arg5` is non-null, copies exactly `0x38` bytes from that raw HITS record into the common metadata buffer;
- returns success in `AL`.

## Dynamic category query `0x14005BCF0`

Observed ABI:

1. `RCX` — category pointer table / dynamic manager view;
2. `RDX` — reference point;
3. `R8` — baseline 16-byte working-point input;
4. `R9` — corrected/output point;
5. stack arg 5 — optional common hit-metadata output;
6. stack arg 6 — category index;
7. stack arg 7 — raw reject mask;
8. stack arg 8 — mode byte.

For the `E7A0` path, category is `0x0E` or `0x11`, and mode is zero.

Direct object/list accesses prove:

- `RCX[category]` selects the category list head directly;
- next link: `+0x328`;
- primitive/object type: `+0x120`;
- observed handled type values: `2..6`;
- 16-bit raw filter flags: `+0xDA/+0xDB`;
- identity/key source: `+0xD8`.

Gameplay names for primitive type values `2..6` remain **UNRESOLVED**.

Within one `BCF0` invocation, accepted contacts update its internal working point before later records in that same linked list are evaluated. This within-category progression must not be confused with cross-category chaining at `E7A0` level.

## Common `0x38` hit-metadata bridge

The fifth `E7A0` argument is promoted from unknown auxiliary pointer to a common 56-byte hit-metadata buffer compatible with raw static HITS records.

Static path:
- copies the complete accepted raw HITS record, `0x38` bytes.

Dynamic path:
- writes a compatible partial record;
- copies dynamic identity/key from object `+0xD8` to metadata `+0x00`;
- writes three floats at metadata `+0x28/+0x2C/+0x30`.

Caller `0x1402C65B2` immediately reads those three floats after successful `E7A0` and uses them in subsequent geometry math, independently confirming that this vector is caller-visible ABI. Its exact semantic name remains evidence-neutral until all static and dynamic producers are closed.

## Caller census

All 51 direct calls were re-enumerated from canonical bytes. Reject-mask distribution remains:

- immediate `0x0000`: 5;
- immediate `0x0002`: 2;
- immediate `0x0004`: 1;
- immediate `0x0008`: 14;
- immediate `0x0010`: 13;
- immediate `0x0040`: 2;
- runtime-derived: 14.

Total: 51.

## Architecture correction

The old P0 description of `E7A0` as an unresolved result comparator is rejected.

Canonical replacement:

- `E7A0` — combined point-query orchestration and ordered output precedence;
- `E880` — static HITS contact pass;
- `BCF0` — per-category dynamic linked-list contact pass;
- geometry-specific selection/correction behavior lives inside downstream helpers, not in an `E7A0` metric/tie-break layer.

## Implementation receipt

DMC3-profile evidence now contains:

- `CanonicalFunctionBodyEvidence` for `E7A0`, `E880`, `BCF0`;
- `SpecializedAbiKind::combined_point_query`;
- `CombinedQueryAbiEvidence`;
- ordered `CombinedQueryPassEvidence`;
- `CombinedHitMetadataEvidence`;
- `DynamicCategoryQueryEvidence`;
- exact-SHA negative tests for packed build `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`.

Commits:

- `e191acf631cc96dedc5a4910f1ded05cc436744e` — initial combined-query promotion;
- `bb04f4f19c1e1c63be4ff19dbd199ed2b878d4d2` — initial regression coverage;
- `123ce6331a34d994e3f0121804568efc5ce5c5a4` — correction: both dynamic passes share the same static/input baseline;
- `1f0360661624acf41dfdfe776fe8229c5f2edb0b` — corrected regression coverage.

The superseded progressive-cross-category model must not be restored.

## Updated P0 boundary

Closed for `0x14005E7A0`:

- complete wrapper body and body hash;
- six-argument ABI;
- 51-caller census;
- static/dynamic call ordering;
- dynamic categories `0x0E/0x11`;
- total-miss behavior;
- caller-visible output precedence;
- common `0x38` metadata bridge.

Still open below/adjacent to the wrapper:

- exact semantic names of dynamic primitive types `2..6`;
- exact semantic interpretation of metadata vector `+0x28..+0x30` across all producers;
- deeper geometry helpers where needed for source-equivalent reconstruction;
- `0x14005FEC0` exact source-1 output ABI;
- `0x1400601E0` exact in/out/fourth-component/accumulation semantics;
- dynamic-world update path must remain separate from query-path `BCF0`.
