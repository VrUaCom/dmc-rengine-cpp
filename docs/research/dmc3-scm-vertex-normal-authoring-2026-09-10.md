# DMC3 HD SCM vertex-normal authoring gate — 2026-09-10

**Status:** `STRUCTURAL_CONFIRMED`  
**Repository:** `VrUaCom/dmc-rengine-cpp`  
**Canonical working branch:** `reverse/mod-completion-20260907`  
**Baseline:** merge of PR #381 (`c689d5e439b0cd81bdfed2506ca09e3a1692a7ee`)

## Scope

This bounded slice adds explicit authoring of one existing SCM vertex normal. It does not introduce a normal-generation algorithm and does not claim that DMC Rengine can recalculate normals from geometry.

The serialized normal domain is already structurally established as three little-endian `float32` components at:

```text
mesh.normals_offset + vertex_index * 12
```

The existing typed edit API `set_vertex_normal()` accepts finite components and rejects non-finite authored values. The preserve-layout writer serializes those components directly.

## Authoring policy

The CLI command is:

```text
scm-set-vertex-normal <input.scm> <object-index> <mesh-index> <vertex-index> <x> <y> <z> <output.scm>
```

Current policy is `PRESERVE_AUTHORED_COMPONENTS`:

- caller supplies the three components explicitly;
- the command does not normalize them;
- the command does not derive them from adjacent triangles;
- all components must be finite;
- object/mesh/vertex indices must resolve in the parsed source.

This is intentionally narrower than a future `Normalize` or `Recalculate` policy.

## Exact-image contract

For a successful bounded edit, the command constructs an expected byte image independently from writer output:

```text
expected = source
patch expected[normalOffset + 0..11] with authored float32 x/y/z
require writerOutput == expected
```

Therefore every changed byte must be inside the selected 12-byte serialized normal record. Position streams, UVs, topology bytes, object fields, bounding sphere metadata, scene transforms, unknown/reserved bytes and physical offsets remain source-identical.

The authored image is reparsed with the canonical SCM parser and the selected normal must survive bit-exactly.

Publication uses `publish_bytes_no_replace()`; an existing destination fails closed and is not replaced.

## Regression coverage

The isolated synthetic test proves:

- finite explicit normal edit succeeds;
- writer output equals the independently constructed expected image;
- changed bytes are confined to one 12-byte normal record;
- canonical reparse preserves the authored normal;
- object bounding radius remains unchanged;
- `A -> B -> A` restores exact original source bytes;
- non-finite normal edit is rejected;
- out-of-range vertex edit is rejected;
- replay to an existing output fails closed and preserves the first output.

## Evidence boundary

This gate does **not** promote:

- real-corpus vertex-normal authoring;
- normal normalization semantics;
- geometry-derived normal recalculation;
- PAC reintegration;
- retail NBZ acceptance;
- vanilla `dmc3.exe` acceptance;
- generic SCM writer authority.

No proprietary retail payload bytes are committed. A provenance-bound real SCM/PAC artifact is still required before corpus-level promotion.
