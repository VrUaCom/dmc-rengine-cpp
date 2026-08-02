# GDSpaces Contract

GDSpaces is the single public resource-access architecture of DMC Rengine.

## Current C++ contracts

### `ResourceId`

Canonical identity fields:

- source ID;
- logical path;
- container chain;
- byte offset;
- byte size.

`ResourceId` is not a display label and does not contain transient OS handles.

### `ResourceRef`

Adds presentation and classification metadata:

- display name;
- format;
- game/profile label;
- synthetic-name flag;
- container flag.

### `ResourcePayload`

Carries:

- the resolved reference;
- owned read-only bytes;
- diagnostics.

A payload is readable only when its reference is valid and it has no error diagnostics.

### `ISource`

A source owns transient access to a mounted origin. It can enumerate references and read a resource by canonical ID.

Current implementation:

- `LocalDirectorySource` — recursive/non-recursive read-only local mount with root-containment checks.

Planned implementations:

- game-folder source;
- NBZ volume source;
- AFS source;
- nested PAC/PNST expansion;
- generated/in-memory test source.

### `SourceRegistry`

Owns mounted sources, rejects duplicate source IDs, enumerates all sources deterministically, and routes reads to the owning source.

### `ResourceGraph`

Stores stable resources and typed edges:

- contains;
- depends-on;
- stage-member;
- evidence-for;
- opens-with.

### `OpenRouter`

Maps a resource/context to a tool target. Explicit preferred targets override defaults. Menu and stage contexts are first-class.

Unknown formats route to Binary Inspector rather than disappearing.

### `StageBundle`

Groups resolved resources under one stage identity and typed categories. It accepts unknown resources and scoped diagnostics.

## Dependency direction

```text
Source implementation
  → ResourceId / ResourceRef / ResourcePayload
  → SourceRegistry
  → ResourceGraph / OpenRouter / StageBundle
  → Tool-specific parser/editor
```

The direction may not be reversed. UI and format editors do not own source resolution.

## Identity rules

1. Display name is presentation only.
2. Synthetic names are explicit.
3. Container slots/offsets remain distinct from names.
4. EXE-backed semantic identity may link to, but does not erase, source identity.
5. Re-enumeration should reproduce canonical IDs for unchanged sources.
6. Future source revision/hash fields must extend identity without making tools source-aware.

## Read-only rules

- source bytes are immutable;
- reads return owned payloads;
- malformed or inaccessible resources produce diagnostics;
- traversal outside a mounted root is rejected;
- a resource may remain visible even if its format is unknown.

## Future working-copy boundary

Editing will create a separate working copy with operation history and validation. A `ResourcePayload` will never silently become mutable original storage.

## Architecture anti-patterns

- editor opens a local path directly;
- editor parses an archive to locate its own child resources;
- UI stores `FileSystemHandle` as canonical identity;
- Binary Inspector reopens a path instead of consuming bytes;
- stage loader special-cases PAC inside generic source resolution;
- display names are used as map keys across tools.

These patterns are rejected as the Second Resolver.
