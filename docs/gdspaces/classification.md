# Resource Classification

`ResourceClassifier` is the single low-level classifier for GDSpaces resources.

This document describes **current clean-C++ classifier behavior**, not the complete universe of DMC3-HD formats. The wider evidence-backed resource-purpose inventory is maintained in [`docs/formats/dmc3-hd-format-catalog.md`](../formats/dmc3-hd-format-catalog.md).

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

## Current byte/signature recognition

The current classifier directly recognizes these generic/format signatures:

- `MZ` -> `pe`;
- `PAC\0` -> `pac`;
- structurally valid `PNST` relative-slot container -> `pnst`;
- `SCM` -> `scm`;
- `DCA\0` -> `dca`;
- four-byte `HITS` -> `hits`;
- `DDS ` -> `dds`.

### PNST validation rule

A textual `.index` manifest can begin with a line such as:

```text
PNST\r\n
```

Therefore a raw four-byte prefix match is not enough to classify binary PNST. The current implementation validates the PNST relative-slot envelope before returning `pnst`.

This avoids turning extraction/naming metadata into a false binary-container identity.

### HITS correction

The canonical current HITS signature is:

```text
HITS
```

not historical `HITS$`.

See [`docs/formats/hits.md`](../formats/hits.md) for the current grid/triangle collision structure and the explicit rejection of `0x18060001` as a universal record marker.

## Extension fallback

When no supported byte signature is recognized, the normalized lowercase extension is used as a **classification label**.

This fallback does not promote the extension to semantic truth.

Examples:

```text
*.ukn + validated ITM bytes  -> ITM at a higher semantic/format layer
*.ukn + validated HITS bytes -> HITS
*.pac + validated PNST bytes -> PNST
```

Unknown extensions remain visible rather than being hidden or force-mapped.

## Container classification

Current internal container/source labels include:

- `nbz`;
- `afs`;
- `pac`;
- `pnst`.

Important DMC3-HD boundary:

- `GData.afs/` / `GDataX360.afs/` are established logical namespace prefixes;
- that fact is not proof of an opaque binary AFS archive backend in the current HD runtime path.

A container flag does not imply the container is already parsed, semantically understood or writable.

## Local source integration

`LocalDirectorySource` uses extension/path classification during enumeration. After bytes are read, it classifies again with byte evidence, allowing a signature to correct a misleading extension.

Example:

```text
logical path: unknown/blob.bin
bytes: MZ ...
result: format=pe, magic_confirmed=true
```

## Classifier coverage is intentionally narrower than format knowledge

Current original-runtime/corpus evidence includes many resource families beyond the low-level byte classifier, for example:

```text
MOD EFM SHW PTX TM2
CAM LIG/LIG2 EVE POS ITM STE EST
MOT variants
ADX/OGG SFD/WMV
and additional research targets
```

Those families must not be added to this low-level classifier through weak extension heuristics merely to make the list look complete.

Use the canonical [DMC3 HD format and resource-purpose catalog](../formats/dmc3-hd-format-catalog.md) to distinguish:

- known original-runtime family;
- corpus-only/research family;
- current product integration maturity;
- unresolved purpose/schema.

## Rules

1. Classification never creates canonical resource identity.
2. Display names do not influence format truth.
3. A path-derived profile is a hint, not evidence.
4. Magic confirmation applies only to the recognized signature/validated envelope, not the complete schema.
5. Extension fallback is a label, not semantic authority.
6. Unsupported formats remain unknown or extension-labelled and route to neutral inspection until stronger evidence exists.
7. Tool-specific classification logic is prohibited when the responsibility belongs here or in the shared format/integration registry.
8. `.ukn`, `.bin`, `.pac` and other source suffixes may be misleading; preserve them as metadata even when semantic classification changes.
9. A middleware capability string or short ASCII hit does not become a format signature without contextual validation.

## Planned work

- configurable profile hints from mounted game roots;
- richer confidence/provenance on classification results;
- reconcile direct LIG2 identifier evidence with the clean structural scanner/probe path;
- migrate additional strong byte grammars only when direct evidence justifies low-level recognition;
- ambiguous-signature diagnostics;
- nested child classification after container expansion;
- EXE-backed semantic identity links.

## Related documentation

- [Format documentation index](../formats/README.md)
- [DMC3 HD format and resource-purpose catalog](../formats/dmc3-hd-format-catalog.md)
- [Residual format census](l3-residual-format-pass-2026-08-26.md)
