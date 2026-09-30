# DMC3 MOD cross-model default-joint attachment contract — 2026-09-15

## Scope

This note records the narrow read-side contract required by DMC Native Reader for cross-MOD preview placement. It does **not** claim automatic ownership between arbitrary MOD resources.

## Confirmed inputs

- MOD header `+0x13` is exposed as `Header::default_joint_index()` and is EXE-confirmed as the runtime default joint selector.
- The canonical MOD world-transform path reconstructs DMC3 row-vector hierarchy evaluation and accepts an external/root base equivalent to runtime manager `+0x1B0`.
- Given an explicit host MOD, its model-space world matrix array is canonical read-side authority when `supports_spatial_hierarchy()` succeeds.

## New ReaderCore boundary

`dmc::rengine::formats::mod::attachment::resolve_default_joint(child, host)` performs only:

1. read `child.header.default_joint_index()`;
2. build the explicit host's canonical model-space world matrices;
3. bounds-check the selector against the host matrix domain;
4. return the selected host joint matrix.

The function fails closed when the host has no canonical spatial hierarchy or the selector is outside the host joint domain.

## Non-claims

This contract does **not** infer:

- which MOD is the host;
- ownership from file names;
- ownership from load/selection order;
- actor family from `runtime_metadata_u32`;
- that every MOD must be attached through `default_joint_index`;
- that header `+0x13` is a universal root transform by itself.

Host discovery among several candidate MODs therefore remains a separate policy layer. A consumer may auto-resolve only when its candidate policy produces one unambiguous host; otherwise it must preserve source coordinates or require explicit user/runtime context.

## Native Reader integration rule

Native Reader may consume this ReaderCore resolver but must not duplicate its selector/world-matrix semantics. The source-local `RenderScene` remains authoritative. Cross-MOD placement modifies only the derived composite projection and can be reset without reparsing the resource.
