# GDSpaces Layer 3 — Level-E lifecycle receipt runbook

**Owning roadmap:** #88 static/runtime-model reverse.  
**Dynamic acceptance ledger:** #217.  
**Schema:** `dmc-rengine.gdspaces-l3-lifecycle-trace.v1`.

Layer 3 is the **original resource runtime/lifecycle**. It starts after L1 has established the materialized resource bytes and L2 has established which provider/member was selected. Stage Ops and other tools are downstream consumers and do not own this runtime model.

## Safety boundary

Schema v1 deliberately separates three levels:

1. **structurally valid** — schema, identities, event types and scenario ordering are internally consistent;
2. **promotion content candidate** — the self-described run also satisfies the content-side acceptance predicate (`captured`, original-process claim, clean completion, no loss/overflow/intrusion, rollback when required);
3. **promotion eligible** — requires a trusted original-process publisher/binder that cannot be manufactured by editing JSON.

The current manual JSON importer can establish levels 1 and 2 only. It can **never** establish level 3. `run.original_process = true` is carried as an observation claim, not accepted as authority by itself.

This is a hard anti-laundering rule. Synthetic CI fixtures exercise parser and content predicates only. They are not Evidence Packets and must never be cited as Level-E runtime evidence.

Validate any trace:

```text
dmc-rengine validate-l3-lifecycle <trace.json>
```

Test the final promotion gate:

```text
dmc-rengine validate-l3-lifecycle <trace.json> --require-promotable
```

For schema-v1 manual imports the second form intentionally fails, even when the content is a perfect promotion candidate, because trusted runtime origin is not yet bound. A later instrumentation/publisher slice must introduce that authority through a non-forgeable type/binding rather than another editable JSON boolean.

## Identity chain

Every trace must carry all three claimed identities instead of collapsing them:

```text
original executable identity claim
  + L2 selected provider/member identity
  + L1 materialized byte identity/provenance
  -> ordered L3 lifecycle events
```

Required identity fields:

- exact executable SHA-256, size and explicit authority role;
- logical resource identity;
- exact selected-provider identity from the L2 evidence path;
- materialized SHA-256, size and provenance receipt/id from L1;
- observer name/version/config SHA-256;
- run id and original-process claim.

SHA-256 strings are canonical lowercase hexadecimal. The validator does not silently lowercase or repair them.

These fields are necessary for a future trusted binder, but self-asserting them in JSON is not sufficient to prove original-process origin.

## Event integrity

Events use a strictly increasing `sequence`. Wall-clock screenshots are not a replacement for event order. Sequence values may have gaps; gaps are not interpreted as loss by themselves. Loss is separately explicit through `observer.dropped_events` and `observer.overflow_detected`.

Known event kinds in schema v1:

```text
resource_request
materialization_submit
state_write
materialization_complete
typed_postload_enter
typed_postload_exit
ready_callback
consumer_visible
loader_claim_inc
loader_claim_dec
zero_claim_release
cancel_mark
quiescence_wait
quiescence_reached
group_reset
full_reset
backing_release
scene_boundary
shutdown_boundary
```

`state_write` is the only event that may carry `state_from` / `state_to`. Schema v1 recognizes the recovered LoadedResource state domain `0..4`; values outside it fail closed rather than being silently treated as new state semantics.

Unknown numeric observations that must remain raw can be carried under `raw_numeric`. The validator preserves the numeric domain without assigning semantic names.

## V1 — representative initial load

A captured V1 trace must establish this ordered subsequence:

```text
resource_request
 -> materialization_submit
 -> state 0 -> 1
 -> materialization_complete
 -> state 1 -> 2
 -> typed_postload_enter
 -> typed_postload_exit
 -> [optional ready_callback]
 -> state 2 -> 3
 -> consumer_visible
```

If `ready_callback` is observed, it must remain after typed post-load and before state-3 publication, matching the recovered `0x1401B92D0` ordering.

## V2 — room/stage transition

Schema v1 requires the bounded outgoing/incoming lifecycle spine:

```text
scene_boundary
 -> loader_claim_dec
 -> zero_claim_release
 -> incoming materialization_submit
 -> incoming state 2 -> 3
 -> incoming consumer_visible
```

Selective group preservation and additional cancellation events can be carried in the same trace, but the validator does not invent semantic labels for transition result codes.

## V3 — restart/reload

A captured V3 trace must prove replacement is deferred behind cleanup/quiescence:

