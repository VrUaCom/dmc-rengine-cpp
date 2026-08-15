# HITS Pass 10 — Slice 16 Acquisition Matrix

**Date:** 2026-08-15  
**Status:** EXECUTION PLAN / NO NEW REVERSE CLAIM  
**Canonical executable SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

This document converts the existing Slice-16 evidence plan into exact reacquisition commands for targets whose body boundaries and hashes are already known.

The generic acquisition command is implemented in PR #98:

```text
dmc-rengine extract-exe-window <exe> <expected-sha256> <va> <size> [--hex]
```

The command is generic EXE/Reverse Core infrastructure; it does not encode HITS semantics.

## Rule: metadata first

For every known-body target:

1. run without `--hex`;
2. verify `artifact_sha256` equals the canonical executable SHA;
3. verify returned `window_sha256` equals the already recorded body SHA;
4. only then rerun locally with `--hex` if fresh disassembly bytes are required;
5. never commit raw `bytes_hex` to the public repository.

A mismatch is an evidence failure. Do not rebase addresses or accept “close enough” bytes.

## C260 — manager source initializer

Recorded authority:

- VA: `0x14005C260`
- size: `184` bytes
- expected body SHA-256: `9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7`

Metadata-only verification:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C260 184
```

Expected `window_sha256`:

```text
9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7
```

Local-only raw reacquisition after exact match:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C260 184 --hex
```

## C630 — indirect-transform builder

Recorded authority:

- VA: `0x14005C630`
- size: `257` bytes
- expected body SHA-256: `97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a`

Metadata-only verification:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C630 257
```

Expected `window_sha256`:

```text
97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a
```

Local-only raw reacquisition after exact match:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C630 257 --hex
```

## C740 — direct-table transform builder

Recorded authority:

- VA: `0x14005C740`
- size: `253` bytes
- expected body SHA-256: `0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6`

Metadata-only verification:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C740 253
```

Expected `window_sha256`:

```text
0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6
```

Local-only raw reacquisition after exact match:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C740 253 --hex
```

## C8D0 — runtime-object initializer / transform handoff

Recorded authority:

- VA: `0x14005C8D0`
- size: `205` bytes
- expected body SHA-256: `f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe`

Metadata-only verification:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C8D0 205
```

Expected `window_sha256`:

```text
f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe
```

Local-only raw reacquisition after exact match:

```text
dmc-rengine extract-exe-window <dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14005C8D0 205 --hex
```

## `0x1400594B0` — discovery target, not known-body verification

Current Slice-16 authority does **not** contain an exact canonical body size or body SHA for `0x1400594B0`.

Therefore do not create a fake exact-body command such as:

```text
extract-exe-window ... 0x1400594B0 <guessed-size>
```

and then label the result as the full function body.

A local probe window may be used only when explicitly recorded as a **probe/context window**, with its chosen size separated from function-body identity. The resulting window hash is evidence for that probe range only.

The required closure remains:

- complete body boundary;
- complete body hash;
- caller setup;
- slot-38 relative-offset consumption;
- produced pointers/counts/owner writes;
- relation, if any, to C630/C740 transform providers.

## Stage-CFG setup anchors — caller-context targets

The preserved Stage-CFG anchors are:

- modern observed C260 setup callsite: `0x14009823F`;
- legacy observed C260 setup callsite: `0x1400B6483`.

These are callsite anchors, not automatically function starts or exact function bodies.

Any acquisition around them must be labeled as a caller/context window until separate function/range evidence closes the enclosing body.

## Output preservation

Public-safe receipt data may retain:

- artifact SHA;
- artifact size;
- image base;
- VA/RVA/file offset;
- section;
- exact window size;
- window SHA;
- whether the range was `known-body` or `probe/context`;
- comparison result against an already recorded body hash.

Raw executable bytes stay local/private.

## Slice-16 promotion relation

Reacquiring C260/C630/C740/C8D0 byte-identically strengthens artifact continuity and enables fresh disassembly, but it does **not** close Stage-CFG transform-source provenance by itself.

Slice 16 remains `RESEARCH REQUIRED` until direct dataflow/caller evidence proves:

```text
Stage-CFG entry+0x01
  -> exact provider/base/object/table
  -> exact selector bounds/count
  -> C8D0 stack arg5
  -> runtime +0x20 transform
```

If the transform source is constructed, lifecycle/ownership must also be preserved.
