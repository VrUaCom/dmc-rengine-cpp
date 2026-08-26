# GDSpaces L1 — Connected Retail Artifact Access Reconciliation

**Date:** 2026-08-26  
**Review base:** `main@2ed43b438f1bf01638f3e56341e98f6085e5b0fd`  
**Layer:** L1 — Resource Materialization / external evidence access  
**Verdict:** RETAIL INSTALL ARTIFACTS LOCATABLE; EXACT NBZ MEMBER-BYTE ACQUISITION STILL BLOCKED BY CONNECTED TRANSFER BOUNDARY.

## 1. Purpose

This addendum corrects one stale evidence-access premise without weakening any L1 acceptance gate.

Older L1 status text said that the connected automation environment did not expose exact raw protected-install `dmc3.exe` / `DMC3-0.nbz` artifacts. A fresh 2026-08-26 connected-Drive census disproves the artifact-absence part of that statement.

The correct boundary is narrower:

```text
protected retail install is locatable in connected Drive
 -> protected dmc3.exe is locatable
 -> executable-relative data/dmc3/dmc3-0.nbz is locatable
 -> dmc3-0.nbz exact observed size = 960,358,951 bytes
 -> connected raw transfer/materialization cannot deliver that archive
 -> no exact central-directory/member surface is exposed separately
 -> no separately extracted em000.pac was found by connected search
 -> exact obj\em000.pac retail member-byte/provenance receipt remains OPEN
```

This is an access/transport correction, not L1-C completion.

## 2. Connected artifact observations

Fresh Drive/Library enumeration locates:

- protected install root `DMC HD Vanilla/Devil May Cry HD Collection`;
- root `dmc3.exe`, observed size `6,567,320` bytes;
- `data/dmc3/dmc3-0.nbz`, observed size `960,358,951` bytes.

The executable size is consistent with the already-recorded protected-distribution/original-execution candidate authority. This census does not replace executable SHA/preflight authority; the real run must still pass the canonical protected executable gate.

No separately extracted file named `em000.pac` / matching `em000*` was found by the connected Drive search performed during this pass. That search miss is not evidence that a member with that identity is absent from the NBZ.

## 3. Transfer/materialization negative receipt

Three independent connected paths were attempted against the exact `dmc3-0.nbz` Drive artifact:

1. **Google Drive raw fetch** — rejected because the stored archive exceeds the connected raw-transfer ceiling (`268,435,456` bytes).
2. **Files raw materialization** — failed with HTTP `413` while attempting to materialize the same archive into the execution container.
3. **Files direct read/search** — full read fails at the same download boundary; scoped semantic search exposes no parsed ZIP central-directory/member surface and therefore cannot serve as provenance authority.

The archive is `960,358,951` bytes, so the connected channel cannot currently deliver the original archive bytes to `dmc-rengine` for canonical resolver/member acquisition.

A zero-result semantic search is not evidence that a ZIP member does not exist; it only confirms that no usable parsed/member surface is available through that route.

## 4. Exact L1-C classification

### What is now proven

- the retail install artifact set is present/locatable in connected Drive;
- the protected `dmc3.exe` file is present at the expected install root;
- `data/dmc3/dmc3-0.nbz` is present;
- the exact observed NBZ file size is `960,358,951` bytes;
- connected full-byte transfer/materialization of that NBZ is unavailable through the tested channels;
- no separately exposed `em000.pac` derivative was found by the connected search that could substitute for direct archive acquisition.

### What is still not proven

- the exact resolver winner for game request `obj\em000.pac` on this retail install;
- the selected central-entry/member identity;
- selected member compression metadata / stored span;
- exact materialized `em000.pac` SHA/size/bytes;
- representation classification of those exact retail bytes;
- any real edit/rebuild/rematerialization receipt derived from them;
- original-game consumption.

Therefore **L1-C remains `REAL RECEIPT OPEN`**.

## 5. Required next acquisition seam

The next real action must preserve provenance and must not infer a member path.

Acceptable routes are:

1. run canonical `extract-dmc3-retail-member` directly on a machine with filesystem access to the protected installation; or
2. provide an authorized cryptographically bound derivative that exposes enough exact archive/member data to reproduce the canonical acquisition observation without laundering identity.

Preferred request remains:

```text
obj\em000.pac
```

The actual archive/member winner must be observed by the resolver.

A copied loose `em000.pac` without archive/member provenance is not sufficient for L1-C.

## 6. Supersession scope

This document supersedes only the stale **artifact-absence/environment-access premise** in earlier status surfaces, including the older environment-boundary wording in:

- `docs/gdspaces/l1-roadmap.md`;
- `docs/gdspaces/l1-final-audit-2026-08-25.md`;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/canonical-status.json`;
- the pre-correction body of issue #209.

It does **not** supersede their gate definitions, implementation verdicts, Level-E requirements, V/LV ownership, or completion criteria.

Issue #209 has been updated in place with the corrected connected-artifact boundary.

## 7. Promotion boundary

Do not promote any of the following from this access census:

```text
artifact locatable != artifact bytes acquired
Drive filename/size != archive SHA authority
archive present != member identity proven
semantic-search miss != member absence
loose derivative != retail archive provenance
product implementation ready != real retail receipt
real retail receipt != original-game consumption
```

`L1 COMPLETE / 100%` remains forbidden until the existing L1-C..G real receipts and final acceptance audit are valid.
