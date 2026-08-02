# Status System

The status directory separates implementation truth from long-term vision.

- `current.md` — human-readable current snapshot;
- `canonical-status.json` — machine-readable implementation snapshot;
- `phase-map.md` — phase definitions and exits;
- `blockers.md` — unresolved dependencies;
- `risks.md` — architecture/project risk register;
- `weekly/` — dated repository-backed reports.

## Update rules

Update status when:

- an implemented component enters the build;
- tests or CI change completion confidence;
- a blocker opens or closes;
- a historical feature is migrated;
- an architecture decision changes scope;
- a phase exit is met.

Do not mark historical/private functionality as implemented in this repository.
