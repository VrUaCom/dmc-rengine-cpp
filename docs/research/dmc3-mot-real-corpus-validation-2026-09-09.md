# DMC3 HD MOT real-payload validation — 2026-09-09

**Status:** CORPUS_CONFIRMED structural validation
**Canonical executable:** `dmc3.exe`
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The canonical modular MOT parser was executed against the three hash-bound real `.mot` payloads currently available in the local corpus. Payload bytes are not committed; the machine-readable receipt records their paths, sizes and SHA-256 values.

All three payloads pass the complete structural grammar:

- header size equals the aligned channel-mask table extent;
- channel-mask popcount equals the declared track count;
- compression-3 track spans equal `0x20 + 8 * key_count`;
- key time control is decoded as low 15-bit time plus independent bit 15 flag;
- low-15-bit key times are nondecreasing (the modular parser permits equality);
- the track chain ends at zero alignment padding;
- the post-track tail is accepted at the observed 4-byte and 12-byte lengths.

Observed payloads:

| Payload | Size | Domains | Tracks | Compression | Tail | Raw f32 +0x0C |
|---|---:|---:|---:|---|---:|---:|
| `st001/st001_007/st001_007_000.mot` | 63,440 | 24 | 69 | 69 × 3 | 4 | 650 |
| `st600/st600_007/st600_007_000.mot` | 9,248 | 24 | 69 | 69 × 3 | 12 | 120 |
| `st600/st600_005/st600_005_011/st600_005_011_000.mot` | 12,768 | 18 | 48 | 48 × 3 | 12 | 100 |

This pass confirms that the old `st001` observation is one member of a wider grammar and that the tail is alignment padding rather than a universal four-byte terminator. It does not independently expand compression-2 coverage; that boundary remains supported by the existing 82-file em000 receipt and synthetic regression, while fresh real compression-2 payload acquisition remains desirable.

Receipt: `data/reverse/dmc3-mot-real-corpus-receipt-20260909.json`.
