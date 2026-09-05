# GDSpaces Proof Roadmap — 2026-09-05

**Reviewed canonical base:** `main@76841d6f1387b08df40bb65e0083513f9dc7c5bb`  
**Current integration slice:** PR #288 — evidence-backed SHW Native Reader  
**Canonical analysis EXE:** `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Rule:** implementation and proof are separate gates.

## Legend

| Mark | Meaning |
| --- | --- |
| ✅ | Claim is closed at the authority required by the claim. |
| ⚠️ | Implementation/research exists, but the required proof or canonical promotion is incomplete. |
| ❌ | Mandatory work/evidence is open. |
| ➖ | Deliberate DMC Rengine product/safety policy; original-game reverse is not the acceptance authority. |

For original-DMC3 claims, the default proof is a bounded canonical-EXE reverse pass with exact address/range provenance and contradiction review. Corpus claims additionally require hash-bound real data. Original-process claims require a trusted runtime receipt.

## A. Core resource bootstrap and resolution

| Status | Claim | Current evidence | Required next proof |
| --- | --- | --- | --- |
| ✅ | executable-relative `data\\dmc3\\` root | `0x14002E930` bootstrap reverse | none absent contradiction |
| ✅ | `%sDMC3-%d.nbz` numbered discovery starts at 0 | `0x14002E930` direct instructions | none absent contradiction |
| ✅ | discovery stops at first missing filename | canonical bootstrap reverse | none absent contradiction |
| ✅ | successful archive registrations prepend to mount list | `0x140326DA0`, head `0x140CF3180` | none absent contradiction |
| ✅ | clean archive precedence is highest numbered -> zero | ascending discovery + prepend construction + `0x140327430` traversal | none absent contradiction |
| ✅ | six-prefix request table | pointer table `0x14055AEF8` | none absent contradiction |
| ✅ | direct-call policy is six archive candidates then six physical candidates | `OpenGameResource 0x14002FCA0`, three direct callers with `EDX=1` | none for bounded direct-call claim |
| ✅ | archive normalizer is `0x0E`; physical is `0x0C` | `0x140327160` instruction-backed bit semantics | none absent contradiction |
| ✅ | archive hit requires wrapper/open success | `0x140328160 -> 0x140328290`; wrapper failure is terminal for that resolve attempt | none absent contradiction |
| ✅ | discovery and successful mount topology are distinct in product API | static reverse is closed; product correction promoted by PR #287 with sparse-mount/no-physical regressions | none for product-model claim absent contradiction; original topology still needs runtime receipt |
| ❌ | trusted original selected volume/provider/member | static reverse cannot establish a real protected-process winner | R2B mapped observer + trusted R3 receipt |

Primary static authority: `docs/gdspaces/l2-exe-reverse-pass-2026-08-26-pass2.md` and `data/reverse/dmc3-gdspaces-l2-resolver-static-census-2026-08-26.v1.json`.

## B. Archive-key collision authority

| Status | Claim | Evidence | Required next proof |
| --- | --- | --- | --- |
| ✅ | `dmc3-0.nbz` has zero `0x0E` normalized-key collisions | archive SHA `2c2302...13df`; surface SHA `061668...50af`; 4,333 file keys = 4,333 unique | none for this exact artifact |
| ❌ | every mounted DMC3 volume is collision-free | only one retail volume is bound | per-volume census for every claimed mounted volume |
| ❌ | cross-volume winner space is collision-free | not yet measured | cross-volume normalized-key census using recovered policy |

Primary receipt: `data/reverse/dmc3-nbz-archive-key-census-20260903.json`.

## C. L1 materialization

| Status | Claim | Current evidence | Required next proof |
| --- | --- | --- | --- |
| ✅ | acquisition publishes LoadedResource state1 only after materialization-dispatch success | direct reverse of `0x1401B84E0` and seam `0x1401B8CA0` | none for bounded state1 claim |
| ✅ | packed-vs-`.lst` representation selector exists; external `.index` is not runtime materialization authority in recovered path | fresh whole-image reverse pass on canonical EXE, including `0x1401B79E0`, `0x1401B8CA0`, `.lst` positive controls | none absent contradiction |
| ⚠️ | NBZ STORE/raw-DEFLATE product reader | implementation/tests are strong; only bounded original behavior is recovered | close only if broader original-equivalence claim needs `0x140328540`, `0x140328FE0`, malformed/partial-read semantics |
| ✅ | PAC/PNST typed traversal exists in original post-load path | `0x1401B92D0` + `0x1401B9FA0` | none for bounded traversal claim |
| ✅ | PAC physical slot 0 is traversed | direct recovered walk evidence | none absent contradiction |
| ⚠️ | size-changing PAC/PNST and nested authoring is usable by DMC Rengine | product writers and round-trip verification exist | original-game selection/consumption receipt of authored result |
| ❌ | real protected-install direct-retail provenance | no trusted selected original winner receipt | B-L1-01 execution |
| ❌ | exact selected retail representation classified | depends on previous receipt | classify exact selected bytes, fail closed outside writer domain |
| ❌ | real edit/rebuild/rematerialization acceptance | no same-lineage protected-install receipt | selected member -> edit -> rebuild -> overlay -> canonical rematerialization |
| ❌ | original game consumes authored bytes | no Level-E receipt | deterministic original consumer observation |
| ❌ | rollback proves retail immutability | no Level-E rollback receipt | remove test overlay + hash-check original artifacts |

Fresh `.index` authority is preserved in `docs/reverse/dmc3-index-exe-reverse-2026-09-03.md`; repository documentation must not regress to treating `.index` as original runtime manifest authority.

## D. L3 lifecycle

| Status | Claim | Current evidence | Required next proof |
| --- | --- | --- | --- |
| ✅ | state1 is post-materialization-dispatch-success | `0x1401B84E0` | none for bounded claim |
| ✅ | normal completion publishes `1 -> 2` | `0x1401B8DC0` | none for normal bounded path |
| ✅ | state2 finalization order is typed post-load -> optional callback -> state3 | `0x1401B92D0` direct reverse | none for bounded path |
| ✅ | cancellation writer changes only `1|2 -> 4` | `0x1401B8430` | none for bounded writer |
| ✅ | quiescence accepts only `{0,3}` | `0x1401B84B0` | none for bounded predicate |
| ✅ | ordinary release, cancellation cleanup and forced reset have distinct ordering | `0x1401B9530`, `0x1401B8F00`, `0x1401B9560/0x1401B95E0` | none for bounded functions |
| ⚠️ | L3-R1 whole writer census is contradiction-gated closed | final research conclusion exists on historical/reconciliation work but not current canonical `main` | current-main contradiction sweep + semantic promotion |
| ❌ | exact terminal dependency preventing premature/failed state2 publication | normal `0x1401B8DC0` receives no transport status; earlier reverse narrowed but did not close the scheduler bridge | raw canonical pass over `0x1402EF4D0`, `0x1402EF790`, `0x1400333E0`, `0x140033390`, `0x1400335A0`, `0x1402EF460` |
| ❌ | V1 initial-load receipt | no trusted original-process trace | capture |
| ❌ | V2 transition receipt | none | capture |
| ❌ | V3 reload receipt | none | capture |
| ❌ | V4 reset/menu receipt | none | capture |
| ❌ | V5 in-flight cancellation receipt | none | capture |
| ❌ | V6 shutdown receipt | none | capture |
| ❌ | V7 family/build breadth | none | aggregate accepted traces |

Primary static authority: `docs/gdspaces/l3-raw-exe-pass-2026-08-26.md`, `docs/gdspaces/materialization-completion-boundary-pass-2026-08-26.md`, `docs/gdspaces/materialization-completion-dependency-pass2-2026-08-26.md`.

## E. Native Reader / format proof

| Status | Format/capability | Proof state | Next gate |
| --- | --- | --- | --- |
| ✅ | SCM structural read | dedicated layout/runtime/hierarchy EXE+corpus evidence; canonical Native Reader module | writer/edited-game acceptance separate |
| ✅ | MOD structural read | recovered model-family grammar + hash-bound real MOD payload + canonical Native Reader module | writer/variant/edited-game acceptance separate |
| ✅ | DDS structural read | evidence-backed DMC3 profile; canonical module | texel authoring not implied |
| ✅ | PTX structural read | evidence-backed bundle/descriptor/DDS-child framing; canonical module | production authoring not implied |
| ✅ | HITS/DCA/LIG2/Stage TXT/PE readers | canonical modules at their declared evidence maturity | semantics/writer gates remain per format |
| ⚠️ | SHW structural Native Reader | implemented on PR #288 from canonical EXE + hash-bound real payload; typed hull/topology/adjacency/float4/selector parsing plus workspace routing and tests | exact-head Windows/Ubuntu CI + final diff review + promotion; writer/matrix-palette/variant breadth remain separate |
| ❌ | EFM canonical Native Reader module | reverse evidence exists but current `main` registry has no EFM reader module | fresh semantic integration onto modular registry + tests + CI |
| ❌ | MOT canonical Native Reader module | research/parser exists outside current canonical reader set | evidence audit + integration + tests + CI |
| ❌ | MRP/MCV/CAM/CLT/TSC structural readers | recognition/runtime references do not prove a common grammar | samples + consumer reverse + bounded parser evidence |

SHW proof boundary for PR #288:

- real payload SHA-256 `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e`, size 9,488;
- self-contained shadow-hull topology is data-confirmed and canonical-EXE corroborated;
- selector semantics are EXE-confirmed as per-vertex transform-matrix selection;
- matrix-palette ownership/construction, universal revision coverage, writer authority and original-game authored SHW acceptance remain open;
- synthetic tetrahedron fixtures validate product behavior only and do not replace the bound real-payload evidence.

## F. Product-only safety invariants

These are not claims about Capcom behavior.

| Status | Invariant | Acceptance |
| --- | --- | --- |
| ➖ | atomic/no-replace publication | implementation + concurrency regression + Windows/Ubuntu CI |
| ➖ | SHA/artifact-stable observation | implementation + regression |
| ➖ | ByteProvenance | implementation + regression; original-selection wording additionally needs runtime receipt |
| ➖ | fail-closed unsupported format/writer behavior | implementation + regression |
| ➖ | source retail immutability by default | implementation + Level-E rollback confirms operational use |

## G. Current execution queue

1. ✅ Reconcile the project roadmap to proof-level statuses.
2. ✅ Promote discovery-vs-successful-mount topology correction via PR #287.
3. ⚠️ Finish SHW Native Reader PR #288 with exact-head cross-platform CI/review and canonical promotion.
4. ❌ Close the materialization scheduler terminal dependency with a fresh raw canonical-EXE pass when the exact executable bytes are available.
5. ❌ Produce real R2B protected-process multi-anchor mapping.
6. ❌ Produce trusted R3 selected-provider/member identity.
7. ❌ Bind selected original member to independent SHA/materialization provenance.
8. ❌ Observe the same resource through typed post-load/state3.
9. ❌ Repeat the chain using an authored higher-numbered NBZ.
10. ❌ Record deterministic consumer effect + rollback.
11. ❌ Run final independent L1, L2 and L3 audits.

## Proof discipline

Do not promote a box because code exists. Do not promote a parser because a synthetic fixture parses. Do not promote an original-runtime claim because a string exists. Do not promote a protected-build address from the canonical analysis executable without an independent mapping receipt.

Every new ✅ must identify which of these closed it:

1. exact canonical-EXE reverse;
2. hash-bound corpus measurement using the recovered algorithm;
3. trusted original-process observation;
4. deliberate product policy with implementation/tests/CI;
5. authoring acceptance with generated artifact identity, original selection/consumption and rollback.
