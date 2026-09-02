# GDSpaces Layer 3 — raw-EXE boundary audit — 2026-08-26

**Canonical executable:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**ImageBase:** `0x140000000`  
**EntryPoint:** `0x14034615C`  
**Ownership reconciliation:** 2026-08-27 — see `layer-boundary-status-reconciliation-2026-08-27.md`

> **Supersession note:** raw findings remain historical evidence. The corrected ownership split does not assign all FileSlot/AsyncIO work to either L1 or L3. Exact byte-result semantics are L1 where they determine materialized bytes; request/queue/callback ownership and LoadedResource state publication remain L3. Normal `state1 -> state2` is L3 lifecycle publication, gated by an L1 terminal result through a cross-layer seam.

## Canonical Layer-3 definition

Layer 3 is **Original Runtime / Resource Lifecycle and scheduling ownership**.

```text
[L2] usable selected identity
 -> [L1] exact byte representation / materializer result
 -> [SEAM] terminal-result completion eligibility
 -> [L3] scheduler / callback ownership
 -> normal state1 -> state2 publication
 -> typed post-load
 -> optional ready callback
 -> state2 -> state3
 -> consumer-ready visibility
 -> claims/reuse/cache/factory ownership
 -> cancellation/replacement
 -> state4 cleanup
 -> owner release / group reset / full reset
 -> runtime / CRT / process-lifetime teardown
```

Stage Assembly / Stage Ops / ModViz remain downstream DOMAIN work and are not L3.

## Behavior-level boundary split

### L1 behavior inside shared I/O/materialization machinery

- logical/materialized size semantics;
- exact byte extent, EOF, short-read/progress and final-chunk clamp;
- selected-byte transform/decompression;
- destination capacity/initialization relevant to exact bytes;
- PAC/PNST/.lst byte representation;
- terminal materializer success/error returned through `0x1401B8CA0`;
- product provenance/edit/rebuild/repack/rematerialization.

### L3 behavior inside shared I/O/materialization machinery

- FileSlot/ReadRequest object ownership and request lifetime;
- queue insertion, persistence, polling, retirement and callback lifetime;
- normal `0x1401B8DC0` completion dispatch;
- LoadedResource state1 -> state2 publication;
- cancellation/replacement policy and scheduler suppression initiated by lifecycle decisions;
- runtime service/pool/shutdown ownership.

A helper can contain both categories. Do not classify the entire helper by its name.

## Raw-EXE lifecycle anchors

### Registry topology

- LoadedResource registry base `0x140C99D30`;
- `363` records;
- stride `0x48`;
- seven groups with counts `[4,136,60,28,1,128,6]`.

Known fields remain bounded by the original audit and later writer-census passes.

### State spine

The state transitions remain L3 lifecycle authority:

- `0x1401B84E0` — acquisition/lifecycle setup; materializer invocation is an L1 interaction, while LoadedResource bookkeeping/state ownership is L3;
- `0x1401B8DC0` — normal completion callback, `state1 -> state2`; callback ABI/context remains strong L3 evidence;
- `0x1401B92D0` — typed post-load -> optional callback -> state3;
- `0x1401B8430` — cancellation/replacement marks states1/2 -> state4;
- `0x1401B8F00` — state4 cleanup -> state0;
- `0x1401B9530` — ordinary owner-driven release;
- `0x1401B9560` — group reset;
- `0x1401B95E0` — full reset.

State2 is a lifecycle publication that indicates materialization completion has been accepted by the runtime. It does not make the byte-production mechanics themselves L3.

## L1/L3 completion seam

Normal `0x1401B8DC0` receives only one registry-relative u32 context. It cannot inspect raw FileSlot transfer status, byte count or transform error directly.

Therefore the remaining question spans the boundary:

```text
[L1] exact materializer terminal result
 -> [SEAM] scheduler eligibility / suppression
 -> [L3] normal B8DC0 dispatch -> state2
```

For layer accounting:

- byte-terminal semantics, final extent and transform success are L1;
- `0x1402EF790` scheduler persistence/re-poll/retirement is L3;
- the exact byte-producing role/context inside `0x1402EF4D0` is L1-relevant;
- its queued-job ownership/lifetime is L3-relevant;
- `0x1402EF460` remains a semantic seam until exact action is known;
- state1 -> state2 publication itself is L3.

This corrects both over-broad interpretations: “all AsyncIO is L3” and “all completion machinery through state2 is L1.”

## Typed post-load

The central typed-dispatch/finalization path remains L3, including representative MOD/EFM/SCM/SHW helpers and recursive PNST typed processing where evidenced.

Open breadth remains external factory/dependency failures, SCM `mesh +0x28`, family differences and profile/build differences.

## Shared ownership above LoadedResource

The bounded loader-node claim/release model remains L3:

- `0x1401AE220` claim increment;
- `0x1401AF6A0` decrement;
- `0x1401AF6F0` zero-claim sweep and underlying release.

No universal LoadedResource refcount is claimed.

## Cancellation interaction

Cancellation/replacement is L3 policy. It may invalidate work whose byte production belongs to L1.

```text
[L3] lifecycle decides unfinished resource is invalid
 -> [SEAM] pending completion may be suppressed/rolled back
 -> [L1] incomplete/failed materializer must not be treated as terminal success
 -> [L3] state4 cleanup/release
```

Do not relabel `0x1402EF460` as OS `CancelIo` without direct evidence.

## Release and teardown

L3 retains:

- ordinary resource release;
- cancellation cleanup;
- group/full reset;
- loader-node zero-claim release;
- FileSlot/AsyncIO service/pool lifetime;
- runtime backing lifetime distinct from CRT/static destruction;
- process-lifetime infrastructure distinctions.

Closing a backend/request as part of a single selected-byte operation may also carry L1 terminal evidence; classify that concrete action separately.

## Current L3 completion state

### Strong/bounded

- registry topology and groups;
- normal state1 -> state2 callback ABI/state publication;
- state2 typed post-load -> optional callback -> state3;
- representative typed families;
- state3 ready meaning;
- cancellation `1|2 -> 4`;
- quiescence `{0,3}`;
- state4 cleanup;
- distinct release/reset policies;
- bounded loader-node claims;
- runtime vs CRT vs process-lifetime distinctions.

### Still mandatory

1. residual alias-aware writer/value-flow census outside bounded paths;
2. family-complete ownership of `+0x08/+0x18/+0x20/+0x28` and stable fields;
3. external typed/factory/dependency failure breadth;
4. SCM `mesh +0x28` reconciliation;
5. shared-owner breadth;
6. scheduler persistence/retirement details required by the L1/L3 completion seam;
7. cross-build/profile differences;
8. original-process V1–V7 lifecycle receipts;
9. final contradiction-free L3 audit.

The L1 byte-exactness gaps themselves are not L3 blockers. The L3 scheduler half of the seam is L3 work.

## Acceptance rule

L3 cannot be completed by a static pass, Stage Ops success, synthetic trace or crash-free launch.

Completion requires static lifecycle closure plus trusted original-process evidence at the declared breadth, bound to the same L2 identity and L1 materialized bytes.

**Current status: L3 INCOMPLETE / NOT 100%.**