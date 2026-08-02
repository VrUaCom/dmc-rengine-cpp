# Deprecated and Rejected Architecture

This document prevents old assumptions from returning under new names.

## PAC Editor / PAC Manager as central architecture

**Status:** deprecated

Rejected model:

- users open PAC as the primary project object;
- each editor owns PAC parsing and writing;
- PAC names define tool routing;
- stage workflows are built around manual PAC selection;
- PAC Manager acts as a central integration service.

Replacement:

- GDSpaces mounts sources and expands containers;
- PAC is an internal format module;
- editors receive typed resources and working copies;
- stages use `StageBundle`;
- OpenRouter selects tools from resource context.

Historical PAC parsing knowledge may be migrated. The old product boundary may not.

## Tool-owned resource resolution

**Status:** rejected

Examples:

- Stage Ops creating its own `ResourceResolver`;
- ModViz opening NBZ/PAC directly;
- Item Editor carrying local file handles as identity;
- Texture pipeline independently expanding containers;
- Binary Inspector re-opening source paths instead of consuming supplied bytes.

Replacement: `ResourceId`, `ResourceRef`, `ResourcePayload`, `SourceRegistry`, `ResourceGraph`, and `OpenRouter`.

## File handles as UI identity

**Status:** rejected

A transient OS/browser file handle is not a stable resource identity. It cannot express container chains, semantic links, stage membership, hashes, evidence, or source revisions.

Replacement: stable canonical resource IDs with transient handles hidden inside source implementations.

## Display name equals identity

**Status:** rejected

Synthetic names and friendly names are presentation data. They do not prove physical path, runtime identity, or EXE identity.

Replacement: layered identity model.

## `.index` as a normal runtime asset

**Status:** corrected

`.index` may provide navigation or metadata, but it must not automatically appear as an independent runtime resource equivalent to the data it describes.

Replacement: metadata/link attachment with explicit provenance.

## Stage-only PAC special casing at the container layer

**Status:** rejected

Generic container handling must not depend on whether a PAC is used by a stage. Stage semantics are applied after resources are resolved.

## Direct write-back as the default save path

**Status:** rejected

Saving directly into original archives or executables before validation creates corruption, irreproducibility, and unclear rollback.

Replacement: working copy → validate → manifest → explicit export/build.

## Byte patches without source guards

**Status:** rejected

An offset and target bytes alone are insufficient. Patches require artifact identity, source bytes, semantic purpose, dependencies, rollback, and tests.

## Decompiled output equals recovered source

**Status:** rejected

Generated pseudocode or decompiler C is evidence, not automatically correct source. It may contain wrong types, ownership, control flow, calling conventions, and invented names.

Replacement: reviewed decompilation units with evidence and behavioral validation.

## Architecture by UI screen

**Status:** rejected

A screen, panel, or hexagon is not automatically a subsystem. Product boundaries follow responsibilities and data ownership.

## Automatic AI output as Canon

**Status:** rejected

AI-generated hypotheses, code, names, or summaries enter the Sect of Neuroslop pipeline and must pass triage, evidence, tests, correction, and review before becoming Canon.

## Blender Bridge inside the core engine

**Status:** out of scope

Blender integration may exist as an external product/bridge using published contracts. It is not part of the core C++ architecture unless a future specification explicitly changes that boundary.
