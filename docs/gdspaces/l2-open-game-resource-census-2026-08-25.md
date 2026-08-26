# GDSpaces Layer 2 — OpenGameResource caller/overflow census — 2026-08-25

**Scope:** Layer 2 / Runtime Resolver only.

**Authority:** canonical unpacked reverse target `dmc3.exe`, size `6,356,432` bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Result

The canonical DMC3 direct-call runtime surface of `OpenGameResource` at `0x14002FCA0` is now bounded well enough to close the roadmap caller-mode and candidate-overflow questions for that surface.

## Direct caller census

A whole-image direct-call census found three direct call sites to `0x14002FCA0`:

- `0x14003340A`
- `0x1403380C7`
- `0x1403381F7`

All three materialize `EDX = 1` immediately before the call.

No literal 64-bit pointer to `0x14002FCA0` is stored in the canonical image, so the census did not identify a static function-table entry that would introduce another flag mode.

### Consequence

For the recovered direct-call DMC3 runtime path, `flags = 1` is the observed caller mode. That mode executes the already-recovered generic resolver branch:

1. strip the input to basename;
2. attempt the six namespace prefixes in order;
3. first pass uses provider mask `1` (archive);
4. second pass uses provider mask `2` (physical);
5. return the first successful opened resource;
6. return `-1` when all 12 attempts miss.

The other internal branches present inside `OpenGameResource` remain implementation code, but no canonical direct caller was found selecting them. They therefore must not be promoted as active fallback policy for the recovered DMC3 direct-call resource path without a separate indirect-call/runtime receipt.

## Exact `0x400` candidate overflow aftermath

For each active attempt, `OpenGameResource` calls the bounded join helper at `0x1403272C0` with destination capacity `0x400`.

The helper requires the generated C string, including its terminating NUL, to fit in that destination. Therefore the candidate text itself must be shorter than `0x400` bytes.

If the helper returns false:

- `OpenGameResource` immediately destroys/releases the newly allocated file slot/object through `0x140048DF0`;
- it returns `-1`;
- it does **not** increment the prefix index;
- it does **not** continue to a shorter namespace prefix;
- it does **not** enter the physical-provider pass.

This closes the previously unresolved caller-level overflow aftermath.

### Important boundary consequence

The first recovered prefix is `GDataX360.afs/`, length 14, and is also the longest prefix. With a `0x400` destination, the first candidate can contain at most 1023 non-NUL bytes, so a basename longer than 1009 bytes fails on the first candidate and aborts the whole request.

A basename that could theoretically fit with the later empty prefix is still not tried if the first prefixed candidate overflows.

This makes the existing GDSpaces fail-closed whole-plan validation equivalent to the recovered canonical direct-call mode: if the first/longest candidate does not fit, the original runtime aborts before any shorter prefix can be considered.

## Status impact

- `OpenGameResource` direct caller-mode census: **CLOSED for canonical direct-call surface**.
- caller-level `0x400` candidate-overflow aftermath: **CLOSED**.
- generic six-prefix archive-before-physical resolver branch: remains confirmed.
- alternate internal flag branches: **not promoted as active runtime policy** without separate indirect-call/runtime evidence.
- direct-retail corpus receipt, controlled physical/missing/fallback receipts and original-process selected-identity receipt remain open.
