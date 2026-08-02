# Specification 001 — C++ Foundation

## Status

Implemented, validation pending.

## Problem

The legacy project accumulated valuable functionality but mixed UI, source resolution, format logic, and runtime patching. A clean, portable foundation is required before migration.

## Goals

- C++20 and CMake project;
- Windows/Linux CI;
- core library and CLI;
- tests using CTest;
- repository governance and content policy;
- stable namespaces and public include layout;
- no proprietary inputs in source control.

## Non-goals

- importing legacy source wholesale;
- production GUI;
- parsing original game containers;
- writing archives or executables;
- claiming full decompilation.

## Architecture

- library target: `dmc_rengine_core`;
- alias: `DMCRengine::Core`;
- CLI: `dmc-rengine`;
- public headers: `include/dmc_rengine/`;
- implementation: `src/`;
- synthetic tests: `tests/`.

## Acceptance criteria

- configure/build/test succeeds on Windows and Linux;
- `dmc-rengine version` reports the project version;
- `dmc-rengine doctor` prints constitutional invariants;
- warnings are enabled;
- public policy documents exist;
- `.gitignore` excludes known proprietary file families.

## Risks

- early API churn;
- over-documentation without executable contracts;
- accidental migration of legacy architecture debt.

## Evidence

Completion is evidenced by CI checks, CTest results, repository tree, and current status JSON.
