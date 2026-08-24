# Current Project Status

**Snapshot date:** 2026-08-24  
**Canonical implementation base:** `main@c4920c8602dd7492b6c89e9fc8ecf8a6d8397ee0`  
**Latest main promotion at snapshot:** PR #192 — structurally validated PNST classification  
**Primary execution program:** GDSpaces Layer 1 — Resource Materialization  
**Overall status:** NOT COMPLETE; many bounded implementation/reverse slices are canonical, but real-retail provenance and original-game acceptance remain open.

## Authority split

- GitHub `main` is reviewed implementation/documentation truth.
- Active PRs are branch truth only until merged.
- Reverse evidence is valid only at its recorded artifact/range/scope.
- Synthetic CI validates product composition, not original-game equivalence.
- Recovered original-game code belongs to the Recovered Game Source Tree, not GDSpaces.

## Canonical GDSpaces layer model

- **L1 — Resource Materialization:** physical bytes -> member/container bytes -> transform -> exact materialized bytes -> nested expansion -> bounded edit -> rebuild/repack -> reopen -> validation.
- **L2 — Resource Resolution:** logical request -> candidates -> normalization -> provider/volume -> exact resource identity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/AsyncIO/LoadedResource/post-load/cache/claim/reset/release behavior beyond the minimum byte-materialization contract.
- **V — Validation:** cross-cutting receipts, not a fourth subsystem layer.

The canonical L1 execution plan is [GDSpaces Layer 1 Roadmap](../gdspaces/l1-roadmap.md). Gate closure, not a percentage, controls completion.

## Current canonical L1 capability

`main` now contains strong bounded authority for:

- NBZ classic ZIP indexing and STORE/raw-DEFLATE materialization;
- CRC/size/product-budget validation and ByteProvenance;
- PAC and PNST relative-slot parsing with sparse/empty/alias identity preservation;
- recursive container expansion;
- same-size and evidenced size-changing PAC/PNST authoring/reintegration paths;
- synthetic nested A-to-Z composition through retail-NBZ repack/reopen;
- transformed DDS-bearing texture framing and bounded size-changing authoring for the safe subset;
- original runtime non-TM2 serialized `gfxTexture` relocation compatibility for that writer output;
- recovered numbered-volume bootstrap and higher-volume precedence;
- deterministic next-volume STORE NBZ authoring;
- canonical resolver validation of generated higher-volume overrides;
- protected-distribution vs unpacked-analysis executable authority separation;
- current-main structural PNST classification hardening.

These capabilities are real, but they do not close L1 without direct-retail evidence and game execution.

## Active L1 frontier

### 1. Publication integrity

Review found a cross-stack product defect: retail-NBZ repack has a true no-replace publication seam, while CLI artifact paths still contain `exists() -> ofstream` publication logic. That pattern is vulnerable to a destination TOCTOU race and must not be called no-clobber.

Required next change: one shared atomic/no-replace publication primitive used by overlay, acquisition and evidence outputs, with Windows/Ubuntu regression coverage.

### 2. Artifact-stable retail member acquisition

Active PR #191 provides the right high-level composition:

```text
retail volume discovery
 -> VolumeBootstrapPolicy
 -> NbzZipSource
 -> RuntimeResourceResolver
 -> SourceRegistry::read
 -> member provenance receipt
```

but is **DO NOT PROMOTE** until three review blockers are corrected:

- atomic/no-replace output publication;
- archive snapshot stability binding index + selected member + archive identity;
- rejection of acquisition output inside the measured retail game tree.

`NbzZipSource` currently indexes from one file open and reopens the archive for `read()`, so provenance-grade acquisition needs an explicit artifact-stability contract.

### 3. Direct-retail representative receipt

After #191 correction, first high-value request is:

```text
obj\em000.pac
```

The canonical resolver must determine the actual winning archive member. Documentation must not predeclare `GData.afs/obj/em000.pac`; the recovered request path is basename-oriented and may resolve through another candidate/volume.

### 4. Representation classification and real edit

The exact retail bytes must be compared with the preserved transformed DDS-bearing/runtime evidence. Only an observed representation inside a proven writer domain may advance to a real edit/rebuild receipt.

### 5. Real rebuild, next-volume publication and game consumption

Required closing chain:

```text
retail-selected member
 -> exact editable child
 -> bounded edit
 -> bottom-up PAC/PNST rebuild
 -> next-contiguous DMC3-N.nbz
 -> canonical resolver/reopen/rematerialization
 -> exact byte receipt
 -> original DMC3 successful consumption
```

## Supporting EXE reverse frontier for GDS

Strong recovered boundaries that should not be restarted without contradictory evidence include bootstrap, numbered-volume registration, basename candidates, archive-first/physical-second ordering, archive normalization/indexing, ZIP read/inflate behavior, the bounded FileSlot/AsyncIO materialization spine, LoadedResource `0->1->2->3`, PAC/PNST typed traversal and the main `.lst` fallback/synthesis structure.

Still relevant open EXE targets:

- exact type-0 physical-provider final Win32 filename/open/failure semantics after `0x0C` normalization;
- complete ZIP stream initializer `0x140328540` body/lifetime;
- complete compressed seek/reset/reinflate `0x140328FE0` behavior;
- malformed/partial-read error equivalence where a promoted claim depends on it;
- dynamic `.lst` lifetime/error/cycle behavior only if real loose-container validation requires it;
- representative original-process load/reload/transition/release receipts after L1 reaches game execution.

## Explicit non-blockers and freezes

- `.afs/` namespaces do not establish a binary AFS backend.
- Historical PACK product parsing does not establish original DMC3 PACK runtime authority.
- Stage Ops/ModViz/HITS semantic work does not count as L1 closure.
- Capcom offline-packer equivalence is not required for DMC Rengine product authoring acceptance.
- `st001` remains a regression fixture, not the complete Stage model.

## Other subsystems

### EXE Editor / Recovered Game Source Tree

The recovered runtime body is substantial but not a fully behaviorally equivalent decompilation. EXE Editor should consume one canonical recovered-source/evidence tree and preserve exact binary mappings, unresolved boundaries and artifact authority roles.

### Stage Ops / Stage Semantic Graph / ModViz

These remain downstream consumers. Stage Ops owns product-side assembly/orchestration, Stage Semantic Graph represents that state, and ModViz consumes it. None may create a private resource resolver/materializer.

### HITS / collision

Many bounded functions/layouts are reconstructed and validated. Full runtime/source2/transform-provider/lifecycle and modified-resource game behavior remain separately open.

## Current work order

1. shared atomic/no-replace publication primitive;
2. artifact-stable NBZ acquisition contract;
3. correct/promote #191;
4. direct-retail `obj\em000.pac` request receipt;
5. exact retail representation classification;
6. bounded real edit + bottom-up rebuild;
7. next-volume NBZ + canonical reopen/rematerialization;
8. original-game consumption receipt;
9. final L1 cross-stack audit;
10. only then state `L1 COMPLETE`.

## Navigation

- [Canonical GDSpaces L1 roadmap](../gdspaces/l1-roadmap.md)
- [Project roadmap](../roadmap.md)
- [Blockers](blockers.md)
- [Phase map](phase-map.md)
- [Risks](risks.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)
- [Layer classification](../gdspaces/decompilation-layer-classification.md)

No statement in this document upgrades a bounded implementation, branch result or synthetic regression into subsystem-wide original-game equivalence.