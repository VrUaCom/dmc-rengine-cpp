# Status System

Current status is gate-based and must stay synchronized with canonical `main`.

- [`current.md`](current.md) — human-readable current snapshot;
- [`canonical-status.json`](canonical-status.json) — machine-readable current gate state;
- [`phase-map.md`](phase-map.md) — subsystem/dependency phase map;
- [`blockers.md`](blockers.md) — unresolved mandatory gates;
- [`risks.md`](risks.md) — architecture/project risk register;
- [`../gdspaces/l1-roadmap.md`](../gdspaces/l1-roadmap.md) — canonical GDSpaces L1 execution/completion roadmap;
- `weekly/` — dated repository-backed reports and historical snapshots.

## Update rule

When a material L1 gate changes, synchronize in the same documentation pass:

- root `README.md`;
- `docs/README.md`;
- `docs/roadmap.md`;
- this status set;
- `docs/gdspaces/l1-roadmap.md`;
- `docs/gdspaces-contract.md` if an architecture contract changed;
- issues #100/#182 when their acceptance wording changes.

Historical evidence/research documents should remain historical. Do not rewrite old receipts to look current; add supersession/reconciliation notes instead.

Branch/PR truth is never reported as canonical `main` truth. Synthetic CI is never reported as original-game equivalence.