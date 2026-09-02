# Status System

Current status is gate-based and must stay synchronized with canonical `main` plus any explicitly reviewed reconciliation branch being proposed for merge.

- [`current.md`](current.md) — human-readable current snapshot;
- [`canonical-status.json`](canonical-status.json) — machine-readable current gate state;
- [`phase-map.md`](phase-map.md) — subsystem/dependency phase map;
- [`blockers.md`](blockers.md) — unresolved mandatory gates;
- [`risks.md`](risks.md) — architecture/project risk register;
- [`../gdspaces/master-roadmap.md`](../gdspaces/master-roadmap.md) — canonical cross-layer dependency roadmap;
- [`../gdspaces/l1-roadmap.md`](../gdspaces/l1-roadmap.md) — Layer 1 Resource Materialization roadmap;
- [`../gdspaces/l2-roadmap.md`](../gdspaces/l2-roadmap.md) — Layer 2 Resource Resolution roadmap;
- [`../gdspaces/l3-roadmap.md`](../gdspaces/l3-roadmap.md) — Layer 3 Original Runtime/Lifecycle roadmap;
- `weekly/` — dated repository-backed reports and historical snapshots.

## Current layer status

```text
L1  INCOMPLETE / NOT 100%
    advanced product materialization/authoring capability
    original byte/result reverse + real acceptance open

L2  ADVANCED / INCOMPLETE
    resolver/candidate/mapping/binder tooling strong
    successful-mount topology correction + trusted real evidence open

L3  INCOMPLETE
    R1 static writer census bounded-closed / contradiction-gated
    R2 field/backing ownership ACTIVE
    trusted lifecycle validation/receipts open
```

## Update rule

When a material boundary, gate or authority status changes in **any** layer, synchronize the same documentation pass across:

- `docs/gdspaces/master-roadmap.md`;
- the affected `l1-roadmap.md`, `l2-roadmap.md`, `l3-roadmap.md`;
- `docs/gdspaces/decompilation-layer-classification.md` when ownership/seams change;
- this status set (`current`, `blockers`, `phase-map`, `risks`, `canonical-status.json`);
- `docs/README.md` and `docs/roadmap.md` when navigation/project priority changes;
- affected issues/PR ledgers;
- `docs/gdspaces-contract.md` only when the product architecture contract itself changes.

Historical evidence/research documents remain historical. Do not rewrite old receipts to look current; add a current reconciliation/supersession document instead.

## Authority discipline

- Branch/PR truth is never silently reported as merged `main` truth.
- A stale open branch should be semantic-ported onto current main rather than mechanically merged when the base/architecture has diverged.
- Synthetic CI is never original-game equivalence.
- A percentage is never a completion authority.
- Same-looking names are not identity; cross-layer joins require the exact identity/provenance contract declared by the layer boundary.
