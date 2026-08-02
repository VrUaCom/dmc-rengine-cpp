# CI Validation Branch

This file triggers and documents pull-request validation of the expanded C++ foundation.

## Validation iteration 3

Scope:

- Windows and Linux configure/build/test;
- Release tests with assertions explicitly enabled;
- Evidence Registry and deterministic JSON packet tests;
- GDSpaces local-source vertical slice;
- revisioned WorkingCopy and undo;
- bounds-checked binary reader;
- synthetic PE32+ reader fixture;
- SHA-256 known vectors;
- guarded atomic patch-plan tests;
- CLI version, doctor, scan, hash, route, and PE inspection build.

Previous findings:

1. The first run exposed an incorrect escaped-newline test expectation.
2. The second run exposed Release `NDEBUG` removing side-effectful `assert` expressions on Windows.
3. The CMake test helper now undefines `NDEBUG` for every test target.

After a green matrix run, the results are recorded in `docs/status/current.md`.
