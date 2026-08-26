# GDSpaces V / LV — Validation & Equivalence Architecture

**Status:** architecture integration draft  
**Date:** 2026-08-26  
**Base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`

## Purpose

GDSpaces uses three decompilation/runtime ownership layers:

- **L1 — Resource Materialization**;
- **L2 — Resource Resolution**;
- **L3 — Original Runtime / Lifecycle**.

Validation is not a fourth decompilation layer. This document makes the existing canonical `[V] Validation / Equivalence` category a single project-wide validation authority for L1/L2/L3 and introduces **LV — Live Validation / Original-Process Observation** as V-owned evidence acquisition.

The architecture rule is:

```text
L1  Materialization ----\
L2  Resolution ----------+--> V  Validation / Equivalence --> promotion / COMPLETE
L3  Runtime/Lifecycle ---/                ^
                                          |
                                          LV
                               live/original-process observation
```

L1/L2/L3 produce implementation state, reverse claims and layer-local evidence. **V alone owns equivalence and completion promotion.** LV observes the original process but cannot promote claims by itself.

## 1. Canonical ownership

### L1 — Resource Materialization

Owns the contract from selected physical/container representation to exact editable/materialized bytes and supported rebuild/repack/rematerialization.

L1 may report implementation gates such as `implemented`, `bounded-closed`, `real-receipt-required` or `evidence-submitted`. L1 must not independently declare original-game equivalence.

### L2 — Resource Resolution

Owns request identity, candidate construction, normalization, provider/source/volume selection, ambiguity/fallback and exact selected identity.

L2 may report resolver implementation/reverse closure. L2 must not relabel a product resolver probe as original-process truth.

### L3 — Original Runtime / Lifecycle

Owns original FileSlot/request lifecycle beyond the minimum L1 byte-read contract, LoadedResource state transitions, typed post-load, consumer visibility, claims/ownership, cancellation/reset/release/shutdown.

L3 may report static/runtime-model reverse closure. L3 must not equate a product-ready resource with original state-3 readiness without V-accepted evidence.

### LV — Live Validation / Original-Process Observation

LV is an **evidence-acquisition plane inside V**, not `L4` and not a decompilation layer.

LV owns acquisition mechanisms such as:

- exact live process identity and module-base acquisition;
- protected-runtime RVA mapping observations;
- runtime probes, breakpoints/hooks/watchpoints where used;
- original resolver candidate/selection observations;
- FileSlot/request events;
- LoadedResource state writes;
- typed-postload enter/exit observations;
- ready callback and consumer-visibility observations;
- loader claim/release, reset, cancellation and shutdown observations;
- ordered event sequence/timestamps;
- dropped-event/overflow/instrumentation-integrity markers.

LV outputs raw/sanitized observations and **LV receipts**. An LV receipt is evidence input, not a promotion verdict.

### V — Validation / Equivalence

V is the single validation authority across all GDSpaces layers.

V owns:

- artifact/build identity validation;
- provenance and receipt integrity;
- product validation classification;
- real-corpus validation classification;
- original-process evidence binding;
- L1/L2/L3 cross-layer identity binding;
- original-vs-reconstruction comparison;
- contradiction detection;
- validation verdicts;
- promotion eligibility;
- layer/subsystem `COMPLETE` authority.

## 2. Evidence classes

V must preserve evidence strength instead of flattening every passing test into one `validated` state.

### V-A — Evidence / provenance integrity

Examples:

- SHA-256 and size binding;
- executable authority role;
- archive/member identity;
- VA/RVA/file-range provenance;
- exact PR/commit/run identity;
- receipt-schema validation;
- tamper detection;
- trusted publisher/binder identity.

V-A answers whether evidence is authentic, attributable and internally consistent. It does not by itself prove behavior.

### V-B — Product validation

Examples:

- Windows/Ubuntu CI;
- unit and integration tests;
- synthetic fixtures;
- bounded fail-closed behavior;
- deterministic parser/writer/reopen tests.

V-B proves the DMC Rengine implementation behaves according to its product contract. **V-B is not original-game equivalence.**

### V-C — Real-corpus validation

Runs the product implementation on exact legally available real artifacts bound by hash/provenance.

V-C can prove bounded parser/materializer/writer behavior on real data. It still does not automatically prove that the original executable selects, consumes or lifecycles the same resource in the same way.

### V-D — Original-process equivalence

Requires trusted original-process observation bound to exact build and resource identity.

A representative vertical receipt must be able to link:

```text
protected/original execution authority
 -> L2 original selected provider/volume/member
 -> L1 exact materialized byte identity
 -> authored/rebuilt/rematerialized identity when applicable
 -> L3 original typed-ready/consumer/lifecycle observation
 -> deterministic consumer effect where required
 -> rollback / cleanup integrity
