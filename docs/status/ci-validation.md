# CI Validation Branch

This file triggers and documents pull-request validation of the expanded C++ foundation.

## Validation iteration 7

Scope:

- Windows and Linux configure/build/test;
- compiler-level `/UNDEBUG` / `-UNDEBUG` test invariant;
- canonical DMC3 known-target registry;
- SHA-256 and parsed PE metadata matching;
- CLI known-target recognition;
- centralized profile-aware resource classifier;
- conservative `StageBundleAssembler`;
- Binary Inspector `Document` model;
- Evidence Registry/Packet, WorkingCopy, patch, binary reader, PE, and SHA-256 tests.

Validation history:

1. Run 1 exposed an incorrect escaped-newline test expectation.
2. Run 2 exposed Release `NDEBUG` removing side-effectful `assert` expressions on Windows.
3. Run 3 passed Windows and Ubuntu using compiler-level `NDEBUG` removal.
4. Run 4 proved that a forced include alone is insufficient on MSVC.
5. Run 5 confirmed the expanded resource/stage/binary layer compiles and passes on Ubuntu.
6. Run 6 passed the complete resource/stage/binary foundation on Windows and Ubuntu.
7. Run 7 validates the final known-target/evidence integration.

After a green run, the CI blocker is closed and this validation PR is closed as completed provenance.
