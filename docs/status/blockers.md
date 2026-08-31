# Current Blockers

**Snapshot date:** 2026-08-31  
**Canonical implementation base:** `main@08231d669666d2bdfefe3d74f123600ca365cc3d`  
**Active L1 reconciliation:** PR #269 / `ada/l1-status-reconcile-20260831`  
**Latest landed naming/type checkpoint:** #268; direct instruction-level corrections are now on `main`

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md).

## P0 — GDSpaces L1 completion blockers

The old statement “no mandatory internal blocker remains” is superseded. Product capabilities are advanced, but confirmed canonical-EXE reverse leaves a bounded original-L1 frontier and naming/type validation remains open in addition to real-retail/Level-E evidence.

### B-L1-00 — Original materialization reverse closure

**Status:** OPEN / SUBSTANTIALLY NARROWED

Confirmed evidence now proves that upstream writer/setup booleans are not exact byte-completion receipts:

- `0x1402EF4D0` is type-2 enqueue admission;
- `0x1401B85C0` ignores direct-child enqueue and recursive-writer failures;
- `0x1401B8CA0` has branch-dependent boolean semantics;
- `0x1401B84E0` ignores type-3 completion enqueue failure;
- original chunk/planner arithmetic is 32-bit and wrap-prone;
- status `3` can retire admitted type-2 work without an independent actual-bytes == planned-bytes check.

Canonical evidence:

- `../gdspaces/l1-writer-failure-width-reconciliation-2026-08-28.md`;
- `../gdspaces/l1-terminal-l3-completion-seam-2026-08-28.md`;
- `../../data/reverse/dmc3-l1-writer-failure-width-2026-08-28.v1.json`;
- `../../data/reverse/dmc3-l1-terminal-l3-completion-seam-2026-08-28.v1.json`.

Remaining reverse work:

1. exact recursive `.lst` cycle/depth behavior;
2. recursive allocation/free lifetime semantics;
3. residual allocator/backend failure branches;
4. representative real `.lst` corpus only if real loose-list equivalence is claimed;
5. final original-L1 contradiction sweep.

Dynamic current-slot cancellation/concurrency and broader transition/reset/shutdown behavior remain L3 unless a concrete L1 acceptance run activates them.

### B-L1-N — Naming / type-evidence validation closure

**Status:** MAIN-LANDED / VALIDATION OPEN

The canonical #251-#262 naming architecture, semantically valid #254 contributions and #268 derived-display/runtime-type correction are now in `main`. Landing or CI does not independently prove naming completeness.

Current correction boundary:

- no-`.index` derived names are presentation only;
- exact external/index/stored names remain separate authorities;
- runtime type evidence is not one universal detector;
- the old global “exactly five runtime tags” statement is superseded;
- current main separates a three-byte registry probe, PAC/PNST dispatcher, and four-byte higher-level family-mask classifier;
- EFW/EFE remain evidence-bounded dispatcher sentinel/prefix observations until their normal consumer/schema is recovered;
- MCV evidence belongs to the separate four-byte family-mask path, not the five-tag registry probe.

Still required:

- representative retained effect-corpus replay/reconciliation;
- global naming coverage/collision report across representative PAC/PNST families;
- historical `.index` producer/extractor lineage recovered or explicitly bounded unresolved;
- real-retail runtime-selected identity -> exact L1 parent `ResourceId` receipt;
- historical extraction replay/export/reopen validation;
- final naming/type-evidence contradiction audit after current instruction-level corrections;
- exact-head Windows + Ubuntu green for the current `main@08231d6` and final #269 head.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED

Run canonical direct-retail acquisition against a protected DMC3 installation and preserve:

- protected executable authority;
- observed numbered-volume topology;
- resolver-selected volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- compression transform and ByteProvenance.

`obj\em000.pac` is a high-value request, not a predeclared archive member.

### B-L1-02 — Exact retail representation classification

**Status:** EXTERNAL EVIDENCE REQUIRED