```

### V-E — Breadth / subsystem acceptance

One good vertical receipt does not prove the whole subsystem.

V-E aggregates accepted receipts across the required families/scenarios/builds and is the final authority for layer-wide or subsystem-wide `COMPLETE` claims.

## 3. Cross-layer binding rule

Three independent PASS results are not one end-to-end proof.

A promoted cross-layer validation must prove that the relevant L1/L2/L3 observations refer to the **same validation run and same resource chain**.

Minimum binding fields where applicable:

- `validation_run_id`;
- protected/original executable SHA-256 + size + authority role;
- analysis executable SHA-256 where instruction-derived mapping is used;
- trusted LV observer/publisher identity and configuration;
- logical request identity;
- selected source/provider/volume/member identity;
- nested container/slot identity;
- L1 original/materialized/authored/rematerialized SHA-256 + sizes;
- L3 record/group/index identity where observable;
- ordered lifecycle event range;
- consumer observation/effect;
- rollback/cleanup receipt;
- child receipt hashes.

A receipt from run A must not be silently composed with unrelated receipts from runs B/C merely because names appear compatible.

## 4. Promotion model

Layer modules may submit evidence but do not own equivalence promotion.

Example state flow:

```text
L1 implementation: closed
L1 evidence: submitted
V:L1 validation: pending
```

then either:

```text
V:L1 = accepted at declared scope
```

or:

```text
V:L1 = blocked: missing original-process consumption receipt
```

The same rule applies to L2 and L3.

`COMPLETE`, `100%`, `original-equivalent`, `game-ready-equivalent` and similar promotion terms require a V-owned verdict for the declared scope.

## 5. Mandatory anti-laundering rules

### CI laundering

`Windows + Ubuntu green != original-game equivalence`.

### Corpus laundering

`real artifact parses/materializes correctly != original game selects/consumes it equivalently`.

### Static laundering

`canonical disassembly supports a model != original-process timing/order equivalence`.

### Product laundering

`GDSpaces selected X != original executable selected X` without trusted original observation.

### Receipt laundering

`schema-valid JSON != trusted observation`.

Editable fields such as `original_process=true` cannot manufacture promotion authority. Trusted origin must be non-forgeable by the ordinary import path.

### Build laundering

Canonical analysis executable VAs/RVAs must not be applied to another protected build without independently accepted runtime mapping evidence.

### Cross-run laundering

Independent L1/L2/L3 receipts do not form a vertical proof unless V verifies a shared run/resource/evidence chain.

### Effect laundering

Crash-free execution is not evidence that the authored or selected resource was consumed. Ambiguous visible effects require stronger selected-resource/consumer instrumentation.

## 6. Relationship to current ledgers

Existing validation work is preserved and reclassified under V rather than discarded:

- issue **#209** — L1 original-game consumption + rollback gate -> `V:L1 / V-D`;
- issue **#220** and PR **#221** — L2 original-process selected-provider identity -> `LV:L2 acquisition` + `V:L2 / V-D`;
- issue **#217** and PR **#218** — L3 original-process lifecycle validation -> `LV:L3 acquisition` + `V:L3 / V-D/V-E`;
- PR **#219** — protected-runtime RVA mapping tooling -> `LV acquisition infrastructure` + `V-A mapping receipt validation`;
- Windows/Ubuntu CI remains `V-B`;
- real-corpus parser/materializer receipts remain `V-C`.

This architecture does not weaken any current gate. It centralizes their promotion authority.

## 7. Canonical V vertical receipt

The exact serialized schema may evolve, but the conceptual parent receipt is:

```text
ValidationRun
|
+-- authority
|   +-- protected/original execution executable
|   +-- canonical analysis executable when applicable
|   +-- accepted runtime mapping receipt(s)
|
+-- LV
|   +-- trusted observer/publisher
|   +-- run/session identity
|   +-- ordered original-process observations
|
+-- resource_identity
|   +-- logical request
|   +-- L2 selected provider/volume/member
|   +-- nested slot/path identity
|
+-- L1
|   +-- retail/materialized bytes
|   +-- authored bytes
|   +-- rematerialized bytes
|
+-- L2
|   +-- original selected-identity receipt
|   +-- product comparison
|
+-- L3
|   +-- lifecycle/typed-ready/consumer receipt
|
+-- effect
+-- rollback
+-- child_receipt_hashes
+-- contradictions
+-- promotion_verdict
```

Child receipts remain independently auditable. The parent V receipt binds them; it must not merely copy their claims.

## 8. Completion rule

A layer may be implementation-complete while V acceptance is still open.

Examples:

- `L1 implementation = closed; V:L1 = open` is valid.
- `L2 static reverse = bounded closed; V:L2 original selection = open` is valid.
- `L3 runtime model = advanced; V:L3 lifecycle breadth = open` is valid.

A project status surface must distinguish implementation/reverse closure from V acceptance.

No percentage can override an unresolved mandatory V gate.

## 9. Non-goals

- LV is not L4.
- V does not own original game functions.
- V does not become a second resolver/materializer/runtime.
- LV instrumentation must not silently change the runtime branch being validated.
- Product hardening remains product behavior unless original evidence proves equivalence.
- No proprietary game bytes are committed as public validation evidence.
