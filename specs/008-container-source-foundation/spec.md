# Specification 008 — Container Source Foundation

## Status

Approved for implementation; no production container parser is claimed complete.

## Problem

DMC HD resources are stored through layered NBZ, AFS, PAC, and PNST structures. The clean architecture must expose nested resources through GDSpaces without reviving PAC Editor/PAC Manager or allowing each tool to parse archives independently.

## Goals

- define a generic read-only container parser/source boundary;
- expose container entries as child `ResourceRef` values;
- preserve physical source identity, container chain, slot/index, offset, and size;
- separate canonical identity from display/fallback names;
- preserve empty slots and unknown entries when structurally meaningful;
- attach diagnostics per entry and per container;
- connect parent/child resources in `ResourceGraph`;
- classify child resources through the central classifier;
- support synthetic fixtures before game-backed integration.

## Non-goals

- write/repack support;
- a top-level PAC product;
- hiding unsupported children;
- treating `.index` as an ordinary runtime asset;
- stage-specific logic inside generic container parsing;
- publishing original archive bytes.

## Proposed interfaces

- `formats::ContainerEntry`;
- `formats::ContainerDocument`;
- `formats::ContainerParseResult`;
- `formats::IContainerParser`;
- `gdspaces::ContainerSource` or source decorator;
- parser registry keyed by classification/probe result.

## Identity requirements

A child resource identity must be reproducible from:

- owning source ID;
- parent canonical resource ID;
- container format/layer;
- slot/index or equivalent structural identity;
- byte offset and size;
- logical/semantic name when supported;
- explicit synthetic-name flag when fallback naming is required.

Names may not erase slot identity.

## `.index` policy

`.index` data is metadata/linkage. It may enrich navigation and identity but must not automatically become a runtime resource equivalent to the payload it describes.

## Synthetic fixtures

First fixture families:

- minimal PAC-like container with empty and non-empty slots;
- text-index plus binary PNST-like relationship;
- nested child with `DDS ` magic under misleading extension;
- duplicate/overlapping offsets;
- truncated table;
- out-of-range entry;
- slot count greater than populated entries.

Synthetic fixtures must be clearly invented and must not be presented as full real-format compatibility.

## Acceptance criteria

- parser reads only supplied byte spans;
- no parser opens paths;
- every accepted child has a stable `ResourceId`/`ResourceRef`;
- parent/child graph edges are deterministic;
- malformed entries produce diagnostics without discarding unrelated valid entries;
- unknown children remain visible;
- no write API exists;
- Windows/Linux tests pass;
- Stage Ops, ModViz, Item Editor, and Binary Inspector require no container-specific loaders.

## Migration order

1. common parser/result contracts;
2. synthetic slot-container fixture;
3. PAC read-only structural subset backed by fresh evidence;
4. PNST metadata linkage;
5. AFS source layer;
6. NBZ volume source migration;
7. nested classification and graph;
8. local `st001` StageBundle assembly.

## Risks

- guessing real schemas from incomplete summaries;
- collapsing slot and filename identity;
- loading entire very large archives into memory;
- unsafe sizes/offsets;
- container recursion cycles;
- stage-specific special cases entering generic code;
- premature writer implementation.
