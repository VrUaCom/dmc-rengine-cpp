# Status System

Current status is gate-based and must stay synchronized with canonical `main`.

- [`current.md`](current.md) — human-readable current snapshot;
- [`canonical-status.json`](canonical-status.json) — machine-readable current gate state;
- [`phase-map.md`](phase-map.md) — subsystem/dependency phase map;
- [`blockers.md`](blockers.md) — unresolved mandatory gates;
- [`risks.md`](risks.md) — architecture/project risk register;
- [`../gdspaces/l1-roadmap.md`](../gdspaces/l1-roadmap.md) — canonical GDSpaces L1 execution/completion roadmap;
- [`../gdspaces/master-roadmap.md`](../gdspaces/master-roadmap.md) — canonical L1/L2/L3 dependency roadmap;
- `weekly/` — dated repository-backed reports and historical snapshots.

## Truth rules

- GitHub `main` is implementation truth.
- Open PRs remain branch truth until merged.
- Reverse evidence is artifact/range/scope bound.
- Synthetic CI is bounded product/tool validation, not original-game equivalence.
- Real-device member evidence is not automatically original selected-provider or consumer evidence.
- V/LV and RCP are cross-cutting control/validation structures, not L4.
- Historical evidence/pass documents remain chronology; current status is superseded by explicit reconciliation rather than silent rewrite.

## Update rule

When a material GDSpaces gate, reverse boundary, real receipt or implementation promotion changes current truth, synchronize in the same documentation pass:

- root `README.md` / `docs/README.md` / `docs/roadmap.md` when their project-level summaries are affected;
- this complete status set;
- `docs/gdspaces/l1-roadmap.md`;
- `docs/gdspaces/master-roadmap.md`;
- the relevant L2/L3 roadmap/review successor;
- `docs/gdspaces-contract.md` when an architecture contract changes;
- canonical Google Drive Architecture / Layer Classification / Technical Status / Audit documents;
- issues #100, #182 and #209 when their acceptance wording changes;
- relevant L2/L3 tracking ledgers.

The current synchronization baseline is merged PR #242 plus merged #233/#235 authority. Pending #226/#238/#240/#241 must remain explicitly pending until merged.

Documentation synchronization itself never creates `L1 COMPLETE`, `L2 COMPLETE` or `L3 COMPLETE`.
