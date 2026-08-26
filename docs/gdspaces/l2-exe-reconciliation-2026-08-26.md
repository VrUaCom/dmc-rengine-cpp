# GDSpaces Layer 2 — canonical EXE review / reconciliation — 2026-08-26

**Scope:** Layer 2 / Runtime Resolver only.  
**Canonical analysis artifact:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Protected execution candidate:** 6,567,320 bytes, SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`.  
**Status:** EXE-REVIEWED / RECONCILED / L2 NOT COMPLETE.

This checkpoint supersedes only stale status statements in the 2026-08-25 L2 review. It does not rewrite the historical review and does not promote protected-process evidence that has not been acquired.

## 1. Raw canonical EXE re-review

The canonical analysis EXE was independently re-read from raw bytes before this reconciliation. The SHA and size match the machine-readable reverse authority.

### 1.1 `OpenGameResource` direct-call surface

Function: canonical analysis VA `0x14002FCA0` / RVA `0x2FCA0`.

Whole-image direct-call census remains exactly three observed direct callers:

- `0x14003340A`;
- `0x1403380C7`;
- `0x1403381F7`.

Each caller executes `mov edx, 1` immediately before the call. Therefore the canonical direct-call surface uses the recovered `flags = 1` mode.

Inside `OpenGameResource` the raw instructions confirm:

- basename extraction after the last `/` or `\\`;
- candidate buffer capacity `0x400`;
- six namespace prefixes from the EXE table;
- provider phase 1 first, provider phase 2 second;
- `ResourceMountResolve` call at `0x140327430`;
- early request failure when candidate construction fails.

The six prefixes remain:

1. `GDataX360.afs/`
2. `GData.afs/`
3. `Video/`
4. `afs/sound/`
5. `SAVEDATA/`
6. empty prefix

### 1.2 Physical provider / type 0

Bootstrap directly passes `EDX = 0x0C` into the type-0 registration function at `0x140326D20`.

`ResourceMountResolve` at `0x140327430` confirms type-0 behavior:

- provider type `0` is gated by physical mask bit 1;
- candidate is copied into a bounded `0x400` temporary;
- normalization uses the mount's stored `0x0C` flags;
- mounted root + normalized candidate is joined through the bounded builder;
- final open calls `0x140327800`.

The already recovered final-open contract remains valid:

`CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)`.

This static reverse remains CLOSED. PR #215 is merged and is not reopened by the R2/R3 runtime-evidence work.

### 1.3 Archive provider / type 1

`ResourceMountResolve` confirms type `1` archive nodes are gated by archive mask bit 0 and traversed in linked-list order.

Archive lookup helper `0x140328160` re-confirms:

- bounded candidate copy;
- normalization with literal flags `0x0E`;
- lookup against the archive's normalized search representation;
- return of the exact central-entry-backed identity pointer on lookup hit.

The wrapper/open helper `0x140328290` is called only after the normalized archive lookup returns an entry.

## 2. New correction from this EXE review

A normalized archive lookup hit is **not equivalent to a successful selected resource**.

At `0x1403274BE..0x1403274CD`:

1. `0x140328160` returns a matched archive entry;
2. `0x140328290` attempts to construct/open the archive stream wrapper;
3. if that helper returns null, control jumps to the resolver cleanup/null-return path;
4. the current provider traversal does **not** continue to a lower-precedence volume as though the matched archive were a clean lookup miss.

Therefore the R3 v1 selected-identity contract is intentionally limited to the **clean path**:

- `miss`: normalized lookup did not produce a usable provider hit;
- `selected`: provider hit completed successfully and produced the selected identity.

A trace that observes `lookup hit -> wrapper/open/backend failure` must fail closed as unsupported by the clean-path v1 receipt. It must never be rewritten as `miss` in order to continue the synthetic precedence sequence.

This distinction is instruction-backed and is now part of the R3 acceptance boundary.

## 3. Reconciliation with recent implementation

### Closed / integrated

- type-0 physical static reverse: merged in #215;
- controlled physical hit/miss/archive-to-physical product receipts: merged in #215;
- canonical direct-call `flags = 1` census: closed;
- `0x400` failure aftermath: closed for the canonical direct-call surface;
- protected-runtime RVA acquisition + multi-anchor mapping tooling: merged in #219.

### Tooling implemented, real evidence still required

#219 provides the tooling to create a bounded protected-runtime mapping packet. A merge is not a real R2B receipt. Promotion still requires actual child process-window receipts from the protected `81c7...` process that reproduce the canonical L2 anchor windows.

#221 defines the R3 clean-path selected-identity content-candidate contract. It must not be treated as original-process evidence merely because a JSON file is structurally valid.

The promotion binder is required to:

- reconstruct the R2B mapping from supplied child process-window receipts;
- bind the exact mapping file by SHA-256;
- hash the observer artifact itself and match `observer_build_sha256`;
- hash every claimed mounted `DMC3-N.nbz` artifact and match exact size/SHA;
- reject incomplete/lossy traces;
- reject provider/backend-failure events under the clean-path v1 contract;
- emit only a non-promotable bound candidate until a separate trusted-capture/origin mechanism exists.

## 4. Authority boundary

The two EXE roles remain separate:

- `e454272e...` is the instruction/RVA reverse authority;
- `81c7e619...` is the protected distribution/original-execution candidate;
- canonical analysis addresses are not protected-process breakpoint authority by themselves;
- a real bounded R2B mapping packet is the bridge;
- synthetic mapping/selection fixtures are validator tests only.

No result in #219 or #221 may promote global build equivalence.

## 5. Current L2 gates after reconciliation

| Gate | Current status |
| --- | --- |
| Type-0 physical static reverse | **CLOSED / MERGED #215** |
| Controlled physical product receipts | **CLOSED for bounded product scope / MERGED #215** |
| Direct-call `flags=1` census | **CLOSED** |
| `0x400` canonical direct-call behavior | **CLOSED** |
| Retail DMC3 `0x0E` collision census | **BLOCKED — exact retail corpus access** |
| Direct-retail resolver receipt | **OPEN / depends on exact retail artifacts** |
| Protected runtime R2B mapping tooling | **MERGED #219** |
| Real protected runtime R2B mapping receipt | **OPEN — real process required** |
| R3 selected-identity content-candidate tooling | **IN REVIEW #221** |
| Trusted original-process R3 selected-identity receipt | **OPEN — real process + trusted capture required** |
| Final L2 audit/promotion | **OPEN** |

## 6. Current critical path

1. finish #221 review and exact-head Windows + Ubuntu CI;
2. do not merge if any content-candidate path can self-promote or self-declare artifact provenance;
3. obtain a real protected `81c7...` process and produce R2B child-window receipts + bounded mapping packet;
4. hash-bind the exact observer artifact and exact numbered retail NBZ artifacts;
5. capture a zero-loss clean-path R3 trace for a concrete request;
6. bind selected archive member / physical relative identity without inventing it from the request;
7. compare that original-process winner to GDSpaces as a separate bounded comparison;
8. acquire exact retail central-directory/member-list evidence for the `0x0E` collision census;
9. perform the final L2 evidence reconciliation/audit.

Layer 1 materialization and Layer 3 lifecycle are not promoted by this checkpoint.
