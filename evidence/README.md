# Public Evidence Registry

This directory contains sanitized, versioned evidence metadata. It does not contain game binaries, extracted assets, leaked source code, or large copied binary regions.

## Known targets

- [`dmc3-hdc-phase12.evidence.json`](known-targets/dmc3-hdc-phase12.evidence.json) — canonical executable hash and confirmed PE/stage metadata migrated from Phase 12 research.

## Rules

- identify local artifacts by SHA-256 and role;
- preserve confidence and correction history;
- use decimal integers in JSON for offsets/RVA/VA and include hexadecimal forms in summaries or notes;
- do not invent unknown artifact sizes or fields;
- zero size means the historical public migration record did not preserve the exact size and must be regenerated locally before strict matching;
- every record references an artifact declared in the same packet;
- partial maps are explicitly labeled partial;
- evidence metadata does not make a feature implemented.

## Planned tooling

- strict JSON import and diagnostics;
- schema validation in CI;
- locally generated known-target reports;
- Reverse Canon indexes;
- correction-chain validation;
- evidence-to-Binary-Inspector region import.
