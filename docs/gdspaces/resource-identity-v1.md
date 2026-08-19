# GDSpaces Resource Identity v1

**Status:** canonical milestone / implementation expansion required  
**Snapshot date:** 2026-08-08

GDSpaces Resource Identity v1 defines the stable identity needed for resources to remain the same logical object across physical packaging, editor routing, Evidence, Stage Workspace, Binary Inspector, and Reverse Core links.

## Problem

A pathname is not a durable identity. The same logical DMC resource may be encountered through a local directory, NBZ, AFS, PAC, PNST, an extracted working copy, or an executable-backed semantic reference. Evidence must not fragment merely because the physical representation changes.

## Canonical identity layers

A resource identity may include:

- source identity;
- game/profile identity;
- normalized logical path;
- container chain;
- container slot/index identity;
- byte offset and size when meaningful;
- artifact hash when bytes are materialized;
- EXE-backed semantic identity;
- stable canonical `ResourceId`;
- separate display identity and synthetic fallback name.

Display names, local paths, and synthetic names never replace `ResourceId`.

## Required properties

Resource Identity v1 must provide:

1. deterministic identity construction for equivalent inputs;
2. preservation of parent/child container lineage;
3. stable empty-slot/index semantics where the format exposes slots;
4. explicit representation of unknown or partially parsed children;
5. post-read magic/type correction without silently changing logical identity;
6. links from executable evidence to resources without making EXE strings the sole identity;
7. reproducible manifests suitable for local integration reports;
8. identity reuse by Stage Ops, ModViz, Binary Inspector, Item Editor, and Reverse Core.

## Container policy

PAC, PNST, NBZ, and AFS remain implementation layers inside GDSpaces. Production read-only expansion is a current project gate, but container-specific product identities must not be reintroduced.

`.index` data may contribute metadata/linkage where evidence supports it, but it is not treated as runtime truth or a standalone canonical asset identity.

## Acceptance milestone

Resource Identity v1 is considered demonstrated when one legally supplied `st001` resource set can be resolved through production container sources, assembled into one typed StageBundle, opened by Stage Ops and ModViz, inspected by Binary Inspector, and linked to executable/reverse evidence while every consumer refers to the same canonical resource identities.
