# Specification 002 — GDSpaces Resource Contract

## Status

Initial implementation complete; schema stabilization in progress.

## Problem

A resource can be reached through local files, nested containers, stage tables, search, or evidence links. Tool-owned loaders create conflicting identities and behavior.

## Goals

Define one resource contract for all tools:

- `ResourceId` — canonical source/logical/container/region identity;
- `ResourceRef` — stable identity plus presentation/classification metadata;
- `ResourcePayload` — bytes and diagnostics for a resolved reference;
- `ISource` — source enumeration and read interface;
- `SourceRegistry` — mounted source ownership;
- `ResourceGraph` — typed relations;
- `OpenRouter` — tool routing;
- `StageBundle` — typed stage grouping.

## Non-goals

- format-specific parsing;
- original-data writes;
- GUI state management;
- storing OS file handles as public identity;
- implementing all container sources in this phase.

## Invariants

1. A canonical ID does not depend on display name.
2. A source owns transient filesystem/archive handles.
3. Tools never reopen the resource independently.
4. Container expansion produces child resources, not hidden byte slices.
5. Diagnostics travel with payloads/bundles.
6. Unknown formats remain addressable.
7. Partial stage failures do not erase valid members.

## Initial vertical slice

`LocalDirectorySource → SourceRegistry → ResourceRef → ResourcePayload → ResourceGraph/OpenRouter → CLI/test`

## Acceptance criteria

- local directory enumeration is deterministic;
- traversal outside the mounted root is rejected;
- duplicate source IDs are rejected;
- reads return diagnostics instead of silent corruption;
- graph relations require existing nodes and reject duplicates;
- unknown formats route to Binary Inspector;
- item, EXE, stage, and menu contexts route to their canonical tools;
- tests use only synthetic files.

## Future extensions

- artifact hash/revision identity;
- profile-aware classification;
- NBZ/AFS/PAC/PNST sources;
- `.index` metadata links;
- nested DDS exposure;
- working-copy revisions;
- serialized resource manifests.

## Risks

- overloading `ResourceId` with presentation state;
- turning GDSpaces into a monolith;
- format parsers leaking into source ownership;
- path normalization differences across platforms.
