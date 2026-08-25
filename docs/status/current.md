# Current Project Status

**Snapshot date:** 2026-08-25  
**Canonical implementation base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`  
**Latest L1 promotion at snapshot:** PR #208 — protected retail authoring closure receipt  
**Primary execution program:** GDSpaces Layer 1 — Resource Materialization  
**Overall status:** NOT COMPLETE; the product-side L1 authoring/materialization chain is strong, but representative direct-retail evidence and original-game Level-E acceptance remain mandatory.

## Authority split

- GitHub `main` is reviewed implementation/documentation truth.
- Active PRs are branch truth only until merged.
- Reverse evidence is valid only at its recorded artifact/range/scope.
- Synthetic CI validates product composition, not original-game equivalence.
- Recovered original-game code belongs to the Recovered Game Source Tree, not GDSpaces.

## Canonical GDSpaces layer model

- **L1 — Resource Materialization:** physical bytes -> archive/member bytes -> transform -> exact materialized bytes -> nested expansion -> bounded edit -> rebuild/repack -> reopen -> byte/provenance receipt -> original-game consumption receipt.
- **L2 — Resource Resolution:** logical request -> candidates -> normalization -> provider/volume -> exact resource identity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/AsyncIO/LoadedResource/post-load/cache/claim/reset/release behavior beyond the minimum byte-materialization contract.
- **V — Validation:** cross-cutting receipts, not a fourth subsystem layer.

The canonical L1 roadmap remains gate-based. A percentage cannot override an unclosed mandatory gate.

## Current canonical L1 capability

Current `main` contains reviewed bounded authority for:

- NBZ classic ZIP indexing and STORE/raw-DEFLATE member materialization;
- CRC/size/product-budget validation and explicit ByteProvenance;
- artifact-bound NBZ serialization/member observation so selected member bytes are bound to an exact archive identity;
- PAC and PNST relative-slot parsing with sparse/empty/alias identity preservation;
- recursive PAC/PNST expansion;
- same-size and size-changing PAC/PNST authoring/reintegration inside proven product writer domains;
- synthetic nested A-to-Z edit -> PAC/PNST rebuild -> NBZ rebuild/reopen regression;
- transformed DDS-bearing texture framing and bounded size-changing authoring for its safe subset;
- recovered numbered-volume bootstrap and higher-volume precedence;
- deterministic next-contiguous STORE NBZ overlay authoring;
- shared atomic/no-replace artifact publication used by the active L1 artifact seams;
- resolver-based direct-retail member acquisition with exact archive/member/materialized-byte receipt;
- verified immutable-source NBZ copy authoring;
- protected-distribution executable preflight authority separation;
- merged `verify-dmc3-l1-authoring` orchestration from #208: protected preflight -> direct-retail acquisition -> PAC/PNST rebuild -> next-volume overlay -> canonical resolver/rematerialization -> closure SHA receipt.

These capabilities make the product-side L1 chain substantially complete at the currently implemented PAC/PNST authoring scope. They do not prove original DMC3 consumption.

## L1 gate reconciliation after #208

### L1-A — publication integrity

**Status: CLOSED / MERGED.**

#194 established shared atomic/no-replace publication and corrected overlay staging/publication. Acquisition, rebuild and closure seams use non-destructive output policy. Retail source trees remain protected.

### L1-B — artifact-stable retail member acquisition

**Status: CLOSED / MERGED PRODUCT CONTRACT.**

#195-#198 bind selected member materialization to an exact observed archive identity, cover STORE and raw-DEFLATE, preserve first-gap runtime semantics for reads and publish exact acquisition receipts outside the protected retail tree.

### L1-C — direct-retail representative provenance

**Status: OPEN / EXTERNAL RETAIL RECEIPT REQUIRED.**

The command path is implemented, but closure still requires a receipt produced from a real protected DMC3 contiguous volume set. `obj\\em000.pac` remains the highest-value first request unless actual runtime/corpus evidence selects a better deterministic representative.

### L1-D — retail representation classification

**Status: OPEN / DEPENDS ON REAL RETAIL BYTES.**

Exact selected retail bytes must be classified before a writer domain is promoted for that resource. Preserved transformed/runtime texture evidence must not be laundered into retail authority.

### L1-E — bounded real edit and bottom-up rebuild

**Status: PRODUCT WRITERS STRONG / REAL RECEIPT OPEN.**

PAC/PNST size-changing reflow is merged. PR #210 is the active product-hardening slice for recursive root-to-leaf slot-path authoring so nested PAC/PNST edits can rebuild all ancestors bottom-up without manual staging. A real-retail edit receipt remains required.

### L1-F — next-volume publication and canonical reopen

**Status: CLOSED AS PRODUCT PATH / REAL RECEIPT OPEN.**

#208 composes next-volume overlay authoring, canonical runtime resolver mounting/order, exact rematerialization and authored-slot comparison into one fail-closed closure command. Real-retail execution of that path is still required by L1-C/E/G.

### L1-G — original DMC3 consumption

**Status: OPEN / FINAL LEVEL-E GATE.**

Issue #209 is the explicit acceptance contract. A generated overlay must be copied under the first-missing contiguous volume name into a controlled protected-distribution test installation, hash-verified, consumed by the original game through a deterministic resource path, and removed with rollback/original-file integrity evidence. Product reopen or absence of a crash is not sufficient.

### L1-H — final cross-stack acceptance audit

**Status: OPEN.**

After the real-retail and Level-E receipts exist, reconcile #100, #182, #209, this status surface, blockers/risks/phase-map/machine status and exact-head CI. Only then may the project state `L1 COMPLETE` / `100%`.

## Current critical path

```text
#210 nested PAC/PNST slot-path product hardening
 -> integrate slot-path into protected-retail closure command
 -> run representative protected-retail acquisition/classification/edit/rebuild/reopen
 -> run #209 original-game Level-E consumption + rollback
 -> final L1 acceptance audit/status synchronization
 -> L1 COMPLETE
