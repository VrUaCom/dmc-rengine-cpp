# Specification 007 — Known Executable Targets

## Status

Initial generic target model and DMC3 Phase 12 registry implemented.

## Problem

Executable findings must be tied to exact artifacts, but the generic PE parser must not accumulate game/version-specific constants.

## Goals

- define generic known-target metadata;
- identify targets by SHA-256;
- independently validate expected PE metadata;
- keep DMC3 target data in a profile-specific registry;
- connect code to public Evidence Packets;
- allow CLI recognition without changing parser behavior;
- warn when hash and parsed metadata disagree.

## Non-goals

- trusting a hash instead of parsing;
- shipping the executable;
- embedding stage tables in the PE parser;
- automatically applying patches;
- supporting unknown target versions without evidence.

## Current model

`exe::KnownExecutableTarget` contains:

- target ID;
- display name;
- SHA-256;
- PE kind;
- machine;
- image base;
- entry-point RVA.

Profile registry:

- `profiles::dmc3::phase12_canonical_target()`.

Public packet:

- `evidence/known-targets/dmc3-hdc-phase12.evidence.json`.

## Recognition workflow

1. GDSpaces reads a user-supplied file.
2. Core computes SHA-256.
3. PE reader parses structural metadata.
4. Profile registry compares SHA-256.
5. Known target compares expected metadata.
6. CLI reports recognized/unrecognized and match/mismatch.

## Acceptance criteria

- target metadata validates;
- SHA comparison is case-insensitive and exact length;
- malformed hashes do not match;
- correct PE metadata matches;
- modified expected metadata fails;
- generic PE reader remains profile-neutral;
- no proprietary bytes enter tests or repository.

## Planned extensions

- target registry across DMC1/2/3/Launcher;
- executable size and timestamp when locally regenerated;
- export/import directory expectations;
- evidence packet URI/ID field;
- supported patch-plan IDs;
- compatibility reporting for near matches;
- CLI JSON output.

## Risks

- one hash treated as all game versions;
- metadata copied without evidence provenance;
- target recognition used to bypass parser safety;
- profile-specific constants leaking into generic modules.
