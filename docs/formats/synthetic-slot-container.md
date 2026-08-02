# Synthetic Slot Container (`SLTC`)

`SLTC` is an original test-only binary format invented for DMC Rengine's public CI corpus.

It is designed to test:

- declared slot count;
- empty slot preservation;
- slot identity independent from names;
- populated byte ranges;
- optional names and fallback names;
- truncation and out-of-range handling;
- overlapping child diagnostics;
- child magic reclassification;
- GDSpaces child expansion and resource graph links.

It does **not** claim compatibility with PAC, PNST, AFS, NBZ, or any game format.

## Header — 16 bytes

| Offset | Type | Meaning |
|---:|---|---|
| `0x00` | char[4] | magic `SLTC` |
| `0x04` | u16 LE | schema version, currently 1 |
| `0x06` | u16 LE | declared slot count |
| `0x08` | u32 LE | slot table offset |
| `0x0C` | u32 LE | string table offset |

## Slot entry — 16 bytes

| Offset | Type | Meaning |
|---:|---|---|
| `+0x00` | u32 LE | child data offset |
| `+0x04` | u32 LE | child data size |
| `+0x08` | u32 LE | name offset relative to string table; zero means no name |
| `+0x0C` | u32 LE | flags; bit 0 means populated |

## Empty slot policy

When bit 0 is clear, the slot is preserved as empty. Non-zero residual offset/size/name fields generate a warning and are ignored.

## Populated slot policy

A populated slot requires:

- non-zero size;
- range inside the container;
- a valid name or generated fallback.

An out-of-range slot becomes a preserved invalid/empty placeholder and produces an error diagnostic. Other slots remain available.

## Name policy

Names are null-terminated, limited to 256 bytes, and read from the declared string table. Missing, empty, out-of-range, or unterminated names are replaced with deterministic names such as:

```text
slot_0002.bin
```

The `synthetic_name` flag records that the name is presentation fallback rather than physical truth.

## Overlap policy

Overlapping populated slots generate warnings. They remain in the parsed document so Binary Inspector and future evidence tooling can display the conflict.

## Probe score

Exact `SLTC` magic returns probe score 100. No extension-only fallback is used, preventing accidental interpretation of unrelated files.

## Legal/content status

All fixtures are generated from this public specification and contain only invented bytes and tiny generic signatures used to exercise classification.