```

No generic AFS/PACK work, Stage Ops work or unrelated L2/L3 reverse should displace this chain unless direct evidence makes it a dependency.

## Supporting EXE reverse frontier for GDS

Strong recovered boundaries that should not be restarted without contradictory evidence include numbered-volume bootstrap, basename candidate construction, archive-first/physical-second ordering, archive normalization/indexing, ZIP direct/inflate behavior, the bounded FileSlot/AsyncIO materialization spine, LoadedResource `0->1->2->3`, PAC/PNST traversal and the main `.lst` fallback/synthesis structure.

Still relevant bounded reverse targets include exact type-0 physical-provider Win32 final-open semantics, complete ZIP stream initializer/reset paths and malformed/error equivalence where a promoted claim depends on them. These do not replace the current L1 retail/Level-E acceptance gate.

## Explicit non-blockers and freezes

- `.afs/` logical namespaces do not establish a binary AFS backend.
- Historical PACK product parsing does not establish original DMC3 PACK runtime authority.
- Stage Ops/ModViz/HITS semantic work does not count as L1 closure.
- Capcom offline-writer equivalence is not required for DMC Rengine product authoring acceptance.
- Product safety limits such as recursion/output budgets are not original-runtime semantic claims.

## Other subsystems

EXE Editor / Recovered Game Source Tree remains the executable-backed semantic authority. Stage Ops consumes GDSpaces materialized identities and must not create another resolver/materializer. ModViz remains downstream of Stage Ops semantic state.

## Navigation

- [Canonical GDSpaces L1 roadmap](../gdspaces/l1-roadmap.md)
- [Project roadmap](../roadmap.md)
- [Blockers](blockers.md)
- [Phase map](phase-map.md)
- [Risks](risks.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)
- [Layer classification](../gdspaces/decompilation-layer-classification.md)

No statement in this document upgrades a product implementation, branch result or synthetic regression into subsystem-wide original-game equivalence.