```text
cancel_mark
 -> unfinished state 1|2 -> 4
 -> quiescence_wait
 -> deferred state 4 -> 0 cleanup
 -> quiescence_reached
 -> replacement materialization_submit
 -> replacement state 2 -> 3
 -> replacement consumer_visible
```

The wait and cleanup may overlap in time; both must complete before `quiescence_reached`, and replacement submission must occur afterward.

## V4 — return-to-menu / full reset

A captured V4 trace requires:

```text
scene_boundary -> full_reset
```

No `consumer_visible` event may occur after that full reset in the same trace. Per-record backing-release details may be recorded separately without changing the full-reset boundary.

## V5 — in-flight cancellation

A captured V5 trace requires:

- `cancel_mark`;
- unfinished `state 1|2 -> 4`;
- `quiescence_wait`;
- deferred `state 4 -> 0`;
- `quiescence_reached` only after cleanup is complete.

The canonical cancellation slice rejects `state 3 -> 4`; the recovered writer marks only unfinished states 1 and 2.

## V6 — shutdown

A captured V6 trace requires a `shutdown_boundary` and subsequent `backing_release`. Consumer visibility after the shutdown boundary fails closed.

This receipt remains distinct from ordinary group/full runtime reset. Additional scheduler, FileSlot, backend and process-destruction observations should be carried as raw/extended evidence until their exact event vocabulary is promoted in a later schema version.

## V7 — breadth is an aggregate gate

V7 is deliberately **not** accepted as a single `scope` value in schema v1. Breadth is demonstrated by aggregating trusted accepted V1–V6 receipts across `family_tags` during the final L3 audit.

This prevents one broad trace from claiming coverage it cannot prove. #217 currently requires breadth across direct, PAC-backed, nested PNST/PAC, typed post-load, fixed-topology, group-5 dynamic-pool, loader-node shared-owner and preserved common/static cases, plus exact build/profile pairing for cross-build claims.

## Aborted traces

`status = "aborted"` is allowed as a diagnostic artifact and does not need to satisfy the complete V1–V6 sequence. It is never a promotion content candidate and can never become Level-E promotion evidence.

This distinction is important: failed instrumentation runs should remain inspectable without being laundered into acceptance evidence.

## Original-process content fail-closed rules

For `status = captured` + the self-described `run.original_process = true`, the validator rejects the trace when:

- `dropped_events != 0`;
- observer overflow is detected;
- semantic intrusion is detected;
- a published overlay has no exact overlay SHA;
- a published overlay has no verified rollback.

When no overlay was published, overlay SHA/rollback claims are rejected as stale or contradictory metadata.

`completed_cleanly = false` may remain a structurally valid trace for diagnosis, but it is not a promotion content candidate.

Passing all of these checks still does **not** establish trusted origin. It only means that the imported content is shaped like a candidate that a future trusted publisher/binder could promote.

## Minimal JSON shape

The following is a **schema illustration only**, not runtime evidence:

```json
{
  "schema": "dmc-rengine.gdspaces-l3-lifecycle-trace.v1",
  "scope": "V1",
  "status": "aborted",
  "authority": {
    "exe_sha256": "<64 lowercase hex>",
    "exe_size": 1,
    "role": "example-only"
  },
  "resource": {
    "logical_identity": "example/resource.pac",
    "selected_provider_identity": "example-provider",
    "materialized_sha256": "<64 lowercase hex>",
    "materialized_size": 0,
    "materialized_provenance": "example-l1-receipt"
  },
  "observer": {
    "name": "example-observer",
    "version": "0",
    "config_sha256": "<64 lowercase hex>",
    "dropped_events": 0,
    "overflow_detected": false,
    "semantic_intrusion_detected": false
  },
  "run": {
    "id": "example-aborted-run",
    "original_process": false,
    "completed_cleanly": false,
    "overlay_published": false,
    "rollback_verified": false
  },
  "family_tags": [],
  "events": [
    {"sequence": 1, "kind": "resource_request"}
  ]
}
```

Do not copy placeholder hashes into a real receipt.

## What this slice does not prove

The validator does not instrument DMC3, discover runtime addresses, infer a cross-build address map, bind a trace to a live process, or turn a synthetic/manual trace into original-process evidence. It only establishes the fail-closed content contract into which later trusted instrumentation must publish sanitized lifecycle observations.

The next Layer-3 engineering steps remain:

```text
#88 R1/R2/R3 static closure where raw canonical EXE evidence is required
 -> trusted instrumentation/publisher bound to this receipt schema
 -> original-process V1/V5 first
 -> transition/reload/reset/shutdown traces
 -> V7 aggregate breadth
 -> final L3 audit
```
