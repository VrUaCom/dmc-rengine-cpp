# PAC Real-Corpus Validation Receipt — 2026-08-17

**Schema:** `dmc-rengine.pac-real-corpus-validation.v1`  
**Parser:** PR #99 `PacParser` after shared-`ContainerDocument` identity fix  
**Corpus authority:** exact historical `DMC 3 RENGINE (6).zip` stage-drop PAC bytes; no proprietary bytes are committed.

## Summary

- PAC artifacts tested: **32**
- parse success: **32/32**
- failures: **0**
- deterministic second parse: **32/32**
- valid `slotCount == 0`: **6**
- sparse PACs (`empty > 0`): **6**
- maximum declared slot topology: **415**
- observed byte-size range: **16 .. 2,495,392 bytes**

Every artifact was parsed twice by the C++ implementation. Determinism means the second parse produced byte-for-byte identical normalized receipt output for slot count, populated topology, inferred extents and synthetic structural slot names.

Synthetic names such as `slot_0000.bin` are explicit product presentation identities required by the shared `ContainerEntry` contract; they are marked synthetic and do **not** claim an original filename or semantic role.

## Artifact receipts

| artifact | sha256 | bytes | slots | populated | empty | deterministic |
|---|---|---:|---:|---:|---:|---|
| `em035__em035_007.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_008.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_009.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_010.pac` | `bea48779ee18b4f8bfa36b1213d9fb95b8b36b1213d9fb95b8b36ce72fe05b72bfc895af02c469e8` | 1074384 | 95 | 45 | 50 | yes |
| `em035__em035_034.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_035.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_036.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_037.pac` | `821df27004c18405b84ff02de9b643a0bd1316891af93597e864a743174d4c54` | 1017040 | 95 | 8 | 87 | yes |
| `em035__em035_039.pac` | `3b3ee8a525a0be4b898822c7c88c50ad437b628e528809e77fd772c1ee374e6c` | 561104 | 95 | 16 | 79 | yes |
| `em035__em035_040.pac` | `8ed3837b904598773a68c8c7e6df3322a9f9906f3eed88dd59b43dcd634d6d26` | 1536368 | 95 | 33 | 62 | yes |
| `em035__em035_041.pac` | `a7ab32969a5fa977b046d50059d488cf75ae2f3229728d56acbac47ef16d9616` | 986160 | 95 | 22 | 73 | yes |
| `em035__em035_042.pac` | `dde30efe1cb027c672eda10ea69421af76527d9baeb4b78492f1d4d3a8666b45` | 2495392 | 95 | 22 | 73 | yes |

> The full sanitized local receipt contains all 32 SHA-bound rows and per-populated-slot offset/extent records. This committed summary deliberately keeps the repository receipt compact while preserving the corpus-level gate and representative hashes already anchored by Pass 17.

## Boundary

- This validates the bounded structural PAC decoder against the reacquired real PAC corpus.
- It does not validate PNST, NBZ, `.lst`, AFS, semantic slot schemas, write/repack behavior or full DMC3 runtime equivalence.
- Duplicate non-zero offset behavior remains defensive product support because this 32-PAC corpus does not exercise duplicate populated offsets.
- Universal 16-byte alignment remains rejected as a parser validity invariant by the broader Phase-15 corpus evidence.

## Defect discovered during reconciliation

Before this receipt could run, PR #99 had a shared-contract defect: populated `ContainerEntry` objects had empty `logical_name`, while `ContainerEntry::valid()` requires a non-empty name for populated entries. Thus populated PACs would terminate as `invalid_document`.

The fix assigns deterministic positional names (`slot_NNNN.bin` / `slot_NNNN.empty`) with `synthetic_name=true`. These names preserve structural slot identity without inventing source semantics.
