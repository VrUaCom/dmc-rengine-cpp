# GDSpaces Layer 1 Roadmap

**Status:** INCOMPLETE / NOT 100% — PRODUCT AUTHORING CAPABILITY ADVANCED, ORIGINAL-L1 REVERSE + REAL ACCEPTANCE OPEN  
**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**Primary tracking:** #100, #182, #209

Layer 1 owns **Resource Materialization**: given an already selected resource identity, determine and reproduce the exact byte/result semantics required to materialize it, preserve provenance, safely author an evidenced representation, rebuild/repack it and prove exact rematerialization.

A strong product writer/materializer is not the same thing as exhaustive recovery of the original DMC3 materialization implementation.

## Canonical boundary

```text
L2 selected resource identity
 -> representation / logical-materialized size
 -> allocation / capacity
 -> exact byte/span acquisition
 -> final-chunk / EOF / short-read / progress semantics
 -> decompression / transforms
 -> exact destination bytes
 -> PAC/PNST/.lst nested materialization
 -> ByteProvenance
 -> bounded edit
 -> rebuild / repack / publication
 -> reopen / rematerialization
 -> exact authored byte verification
```

L1 stops at the exact byte/result boundary. Original request/scheduler/callback lifetime and `LoadedResource state 1 -> 2` publication remain L3.

## Current merged product capability

Current main has an advanced L1 product stack including:

- classic NBZ/ZIP central/member indexing;
- STORE and raw-DEFLATE materialization;
- CRC/size/SHA and explicit `ByteProvenance`;
- artifact-stable archive/member observations;
- PAC/PNST sparse/empty/alias-preserving parsing and recursive expansion;
- same-size and bounded size-changing relative-slot authoring;
- nested root-to-leaf PAC/PNST slot-path reflow;
- byte-exact untouched-sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-volume overlay authoring;
- staged reopen/rematerialization validation;
- atomic/no-replace publication;
- direct-retail resolver-driven acquisition tooling;
- current DMC3 naming identity/reconciliation architecture separating physical identity, physical slot, extracted ordinal, external `.index` evidence, embedded aliases, semantic evidence and display names;
- exact L2→L1 `RuntimeNamingBridge` by full `ResourceId` equality.

These capabilities are real product maturity. They do **not** justify `L1 COMPLETE` or `100%`.

## Original-L1 reverse status correction

Older current-status text said the internal product path was closed and only external receipts remained. Newer raw-EXE work in #245/#258/#269 shows that this is too strong as an original-L1 reverse statement.

Important confirmed/open boundaries include:

- `0x1402EF4D0` queue admission can reject an occupied slot;
- outer writer/materializer paths can ignore child enqueue/writer failure in bounded paths;
- `0x1401B8CA0` has branch-dependent/coarse boolean semantics and cannot be treated as a universal exact-all-bytes receipt;
- `0x1401B84E0` does not itself prove the later normal completion enqueue succeeded;
- planner/writer arithmetic has original 32-bit width/wrap behavior that product code must not imitate unsafely;
- scan/token ceilings are recovered bounds, not clean universal original error enums;
- original short-success behavior cannot be promoted as product byte-exactness.

Therefore:

> **L1 original materialization reverse remains bounded-open even though the product authoring path is advanced.**

Stronger raw evidence is being used as reconciliation input on this branch; old open branches remain historical sources rather than merge authority.

## L1/L3 terminal-completion seam

Current semantic cut:

```text
[L1]
representation/planner
 -> admitted byte execution
 -> native byte/result status
 -> terminal materializer result

[L3]
request/scheduler/callback lifetime
 -> normal 0x1401B8DC0 state 1 -> 2 publication
 -> typed/ready lifecycle
```

Stronger static evidence reports for admitted type-2 work:

```text
status 2 -> pending / no retirement
status 4 -> retry / no retirement
status 3 -> retire current byte job / FIFO advances
```

