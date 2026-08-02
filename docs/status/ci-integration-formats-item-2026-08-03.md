# Integrated Formats + Item Editor Validation — 2026-08-03

This branch validates the exact current `main` state.

## New integration scope

- Item Runtime Evidence Packet sharing the canonical DMC3 executable artifact with Phase 12;
- artifact-relative range validation and corrected impossible file offsets;
- Item Workspace View, Runtime Change Requests, Spider Hub runtime-request nodes, events, validators, and Item Workspace Manifest;
- slot 50 exact guard ready for planning; slots 51-63 research-gated; inventory limit locations research-gated;
- Evidence Address Resolver for file offset/RVA/VA consistency and artifact SHA checks;
- HITS$, DCA, LIG2, and Stage TXT parsers through FormatRegistry and ResourceAnalyzer;
- Binary Inspector adapters for all implemented structural parsers;
- Stage TXT confirmed tokens and StageSet Evidence linking;
- DMC3 StageWorkspaceBuilder, Stage Ops/ModViz shared views, consistency checks, manifests, and CLI;
- all prior GDSpaces, Evidence, EXE, ProjectWorkspace, ProjectGraph, WorkingCopy, patch safety, and malformed-input tests.

## Required invariants

- no second resolver;
- immutable source bytes;
- no direct executable patching from Item Editor;
- executable evidence scoped by exact artifact SHA-256;
- impossible physical file ranges rejected transactionally;
- unknown format fields remain unknown;
- PAC/PNST/AFS/NBZ remain read-only;
- Stage Ops and ModViz share one canonical project state;
- Windows and Ubuntu must configure, compile, and pass every test.
