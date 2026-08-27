# GDSpaces Layer 1 Roadmap

**Status:** **INCOMPLETE / NOT 100% — active original-runtime reverse + real acceptance open**  
**Snapshot date:** 2026-08-28  
**Canonical implementation base:** `main@94692e8f9971cf8249b4b16ee88d309de8b49f11`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes  
**Primary tracking:** #100, #182, #209; reverse frontier from #244 plus 2026-08-28 raw follow-up  
**Latest L1 layout promotion:** #255

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

The previous wording `INTERNAL PRODUCT PATH CLOSED` is superseded. Product implementation is advanced and many representative paths are green, but fresh canonical-EXE review proved that the original materialization reverse was not exhaustive and exposed additional byte/result/failure semantics. L1 must not be reported as `COMPLETE / 100%` until both the reverse and real acceptance gates are closed.

## 1. Canonical L1 question and boundary

L1 answers:

> Given an already selected resource identity, how are its exact bytes/capacity/materialized representation produced, transformed, written into the destination, reconstructed when loose/nested, safely authored back into containers/archives, reopened/rematerialized, and proven to reach the original game consumer without loss or authority laundering?

Canonical semantic cut:

```text
[L2] selected resource/provider/member identity
 -> [L1] cached/logical/materialized size
 -> [L1] capacity/allocation
 -> [L1] exact byte acquisition / decompression / transfer
 -> [L1] exact destination bytes
 -> [L1] packed or .lst representation construction
 -> [L1] terminal byte/result state
 ===== END L1 BYTE/RESULT AUTHORITY =====
 -> [L3] request/callback lifecycle publication / LoadedResource state transitions
```

FileSlot/AsyncIO helpers are classified by concrete behavior, not wholesale by subsystem. A helper can contain L1 byte-result semantics and L3 scheduling/lifetime semantics around the seam.

## 2. Current product capabilities

Current `main` includes, at bounded/evidenced scope:

