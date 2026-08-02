# Resource Classification

`ResourceClassifier` is the single low-level classifier for GDSpaces resources.

## Inputs

- canonical/logical resource path;
- optional read-only byte span.

## Outputs

`ResourceClassification` contains:

- normalized format string;
- `GameProfile`;
- container flag;
- `magic_confirmed` flag.

## Current profile set

- `dmc1-hd`;
- `dmc2-hd`;
- `dmc3-hd`;
- `dmclauncher-hd`;
- `unknown`.

Path detection is a hint. It does not prove runtime ownership or executable identity.

## Current magic signatures

The classifier recognizes independently known generic/format signatures:

- `MZ` → `pe`;
- `PAC\0` → `pac`;
- `SCM` → `scm`;
- `DCA\0` → `dca`;
- `HITS$` → `hits`;
- `DDS ` → `dds`.

When no supported magic is present, the normalized lowercase extension is used. Unknown extensions remain visible.

## Container classification

Current internal container format labels:

- `nbz`;
- `afs`;
- `pac`;
- `pnst`.

A container flag does not imply the container is already parsed or writable.

## Local source integration

`LocalDirectorySource` uses extension/path classification during enumeration. After bytes are read, it classifies again with magic, allowing a signature to correct a misleading extension.

Example:

```text
logical path: unknown/blob.bin
bytes: MZ ...
result: format=pe, magic_confirmed=true
```

## Rules

1. Classification never creates canonical identity.
2. Display names do not influence format truth.
3. A path-derived profile is a hint, not evidence.
4. Magic confirmation applies only to the recognized signature, not the complete schema.
5. Unsupported formats remain `unknown` or extension-labelled and route to Binary Inspector.
6. Tool-specific classification logic is prohibited when the responsibility belongs here.

## Planned work

- configurable profile hints from mounted game roots;
- confidence/provenance for classification;
- plugin-free parser registry;
- ambiguous-signature diagnostics;
- nested child classification after container expansion;
- EXE-backed semantic identity links.
