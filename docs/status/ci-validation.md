# CI Validation Branch

This file triggers and documents pull-request validation of the expanded C++ foundation.

## Validation iteration 6

Scope:

- Windows and Linux configure/build/test;
- compiler-level `/UNDEBUG` / `-UNDEBUG` test invariant;
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
3. Run 3 passed Windows and Ubuntu using compiler-level `NDEBUG` removal.
4. Run 4 proved that a forced include alone is insufficient because MSVC's later `<cassert>` can redefine `assert`.
5. Run 5 confirmed all new 0.3 modules compile and pass on Ubuntu; Windows compiled fully but used the stale assertion-shim configuration.
6. Run 6 validates the latest head with the proven compiler-level assertion invariant.

After a green run, the CI blocker is closed and the validation PR can be closed as completed provenance.
