# Specification 005 — EXE-Backed Stage Bundle: st001

## Status

Data model implemented; assembly pending PE/evidence support.

## Problem

Historical workflows opened individual stage-related files manually and sometimes resolved them differently in Stage Ops and ModViz. The project needs one reproducible stage identity and one typed bundle.

## Target

Use `st001` as the first vertical integration target because it has broad historical research coverage and exercises scripts, room configuration, effects, sound, nested resources, Stage Ops, ModViz, and EXE evidence.

## Goals

- represent an EXE-backed `StageIdentity`;
- map the four known stage-table roles:
  1. script;
  2. room configuration;
  3. room effects;
  4. room sound;
- resolve those logical identities through GDSpaces;
- expand available nested resources;
- classify members into typed stage categories;
- preserve unknown members;
- attach evidence and diagnostics;
- allow partial bundle construction;
- provide the same resource IDs to Stage Ops, ModViz, and Binary Inspector.

## Non-goals

- editing or repacking the stage;
- hardcoding local paths;
- assuming every stage uses identical nested content;
- implementing all stage formats;
- treating the table strings as the only possible runtime identity evidence.

## Architecture

Input:

- known-target executable evidence packet;
- stage row metadata;
- mounted user-supplied game sources;
- GDSpaces classifier/container expansion.

Output:

- `StageBundle` with `StageIdentity`;
- members categorized as scripts, models, textures, animations, cameras, lighting, events, positions, effects, collision, sounds, or unknown;
- diagnostics for missing, ambiguous, unsupported, and malformed resources;
- graph edges linking stage identity, source resources, and evidence.

## Partial failure policy

Missing sound, an unsupported child format, or one malformed member must not discard valid scripts/models. Errors are scoped to members and summarized at bundle level.

## Acceptance criteria

- no Stage Ops or ModViz file resolution code is introduced;
- one canonical resource ID is reused across all consumers;
- repeated assembly is deterministic;
- missing members produce diagnostics;
- unknown resources remain visible;
- tests use synthetic stage-table and container fixtures;
- a local integration test can be run by a user with legally obtained matching files;
- no source archive is modified.

## Dependencies

- Specification 002 — GDSpaces Resource Contract;
- Specification 003 — Evidence Registry;
- Specification 004 — Read-Only PE Inspector;
- future container-source specification.

## Risks

- overfitting the generic stage model to one known stage;
- treating fallback names as canonical runtime identity;
- hiding unsupported members;
- stage-specific logic leaking into generic container parsing.
