# DMC3 SCM vertex-position authoring gate — 2026-09-10

## Scope

This note records the bounded SCM vertex-position authoring safety gate added on `reverse/mod-completion-20260907` in PR #381.

This is **not** a real-retail authoring promotion. The connected evidence surfaces available during this pass did not contain the provenance-bound raw `st001.scm` / `st001.pac` payloads used by the earlier alpha/translation/rotation acceptance work.

Evidence status for this slice: **STRUCTURAL_CONFIRMED**.

## Serialized domain

The canonical SCM parser/writer already models a mesh position stream as `float3` XYZ values, 12 bytes per vertex. Object `bounding_radius` is serialized at object record `+0x3C` as a 32-bit float and is writer-owned derived metadata when geometry or the bounding center changes.

For one bounded vertex edit the only serialized spans permitted by this gate are:

- selected position record: `mesh.positions_offset + vertex_index * 12`, width 12;
- selected object's derived radius: `object.record_offset + 0x3C`, width 4.

The bounding center is not authored by this command and must remain source-identical.

## Source-radius fail-closed boundary

Before authoring, the command independently reconstructs the radius of the unmodified source object as the maximum Euclidean distance from `bounding_center` across all mesh positions.

The reconstructed source radius must be **bit-identical** to the serialized source `bounding_radius`.

If the source differs even by one ULP, vertex authoring fails closed before output publication. This prevents an edit from silently normalizing source-derived metadata when the exact retail builder floating-point path is not proven for that source.

This guard is intentionally stricter than merely accepting whatever derived value the writer would emit after a geometry change.

## Exact-image guard

The command builds an expected byte image from the retained source bytes and patches only:

1. authored X, Y, Z as little-endian float32 values;
2. independently derived post-edit `bounding_radius` as little-endian float32.

The preserve-layout writer output must equal this expected image byte-for-byte. A subset-only diff test is not sufficient.

After writing, the canonical parser must reopen the result and reproduce the authored position and derived radius bit-exactly while preserving the source bounding center.

Publication uses the existing race-safe `publish_bytes_no_replace()` contract.

## Synthetic regression coverage

The dedicated `scm_vertex_authoring` test covers:

- one isolated position edit `(1, 0, 0) -> (4, 0, 0)`;
- derived radius `2.0 -> 4.0`;
- exact expected-image equality;
- all changed bytes confined to the 12-byte position record plus 4-byte radius record;
- canonical reparse;
- inverse `A -> B -> A` restoration to exact original bytes;
- non-finite position rejection;
- out-of-range vertex rejection;
- source radius differing by one ULP rejected before output creation;
- existing-output replay rejected by no-replace publication while preserving the first output.

No proprietary retail payload bytes are committed by this test or note.

## Promotion boundary

This slice does **not** prove or claim:

- real-corpus SCM vertex-position authoring;
- `st001.pac` slot-2 reintegration/reopen for a vertex edit;
- arbitrary vertex edits across the SCM corpus;
- universal Capcom bounding-radius reconstruction;
- retail NBZ authored acceptance;
- vanilla `dmc3.exe` authored-resource acceptance;
- generic product/UI SCM write authority.

The generic SCM registry remains read-only.

The next promotion requires a provenance-bound real SCM source whose source radius first passes the bit-exact reconstruction precondition, followed by exact diff, canonical reparse, PAC reintegration/reopen/extract, and inverse/source-image restoration evidence.
