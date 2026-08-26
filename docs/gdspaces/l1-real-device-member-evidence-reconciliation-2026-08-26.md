# GDSpaces L1 — Real-device Member Evidence Reconciliation

**Date:** 2026-08-26  
**Layer:** L1 — Resource Materialization  
**Scope:** connected artifact access + Pocket GDS real-device member evidence bridge  
**Completion claim:** none; `L1 COMPLETE / 100%` remains gated by #209 and the final V verdict.

## 1. Why this reconciliation exists

Two facts discovered after the 2026-08-25 pre-Level-E audit narrow the external L1 boundary:

1. the protected DMC3 installation artifacts are locatable in the connected Google Drive corpus, including protected `dmc3.exe` and executable-relative `data/dmc3/dmc3-0.nbz`; the observed NBZ size is `960,358,951` bytes;
2. the connected ChatGPT/Drive transfer path cannot deliver that NBZ into the execution container because the observed raw transfer/materialization ceiling is `268,435,456` bytes.

Therefore any older wording that says the artifacts themselves are absent from the connected environment is superseded. The accurate connected-session blocker is **byte transport/materialization of the large NBZ**, not artifact discovery.

This does not weaken the protected-original-process acceptance gate.

## 2. Pocket GDS evidence bridge

`VrUaCom/pocket-gdspace` uses a mobile-minimal snapshot of the canonical GDSpaces C++ implementation behind `mobile::ArchiveSession`. Android owns SAF import/export and presentation; it does not implement an independent NBZ/PAC/PNST parser.

The relevant native sequence is:

```text
SAF-selected NBZ
 -> app-private immutable snapshot
 -> full-file SHA-256
 -> expected-SHA equality
 -> canonical NbzZipSource validation/indexing
 -> selected ResourceIdentity
 -> NbzZipSource::read(...)
 -> exact ResourcePayload bytes + ByteProvenance
 -> ArchiveSession::export_node(...)
 -> exact exported file bytes
```

Pocket GDS PR #2 adds `gdspaces.l1.member-acquisition-receipt.v1` around that existing canonical export path. The receipt is intentionally byte-free and records:

- producer app version and exact build-source revision when CI-bound;
- pinned native-upstream revision recorded by the Pocket GDS source tree;
- selected NBZ snapshot display name, SHA-256 and size;
- canonical `ResourceIdentity`;
- logical path, format/profile and container status;
- central index or nested slot identity where available;
- compression method;
- full `ByteProvenance` authority, offset, stored/materialized sizes, transform and CRC32 where available;
- exact SHA-256 and size of the bytes emitted by the canonical export path;
- explicit representation class;
- `original_game_consumption = not-claimed`.

Receipt emission fails closed if canonical materialization, resource identity, byte provenance or exact size equality is missing.

## 3. What a real Pocket receipt can prove

When a receipt is produced from the operator's actual DMC3 NBZ on the device, it is admissible evidence for the following bounded claims:

- the selected local NBZ snapshot had the recorded SHA-256 and size;
- canonical GDSpaces accepted/indexed that snapshot;
- the recorded member/resource identity was canonically materialized;
- the materialized bytes had the recorded ByteProvenance;
- the exact exported bytes had the recorded SHA-256 and size;
- the observed representation can be classified from the canonical node/container metadata.

For a target such as `obj/em000.pac`, this can close the **member-byte acquisition/materialization sub-gate** and provide direct evidence for L1-D representation classification.

## 4. What a Pocket receipt cannot prove by itself

A mobile receipt does **not** prove any of the following without additional same-lineage evidence:

- that the NBZ snapshot is the exact archive selected by the protected original process for a particular game request;
- the protected `dmc3.exe` authority or original-process resolver winner;
- a full direct-retail request-resolution receipt when L2 selected-provider identity is part of the claim;
- a real edited PAC/PNST rebuild or generated next-volume overlay unless those operations are separately receipted;
- original-game consumption;
- deterministic consumer-visible effect;
- rollback or original-retail immutability.

The first full vertical proof must preserve the same lineage:

```text
exact L2 selected identity
 -> exact L1 materialized byte identity
 -> supported L1 authoring/rebuild/rematerialization
 -> L3 acquisition/typed-ready lineage as needed
 -> deterministic original consumer effect
 -> rollback
```

An unrelated mobile PASS and unrelated protected-process PASS must not be composed by matching filenames alone.

## 5. Updated L1-C through L1-H interpretation

### L1-C — representative provenance

**Implementation closed. Real acceptance receipt still open.**

A Pocket member receipt can now supply the exact member-byte/materialization half of this gate without transferring the 960 MB NBZ through the connected channel. Resolver/original-retail selection authority must still be bound when the acceptance claim requires it.

### L1-D — representation classification

**Executable from a real Pocket receipt.**

The exact materialized resource can be classified using its canonical format/container identity and provenance. Do not infer writer authority solely from the filename.

### L1-E — real edit + PAC/PNST rebuild

**Product implementation closed; real acceptance execution still required.**

If the L1-D receipt places the target inside the supported PAC/PNST writer domain, perform the bounded edit through the canonical authoring path and preserve source/output identities.

### L1-F — next-volume NBZ + reopen

**Product implementation closed; real acceptance execution still required.**

Canonical next-volume authoring and resolver/reopen/rematerialization are already implemented. A real same-lineage closure receipt is still required.

### L1-G — original-game consumption + rollback

**OPEN. Mandatory.**

Issue #209 remains unchanged in authority. A Pocket receipt does not close this gate.

### L1-H — final cross-stack audit

**OPEN.**

Run only after the same controlled lineage satisfies C through G. Final `L1 COMPLETE / 100%` is a V verdict, not a local tool flag.

## 6. Operator path after Pocket GDS PR #2

On the device holding the actual NBZ:

```text
open actual NBZ
 -> search/navigate to representative member (prefer obj/em000.pac when suitable)
 -> Export
 -> canonical materialization occurs
 -> exact exported bytes are SHA-256 hashed
 -> JSON evidence receipt is emitted under Downloads/GDSpace Manager/
```

Preserve both the exported member and JSON receipt. The receipt can be reviewed without committing proprietary bytes to Git.

Then continue the canonical protected-install path on the machine that can run DMC3:

```text
bind selected identity / protected authority
 -> real supported edit/rebuild
 -> next-volume closure receipt
 -> controlled overlay publication
 -> protected original-game deterministic consumption
 -> rollback + retail immutability
 -> final audit
```

## 7. Canonical non-overclaim rule

Until the real receipt exists, Pocket GDS PR #2 is **tooling/readiness**, not L1-C completion evidence.

Until #209 has a valid same-lineage original-game consumption + rollback bundle, L1 remains **NOT COMPLETE**.
