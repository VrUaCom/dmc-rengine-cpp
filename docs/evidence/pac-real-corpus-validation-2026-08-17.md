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
| `em035__em035_007.pac` | `46b4fd9f2a071c2f62e4c10cf38be17eeb61bcd53d607589088630ae38422206` | 25184 | 2 | 2 | 0 | yes |
| `em035__em035_008.pac` | `e502ad5093e60625c1ed46047234ea477beee25d47549252c444c4fdd678678d` | 75680 | 18 | 8 | 10 | yes |
| `em035__em035_009.pac` | `d62248804e2148297e1fd8b8d036f2cfb3e349a5f6472d96ff6cbb5acbc85de4` | 24544 | 1 | 1 | 0 | yes |
| `em035__em035_010.pac` | `bea48779ee18b4f8bfa36b1213d9fb95b8b36ce72fe05b72bfc895af02c469e8` | 374128 | 95 | 45 | 50 | yes |
| `em035__em035_034.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_035.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_036.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_037.pac` | `821df27004c18405b84ff02de9b643a0bd1316891af93597e864a743174d4c54` | 3024 | 95 | 8 | 87 | yes |
| `em035__em035_039.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_040.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_041.pac` | `4d3ebdb031d02e40e9b6f39fc6d29ae1f7ea6cf4fd06894b4f8c90e22cfea576` | 16 | 0 | 0 | 0 | yes |
| `em035__em035_042.pac` | `dde30efe1cb027c672eda10ea69421af76527d9baeb4b78492f1d4d3a8666b45` | 5952 | 95 | 22 | 73 | yes |
| `m20_b00__m20_b00_002.pac` | `94ed084e993786d266b96db11681e70cc864f63a4a9f1f1df8c2156de96d0921` | 106144 | 84 | 84 | 0 | yes |
| `m20_b00__m20_b00_006.pac` | `e75220967dd95e92e66fbeb4baba873cec41044e16f1a37a842e85ce56a18930` | 1332912 | 225 | 225 | 0 | yes |
| `m20_b00__m20_b00_008.pac` | `e8514a418530aa8c80ca7590ba32aa25654653a44bf76033bdcbb64b2aff5a02` | 416 | 5 | 5 | 0 | yes |
| `m20_b00__m20_b00_010.pac` | `c06b6c37e039d2187fb799374b5fe68a9d96ca295817386dd8d65a1a3e8af81e` | 30800 | 29 | 29 | 0 | yes |
| `m20_c00__m20_c00_002.pac` | `c61c4a613decb15563dae609e5258ed922044be61f159efea64edbc4bebcfb30` | 166032 | 87 | 87 | 0 | yes |
| `m20_c00__m20_c00_006.pac` | `5fa5201619ce6f5cd7615e53a5c4b431d63a1ef92dbb6a32860018fc4d8ba666` | 2159408 | 415 | 415 | 0 | yes |
| `m20_c00__m20_c00_008.pac` | `41550f9e19e8cc8c505149b8c37fe3d3eabf9ecb8088ac0c9cf9203603f32ee3` | 1984 | 7 | 7 | 0 | yes |
| `m20_c00__m20_c00_010.pac` | `c2489c39c18c0336efe834e3586f6f066270aa6c7d41b5bc766c2cb8261013bb` | 71376 | 60 | 60 | 0 | yes |
| `m20_s00__m20_s00_002.pac` | `ce183899d002d3a94e3cb372532c815601267cb0278b8446c8b04a63281c3950` | 130208 | 93 | 93 | 0 | yes |
| `m20_s00__m20_s00_006.pac` | `8c1c81c730f91a85878ac4761c673798a704bcc24480c4818cb1be9f0dfdaabc` | 2495392 | 270 | 270 | 0 | yes |
| `m20_s00__m20_s00_008.pac` | `6939c2bbae05d9dcca603066e46f1995ed08d2de0884a33ea1d9fd2c0bab17db` | 176 | 2 | 2 | 0 | yes |
| `m20_s00__m20_s00_010.pac` | `9e7bdbe606c1c418f6b355f78f2c53f15850b04f73effd5abf6eaf25367d0757` | 49744 | 38 | 38 | 0 | yes |
| `pl011__pl011_002.pac` | `987788ccb6affc227caa10847bb610a9cf11532ca92aea8b96e2c13fbffc1a69` | 177264 | 59 | 22 | 37 | yes |
| `pl011__pl011_003.pac` | `1a1212d69f0e374055a21d8da91aebce1a5d7ac41eb874df62a70931668752b4` | 130256 | 17 | 14 | 3 | yes |
| `pl011__pl011_004.pac` | `4cd09394503f283229caeedd29f51aa3cb9b91b9afc6fc39f0a23392999ee90d` | 38800 | 4 | 4 | 0 | yes |
| `st001 - copia__st001 - copia__st001_007.pac` | `f0c0a2252f26d8e6f1538bc307b9c2ebf9a785e5d11b1fbe4ffc45672a3a5487` | 63456 | 1 | 1 | 0 | yes |
| `st445__st445_005__st445_005_009.pac` | `21ceef3bdfbb1f257778bbdbc649c0051e568db7b5f1de5759d02da2bd4feb25` | 12784 | 1 | 1 | 0 | yes |
| `st445__st445_005__st445_005_035.pac` | `70c8aec68ddef3059ad8dc3127858f5a2af9e553bb62ab452f779e5e3cb9c4e0` | 1216 | 1 | 1 | 0 | yes |
| `st445__st445_005__st445_005_037.pac` | `70c8aec68ddef3059ad8dc3127858f5a2af9e553bb62ab452f779e5e3cb9c4e0` | 1216 | 1 | 1 | 0 | yes |
| `st445__st445_005__st445_005_039.pac` | `70c8aec68ddef3059ad8dc3127858f5a2af9e553bb62ab452f779e5e3cb9c4e0` | 1216 | 1 | 1 | 0 | yes |

## Boundary

- This validates the bounded structural PAC decoder against the reacquired real PAC corpus.
- It does not validate PNST, NBZ, `.lst`, AFS, semantic slot schemas, write/repack behavior or full DMC3 runtime equivalence.
- Duplicate non-zero offset behavior remains defensive product support because this 32-PAC corpus does not exercise duplicate populated offsets.
- Universal 16-byte alignment remains rejected as a parser validity invariant by the broader Phase-15 corpus evidence.

## Defect discovered during reconciliation

Before this receipt could run, PR #99 had a shared-contract defect: populated `ContainerEntry` objects had empty `logical_name`, while `ContainerEntry::valid()` requires a non-empty name for populated entries. Thus populated PACs would terminate as `invalid_document`. The fix assigns deterministic positional names (`slot_NNNN.bin` / `slot_NNNN.empty`) with `synthetic_name=true`. These names preserve structural slot identity without inventing source semantics.
