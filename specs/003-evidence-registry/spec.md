# Specification 003 — Evidence Registry

## Status

In-memory core implemented; persistence planned.

## Problem

Reverse-engineering knowledge becomes unreliable when claims, confidence, artifact versions, corrections, and locations are stored only in prose or memory.

## Goals

- canonical confidence enum;
- evidence records with stable IDs;
- claims separated from individual evidence records;
- artifact/location metadata;
- tags and supersession links;
- duplicate rejection;
- query by ID, claim, and confidence;
- future deterministic JSON persistence.

## Non-goals

- deciding technical truth automatically;
- storing proprietary artifacts;
- replacing human review;
- embedding decompiler databases in the first schema.

## Data model

An evidence record contains:

- evidence ID;
- claim ID;
- title and summary;
- confidence;
- one or more optional artifact locations;
- tags;
- IDs it supersedes.

A location may contain:

- public artifact ID/hash reference;
- file offset and size;
- RVA and VA;
- symbol;
- sanitized note.

## Invariants

- IDs are non-empty and unique;
- claim, title, and summary are required;
- every supplied location names an artifact;
- correction does not erase prior records;
- `confirmed` is a reviewed status, not a parser-generated default.

## Acceptance criteria

- add rejects invalid and duplicate records;
- replace updates only existing IDs;
- query by claim and confidence is deterministic;
- confidence text conversion round-trips;
- tests contain no original game bytes.

## Persistence milestone

Schema version 1 must support:

- deterministic JSON output;
- strict required fields;
- unknown-field tolerance policy;
- artifact SHA-256 references;
- correction chains;
- validation diagnostics;
- stable ordering for review diffs.

## Risks

- confidence labels used without evidence quality review;
- one evidence record overloaded with several claims;
- mutable replacement hiding corrections;
- machine-generated records flooding the Canon.
