# Specification 004 — Read-Only PE Inspector

## Status

Approved for implementation.

## Problem

The project has historical PE facts and executable findings but no current C++ service that reproduces basic executable identity and address mapping safely.

## Goals

Implement a read-only PE32/PE32+ inspector that can:

- validate DOS and PE signatures;
- parse COFF and optional-header identity;
- report machine, section count, image base, entry point, image size, and subsystem;
- enumerate sections with file/RVA ranges and characteristics;
- convert file offset ↔ RVA ↔ VA with overflow and bounds checks;
- distinguish PE32 and PE32+;
- emit diagnostics for malformed/truncated input;
- operate on `ResourcePayload` or an explicit local CLI input routed through GDSpaces;
- support synthetic fixtures.

## Non-goals

- disassembly;
- import/export reconstruction beyond basic directory metadata;
- executable modification;
- unpacking/protection bypass;
- publishing the target executable;
- claiming recovered source.

## Architecture

Proposed public types:

- `exe::PeImage`;
- `exe::PeKind`;
- `exe::Machine`;
- `exe::Section`;
- `exe::Address` and checked conversion results;
- `exe::PeDiagnostic` or shared diagnostics;
- `exe::PeReader`.

The parser consumes a byte span. Source acquisition remains outside the EXE module.

## Safety requirements

- every read is bounds checked;
- checked addition/multiplication for table sizes;
- section ranges are validated against file/image boundaries;
- overlapping and impossible ranges produce diagnostics;
- no allocation based on an unbounded count;
- parser never trusts a target hash as proof of structural validity.

## Synthetic fixtures

Create original test fixtures for:

- minimal PE32+ with one section;
- minimal PE32;
- truncated DOS header;
- invalid `e_lfanew`;
- truncated section table;
- overflowing/overlapping sections;
- RVA and offset boundary cases.

## Known-target evidence

The canonical DMC3 target metadata may be represented as a separate evidence document keyed by SHA-256. The parser itself remains generic.

## CLI milestone

```text
dmc-rengine inspect-exe <path>
```

Output must be sanitized, deterministic, read-only, and clear when the file is unsupported or malformed.

## Acceptance criteria

- all synthetic valid/invalid fixtures pass tests;
- no original game bytes are committed;
- Windows and Linux CI pass;
- known target fields can be reproduced locally by a user who supplies the matching executable;
- no write APIs exist in this phase.
