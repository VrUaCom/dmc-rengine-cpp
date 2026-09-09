# DMC3 HD MOD — preserve-layout writer gate

**Date:** 2026-09-09  
**Working branch:** `reverse/mod-completion-20260907`  
**Evidence class:** `WRITER_GATE`  
**Gate:** `WRITER_GATE_1_PRESERVE_LAYOUT`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

This slice starts MOD authoring without reopening already-consolidated reverse work. It establishes a bounded, preservation-first writer over the existing canonical MOD parser and typed IR.

It is intentionally **not** a canonical rebuild-from-scratch writer. The original serialized MOD image remains physical layout authority.

## Authority model

The writer accepts two independent inputs:

1. a caller-owned immutable source byte image; and
2. a parsed `formats::mod::Document` whose `source_bytes` were retained by the canonical parser.

Before any output is produced, both byte images must match exactly. This prevents a caller from mutating `Document::source_bytes` together with typed fields and then treating the edited image as the preservation baseline.

The immutable source is reparsed through the canonical MOD parser before writing. Output is reparsed through the same parser before success is returned.

## Current allowed fixed-layout edits

The first gate permits only fields whose serialized spans are already typed and whose byte count does not change:

- object bounding center (`f32 x/y/z`);
- object bounding radius (`f32`);
- mesh positions (`float3`, existing element count only);
- mesh normals (`float3`, existing element count only);
- mesh UV values (`int16 u/v`, existing element count only).

These edits are in-place modifications of the retained physical layout. No offset/table/layout synthesis occurs.

## Explicitly blocked in Gate 1

The writer fails closed for:

- header edits;
- object/mesh cardinality changes;
- record/table/stream offset changes;
- stream size changes or reflow;
- transform-domain edits;
- skin-weight or blend-index edits;
- packed control/topology edits;
- texture-slot or GS CLAMP/material-state edits;
- object source-flag / alpha-control edits;
- generated workspace state edits;
- unresolved/preservation-only fields including mesh `+0x0C`, `+0x38`, `+0x4C` and transform `+0x1C`;
- any non-finite bounding, position or normal value.

## Byte-preservation enforcement

The writer starts from the immutable source image and constructs an explicit set of byte spans authorized to change for the requested Gate-1 edits.

After serialization it compares every output byte against the immutable source. If any changed byte falls outside an authorized span, the write fails with `mod.writer.unauthorized-byte-change`.

This is stronger than relying only on typed-field comparisons because it independently protects bytes that are not yet decoded by the MOD IR.

## Write receipt

Successful writes return a `WriteReceipt` containing:

- source SHA-256;
- output SHA-256;
- byte count;
- modified-byte count;
- immutable-source / `Document::source_bytes` binding result;
- unauthorized-byte preservation result;
- output canonical reparse result;
- no-edit byte-identical result.

For a no-edit write:

```text
source SHA-256 == output SHA-256
modified_byte_count == 0
no_edit_byte_identical == true
```

For an authorized fixed-layout edit, the output hash may change, but every changed byte must belong to an explicitly authorized span and the output must reparse successfully.

## Regression gates

`tests/mod_writer_tests.cpp` exercises:

1. parse -> no-edit write -> exact byte parity;
2. source/output SHA equality for no-edit output;
3. controlled bounding-radius + position + UV edit and canonical reopen;
4. unchanged bytes outside the exact edited spans;
5. source-flag edit rejection;
6. undecoded mesh-field edit rejection;
7. stream-cardinality change rejection;
8. transform edit rejection;
9. retained `Document::source_bytes` tamper rejection against the external immutable source.

The test is registered as `dmc_rengine_mod_writer_tests` in the normal Ubuntu/Windows CTest matrix.

## Evidence status

What this gate establishes:

- `parse -> write(no-op) -> reparse` can be made byte-identical for the bounded synthetic canonical fixture;
- source-preservation is enforced independently of the mutable typed document;
- a small set of fixed-size promoted fields can be written without changing unrelated bytes in the fixture;
- writer output must pass the canonical parser reopen gate.

What this gate does **not** establish:

- byte parity across the full 38-MOD retail corpus;
- canonical layout planning from typed IR alone;
- arbitrary model editing;
- transform authoring;
- skin authoring;
- material/texture binding authoring;
- texture companion rewriting/coherence;
- PAC/PNST/NBZ reintegration;
- original `dmc3.exe` acceptance of a no-op rebuilt MOD;
- original `dmc3.exe` acceptance of an edited MOD;
- Capcom authoring-tool equivalence;
- a `100% MOD writer` claim.

## Next evidence frontier

The next useful MOD work must remain inside the no-repeat frontier and advance one of these gates:

1. run the no-edit writer over representative real MOD corpus members and record exact parity/reopen receipts;
2. place writer output back into its canonical texture-companion/container context without changing resource identity or slot topology;
3. validate no-op rebuilt MOD acceptance in the original `dmc3.exe`;
4. only after those gates, expand controlled mutation authority field-by-field.

Do not return to previously consolidated hierarchy, skin-packing, header `+0x14`, bit21 or zero-field reverse passes unless a genuinely new consumer, producer, corpus contradiction or game experiment appears.
