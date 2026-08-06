# Binary Inspector Web → C++20 Cross-Port Plan

## Goal

Bring the useful Binary Inspector capabilities from the installed web generation into the open C++20 project as native, testable, evidence-safe domain services.

This is **behavioral cross-porting**, not a source-code transplant.

The C++20 implementation must not inherit:

- React component state as domain state;
- browser-specific file access;
- tool-local resource resolution;
- hidden write paths;
- deprecated container-editor architecture;
- UI assumptions that cannot be tested independently.

All resources still enter through GDSpaces.

## Parity matrix

| Capability | Web generation | C++20 status after Wave 1 | Notes |
|---|---:|---:|---|
| Structure regions/tree | available | implemented | `Region`, sorted stable IDs |
| Typed Field Inspector model | available | implemented | fields, kinds, values, parent-child structure |
| Ownership map | available | implemented | claims, owner lookup, conflicts |
| Coverage and unknown ranges | available | implemented | union coverage and structural gaps |
| Region conflicts | available | implemented | exact overlap ranges |
| Selection context | available | implemented | single offset and selected byte range |
| Annotations and Evidence links | available | implemented | normalized tags and Evidence IDs |
| Deterministic manifest | available | implemented | JSON domain manifest |
| Format adapters | available | implemented foundation | HITS adapter exists; more adapters remain |
| Byte diff | planned/partial UI direction | **implemented Wave 1** | deterministic offset-aligned spans |
| Entropy map | planned web capability | **implemented Wave 1** | Shannon windows and visualization bands |
| Analysis Cache | available in web generation | planned Wave 2 | hash + adapter/version/options key |
| Duplicate-offset diagnostics | available in web generation | planned Wave 2 | generic table diagnostic service |
| Unknown-region analyzer | planned | planned Wave 2 | entropy, repetition, alignment, pointer candidates |
| Binary templates | planned | planned Wave 2 | data schema, not UI-only templates |
| RVA/VA resolver view | planned | planned Wave 3 | consume EXE workspace mappings |
| Cross-reference navigation | planned | planned Wave 3 | Evidence/graph/function/resource edges |
| Patch safety view | planned | planned Wave 3 | consume GuardedPatchPlan, never write directly |
| Desktop hex UI | web UI exists | planned Wave 4 | adapter over the same C++ domain model |

## Wave 1 — Analysis foundation

Implemented in this package:

1. `aligned_byte_diff()` with equal, modified, inserted, and removed spans.
2. `entropy_map()` with configurable window/step and partial-window policy.
3. Range-based selection across regions, fields, ownership, and annotations.
4. Updated document-model documentation.
5. Cross-platform CTest coverage.

### Safety boundary

The initial diff is intentionally offset-aligned. A middle insertion is not silently resynchronized because guessed alignment can make unrelated bytes appear equivalent.

Entropy bands are visualization heuristics, not Evidence states and not format classification.

## Wave 2 — Reusable analysis services

Planned scope:

- `AnalysisCacheKey` based on resource SHA-256, adapter ID/version, analysis kind, and normalized options;
- immutable cached analysis receipts;
- generic duplicate-offset, overlap, misalignment, out-of-order, and suspicious-stride diagnostics;
- repetition and zero-run maps;
- candidate pointer tables with explicit address-space context;
- unknown-region feature vectors;
- binary template schema and registry;
- deterministic JSON export for diff and entropy results.

Exit criteria:

- no cache reuse across different hashes or adapter versions;
- malformed options fail safely;
- all diagnostics include exact ranges and machine-readable codes;
- no heuristic result is labeled confirmed automatically.

## Wave 3 — EXE and patch bridges

Planned scope:

- file offset ↔ RVA ↔ VA display bridge using the existing EXE workspace;
- selected range → known symbol, Evidence record, source mapping, and graph edges;
- patch preview showing expected bytes, target hash, overlap checks, and rollback status;
- diff against working copy and custom build outputs;
- source-to-binary navigation for recovered C++ units.

Binary Inspector remains read/analysis oriented. Actual patch compilation and execution stay in the shared guarded patch and Build & Test systems.

## Wave 4 — Native interaction layer

Planned scope:

- virtualized hex view;
- drag range selection;
- synchronized Structure, Fields, Ownership, Annotation, Diff, and Entropy panels;
- large-file paging;
- search and jump history;
- adapter diagnostics;
- Evidence and Project Graph navigation.

The UI must consume C++ domain APIs rather than reimplement analysis.

## Cross-port rules

1. Port user-visible capability, not framework-specific code.
2. Preserve stable IDs, byte ranges, Evidence links, and deterministic output.
3. A UI convenience must not become a second resolver or source of truth.
4. Heuristics remain explicitly heuristic.
5. Original files are never modified through Binary Inspector.
6. Every accepted cross-port wave requires tests on Windows and Ubuntu.
7. Web and C++ versions may evolve independently; parity is tracked per capability, not by forcing identical architecture.
