# Custom Build Identity Final Validation — 2026-08-03

This marker validates the exact current feature branch after all final correctness gates.

## Scope

- CustomBuildIdentity and CustomBuildRecord;
- exact original executable and source-tree lineage;
- compiler, linker, target, flags, dependency-lock identity;
- PE32+ structural validation and complete source mappings;
- included source modification versions and unified decision records;
- source-line/source-unit/recovered-symbol to output file offset/RVA/VA mappings;
- mandatory package test enforcement;
- status-specific PE, runtime-smoke, release-attestation, revocation, and rollback gates;
- Spider Hub custom-build, source-binary-mapping, and build-test-result provenance;
- SHA-256 lookup and EXE Editor reopen context;
- file-size and PE metadata mismatch rejection;
- deterministic Custom Build Record manifest;
- 59-test Windows and Ubuntu matrix.

## Product invariant

The canonical binary is a Custom Recompiled Game EXE built from a Composite Source Build. The runtime plugin model remains false. Binary patching remains a lower-level evidence and migration mechanism.
