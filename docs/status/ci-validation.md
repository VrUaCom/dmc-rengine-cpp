# CI Validation Branch

This file triggers and documents pull-request validation of the expanded C++ foundation.

## Validation iteration 5

Scope:

- Windows and Linux configure/build/test;
- permanent release-safe test helper;
- centralized profile-aware resource classifier;
- magic correction in `LocalDirectorySource`;
- conservative `StageBundleAssembler`;
- Binary Inspector `Document` regions, ownership, coverage, unknown gaps, and conflicts;
- Evidence Registry and deterministic JSON packet tests;
- revisioned WorkingCopy and guarded patch tests;
- binary reader, synthetic PE32+, and SHA-256 tests;
- CLI build.

Validation history:

1. Run 1 exposed an incorrect escaped-newline test expectation.
2. Run 2 exposed Release `NDEBUG` removing side-effectful `assert` expressions on Windows.
3. Run 3 passed Windows and Ubuntu using the temporary `NDEBUG` override.
4. Run 4 validates the permanent assertion shim.
5. Run 5 validates the expanded 0.3 resource/stage/binary domain layer.

After a green run, status and blocker records are synchronized with the latest head.
