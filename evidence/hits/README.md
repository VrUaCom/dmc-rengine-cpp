# HITS Evidence Records

This directory stores public, non-proprietary evidence metadata for HITS reverse-engineering promotions.

- `hits-pass8-wide-runtime-integration-review.evidence.json` — Pass 8 authority record linking the canonical executable hash, Drive preservation documents, retained findings, implementation freezes and then-remaining reverse targets.
- `hits-pass10-query-runtime-evidence.evidence.json` — aggregate Pass-10 query/runtime snapshot. Some historical labels in this aggregate are superseded by later slice-specific evidence.
- `hits-pass10-slice7-validation.evidence.json` — validated primitive-shape mapping receipt.
- `hits-pass10-slice8-contact-normal.evidence.json` — validated canonical evidence that common metadata `+0x28/+0x2C/+0x30` is a collision/contact surface normal float3. Exact global orientation/handedness remains unresolved.
- `hits-pass10-slice9-primitive-descriptor-ownership.evidence.json` — validated primitive-descriptor ownership: manager entry table, `0x50` descriptor table, runtime `+0x118` descriptor pointer, separate runtime `+0x20` transform pointer, and bounded static descriptor census.
- `hits-pass10-slice10-primitive-type01.evidence.json` — validated structural semantics for runtime type `0` (one-point representation) and type `1` (two-endpoint segment representation). Source-text vocabulary for types `0/1` remains unresolved.
- `hits-pass10-slice12-stage-cfg-pac.evidence.json` — validated Stage-CFG PAC collision-slot bridge: resource kind 1 -> `room\\stXXXcfg.pac`; modern entry/descriptor slots 39/40, legacy observed slots 22/23. This record supersedes the earlier abstract inner-blob interpretation of `+0xA4/+0xA8` and `+0x60/+0x64`.
- `hits-pass10-slice13-collision-triplet.evidence.json` — validated `0x40 / 0x04 / 0x50` serialized dynamic-collision three-span ABI with Phase-15 `em000.pac` data-side confirmation and `id100.pac` negative control. PAC slot numbers are not globally semantic, and Stage-CFG slot38 is not promoted as the transform table.
- `hits-pass10-slice14-stage-cfg-collision-view.evidence.json` — validated Stage-CFG entry/primitive-descriptor adapter over the existing `ContainerDocument`; exposes flags, raw transform selector, descriptor index/type and broken descriptor references without inventing transform-selector bounds. Includes the failed-first-head → real-container-API fix → green Ubuntu/Windows debug receipt.
- `hits-pass10-slice15-referenced-stage-cfg-descriptor-census.evidence.json` — validated referenced-only census implementation over the Slice-14 Stage-CFG view. It counts only actually referenced descriptors, preserves raw type/reference counts and broken references, and deliberately makes no real Stage-CFG type-5 presence/absence claim until representative proprietary corpus data is supplied through the canonical GDSpaces/container path.

## Evidence precedence

When a later slice-specific record explicitly marks an earlier statement `SUPERSEDED`, `CORRECTED`, or `REJECTED`, the later record is authoritative for that claim. In particular:

- do not resurrect the old `E7A0` metric/tie-break model;
- do not treat metadata `+0x28..+0x30` as an unresolved vector after Slice 8;
- do not treat runtime object `+0x20` as the primitive descriptor pointer after Slice 9;
- do not collapse structural primitive names into source tokens: Slice 10 confirms geometry for runtime types `0/1`, not original text vocabulary;
- do not model `+0xA4/+0xA8` or `+0x60/+0x64` as universal inner resource-header fields after Slice 12: they are PAC slot-offset table entries in the current stage `room\\stXXXcfg.pac`;
- do not infer that matching PAC slot numbers imply matching schemas: Slice 13 proves `em000.pac` 38/39/40 is a valid collision three-table set while `id100.pac` 38/39/40 is not;
- do not wire Stage-CFG slot38 as a transform table without new direct evidence. Stage-CFG entry/descriptor slots remain confirmed independently;
- do not validate Stage-CFG `entry+0x01` transform selectors merely because the raw selector byte is decoded. Slice 14 explicitly leaves transform provenance unresolved;
- do not interpret Slice-15 synthetic raw type-5 regression as evidence that a real Stage-CFG resource contains type 5. Real presence or absence requires a provenance-preserving representative corpus census.

All DMC3 build-specific VAs/body hashes here are profile evidence for canonical executable SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082` unless a record explicitly states otherwise.

Raw proprietary HITS/PAC/EXE bytes are never stored here.
