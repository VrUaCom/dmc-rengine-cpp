# HITS integration evidence contract

## Target

- Project: DMC Rengine
- Executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- Coordination issue: #25

## Canonical statuses

Only these statuses are valid:

- `HYPOTHESIS`
- `EXE CONFIRMED`
- `DERIVED FROM VERIFIED RUNTIME`
- `GAME VERIFIED`
- `GAME + SAVE VERIFIED`
- `RESEARCH REQUIRED`

## Integrated facts

- `HITS` is a four-byte magic. `HITS$` is rejected.
- A terminal record is `0x38` bytes: raw flags, three points, plane normal, plane D.
- Header grid counts are read from `+0x2C/+0x30/+0x34`.
- Spatial and triangle bases are relative to `raw + 8` using fields `+0x3C/+0x40`.
- Each spatial cell resolves to a signed `int32` list terminated by `-1`.
- Non-negative list entries are byte offsets into the triangle array and must be divisible by `0x38`.
- Unknown header bytes, raw flags and padding remain byte-preserved by future writers.

## Corrections preserved by tests

- `0x18060001` is a flag value, not a record marker.
- Records are triangle-plane structures, not AABBs.
- PAC member index is provenance, not a universal format identifier.
- Source 2 is not represented as a third HITS resource.

## Scope of this branch

This branch migrates the existing HITS parser and Binary Inspector adapter in place. It intentionally does not modify CMake, ProjectWorkspace, ProjectGraph, CustomBuildIdentity or source-integration files changed by draft PR #26.

All fixtures are synthetic and contain no copyrighted game payloads or executable bytes.
