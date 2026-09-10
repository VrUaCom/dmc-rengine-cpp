# DMC3 HD MOD — header +0x14 canonical runtime consumer closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Evidence follow-up:** 2026-09-10

## Serialized ABI and runtime transfer

The serialized field is a raw little-endian `u32` at MOD header `+0x14`.

Canonical MOD manager initialization at `0x1402F9570` proves:

```text
0x1402F95C2  read serialized header +0x14
0x1402F95C5  write the same u32 to model manager +0xE4
```

This is `EXE_CONFIRMED` runtime carriage. It is not a semantic name.

## Corpus falsification of the old decimal hypothesis

Observed examples include:

```text
pl000        -> 217
pl000 cloth  -> 217
id100        -> 1000000
```

They falsify the earlier universal interpretation:

```text
family * 100000 + model_set * 100 + sub_index
```

That formula remains `REJECTED`; MOD header `+0x14` must not be exposed as a universal `LegacyResourceCode`.

## Whole-image raw displacement control

A whole-executable disassembly scan contains 83 instructions with raw object displacement `+0xE4`. Offset equality is not type provenance. Width/fingerprint classification found no additional provenance-confirmed read of the model-manager `u32 +0xE4` field.

Examples already rejected include unrelated blob-relative and float-layout accesses. The first-hop hit `0x14030F8AE` is also a false positive: it writes `CMotion destination +0xE4` while the model manager is the separate second argument in `RDX/R15`.

## Typed owner-to-manager roots

Two canonical accessors expose the embedded model-manager domains:

```text
0x140089DE0 -> parent +0x80
0x140089DF0 -> parent +0x50
```

Whole-image direct call census:

```text
0x140089DE0 calls  2695
0x140089DF0 calls   131
```

Immediate caller-side use contains zero provenance-confirmed `manager+0xE4` reads.

Across 67 unique first-hop helpers, only `0x14030F850` contains any raw `+0xE4` operand; its `+0xE4` target is the first-argument CMotion object, not the manager.

## Recursive typed manager dataflow

A `.pdata`-bounded register/stack dataflow pass propagated the accessor-derived manager pointer through direct argument transfers and local spills.

Bounded result:

```text
recursive taint states                     70
unique functions reached                  49
provenance-confirmed manager+0xE4 reads    0
pointer-storage escapes                     3
```

The three storage escapes are:

```text
0x1402ECCD5  wrapper +0x08 = model manager
0x1402ECD0D  wrapper +0x10 = model manager
0x14030F8A7  CMotion +0xE8 = model manager
```

The first two wrapper domains and the CMotion domain were then audited separately because a stored pointer cannot be dismissed by caller-local dataflow alone.

## Wrapper escape closure

`0x1402ECCC0` and `0x1402ECCF0` build wrappers with model-manager pointers at `+0x08` / `+0x10`.

Their runtime methods consume known manager fields such as object count `+0xE8` and pointer `+0x108`, then enter the helper family:

```text
0x1402F7350
0x1402F73A0
0x1402F7430
0x1402F7480
0x1402F74E0
0x1402F75D0
```

No wrapper method or helper in this family reads model-manager `+0xE4`. No further manager-pointer storage escape was found in this wrapper family.

## CMotion escape closure

Both CMotion materializers store the manager as a backreference:

```text
0x14030F8A7  path A: CMotion +0xE8 = manager
0x14030FB1F  path B: CMotion +0xE8 = manager
```

The CMotion initializer `0x14030FE00` clears this pointer at `0x14030FE24` before materialization.

The canonical executable RTTI identifies the exact type:

```text
TypeDescriptor  .?AVCMotion@@
CompleteObjectLocator  0x140520018
vtable                 0x140507938
vtable methods         45
```

None of those 45 CMotion virtual methods contains an access to `this+0xE8`.

A wider motion-family scan over:

```text
0x14030E000 .. 0x140311500
```

finds zero non-stack qword reads of `[*+0xE8]`. In particular, the direct CMotion transform rebuild path `0x14030F530` operates on node count `+0x20`, node array `+0x28`, and per-node runtime state without reading the manager backreference.

## Whole-executable `qword [object+0xE8]` negative control

Only six non-stack qword reads at displacement `+0xE8` exist in the canonical executable.

Four are RTTI-classified unrelated enemy/common objects:

```text
0x140064FD8 -> CComEm000
0x14006D406 -> CComEm005
0x140070F36 -> CComEm006
0x14007DF16 -> CComEm008
```

They are `REJECTED` as CMotion/model-manager-backreference consumers.

The remaining two candidates still do not reach manager `+0xE4`:

- logical function rooted at `0x1400935F0` uses its `+0xE8` pointee as an indexed pointer array and never reads or forwards pointee `+0xE4`; its local layout also conflicts with canonical CMotion field packing (for example distinct `+0xD0` state overlaps the CMotion qword written at `+0xCC`), so it is not the stored CMotion backreference domain;
- `0x1402BBA10` reads its `+0xE8` pointee at `+0x18/+0x20` only. Even if treated as a manager-like pointer candidate, it neither reads `+0xE4` nor forwards the original pointer to a downstream call before reusing `RDX`.

Therefore the stored-pointer escapes do not reveal a hidden consumer of MOD header `+0x14`.

## Canonical conclusion

The strongest evidence-safe result is now:

```text
serialized offset/width                    STRUCTURAL_CONFIRMED
serialized +0x14 -> manager +0xE4          EXE_CONFIRMED
multi-corpus formula falsification          CORPUS_CONFIRMED / REJECTED old formula
canonical downstream manager+0xE4 consumer EXE_CONFIRMED: none found in completed typed census
high-level serialized semantic              PRESERVED_UNDECODED
writer policy                               preserve exact source u32
```

This closes the canonical runtime-consumer gate. It does **not** convert the serialized field into padding, reserved space, or a semantic identifier.

A future executable/version or new corpus may establish a semantic use, so the source value remains first-class preserved ABI.

## C++ / writer boundary

The evidence-safe contract remains the raw `runtime_metadata_u32` representation. No writer may regenerate the field from filenames, model IDs, or the rejected decimal decomposition. No-edit writeback must reproduce the exact source bits.

## Rejected hypotheses

- universal `LegacyResourceCode = family*100000 + model_set*100 + sub_index` — `REJECTED`;
- every executable access at displacement `+0xE4` consumes this manager field — `REJECTED`;
- CMotion destination `+0xE4` at `0x14030F8AE` is model-manager metadata — `REJECTED`;
- any of the four `CComEm*` `+0xE8` readers follows the CMotion manager backreference — `REJECTED`;
- canonical runtime non-consumption permits zero-normalization or semantic renaming — `REJECTED`.

## Closure

No further canonical-DMC3-HD manager `+0xE4` consumer tracing is required before the MOD unknown-field phase advances. The remaining work is semantic only if new evidence appears; the current writer contract is already determined: preserve the serialized `u32` exactly.
