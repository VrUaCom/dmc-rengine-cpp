# DMC3 HD MOD — historical 0x00200000 domain-collision note

**Status:** `REJECTED` as a serialized source-flag document name.  
**Branch:** `reverse/mod-completion-20260907`

This historical filename is misleading. The evidence originally documented here concerns **manager `+0xE0` bit `0x00200000`**, not serialized MOD object source flag `0x00200000` at object `+0x10`.

The two domains must not be merged merely because the numeric mask is equal.

## Canonical replacements

Manager/runtime bit-21 chain:

```text
data/reverse/dmc3-mod-manager-bit21-runtime-vector-20260909.json
```

Serialized object source flag `0x00200000`:

```text
docs/research/dmc3-mod-source-flag-00200000-reverse-2026-09-09.md
data/reverse/dmc3-mod-source-flag-00200000-20260909.json
```

## Domain boundary

The manager bit is raised on the separately recovered source-flag `0x00000200/0x00000400` path and gates runtime vector generation through `0x140306560`, followed by distribution to qualifying runtime objects at `+0x160/+0x164/+0x168`.

The serialized object source bit `0x00200000`, by contrast, is copied intact by `0x140302AB2/0x140302ABF/0x140302AC9` into runtime object `+0x10/+0x14`. Its distinct downstream semantic remains `PRESERVED_UNDECODED`.

Therefore:

```text
manager +0xE0 mask 0x00200000          != serialized object source flag 0x00200000
runtime object +0x304 mask 0x00200000  != proven derivative of serialized source flag 0x00200000
```

Any earlier interpretation that treated these equal masks as one evidence chain is `REJECTED`.
