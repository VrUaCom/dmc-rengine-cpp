# DMC3 HD SCM vertex-UV authoring gate — 2026-09-10

**Status:** `STRUCTURAL_CONFIRMED`  
**Repository:** `VrUaCom/dmc-rengine-cpp`  
**Canonical working branch:** `reverse/mod-completion-20260907`  
**Baseline:** merge of PR #382 (`75820f4ded862e168ffcc3ee4fa293409279f8d8`)

## Scope

This bounded slice adds explicit authoring of one existing SCM vertex UV pair. It uses the already established SCM serialized representation:

```text
u = signed int16
v = signed int16
scale = 1 / 4096
record width = 4 bytes
```

The selected serialized record is:

```text
mesh.uv_offset + vertex_index * 4
```

## Encoding contract

The command accepts floating-point `u` and `v`, then independently computes the expected signed-int16 representation before invoking the typed edit API. It requires the typed `set_uv()` result to agree with that independent encoding guard.

Encoding is:

```text
raw = llround(value * 4096)
```

and fails closed if the rounded result is outside signed-int16 range. Values are never silently masked or wrapped.

Exact regression points include:

```text
0.5                ->  2048
-0.25              -> -1024
+1/8192            ->     1
-1/8192            ->    -1
32767/4096         -> 32767
-8.0               -> -32768
8.0                -> rejected
-32769/4096        -> rejected
```

The half-step cases lock C++ `llround` half-away-from-zero behavior for this authoring path.

## Exact-image contract

For a successful edit:

```text
expected = source
patch expected[uvOffset + 0..1] with encoded U
patch expected[uvOffset + 2..3] with encoded V
require writerOutput == expected
```

Therefore every changed byte must be inside one 4-byte UV record. Geometry, normals, topology bytes, object metadata, bounding sphere, scene transforms, physical offsets and unknown/reserved bytes remain source-identical.

A requested float pair that quantizes to the already serialized raw pair is rejected as a no-op before writing.

Publication uses `publish_bytes_no_replace()` and canonical reparse must preserve the expected raw UV pair exactly.

## Regression coverage

The isolated synthetic gate checks:

- known-value fixed-point encoding;
- positive/negative half-step rounding;
- exact signed-int16 extremes;
- positive and negative overflow rejection;
- explicit typed encoder vs independent guard agreement;
- exact 4-byte span confinement;
- canonical reparse;
- source bounding radius unchanged;
- `A -> B -> A` exact byte restoration;
- quantized no-op rejection with no output file;
- out-of-range vertex rejection;
- existing-output no-replace preservation.

## Evidence boundary

This is a code/synthetic authoring safety gate only. It does **not** promote:

- real-corpus UV mutation acceptance;
- PAC reintegration of an authored UV edit;
- texture-companion coherence or texture binding authoring;
- retail NBZ acceptance;
- vanilla `dmc3.exe` acceptance;
- generic SCM write authority.

No proprietary payload bytes are committed. A provenance-bound real SCM/PAC source remains required for corpus-level promotion.
