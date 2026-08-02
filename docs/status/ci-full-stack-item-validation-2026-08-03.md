# Full Stack + Item Editor Validation — 2026-08-03

This branch validates the exact current `main` state.

## Scope

- complete GDSpaces/ProjectWorkspace/Event Journal stack;
- Artifact, Evidence, and Evidence Packet registries;
- public Phase 12 and Item Runtime Evidence Packets sharing one canonical executable artifact;
- strict Evidence JSON validation;
- Binary Inspector and HITS integration;
- generic PE/EXE analysis, exact artifact-SHA evidence linking, and EXE Workspace Manifest;
- DMC3 `st001` StageWorkspaceBuilder;
- shared Stage Ops and ModViz views with consistency validation;
- Spider Hub ProjectGraph and manifests;
- Item Workspace View over an ITM session;
- Item Runtime Change Requests delivered from Item Editor to EXE Editor;
- Evidence Registry and Build & Test Lab as mandatory validators;
- slot 50-63 guarded registration request model;
- inventory limit 1-255 range model with incomplete-location research gate;
- no direct EXE patching from Item Editor;
- all CLI commands and all previous tests;
- Windows and Ubuntu configure/build/test.

## Invariants

- one canonical resource identity;
- no second resolver;
- immutable source bytes;
- WorkingCopy-only edits;
- exact artifact-scoped executable evidence;
- Stage Ops and ModViz share one state;
- Item Editor emits requests, EXE Editor/Patch Engine owns executable changes;
- PAC/PNST/AFS/NBZ remain read-only;
- legacy PAC Editor/PAC Manager architecture remains excluded.