Classify the exact bytes from B-L1-01. Do not infer writer authority from a filename, display suffix, transformed DDS/TM2 data or unrelated runtime type evidence.

If representation lies outside a supported writer domain, stop and create a bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** EXTERNAL VALIDATION REQUIRED

Required receipt:

```text
retail-selected member
 -> supported bounded edit
 -> top-level or nested bottom-up rebuild
 -> byte-exact untouched sibling validation
 -> next-contiguous NBZ
 -> canonical higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: issue #209.

The exact generated overlay must be copied into the protected installation under controlled conditions, hash-verified, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

A crash-free launch alone is insufficient.

### B-L1-05 — Final L1 cross-stack audit

**Status:** OPEN / DEPENDS ON B-L1-00, B-L1-N, B-L1-01..04

Before `L1 COMPLETE / 100%`:

- original-L1 reverse frontier is contradiction-free for the claimed scope;
- applicable naming/type validation gates are closed;
- real acquisition provenance exists;
- real representation classification exists;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves original retail immutability;
- exact-head Windows + Ubuntu CI is green;
- #100, #182, #209, code and canonical documentation agree;
- no unresolved contradiction alters the declared supported L1 scope.

## Layer 2 evidence blockers

These remain L2 closure gates. They may support L1 but are not substitutes for L1 byte/materialization or Level-E closure.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED

A cryptographically bound retail DMC3 member-name/central-directory surface is still required before a real collision census can be promoted.

### B-L2-02 — Real protected-distribution runtime RVA mapping receipt

**Status:** TOOLING INTEGRATED / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Canonical analysis executable:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432.

Protected distribution execution candidate:

- SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`;
- size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent mapping evidence.

### B-L2-03 — Trusted original-process selected-provider identity

**Status:** TOOLING AVAILABLE / REAL TRUSTED CAPTURE REQUIRED

A promoted R3 selection still requires real protected-process mapping, a trusted publisher/observer, zero-loss trace, observer artifact binding and exact mounted NBZ artifact binding.

Provider/backend failure must remain distinct from a clean miss.

### B-L2-04 — Direct-retail resolver identity receipt

**Status:** BLOCKED BY REAL RETAIL MEMBER-SURFACE EVIDENCE

Synthetic/DMCL collision results do not close a DMC3 retail selection receipt.

### B-L2-05 — Final L2 audit

**Status:** OPEN

Layer 2 remains incomplete until real-retail collision evidence, protected-runtime mapping, trusted original selected identity, exact-head CI and canonical docs agree.

## Closed / bounded L1 product capabilities

Do not reopen absent contradictory direct evidence:

- atomic/no-replace publication;
- artifact-bound archive/member stability;
- direct-retail acquisition tooling;
- STORE/raw-DEFLATE member materialization;
- PAC/PNST sparse/empty/alias-preserving parsing;
- recursive PAC/PNST expansion;
- same-size, size-changing and nested slot-path authoring;
- verified NBZ copy rebuild;
- next-volume overlay authoring and canonical reopen/rematerialization tooling;
- runtime-synth direct `0x800` vs recursive `0x40` extent distinction and zero-filled synthesized image;
- static admitted-job L1-terminal -> L3 callback eligibility seam;
- `.index` extracted-ordinal mapping rule and physical-slot separation;
- exact-`ResourceId` runtime-to-L1 naming bridge implementation;
- no-`.index` derived-display authority separation;
- runtime-type evidence separation into scoped instruction-backed paths.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` logical namespaces.
- Web DMC Rengine PACK parser code does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for safe DMC Rengine authoring.
- Stage Ops / ModViz do not count as L1 closure.
- Exhaustive malformed-input parity is separate breadth unless explicitly claimed.

## Environment blocker

The connected environment still does not expose every exact protected-install artifact/process condition required for the complete real-retail/original-process acceptance chain.

This external evidence/access limitation must not be hidden by synthetic CI or converted into a weaker completion criterion.
