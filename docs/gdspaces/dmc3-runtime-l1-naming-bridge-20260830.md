# DMC3 runtime resolver -> GDSpaces L1 naming bridge — 2026-08-30

Status: **IMPLEMENTED CONTRACT / SYNTHETIC REGRESSION — REAL RETAIL SELECTED-IDENTITY RECEIPT STILL REQUIRED**

## Purpose

This checkpoint joins two already recovered domains without collapsing their authorities:

```text
DMC3 runtime request
  -> recovered Layer 2 lookup policy
  -> RuntimeResourceResolver
  -> exact resolved ResourceRef / ResourceId
  -> RuntimeNamingBridge (exact ResourceId equality only)
  -> L1 ContainerNamingIdentitySnapshot parent
  -> physical slots
  -> extracted ordinals
  -> external .index extraction labels
  -> embedded/enclosing naming evidence
  -> semantic format evidence
  -> canonical display / legacy replay
```

The bridge does **not** make `.index` a runtime manifest and does not use a filename, display name, embedded alias or semantic name as a runtime identity key.

## Existing EXE-backed Layer 2 boundary

The canonical analysis executable authority remains the unpacked/decrypted DMC3 HD image with SHA-256:

`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Current recovered Layer 2 evidence already establishes:

- `OpenGameResource` at `0x14002FCA0`;
- three direct call sites `0x14003340A`, `0x1403380C7`, `0x1403381F7`, each using direct-call `flags = 1`;
- active direct-call policy: basename extraction, six candidate prefixes, archive phase, then physical phase;
- candidate construction through bounded helper `0x1403272C0` with capacity `0x400`;
- archive lookup uses recovered `0x0E` normalization and the ZIP/NBZ normalized sorted lookup representation;
- physical-provider path uses recovered `0x0C` normalization, joins the physical root and candidate, then reaches direct Win32 path APIs;
- recovered physical open: `CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)`;
- `.afs/` prefixes are logical namespaces; this evidence does not imply a binary AFS backend.

This is sufficient to state that runtime resource selection and legacy extraction naming are different authority domains.

## New bridge contract

`RuntimeNamingBridge` accepts:

1. a `RuntimeResolutionReport` that resolved to one exact `ResourceRef`;
2. a valid `ContainerNamingIdentitySnapshot` produced by L1 naming reconciliation.

It links them only when:

```text
runtime.resolved->id == naming.parent_resource
```

where `ResourceId` equality covers the complete physical identity fields:

- `source_id`;
- `logical_path`;
- `container_chain`;
- `offset`;
- `size`.

No fallback comparison exists.

## Explicitly forbidden joins

The bridge must never match by:

- runtime request text alone;
- runtime candidate string alone;
- basename alone;
- logical path alone when another `ResourceId` field differs;
- `ResourceRef.display_name`;
- external `.index` name;
- embedded alias;
- enclosing-container stored name;
- canonical display name;
- semantic extension.

A same-looking path from a different source/container/span is a different physical resource and fails closed as `physical_identity_mismatch`.

## Why this boundary matters

The recovered domains now line up without authority laundering:

```text
runtime semantic/resource request
        |  Layer 2 authority
        v
exact materialized container ResourceId
        |  shared physical identity
        v
PAC/PNST physical topology
        |  Layer 1 authority
        v
physical_slot_index
        v
extracted_ordinal
        v
external .index extraction label
        + embedded/enclosing evidence
        + semantic format evidence
        v
canonical GDSpaces presentation / replay
```

The executable tells us how the game searches for the container/resource. L1 tells us how the bytes inside the selected container are physically materialized and how historical extraction/semantic names are reconciled. Neither domain is allowed to invent the other.

## Regression coverage

`runtime_naming_bridge_tests` verifies:

- one resolved runtime `ResourceRef` links to the L1 naming snapshot when the exact parent `ResourceId` matches;
- a different source identity with the same visible/logical path is rejected;
- unresolved runtime lookup cannot be linked;
- an invalid/contradictory L1 naming snapshot cannot be linked.

The test is deliberately synthetic. It proves the authority boundary and fail-closed implementation, not a retail runtime observation.

## Still not proven

This pass does **not** prove:

- an original-process selected-identity receipt for a retail request such as `scr\\st001.pac`;
- the exact runtime-selected NBZ volume/member `ResourceId` for a real request;
- archive normalized-key collision behavior on the complete retail corpus;
- physical-provider parity between the portable product implementation and the original Win32 path for all edge cases;
- that `.index` is read by the original game runtime;
- that embedded aliases are runtime lookup keys;
- that effect slot-0 text is a general runtime naming manifest;
- internal PAC/PNST child lookup by the same `OpenGameResource` mechanism.

## Next evidence gates

1. Capture a direct-retail `OpenGameResource` request and exact selected provider/volume/member identity.
2. Reproduce the same request through `RuntimeResourceResolver` and require exact `ResourceId` parity.
3. Materialize the resolved container through L1 and require `RuntimeNamingBridge::link()` success with the exact same parent identity.
4. Record the resulting child physical slots, extracted ordinals and naming evidence without promoting extraction names to runtime authority.
5. Repeat across representative stage, actor/enemy, item and effect families.
6. Run global naming coverage/collision census once the raw retail member list/corpus artifact is accessible.

Until those receipts exist, this bridge is **implemented and fail-closed but not retail-runtime-complete**.
