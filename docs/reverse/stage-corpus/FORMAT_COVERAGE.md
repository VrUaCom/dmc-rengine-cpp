# Stage resource reverse coverage

This is the current coverage map for the 16,790 files in the 556 `st*` folders. It separates confirmed parsers from structural-only probes.

| Role | Count | Current authority / parser | Status |
|---|---:|---|---|
| SCM geometry/model | 1495 | dmc-rengine-cpp `formats/scm.*` | TEST VERIFIED |
| HITS collision | 281 | dmc-rengine-cpp `formats/hits.*` | TEST VERIFIED |
| PNST/name index | 2093 | local text index validator | TEST VERIFIED |
| PAC/PNST container | 1019 | `formats/relative_slot_container.*` + local validator | TEST VERIFIED |
| DDS texture | 3755 | `profiles/dmc3/dds_profile.*` + local validator | STRUCTURAL / IN PROGRESS |
| stage/config text | 524 | stage TXT modules + local token scan | STRUCTURAL / IN PROGRESS |
| spawn/position table | 189 | `lib/dmc3/posSpawnParser.ts` + local validator | TEST VERIFIED |
| camera data | 189 | `lib/camCore.ts` + local structural validator | STRUCTURAL / IN PROGRESS |
| lighting data | 189 | dmc-rengine-cpp `formats/lig2.*` + local validator | TEST VERIFIED |
| motion data | 600 | dmc-rengine-cpp `formats/mot/*` + local validator | TEST VERIFIED |
| effect data | 393 | local EFM magic/prefix validator | STRUCTURAL / IN PROGRESS |
| event data | 189 | `lib/dmc3/eveParser.ts` + local validator | TEST VERIFIED |
| effect/scene data | 183 | `lib/sefCore.ts` v3 contract + versioned validator | STRUCTURAL / IN PROGRESS |
| binary data | 667 | format-specific identification required | RESEARCH REQUIRED |
| unknown | 361 | magic and path inventory only | RESEARCH REQUIRED |

## Boundary

SCM, HITS, MOT-family, LIG2 and stage TXT counts above have byte-level or grammar validators and evidence artifacts. Other families are currently catalogued by path, extension and magic; their semantic readers must be run and recorded in later passes before this task is marked complete.

Evidence files: `evidence/resource_profiles.json`, `evidence/resource_profiles.csv`, `evidence/resource_role_summary.json`, `evidence/mot_validation.json`, `evidence/lig_validation.json`, `evidence/txt_validation.json`.



Additional validation artifacts: evidence/pos_validation.json, evidence/eve_validation.json, evidence/cam_validation.json.
Additional validation artifacts: evidence/efm_validation.json, evidence/sef_validation.json.
Additional validation artifacts: evidence/container_validation.json, evidence/dds_validation.json.
Additional validation artifact: evidence/index_validation.json.

