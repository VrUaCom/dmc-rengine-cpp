# Trial Chamber + Full Stack Validation — 2026-08-03

This branch validates the exact current `main` after Build & Test Lab integration.

## New scope

- Item Runtime Validation Plans;
- Spider Hub validation-plan and validation-requirement nodes;
- validation provenance to runtime request, ITM resource, executable artifact, analyzed EXE resource, Evidence Records, and Build & Test Lab;
- duplicate plan rejection and append-only `validation-plan-created` events;
- deterministic Validation Plan Manifest;
- ready, blocked, and pending-execution requirement states;
- all previously integrated GDSpaces, Evidence, Binary Inspector, EXE, Item, Stage Ops, ModViz, Stage Workspace, format parsers, manifests, CLI, and safety tests.

## Gates

- Windows and Ubuntu configure/build/test green;
- no second resolver;
- immutable source bytes;
- exact artifact-scoped evidence;
- no direct executable patching from Item Editor;
- Validation Plan must not become executable when any mandatory prerequisite is blocked;
- PAC/PNST/AFS/NBZ remain read-only.
