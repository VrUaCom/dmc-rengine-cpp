# Model texture companion: the framing the corpus actually uses — 2026-09-13

**Branch:** `main`
**Scope:** the serialized companion envelope read by `formats::model_family::parse_texture_companion`. No new field semantics.

## What was wrong

The companion parser required the little-endian `TM2\0` signature at each
payload's offset. No preserved specimen satisfies it.

Every `.ptx` available to this project carries a `0x70` texture-slot descriptor
at the payload offset with the DDS image behind it, and the first four bytes of
the payload are zero:

| Specimen | Slots | `TM2` at payload start | `DDS ` at payload +0x70 |
|---|---:|---:|---:|
| `em000_000.ptx` (reference corpus) | 4 | 0 | 4 |
| `at.ptx` | 17 | 0 | 17 |
| `st114` slot bundle | 17 | 0 | 17 |
| single-slot specimen | 1 | 0 | 1 |

The consequence was not a cosmetic status code.
`analysis::mod::analyze_texture_binding` guards its entire result on
`companion.ok()` — with an invalid companion it reports `companion_valid =
false`, an empty out-of-range list and `runtime_bindings_valid() == false`. So
the analysis that exists to check which texture slots a model references, and
which fall outside the companion's domain, answered "no valid companion" for
every real model in the project.

## Why it was not caught

`model_family_tests` built a synthetic companion with `TM2\0` written into it,
and constructed the `TextureCompanionParseResult` for the binding assertions by
hand. A test that builds the input its parser wants proves the parser reads that
input and nothing about whether the input exists. This is the same shape as the
`dds`-has-no-view and `hits/dca/lig2`-have-no-view assertions found earlier in
the Android tree, one layer down: the fixture, not the assertion, encoded the
gap.

## The change

`parse_texture_companion` now recognizes both framings and records which one it
found:

- `tm2_at_payload_start` — `TM2\0` at payload offset zero;
- `descriptor_wrapped_dds` — `DDS ` at payload offset + `0x70`.

Both constants are taken from the canonical contracts rather than written
fresh: the descriptor size is `TextureSlotFramingParser::k_descriptor_size` and
the magic is `TextureSlotRuntimeMaterializationInspector::k_dds_magic`, so the
companion reader and the texture-slot reader cannot drift apart on what a slot
payload looks like.

Three properties are deliberate:

1. `TextureCompanionEntry::image_offset()` is where the image starts, past the
   descriptor where there is one. `payload_offset` still means the allocation,
   which is what the block table describes. A consumer reading pixels from
   `payload_offset` under the wrapped framing reads descriptor bytes and calls
   the result a corrupt texture.
2. A companion whose payloads disagree is refused with `mixed_framing`. A
   companion is one runtime table materialized one way; a mixture is not a
   per-entry detail for a consumer to average over.
3. A payload matching neither signature is still refused. Widening the contract
   must not turn it into "anything with a block table", and the regression test
   holds that line.

`tm2_magic_mismatch` keeps its name so existing consumers compile; it now means
"neither framing" rather than "not TM2".

## Verified

All four specimens above parse with `framing = wrapped-dds` and
`image_offset() = 0x870` for slot 0. A synthetic bundle with neither signature
is still refused. Core suite 169/169; the framing branch was proved by deleting
it and watching the wrapped fixture fail.

## Not claimed

Nothing here says which framing the original builder emitted, or that the
canonical executable prefers one. The evidence is that both are structurally
readable and that only the second appears in the preserved corpus available
here.