Only after retirement can a later admitted normal completion callback become current. Dynamic cancellation/concurrency remains L3 breadth.

## Naming / identity boundary

The merged naming architecture must remain separate from write authority:

```text
physical ResourceId / slot / bytes -> write authority
physical_slot_index                -> topology identity
extracted_ordinal                  -> extraction-order evidence
external .index                    -> historical naming evidence
embedded alias                     -> naming evidence
semantic type evidence             -> classification evidence
canonical display name             -> presentation
```

`.index` is not original runtime lookup authority. Display names or semantic suffixes cannot retarget writes.

## Gate status

### L1-A — publication integrity

**CLOSED / PRODUCT CANONICAL**

Shared staged atomic/no-replace publication remains required.

### L1-B — artifact-stable acquisition

**CLOSED / PRODUCT CANONICAL**

Archive index, member data and archive identity must describe the same stable artifact snapshot.

### L1-C — direct-retail selected-member provenance

**OPEN / REAL RECEIPT REQUIRED**

Acquire from a real protected DMC3 installation starting from the game request, not a pre-guessed archive member path. Preserve exact selected provider/volume/member identity and materialized byte provenance.

### L1-D — exact retail representation classification

**OPEN / REAL RECEIPT REQUIRED**

Classify the exact selected bytes. Do not force an unsupported representation through a convenient writer.

### L1-E — supported real edit + nested/top-level rebuild

**PRODUCT CAPABILITY PRESENT / REAL-RETAIL RECEIPT OPEN**

Use only a representation and writer domain backed by direct evidence.

### L1-F — next-volume publication + canonical reopen/rematerialization

**PRODUCT CAPABILITY PRESENT / REAL-RETAIL RECEIPT OPEN**

Require exact generated artifact identity, resolver winner and authored-byte rematerialization.

### L1-G — original DMC3 consumption + rollback

**OPEN / FINAL GAME-BACKED ACCEPTANCE**

Tracking: #209.

A crash-free launch is insufficient. The controlled run must attribute a deterministic consumer-visible effect to the authored bytes and prove rollback/retail immutability.

### L1-H — original-L1 reverse closure

**OPEN**

Remaining reverse breadth includes:

1. recursive `.lst` cycle/depth semantics where applicable;
2. recursive allocation/free lifetime and residual allocator/backend failure paths;
3. final original-L1 contradiction sweep;
4. exact completion/error semantics where an acceptance claim depends on them;
5. cross-build/profile differences if the declared compatibility scope expands.

### L1-I — final cross-stack acceptance audit

**OPEN / DEPENDS ON C..H AS ACTIVATED BY THE CLAIMED SCOPE**

No `COMPLETE / 100%` promotion until code, reverse evidence, real-retail lineage, rebuild/rematerialization, original-game consumption, rollback and final documentation agree.

## Evidence-gated freezes

- `.afs/` paths are logical namespaces; do not infer a binary AFS backend without direct evidence.
- PACK product/history code is not original DMC3 runtime authority by itself.
- Capcom offline writer equivalence is not required for DMC Rengine authoring, but must not be falsely claimed.
- Stage Ops/ModViz are downstream consumers and cannot close L1.

## Current work order

```text
1. finish current-main documentation/boundary reconciliation
2. preserve product exactness stricter than unsafe original short-success/wrap behavior
3. complete the remaining original-L1 reverse frontier required by the claimed scope
4. acquire one direct-retail selected-member lineage
5. classify the exact selected representation
6. perform one bounded supported real edit/rebuild
7. publish next-volume overlay and require exact reopen/rematerialization
8. execute #209 original-game consumption + rollback
9. run final L1 contradiction/cross-stack audit
10. mark L1 COMPLETE only after every activated mandatory gate is closed
```

## Completion rule

`L1 COMPLETE / 100%` means both the declared original-materialization semantics and the DMC Rengine product acceptance chain are closed at the stated scope. Synthetic CI, a mature writer, a resolver winner or a successful preview cannot replace the required evidence.
