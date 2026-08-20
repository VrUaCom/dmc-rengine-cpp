# Current Blockers

**Snapshot date:** 2026-08-20  
**Snapshot base:** `main` at `4cf6b34258e95bc6fde19979036c82ba0104d270`

This file lists blockers against the current reviewed product tree. It separates code blockers from artifact/evidence blockers and does not count L2/L3 work as L1 completion.

## P0 — GDSpaces Layer-1 closure

### B-L1-01 — Representative real `.lst` artifact unavailable

**Status:** ARTIFACT REQUIRED

The `.lst` grammar/materialization path and recovered 64-byte runtime-synth layout are already strongly reconstructed. Current connected Drive metadata exposes no raw filename containing `.lst`, so a fresh representative real-corpus receipt cannot currently be produced.

Do not restart parser reverse merely because the artifact is absent.

### B-L1-02 — Fresh representative real PNST/current-parser receipt

**Status:** ARTIFACT REQUIRED

PAC/PNST structural authority is strong and canonical. A fresh current-main parser receipt on a representative legal raw PNST/container artifact remains mandatory before L1 closure. Historical Phase-15 evidence supports PNST strongly but the original raw Phase12-16 packages are not currently exposed in Drive.

### B-L1-03 — Real child-to-slot / intrinsic-byte authority

**Status:** EVIDENCE REQUIRED

The typed verified nested size-changing composition seam is canonical, but representative real resources still need evidence that binds an exact intrinsic child image to the correct physical parent slot. Parser-inferred packed extents and synthetic slot names are not substitutes.

### B-L1-04 — No-loss retail NBZ serialization/repack tier

**Status:** ACTIVE CODE/VALIDATION BLOCKER

`NbzZipSource` is a materialization authority. `NbzStoreOverlayWriter` is a deterministic STORE-overlay authority. Neither is a lossless retail serialization/repack authority.

Before no-loss L1 closure, GDSpaces needs a bounded source-serialization model and metadata-preserving retail repack path covering the original ZIP envelope required by the observed corpus: local/central framing, version/time/attribute fields, raw extra/comment data, EOCD/archive comment and opaque local-region bytes where descriptors/padding/gaps may exist.

This tier must remain separate from compressor/Capcom offline-builder equivalence.

### B-L1-05 — Representative real size-changing A-to-Z round-trip

**Status:** VALIDATION REQUIRED

Required receipt:

```text
original source/member
 -> materialize
 -> nested edit
 -> size-changing bottom-up rebuild
 -> root PAC/PNST
 -> generated retail/overlay NBZ
 -> reopen
 -> canonical reparse/compare
```

Synthetic regression is insufficient for this gate.

### B-L1-06 — Controlled original-game consumption receipt

**Status:** VALIDATION REQUIRED

At least one representative authored output must be consumed successfully through the original DMC3 path under a controlled receipt. Product reopen/reparse alone cannot prove game compatibility.

## Non-blocking evidence-gated families

### AFS binary backend

`.afs/` strings are confirmed logical namespaces. A dedicated binary AFS backend is not currently evidenced on the canonical DMC3 HD path and does not block L1 absent new direct evidence.

### PACK

PACK remains evidence-gated and does not block DMC3 HD L1 absent new raw/runtime evidence that places it on the supported materialization path.

## Other major open blockers

### Original runtime/lifecycle

Full FileSlot/AsyncIO/scheduler ownership, callbacks, unresolved mode semantics, dynamic lifecycle and release/unload/shutdown behavior remain L3 work.

### SCM contradiction/reconciliation

Historical fixed-stride assumptions must not be promoted where direct executable evidence indicates pointer/offset traversal. Real-corpus reconciliation remains required before stronger SCM runtime claims.

### Stage Ops runtime semantics

Evidence-backed runtime links for camera, doors/transitions, enemies/spawns, effects/audio and events remain incomplete. This blocks richer validated editor verticals, not L1 materialization closure.

### Whole-game recompilation/equivalence

Full DMC3 decompilation, progressive recompilation, behaviorally equivalent rebuilt executable and whole-game equivalence remain long-term open milestones.

## Resolved/highly reduced L1 blockers

The following are no longer primary Layer-1 blockers in canonical `main`:

- NBZ STORE/raw-DEFLATE read/materialization;
- PAC structural parsing;
- PNST shared relative-slot structural parsing;
- recursive PAC/PNST expansion;
- ByteProvenance;
- WorkingCopy;
- same-size PAC/PNST authoring;
- validated same-size nested reintegration;
- runtime-synth size-changing PAC/PNST authoring;
- typed verified nested size-changing runtime-synth composition;
- deterministic STORE-only next-volume NBZ overlay authoring/reopen.

See [GDSpaces reverse-progress scale](../gdspaces/reverse-progress-scale.md) for the canonical completion rule.
