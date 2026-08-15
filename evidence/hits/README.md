# HITS Evidence Records

This directory stores public, non-proprietary evidence metadata for HITS reverse-engineering promotions.

- `hits-pass8-wide-runtime-integration-review.evidence.json` — Pass 8 authority record linking the canonical executable hash, Drive preservation documents, retained findings, implementation freezes and then-remaining reverse targets.
- `hits-pass10-query-runtime-evidence.evidence.json` — aggregate Pass-10 query/runtime snapshot. Some historical labels in this aggregate are superseded by later slice-specific evidence.
- `hits-pass10-slice7-validation.evidence.json` — validated primitive-shape mapping receipt.
- `hits-pass10-slice8-contact-normal.evidence.json` — validated canonical evidence that common metadata `+0x28/+0x2C/+0x30` is a collision/contact surface normal float3. Exact global orientation/handedness remains unresolved.
- `hits-pass10-slice9-primitive-descriptor-ownership.evidence.json` — validated primitive-descriptor ownership: manager entry table, `0x50` descriptor table, runtime `+0x118` descriptor pointer, separate runtime `+0x20` transform pointer, and bounded static descriptor census.

## Evidence precedence

When a later slice-specific record explicitly marks an earlier statement `SUPERSEDED`, `CORRECTED`, or `REJECTED`, the later record is authoritative for that claim. In particular:

- do not resurrect the old `E7A0` metric/tie-break model;
- do not treat metadata `+0x28..+0x30` as an unresolved vector after Slice 8;
- do not treat runtime object `+0x20` as the primitive descriptor pointer after Slice 9.

All DMC3 build-specific VAs/body hashes here are profile evidence for canonical executable SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082` unless a record explicitly states otherwise.

Raw proprietary HITS/PAC/EXE bytes are never stored here.