- classic NBZ/ZIP indexing and bounded central/member acquisition;
- STORE and raw-DEFLATE method-8 materialization;
- CRC/size/SHA and `ByteProvenance` published only after successful member materialization/validation;
- artifact-bound serialization/member observations preventing stale-snapshot authority;
- recovered contiguous numbered-volume / first-gap namespace behavior;
- canonical runtime resolver composition and higher-volume precedence;
- atomic/no-replace artifact publication with staged validation;
- direct-retail resolver-based member extraction + provenance sidecar;
- PAC/PNST sparse/empty/alias-preserving relative-slot parsing and recursive expansion;
- size-changing relative-slot reflow with exact untouched-span preservation;
- nested root-to-leaf slot-path authoring with bottom-up parent rebuild receipts;
- verified immutable NBZ copy rebuild;
- deterministic next-contiguous STORE overlay authoring;
- staged NBZ reopen/rematerialization verification;
- runtime-synth PAC/PNST writer with typed child extent authority;
- recovered original `.lst` direct-child `0x800` transfer extent vs recursively synthesized `0x40` complete-image structural extent (#255);
- recovered original zero initialization of the complete synthesized image;
- Windows + Ubuntu CI for promoted product paths.

These capabilities do **not** by themselves prove exhaustive original-runtime equivalence or original-game consumption.

## 3. Reverse gate — L1-R

**STATUS: OPEN / substantially narrowed, not complete.**

Fresh canonical-EXE review on 2026-08-27/28 has now closed or sharply bounded much of the #244 frontier.

### Confirmed / corrected

1. **Materialized size cache**
   - `0x14002F9F0 -> 0x140048E20` copies the cached FileSlot size field; the getter itself returns `0`.
   - physical slots cache low-32-bit `GetFileSize`;
   - NBZ slots use central-directory uncompressed size.

2. **Whole-file transfer granularity vs exact bytes**
   - `0x1400333C0` derives the transfer chunk count;
   - `0x1402EF620` converts that to a `0x800`-granular direct transfer extent;
   - lower reads terminate at actual EOF/produced bytes rather than manufacturing extra payload bytes.

3. **Short-read terminal behavior**
   - the lower worker repeats reads until request filled or backend returns EOF/no-progress/error;
   - callback `0x1400335A0` accumulates actual transferred bytes and does not independently enforce `loadedBytes == plannedBytes` before success publication;
   - a source shortened after the cached size query can therefore produce a short-success condition in the original path.

4. **`.lst` planner/writer layout**
   - header: `align64((slotCount + 2) * 4)`;
   - direct ordinary/packed-sibling child: `0x800` transfer extent in the safe positive arithmetic domain;
   - recursively synthesized child: complete generated image under `0x40` structural alignment;
   - complete planned image is zero-initialized by the original allocator (`0x140337600 -> memset thunk 0x140346BEA`).

5. **Enqueue vs byte-producing body**
   - `0x1402EF4D0` is type-2 queue admission, not the byte-producing body;
   - `0x1402EF790` is the consumer entering the whole-file open/chunk/submit/status path.

6. **Writer failure propagation**
   - `0x1401B85C0` ignores return values from direct child `0x1402EF4D0` calls and recursive `0x1401B85C0` calls;
   - `0x1401B8CA0` has branch-dependent boolean semantics;
   - `0x1401B84E0` ignores the boolean result of type-3 completion enqueue `0x1402EF580`;
   - therefore no single upstream writer/setup boolean is an exact terminal-byte receipt.

7. **Integer width / overflow**
   - original chunk/planner arithmetic is 32-bit and wrap-prone;
   - `0x1400333C0` is only equivalent to mathematical `ceil(size/0x800)` in the safe positive domain;
   - product code intentionally uses checked/fail-closed arithmetic instead of reproducing unsafe original wrap.

8. **Malformed scanner bounds**
   - `0x1FC0` scan and `0x100` token-copy boundaries are confirmed;
   - they are not clean original error enums;
   - product `scan_limit_exceeded` / `token_limit_exceeded` statuses are fail-closed hardening.

Canonical detailed checkpoint: `l1-writer-failure-width-reconciliation-2026-08-28.md`.

### Reverse still open

Before an **exhaustive original L1** claim:

- exact recursive `.lst` cycle/depth behavior and allocation/free lifetime semantics;
- remaining allocator/backend failure branches not already classified;
- final L1-terminal -> L3 normal-completion suppression/eligibility reconciliation, including `0x1402EF460` and relevant `0x1401B8DC0` context;
- representative real `.lst` corpus receipt for any claim covering real loose-list consumption;
- contradiction sweep across current L1/L3 seam docs/code/evidence.

L1-R remains a mandatory gate because the project explicitly targets deep original behavior, not merely a convenient product approximation.

## 4. Product/acceptance gates

### L1-A — publication integrity

**CLOSED / CANONICAL.**

Shared staged atomic/no-replace publication contract is integrated.

### L1-B — artifact-stable member acquisition

**CLOSED / CANONICAL at product scope.**

NBZ member observation/materialization is bound to artifact/member identity; provenance is emitted only after successful materialization/validation.

### L1-C — direct-retail representative provenance

**IMPLEMENTATION AVAILABLE / REAL RECEIPT OPEN.**

Use resolver-selected identity; do not predeclare a guessed archive/member winner.

### L1-D — exact retail representation classification

**REAL RECEIPT OPEN.**

Classify the exact acquired bytes. Only use a writer whose domain is evidenced for that representation.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT CAPABILITY ADVANCED / REAL-RETAIL RECEIPT OPEN.**

PAC/PNST same-size, size-changing and nested authoring capabilities exist. Runtime-synth `.lst` layout now reflects the recovered direct/nested extent distinction. Real representation authority still controls which writer is permitted.

### L1-F — next-volume publication + canonical reopen

**PRODUCT CAPABILITY AVAILABLE / REAL-RETAIL RECEIPT OPEN.**

Required chain:

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged canonical reopen
 -> higher-volume resolver winner
 -> exact rebuilt-member rematerialization
 -> authored-child verification
```

### L1-G — original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E / FINAL MATERIALIZATION ACCEPTANCE.**

Canonical tracking: #209.

A crash-free launch is not sufficient. Required evidence includes deterministic consumer-visible effect attributable to authored bytes and rollback proving retail immutability.

### L1-H — final cross-stack audit

**OPEN.**

Requires:

- L1-R contradiction-free reverse boundary;
- exact executable authority;
- real acquisition/provenance receipt;
- explicit representation classification;
- real authored rebuild/rematerialization receipt;
- original DMC3 consumption receipt;
- rollback receipt;
- final exact-head Windows + Ubuntu CI;
- status/roadmaps/issues all agreeing on the final claim.

Only then may the project state **L1 = COMPLETE / 100%**.

## 5. Original behavior vs product safety rule

GDSpaces is not required to reproduce unsafe original implementation defects.

Keep separate:

| Original runtime evidence | GDSpaces product rule |
| --- | --- |
| 32-bit size/offset wrap possible | checked overflow, fail closed |
| writer may ignore child enqueue failure | successful product receipt must not launder rejected work |
| completion enqueue result may be ignored | product completion authority remains explicit |
| malformed scan/token bounds lack clean error enums | explicit fail-closed statuses |
| short source can terminate with fewer bytes than cached plan | product acquisition validates exact declared/observed bytes where required |

Original behavior is reverse truth; product safety is authoring truth. Neither may be mislabeled as the other.

## 6. Current work order

No broad L2/L3 or tooling work should displace the remaining L1 sequence unless it directly closes an L1 dependency.

```text
1. finish L1-terminal -> L3 normal-completion suppression/eligibility seam
2. finish residual allocator/backend + recursive .lst lifetime/failure branches
3. run a final L1 original-runtime contradiction sweep
4. acquire representative real-retail member/provenance receipt
5. classify exact representation
6. perform one bounded real edit + rebuild + rematerialization
7. publish/reopen next-volume overlay and preserve closure receipt
8. execute #209 original-game consumption + rollback
9. final cross-stack audit
10. only then mark L1 COMPLETE / 100%
```

## 7. Non-blockers / freezes

Unless a real dependency activates them:

- Binary AFS is not assumed from `.afs/` logical namespaces;
- PACK historical product parsing is not original-runtime authority;
- Capcom offline packer equivalence is not required for GDSpaces safe authoring;
- Stage Ops / ModViz are downstream consumers and do not define L1 truth;
- L2/L3 broad completion is independent, except for the exact selected-identity and lifecycle evidence needed by the L1 vertical proof.

## 8. Environment boundary

The canonical analysis `dmc3.exe` is available and has been directly revalidated for this reverse work. The environment still does not expose every protected-install artifact/process condition needed for the final real-retail/original-game acceptance chain.

That external limitation does not justify calling L1 complete.

## 9. Documentation synchronization rule

Any change to L1 completion status or recovered byte semantics must synchronize at least:

- this roadmap;
- `dmc3-loose-container-list.md` when `.lst` behavior changes;
- the current reverse checkpoint/evidence packet;
- `docs/status/current.md`;
- `docs/status/canonical-status.json`;
- `master-roadmap.md` when the cross-layer critical path changes;
- final audit / issues #100, #182, #209 when promotion eligibility changes.

Percentage estimates are planning aids only. Mandatory evidence gates are the completion authority.
