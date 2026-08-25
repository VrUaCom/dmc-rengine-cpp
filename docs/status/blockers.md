# Current Blockers

**Snapshot date:** 2026-08-25  
**Canonical base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`

This file lists unresolved mandatory gates. Closed product foundations are recorded separately so stale documents do not restart them.

## P0 — GDSpaces L1 closure blockers

### B-L1-01 — Composite closure receipt integrity

**Status:** ACTIVE PR #212

The merged #208 closure receipt hashes the retail member but does not bind the artifact-bound acquisition sidecar that carries archive/central-entry/provenance authority. #212 makes the sidecar mandatory and records its path, size and SHA-256 in the closure receipt.

### B-L1-02 — Direct-retail representative receipt

**Status:** EXTERNAL EVIDENCE REQUIRED

Run the canonical request `obj\\em000.pac` against an exact protected DMC3 installation and record the actual resolver-selected volume/member. The connected 960,358,951-byte `dmc3-0.nbz` remains unavailable to this workspace because the connector rejects downloads above 268,435,456 bytes.

### B-L1-03 — Retail representation classification

**Status:** WAITING ON B-L1-02

Classify the exact selected retail bytes before choosing a writer. Preserved transformed DDS-bearing evidence cannot be laundered into direct-retail authority.

### B-L1-04 — Representative real edit and bottom-up rebuild

**Status:** PRODUCT HARDENING ACTIVE #210 / REAL RECEIPT REQUIRED

#210 adds recursive PAC/PNST slot-path reflow. Promotion requires whole-head CI and receipt tamper review. The real run must prove exact replacement bytes plus untouched sibling, empty-slot and alias-partition preservation at every affected level.

### B-L1-05 — Real next-volume publication and canonical reopen

**Status:** PRODUCT PATH MERGED / REAL RECEIPT REQUIRED

The #208 product chain exists. A real receipt must prove the generated first-missing volume, canonical higher-volume selection and exact rematerialized authored bytes.

### B-L1-06 — Original DMC3 consumption and rollback

**Status:** FINAL LEVEL-E GATE / ISSUE #209

A deterministic original-game consumer effect, executed overlay hash and rollback/original-file integrity receipt are mandatory. Crash-free launch is insufficient.

### B-L1-07 — Final cross-stack acceptance audit

**Status:** OPEN

Synchronize issues #100/#182/#209, code, receipts, CI, roadmap, blockers, risks, phase map and machine status. Only then may L1 be stated as COMPLETE/100%.

## Closed product foundations

- shared atomic/no-replace publication (#194);
- artifact-bound archive/member observation (#195–#197);
- first-gap retail volume semantics (#198);
- verified immutable-source NBZ copy authoring (#199);
- PAC/PNST relative-slot rebuild CLI (#201);
- protected-retail product closure orchestration (#208).

## Supporting EXE reverse gaps

- exact type-0 physical-provider final Win32 open/failure semantics after `0x0C`;
- complete ZIP initializer `0x140328540` and seek/reset `0x140328FE0` behavior where a promoted claim requires it;
- malformed/partial-read equivalence and dynamic `.lst` lifecycle only when observed as dependencies.

## Evidence-gated non-blockers

Logical `.afs/` namespaces do not prove a binary AFS backend. Historical PACK product parsing does not prove original DMC3 PACK runtime use. Neither blocks the current vertical L1 receipt absent new direct evidence.
