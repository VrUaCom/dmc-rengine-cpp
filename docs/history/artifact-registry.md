# Historical Artifact Registry

This registry records known historical analysis and project artifacts without embedding proprietary content. Paths may refer to previous private/local workspaces and are retained only to preserve provenance.

## Repository/source snapshots

### Legacy engine snapshot

- historical name: `DMC 3 RENGINE.zip`;
- later workspace label: `copy-of-extrem-dmc-engine-5.0.11`;
- recorded size: approximately 23,812 files;
- status: private legacy source snapshot;
- migration policy: audit and port responsibilities selectively; do not import wholesale.

### Item Editor phase package

- historical role: Item Editor Phase 2 implementation snapshot;
- status: private/local artifact;
- migration focus: schema, validation, guarded patch model, tests, and user-proven behavior.

## EXE research artifacts

### Canonical executable identity

- local historical path: `/mnt/data/dmc3.exe`;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- repository status: prohibited binary; hash metadata allowed.

### Phase 16 package

Historical package:

- `/mnt/data/dmc3_exe_texture_resource_phase16_complete.zip`

Recorded contained outputs:

- `dmc3_exe_phase16_texture_resource_architecture/docs/dmc3_exe_texture_resource_architecture_phase16.md`;
- `analysis/phase16_texture_resource_architecture.json`;
- `analysis/phase16_texture_sample_analysis.json`;
- `analysis/phase16_texture_function_map.json`;
- `annotations/dmc3_exe_phase16_annotations.json`;
- `annotations/dmc3_phase16_texture_sample_annotations.json`;
- recovered C/C++ seed units for PTX runtime/cache, parsed entries, texture objects, DDS validation, and loaded-resource views.

Status: private historical research artifact. Sanitized structural conclusions may be migrated; copied binary content may not.

## MCP/SDD workspace artifacts

Historical local server:

- `C:\Users\ADA\AppData\Local\DMC-Rengine\MCP-Space-DMC-Rengine`;
- endpoint: `http://127.0.0.1:7331/mcp`;
- workspace: `C:\Users\ADA\Documents\DMC Rengine Workspace`;
- UI executable: `C:\Users\ADA\AppData\Local\DMC-Rengine\MCP-Space-DMC-Rengine-UI\MCP Space DMC Rengine.exe`.

Historical Obsidian vault:

- `H:\Mi unidad\DMCRO Memori\DMC Rengine`.

Historical Spec Kit root:

- `C:\Users\ADA\Desktop\Moding DMC\DMC 3 RENGINE`;
- Constitution: `.specify\memory\constitution.md`;
- first spec: `specs\001-item-y-rotation`.

Status: private development infrastructure. Public repository specifications supersede local-only descriptions where both exist.

## Backups

Recorded backups:

- `C:\Users\ADA\Documents\DMC Rengine Backups\pre-spec-kit-20260726-130039`;
- `C:\Users\ADA\.codex\config.toml.backup-20260726-113953`.

Status: local/private; never publish credentials or machine-specific configuration.

## Registry fields for future artifacts

Every new analysis artifact should record:

- stable artifact ID;
- title and role;
- creation date;
- producing tool/version;
- source artifact hash;
- output hash;
- confidence scope;
- public/private classification;
- legal/content classification;
- related claims/specifications;
- superseded-by relationship;
- storage location without credentials.

## Public migration rule

A public artifact should be generated from synthetic fixtures or sanitized metadata. If reproduction requires original game files, the repository should provide a tool and instructions that generate the report locally rather than publishing the source files.
