# DMC3 PC Save — Wide Pass 32 Product Promotion

## Status

Implementation proposal and reviewed product boundary for migrating the confirmed Wide Pass 32 record-envelope/checksum ABI into `VrUaCom/dmc-rengine-cpp`.

This document is not a claim that save semantics are complete. It promotes only the structural and integrity facts supported by the canonical Drive corpus.

## Canonical target

- Game profile: `dmc3-hd`
- Research executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- Research executable size: `3,735,552`
- Canonical save SHA-256: `45379458defd048502f3649e86fcb716ef2ca274a89f025d078b7deaa8df501e`
- Canonical save size: `0x4A30` / `18,992`

The currently mounted vanilla Drive executable has SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6` and size `6,567,320`. It is a different build and is excluded from canonical VA validation.

## Drive provenance

- Pass 32 report: Drive file `1_QTgYctE1hCCBOBfmK9Hv6mPyIt-yL-Lk5Q2M1EBPWs`
- Pass 32 complete ZIP: Drive file `1fjK4bjXiRv7dHMd-yhXyaTdjkD2xr4aj`
  - SHA-256 `75802006890ccd2185dfc5df237bce5adc3841d0635a2269a885ab7561ce1dcd`
- Recovered Source Skeleton v1.7: Drive file `14ShoZhaqym7ifcpaJk7KBJivCG42rXvd`
  - SHA-256 `978fd6c8c8c1e1de1cf2a50bf9328fce6be345b9f3c70ab30ffb92c195ac22d6`
  - 217 files after extraction

The recovered skeleton is an input, not product authority. Its `addresses.json` retains a stale `through_pass: 28`, and the Pass 32 checksum ASM artifact does not by itself provide a trustworthy bounded disassembly of `0x14033EED0`. Product promotion therefore relies on the reconciled report, exact record corpus, independent checksum table, reviewed implementation and regression tests. Odd-length checksum behavior is not promoted because every confirmed record size is even.

## Promoted ABI

```text
PCSaveFile — 0x4A30
├── GlobalRecordEnvelope — 0x138
│   ├── body — 0x134
│   ├── recordState — u16
│   └── checksum — u16
├── SummaryRecordEnvelope[10] — 0x40 each
│   ├── body — 0x3C
│   ├── recordState — u16
│   └── checksum — u16
└── PayloadRecordEnvelope[10] — 0x70C each
    ├── body — 0x708
    ├── recordState — u16
    └── checksum — u16
```

Exact identity:

```text
0x138 + 10 * 0x40 + 10 * 0x70C = 0x4A30
```

## Product implementation

The product module:

- preserves explicit little-endian parsing;
- models envelope sizes with compile-time layout assertions;
- distinguishes `recordState` from checksum;
- calculates the checksum over body plus `recordState`, ignoring the final stored checksum word;
- validates both stored-vs-calculated equality and the full-record `0xFFFF` fold;
- exposes neutral structural observations for the partial global map and payload reserved boundary;
- preserves Pass 31 API aliases where practical;
- warns on deviations from writer-cleared regions without treating unknown semantics as corruption;
- accepts unobserved record-state values structurally instead of inventing a closed enum.

## Explicit non-goals

The implementation does not:

- assign gameplay names to summary fields `+0x0C`, `+0x10`, `+0x14`;
- split payload `+0x000..+0x6EB` into gameplay subsystems;
- provide payload semantic mutation;
- infer record-state meanings beyond observed `0` and `1`;
- claim cross-build VA equivalence for the newer vanilla executable;
- promote odd-byte checksum behavior without direct canonical function-byte evidence.

## Validation gates

1. Local C++20 compile with `-Wall -Wextra -Wpedantic -Wconversion`.
2. Synthetic 21-record regression corpus.
3. Exact layout assertions for global, summary and payload envelopes.
4. Stored checksum and full-fold validation paths.
5. Empty-summary `recordState=0/checksum=0xFFFF` regression.
6. Payload records remain present with `recordState=1` independently of empty summaries.
7. Warning-only validation for structurally reserved regions.
8. Machine-readable Pass 32 Evidence Packet validation.
9. Windows and Ubuntu repository CI before merge.
10. Drive implementation receipt and readback after CI.

## Next gated work

Wide Pass 33 remains semantic recovery:

- acquire at least two controlled save versions;
- perform record-aware byte diffs;
- map global `+0x1C..+0x45` ownership;
- segment payload `+0x000..+0x6EB` by subsystem;
- recover inventory, mission, difficulty, character, style and unlock state;
- retain the 21-record validator as the regression oracle.
