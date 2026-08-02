# CI Validation Branch

This file triggers and documents pull-request validation of the expanded C++ foundation.

## Validation iteration 4

Scope:

- Windows and Linux configure/build/test;
- forced release-safe test helper on every test target;
- Evidence Registry and deterministic JSON packet tests;
- GDSpaces local-source vertical slice;
- revisioned WorkingCopy and undo;
- bounds-checked binary reader;
- synthetic PE32+ reader fixture;
- SHA-256 known vectors;
- guarded atomic patch-plan tests;
- CLI version, doctor, scan, hash, route, and PE inspection build.

Validation history:

1. Run 1 exposed an incorrect escaped-newline test expectation.
2. Run 2 exposed Release `NDEBUG` removing side-effectful `assert` expressions on Windows.
3. Run 3 passed Windows and Ubuntu using the temporary `NDEBUG` override.
4. Run 4 validates the permanent forced `test_support.hpp` assertion shim and latest repository head.

After a green matrix run, the CI blocker is closed and milestone 0.2 is recorded as green.
