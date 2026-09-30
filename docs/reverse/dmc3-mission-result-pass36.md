# DMC3 Mission Result — Wide Pass 36

## Status

`ACTIVE — EVIDENCE ACCESS BLOCKER`

This pass continues the Pass 35 workstream: result-category identity, direct rank-label binding, and the remaining `MissionResultPolicyRecord` semantics. The block is intentionally not closed. The repository now carries the evidence-safe physical ABI so future XREF/DFG work has a compile-checked target without promoting unsupported gameplay labels.

## Canon references

- Pass 35 canonical report: Google Drive file `1N28ouJgvDPej3KFUoYlmcqYvkFvutDI5F4ONYi9bWE0`.
- Pass 36 blocker / discriminating experiment readback: Google Drive file `1QUmWF_rnjrdRmyB4Xm5r2NdGu_nC4vOGOfIVmAh72ww`.
- Canonical research executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
- Rank-label function locator carried from Pass 35: VA `0x14024FE90`.

## Promoted physical ABI

Only the layout is promoted here:

```text
MissionResultPolicyRecordLayout — 0x84
+0x00  float categoryThresholds[5][4]  // 0x50 bytes
+0x50  float aggregateValues[4]        // 0x10 bytes
+0x60  u32 rawValues060[5]             // 0x14 bytes, semantics open
+0x74  byte unknownTail[0x10]           // semantics open

MissionResultPolicyGroupLayout — stride 0x298
+0x000 MissionResultPolicyRecordLayout records[5] // 0x294 bytes
+0x294 u32 rawGroupTail                            // ownership/semantics open
```

The C++ representation is `include/dmc_rengine/save/mission_result_policy.hpp` and is protected by compile-time size/offset assertions plus `tests/mission_result_policy_tests.cpp`.

## Explicitly rejected promotion

The following are **not** canonicalized by this pass:

- `MissionResultMatrix` code `0..5` mapped to `C/B/A/S/SS/SSS`;
- gameplay identities for the five policy categories;
- semantic names for `+0x60..+0x73`;
- semantic names for `+0x74..+0x83`;
- ownership or meaning of the group tail at `+0x294`;
- any direct consumer edge between the Pass 35 rank vocabulary and serialized result codes.

These remain open because layout adjacency is not a substitute for XREF/data-flow evidence.

## PASS36-E01 — minimum discriminating experiment

Goal: prove or reject a direct binding between serialized MissionResultMatrix result codes and the rank-label domain.

1. Acquire the canonical executable matching SHA-256 `e454272e...dd082`, or an equivalent raw disassembly/XREF export from that exact build.
2. Enumerate all XREFs to VA `0x14024FE90` and identify callers/consumers.
3. Recover CFG for each relevant caller and trace inputs/outputs into SSA/DFG form where useful.
4. Independently enumerate readers/consumers of MissionResultMatrix values `0..5`.
5. Accept a code-to-rank binding only if the two domains converge through a direct data-flow edge, a shared lookup table, or a switch/branch whose constants can be independently corroborated.
6. If no convergence exists, reject the direct-binding hypothesis and retain the domains separately.

## Validation debt

- Canonical executable/raw disassembly unavailable in the evidence set used by the Pass 36 execution.
- XREF/CFG/DFG for `0x14024FE90` remains open.
- Behavioral validation of category identity remains open.
- No runtime/editor mutation is authorized from this pass.

## Integration impact

This pass adds a neutral ABI contract only. It does not alter GDSpaces routing, resource resolution, EXE patch behavior, save mutation behavior, or runtime behavior. Future semantic code must be evidence-gated against PASS36-E01.

## DoD

- Evidence: partial / blocker proven.
- Provenance: present.
- Types/Layout: promoted for the physical record and group stride.
- Regression coverage: compile-time + runtime layout assertions.
- XREF/CFG/DFG: blocked pending canonical raw executable evidence.
- Ownership/Lifetime: open where semantics are unknown.
- Behavioral validation: open.
- Block status: **ACTIVE**.
