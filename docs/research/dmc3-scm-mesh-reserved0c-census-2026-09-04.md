# DMC3 HD SCM serialized mesh `+0x0C` — bounded consumer census 2026-09-04

## Target

- executable: `dmc3.exe`;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- ImageBase: `0x140000000`.

## Serialized field

The canonical SCM mesh record is `0x50` bytes. The field under review is:

```text
mesh +0x0C : u32
```

Current stock corpus behavior:

```text
observed value = 0
```

The two directly materializable real specimens also confirm zero across all meshes:

```text
st001.scm : 77 / 77 meshes -> 0
st114.scm : 72 / 72 meshes -> 0
```

## Why offset-only matching is unsafe

The recovered renderer contains many active fields at `runtimeRecord+0x0C` and many generated command words at `command+0x0C`.

Those are **not** evidence for serialized SCM `mesh+0x0C`.

This census therefore requires pointer provenance from the serialized mesh base before accepting a consumer.

## Serialized mesh initialization path `0x1402F9BB0`

`0x1402F9BB0` obtains the serialized mesh pointer through:

```text
argument +0x18
  -> +0x08
  -> serialized mesh pointer
```

That pointer is stored locally and copied to runtime mesh record `+0x10`:

```text
0x1402F9C14..0x1402F9C18
runtimeMesh+0x10 = serializedMesh
```

Direct serialized-field reads in this initializer are:

```text
serialized +0x00  u16 vertex count
serialized +0x02  u16 texture index
serialized +0x10  positions pointer
serialized +0x18  normals pointer
serialized +0x20  UV pointer
serialized +0x38  color/topology pointer
serialized +0x28  continuation span / next serialized mesh advance
```

There is no read of serialized `+0x0C` in this primary runtime-mesh materialization path.

The nearby read:

```text
argument/runtime object +0x0C
```

at `0x1402F9BEC` is a mesh/part count on a different structure. It controls the number of `0x1A0` runtime records and is not the serialized mesh field.

## Runtime helper after initialization

After copying the known serialized streams, `0x1402F9BB0` calls `0x140308C00` on the newly built runtime mesh. That helper initializes runtime-only state at:

```text
runtimeMesh +0xF0
runtimeMesh +0xF4
runtimeMesh +0xF8
```

It does not dereference serialized `mesh+0x0C`.

## Renderer-command false positives

The main command-building path contains writes such as:

```text
0x1402F81DF: command+0x0C = 0x50000003
```

and other `+0x0C` command/runtime accesses.

These bases are generated renderer records, not the serialized SCM mesh pointer. They are rejected as serialized-field evidence.

## Current bounded conclusion

The strongest evidence-backed classification is:

```text
mesh+0x0C
  RESERVED_OBSERVED_ZERO
  NO_ACTIVE_CONSUMER_IN_RECOVERED_PRIMARY_SCM_MESH_MATERIALIZATION_PATH
  PRESERVE_RAW_ON_PARSED_RESOURCES
```

This is intentionally not a whole-program `unused` claim.

A future non-zero specimen or a provenance-clean indirect consumer would supersede this classification.

## C++ / writer policy

Current IR field remains neutral:

```cpp
std::uint32_t reserved0c;
```

For parsed SCM:

```text
preserve exact raw value
```

For newly created canonical SCM:

```text
default = 0
```

Safe editing API must not expose a semantic control for this field.

## Promotion boundary

The field may be promoted beyond `reserved0c` only if at least one of the following is recovered:

1. a non-zero shipped/canonical specimen with correlated runtime behavior;
2. a provenance-clean EXE consumer from serialized mesh pointer +0x0C;
3. a recovered producer/offline writer assigning non-zero values;
4. controlled original-game mutation demonstrating a stable semantic effect.
