# DMC3 HD MOD header +0x13 — default joint index

Date: 2026-09-07

## Scope

This pass closes the MOD-specific semantic role of serialized header byte `+0x13` without broadening that conclusion to EFM/SCM or granting writer authority.

Canonical executable:

- `dmc3.exe`
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The field had already been structurally materialized as `Header::runtime_mode_byte`. Direct executable consumer evidence now supports the stronger MOD-specific semantic name **default joint index**.

## 1. Serialized header -> runtime manager

Shared model-manager initialization copies serialized header `+0x13` into manager storage:

```text
0x1402F960E  movzx eax, byte ptr [resource + 0x13]
0x1402F9616  mov   word ptr [manager + 0xFA], ax
```

The serialized authority is one byte. The manager stores the zero-extended value in its `+0xFA` slot.

## 2. `JntNo` parser fallback

The canonical executable contains the literal parser key:

```text
0x140507884  "JntNo"
```

The parser path beginning at `0x14030AFBC` matches that key. On a match it:

```text
0x14030AFEA  parse integer value
0x14030AFF3  store parsed value in runtime record +0x07
0x14030B005  read manager +0xEA node/joint-domain bound
0x14030B00C  compare parsed selector against that bound
```

If the parsed selector exceeds the accepted bound, the parser substitutes manager `+0xFA`:

```text
0x14030B01B  read byte manager +0xFA
0x14030B022  write fallback to runtime record +0x07
```

Because manager `+0xFA` comes directly from serialized MOD header `+0x13`, this proves the header byte is the default/fallback selector used for `JntNo`.

This pass deliberately does not call it a `root bone`: the executable proves fallback/default selection, not root-hierarchy semantics.

## 3. Current-world consumer

`0x1402FD040` independently consumes the same manager field as a node/world-matrix selector. In both primary and fallback-manager branches it performs the equivalent of:

```text
index = manager->field_FA
matrix = manager->currentWorld[index]   // stride 0x40
translation_row = matrix + 0x30
```

Representative instructions:

```text
0x1402FD0A3  movzx ecx, word ptr [manager + 0xFA]
0x1402FD0AA  imul  rcx, rcx, 0x40
0x1402FD0B5  mov   rdx, [manager + 0x188]
0x1402FD0BC  add   rdx, rcx
0x1402FD0C2  add   rcx, 0x30
```

The alternate manager branch repeats the same `+0xFA -> *0x40 -> +0x188` pattern.

This independently confirms that the field is a joint/node selector into the canonical current-world matrix domain.

## 4. Canonical C++ exposure

The raw member `runtime_mode_byte` is retained temporarily for source/API compatibility with the previous structural promotion. `formats::mod::Header` now exposes:

```cpp
[[nodiscard]] constexpr std::uint8_t default_joint_index() const noexcept;
```

This makes the recovered semantic contract available to callers without forcing a broad parser/storage rename in the same evidence slice.

The common `model_family::DocumentCoreAbi::runtime_mode_byte_field` also remains neutral. The MOD proof does not authorize transferring the same semantic name to EFM or SCM.

## 5. Header +0x14 remains open

The adjacent serialized `u32 +0x14` is copied to manager `+0xE4` by the same common initialization path. A bounded census of the model subsystem did not establish a MOD-specific downstream read of that manager field.

Therefore:

- storage/flow of `+0x14`: `EXE_CONFIRMED`;
- MOD semantic meaning of `+0x14`: `PRESERVED_UNDECODED`.

SCM corpus work has a legacy resource/provenance-code interpretation for its homologous field. That semantic is **not** transferred to MOD by layout analogy.

## Evidence status

- serialized MOD header `+0x13` -> manager `+0xFA`: `EXE_CONFIRMED`;
- `JntNo` fallback to manager `+0xFA`: `EXE_CONFIRMED`;
- manager `+0xFA` indexing `currentWorld[]`: `EXE_CONFIRMED`;
- MOD semantic contract `default_joint_index`: `EXE_CONFIRMED`;
- claim that it is necessarily the hierarchy root: `REJECTED_AS_UNPROVEN`;
- MOD header `+0x14` detailed semantics: `PRESERVED_UNDECODED`.

## Explicit non-claims

This pass does not establish:

- that `+0x13` must always select the hierarchy root;
- cross-family equivalence for EFM/SCM header `+0x13`;
- safe mutation ranges or edited-game acceptance;
- writer authority;
- the detailed semantic role of MOD header `+0x14`;
- complete animation/current-pose ownership.
