# Current Project Status

**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@4dd9f65ec36ce27127fa56eaf1f00879ed4087c8`  
**Latest merged status correction:** PR #233 — connected retail artifacts locatable; exact NBZ/member transfer remains blocked  
**Latest merged L2 promotion:** PR #221 — selected-identity content-candidate / normalizer / artifact-binder tooling  
**Latest merged L3 promotion:** PR #230 — R1 leaf/context and derived-record alias/release authority  
**Active L2 hardening:** issue #229 / PR #236 — one-process-instance R2B/R3 evidence binding  
**Independent focused reverse:** PR #228 — materialization-completion ordering/dependency bridge  
**Primary execution program:** GDSpaces Layer 1 final acceptance + evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 STATIC/PRODUCT TOOLING ADVANCED BUT REAL R2B/TRUSTED R3 OPEN; L3 STATIC SPINE ADVANCED BUT DYNAMIC LEVEL-E OPEN; GDSpaces remains NOT COMPLETE.

## Authority split

- GitHub `main` is canonical merged implementation truth.
- Active branches/PRs are branch truth until merged.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires trusted original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact bytes, transform/decompression, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/async/LoadedResource/typed-ready/claim/reset/release/consumer behavior.
- Validation is cross-cutting; Stage Ops is downstream domain/tooling, not L3.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md).

## L1 current state

Canonical L1 implementation includes:

- NBZ classic ZIP bounded indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- recovered numbered-volume first-gap/runtime-domain behavior;
- resolver-selected direct-retail member acquisition with provenance receipt;
- shared staged atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + expansion;
- size-changing relative-slot reflow;
- root-to-leaf nested PAC/PNST slot-path authoring;
- byte-exact untouched sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-contiguous NBZ overlay authoring;
- staged canonical NBZ reopen;
- higher-volume resolver verification;
- protected distribution executable preflight;
- product closure orchestration through exact authored rematerialization.

Canonical L1 review: [Final Pre-Level-E Audit](../gdspaces/l1-final-audit-2026-08-25.md).
Connected-access correction: [L1 Connected Retail Artifact Access Reconciliation](../gdspaces/l1-connected-retail-access-reconciliation-2026-08-26.md).

### L1 mandatory remaining work

No known mandatory **internal implementation** blocker remains for the current representative DMC3-HD L1 acceptance scope.

The remaining acceptance path requires exact retail bytes from the protected installation:

```text
real retail request
 -> exact resolver winner + acquisition receipt
 -> retail representation classification
 -> one supported real edit/rebuild
 -> next-volume overlay + canonical rematerialization receipt
 -> original DMC3 consumer-visible effect
 -> rollback / original retail immutability
 -> final audit
```

Issue #209 remains the final original-game Level-E gate. `obj\em000.pac` is a high-value target, not a mandatory predeclared member identity; the runtime resolver winner is authoritative.

PR #233 corrected only the access premise: the protected install, `dmc3.exe`, and `data/dmc3/dmc3-0.nbz` are locatable in connected Drive, and the observed NBZ size is 960,358,951 bytes. The connected raw transfer/materialization channels still cannot deliver that archive, no exact parsed central-directory/member surface is exposed, and no separately provenance-bound `em000.pac` derivative was found. Therefore L1-C remains a real-receipt gate.

### Bounded open L1-support reverse breadth

The following remain real gaps but are not automatic L1 blockers unless the chosen acceptance path activates them:

- complete `0x140328540` ZIP stream initializer lifetime;
- complete `0x140328FE0` compressed seek/reset/reinflate behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle semantics and real loose-list corpus validation;
- unsupported/evidence-absent binary backends or formats.

Binary AFS and original-runtime PACK remain evidence-gated. Capcom offline writer equivalence is not an L1 completion requirement.

## L1/L3 materialization-completion seam

Merged #230 preserves the current scheduler/callback context authority. PR #228 independently narrows the remaining seam to the mechanism that prevents L3 state2 publication until required materialization work is valid.

Current focused targets are:

- `0x1402EF4D0` body/callees/queued materialization work and inherited load-context consumption;
- `0x1402EF460` pending-entry clear/rollback semantics;
- `0x1400335A0` transport error -> materialization/scheduler failure bridge;
- `.lst` child failure/return ordering through `0x1401B85C0` only if loose-list scope is activated.

No generic fan-in/outstanding-work counter is currently claimed. `0x1401B8CA0` remains an explicit mixed seam: L1 materialization result -> boolean success -> L3 state1 publication.

## L2 current frontier

Canonical merged L2 capabilities now include:

- #215/#204: type-0 physical-provider static reverse, native physical product path and controlled hit/miss/fallback receipts;
- #219: explicit-PID protected-runtime RVA acquisition and bounded multi-anchor mapping tooling;
- #221: selected-provider content-candidate contract, strict legacy normalizer, artifact-backed binder and EXE reconciliation tooling.

#221 is **merged tooling**, not trusted original-process selection evidence.

The remaining L2 closure is split into independent evidence gates:

1. **L2-R2A real-retail `0x0E` collision census** — exact cryptographically bound DMC3 retail central-directory/member-list evidence required;
2. **L2-R2B real protected-process mapping receipt** — no real protected `81c7...` multi-anchor packet has been promoted;
3. **L2-R3 trusted selected-provider identity** — requires a process-bound trusted publisher/origin mechanism and a real protected-process trace;
4. **final L2 audit** — only after retail corpus + real R2B/R3 receipts and exact-head validation agree.

Fresh canonical EXE review established an important fail-closed boundary: an archive normalized lookup hit followed by wrapper/open failure at `0x140328290` makes `0x140327430` exit through null/cleanup. It is not a lower-volume clean miss. Therefore the clean-path R3 contract supports only `miss -> selected`; provider/backend failure is terminal/fail-closed.

### #229 process-instance hardening

Review found that `PID + module base + image path` is not a sufficient long-lived Windows process-instance identity because PIDs may be reused and a later process may theoretically receive the same module base.

PR #236 versions the real evidence tooling path and propagates the OS-derived Windows process creation `FILETIME` through:

```text
ProcessMemoryWindow
 -> process-window v2 receipt
 -> R2B runtime-mapping v2 packet
 -> R3 selection/candidate v2
 -> artifact binder v2
```

The creation identity is captured with `GetProcessTimes` from the same opened process handle used for image identity and `ReadProcessMemory`. R2B child receipts must agree on PID + creation FILETIME + module base + image path. R3 candidate/binder must agree with the same R2B process instance.

Legacy v1 receipts remain historical/tooling artifacts and must not satisfy the final real R2B/R3 promotion gate once v2 is merged.

**Important remaining #229 boundary:** the future trusted runtime publisher must independently re-query the active process and reject PID reuse/creation-time mismatch. PR #236 does not manufacture trusted origin and does not itself close #229.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent mapping evidence.

## L3 current frontier

Merged #230 strengthens the canonical static lifecycle model:

- LoadedResource registry `363 x 0x48` and seven-group topology;
- normal acquisition/callback context domain bound to `index * 0x48`;
- acquisition-failure rollback writers;
- fixed-family ready-record release helpers;
- group5 stored-record aliases/release ordering;
- no new LoadedResource writer from the reviewed leaf/derived-alias classes beyond the bounded canonical surface.

The central lifecycle remains:

```text
materialization success
 -> state1
 -> normal completion state2
 -> typed post-load
 -> optional ready callback
 -> state3 consumer visibility
```

Canonical cancellation remains `1|2 -> 4`; cleanup returns `4 -> 0`; quiescence requires all records in `{0,3}`.

Still open:

- residual alias/value-flow/state-writer contradiction sweep;
- family-complete field/backing ownership;
- external factory/dependency/SCM edges;
- shared-owner breadth;
- trusted original-process dynamic V1–V7 receipts under issue #217.

For the first vertical proof, L3 only needs enough trusted original-process observation to attribute a deterministic consumer-visible result to the exact authored L1 bytes selected through the same L2 identity chain. Broader L3 completion remains separate.

## Current critical path

### L1 vertical acceptance

1. use a machine/path with direct filesystem access to the locatable protected installation, or an authorized cryptographically bound exact member surface;
2. preserve direct-retail resolver/acquisition provenance;
3. classify the exact selected representation;
4. perform one supported bounded real edit;
5. author the next contiguous NBZ and rematerialize the exact authored bytes;
6. execute #209 original-game consumption + deterministic effect + rollback;
7. run final L1 cross-stack audit;
8. mark `L1 = COMPLETE / 100%` only if every mandatory receipt is valid.

### L2 closure support

1. complete and validate #229 process-instance v2 hardening without claiming trusted origin;
2. obtain real-retail member-list/central-directory evidence and run the `0x0E` collision census;
3. capture one real protected-process R2B v2 multi-anchor packet;
4. implement/use a trusted process-bound publisher that re-checks the exact process-instance identity;
5. capture trusted R3 selected-provider identity from that same process instance;
6. bind observer + exact numbered NBZ artifacts and compare product resolution only after trusted origin is established;
7. reconcile code/docs/evidence and run final L2 audit.

### L3 support

1. finish the now-narrow residual static census without reopening already bounded core writer classes;
2. prioritize V1 initial load and V5 in-flight cancellation because they exercise the ready/cancellation boundaries needed by the first vertical proof;
3. continue V2/V3/V4/V6/V7 breadth after the first same-resource vertical receipt.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

The protected retail installation artifacts are **locatable** in the connected Drive environment, including protected `dmc3.exe` and `data/dmc3/dmc3-0.nbz`. The remaining connected blocker is transport/materialization: the 960,358,951-byte NBZ exceeds the available raw-transfer path, Files materialization/read hit the same large-file boundary, and no exact parsed central-directory/member derivative is currently exposed. Therefore connected artifact presence must not be promoted to exact member-byte acquisition or original-process evidence. Synthetic CI must not substitute for those receipts.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Connected retail access reconciliation](../gdspaces/l1-connected-retail-access-reconciliation-2026-08-26.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [L2 EXE reconciliation checkpoint](../gdspaces/l2-exe-reconciliation-2026-08-26.md)
- [L2 selected-identity runbook](../gdspaces/l2-original-selected-identity-runbook-2026-08-26.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the gate-based completion rule.
