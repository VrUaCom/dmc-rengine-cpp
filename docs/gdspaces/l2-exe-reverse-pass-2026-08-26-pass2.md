# GDSpaces Layer 2 — canonical EXE reverse pass 2 — 2026-08-26

**Scope:** Layer 2 / runtime resolver only.  
**Canonical analysis artifact:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Status:** STATIC REVERSE CHECKPOINT / RECONCILIATION INPUT / L2 NOT COMPLETE.

Machine-readable companion: `data/reverse/dmc3-gdspaces-l2-resolver-static-census-2026-08-26.v1.json`.

This pass re-read the canonical raw executable and re-censused the resolver cluster. It extends the earlier `l2-exe-reconciliation-2026-08-26.md` checkpoint and corrects one important topology assumption: numbered archive **discovery** and successful archive **mounting** are distinct events.

## 1. Direct-call census remains narrow

### `OpenGameResource`

Canonical VA/RVA: `0x14002FCA0` / `0x2FCA0`.

Whole-`.text` `E8 rel32` census again finds exactly three direct callers:

- `0x14003340A`;
- `0x1403380C7`;
- `0x1403381F7`.

Each caller loads `EDX = 1` immediately before the call.

Therefore the recovered 12-attempt archive-then-physical policy remains the canonical **observed direct-call surface**. This pass does not claim that arbitrary alternative flag modes are globally impossible.

### Resolver cluster

Direct-call census:

| Function | VA | Direct callers |
| --- | --- | --- |
| `ResourceMountResolve` | `0x140327430` | `0x14002FDAC` only |
| archive normalized lookup | `0x140328160` | `0x1403274B0` only |
| archive wrapper/open constructor | `0x140328290` | `0x1403274C5` only |
| physical mount registration | `0x140326D20` | `0x14002E9A9` only |
| archive mount registration | `0x140326DA0` | `0x14002E9E8` only |

A whole-image exact-qword scan found no absolute pointer values equal to the listed resolver function VAs. This narrows the static surface but is **not** runtime proof that no indirect call is possible.

## 2. `0x140327800` is shared, not resolver-only

The low-level `CreateFileA` wrapper at `0x140327800` has three direct callers:

- `0x140326DDA` — archive mount/open initialization;
- `0x14032755C` — type-0 physical resolver final-open edge;
- `0x14032858F` — reopening/opening the path stored by an existing stream object.

Therefore the already recovered physical resolver contract must be phrased precisely as:

```text
ResourceMountResolve type-0 edge
0x14032755C -> 0x140327800
```

not as a claim that every caller of `0x140327800` implements resolver selection semantics.

The helper itself still performs the previously recovered read-only open:

```text
CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)
```

with file/path-not-found as the ordinary false/open-miss result and retry behavior for other errors.

## 3. Bootstrap root and numbered-volume discovery

Bootstrap function: `0x14002E930`.

The raw instructions show:

1. `GetModuleFileNameA(NULL, buffer, 0x104)` obtains the executable path;
2. the final `\\` is found and the executable basename is truncated;
3. literal `\\data\\dmc3\\` is appended;
4. that resulting directory is registered through `0x140326D20` with `EDX = 0x0C`;
5. numbered archive paths are formatted from literal `%sDMC3-%d.nbz`;
6. `DMC3-0.nbz` is checked first;
7. discovery increments `N` while the formatted path exists;
8. discovery stops at the first filename that does not exist.

So the original physical root for this bootstrap path is:

```text
<directory containing dmc3.exe>\data\dmc3\
```

and numbered discovery is first-gap bounded.

## 4. New correction: discovery range != successful mounted set

This pass found a stronger distinction than the previous roadmap wording.

For every existing `DMC3-N.nbz`, bootstrap calls archive mount helper `0x140326DA0` at `0x14002E9E8`.

Crucially, bootstrap does **not** branch on or otherwise consume the helper's return value. It immediately:

```text
increments N
formats DMC3-N.nbz for the next index
checks whether that next filename exists
```

The mount helper itself returns failure when archive open/index initialization fails and prepends a type-1 archive node only on successful initialization.

Therefore an original execution can theoretically have:

```text
DMC3-0 exists + mounts
DMC3-1 exists + mount fails
DMC3-2 exists + mounts
DMC3-3 missing -> discovery stops
```

while the effective resolver list contains archive 2, archive 0 and the physical root, with archive 1 absent.

### Consequence for R2/R3 evidence

`first_missing_archive_volume = N` proves only that the numbered **filename discovery range** was `0..N-1` under this bootstrap path. It does not independently prove that every discovered archive was mounted.

The current R3 clean-path validator already expects every higher-precedence discovered volume to appear in the observed archive probe sequence. Therefore a discovered-but-failed mount naturally makes the candidate fail closed. Keep that behavior.

Do **not** weaken the validator by treating an omitted discovered archive as an ordinary lookup miss.

For clean-path v1 acceptance, state the precondition explicitly:

> all discovered numbered archives participating in the claimed contiguous topology must have mounted successfully; otherwise the trace is outside the supported clean-path receipt and fails closed.

## 5. Exact mount-list construction proves clean precedence

Global mount-list head: `0x140CF3180`.

### Type 0 / physical registration — `0x140326D20`

The function:

- allocates a `0x58`-byte mount node;
- writes type `0` at `+0x00`;
- stores supplied normalization flags at `+0x04`;
- duplicates the physical root string into `+0x08`;
- stores the previous global head at node `+0x50`;
- writes the new node as global head.

Bootstrap calls this first with flags `0x0C`.

### Type 1 / archive registration — `0x140326DA0`

On success the function:

- opens the NBZ path through the shared low-level file-open helper;
- initializes archive state/index structures;
- writes type `1` at `+0x00`;
- duplicates the archive path at `+0x08`;
- stores the previous global head at node `+0x50`;
- writes the archive node as global head.

Because discovery is ascending but insertion is prepend, a clean successful sequence constructs:

```text
DMC3-(N-1)
 -> ...
 -> DMC3-2
 -> DMC3-1
 -> DMC3-0
 -> physical root
```

This is stronger than merely inferring that higher numbered archives win: the precedence follows directly from list construction.

## 6. Provider masks prove archive phase then physical phase

`ResourceMountResolve 0x140327430` traverses the global linked list from head through node `+0x50`.

For type `1` archive nodes:

```text
test provider_mask, 0x2
```

causes mask `2` to skip archives.

For type `0` physical nodes:

```text
test provider_mask, 0x1
```

causes mask `1` to skip physical.

`OpenGameResource` runs two provider phases for every six-prefix policy:

```text
phase 0 -> ResourceMountResolve(..., 1) -> archives only
phase 1 -> ResourceMountResolve(..., 2) -> physical only
```

So the direct-call clean path is now instruction-backed as:

```text
for each of six archive candidates:
    traverse mounted archives in effective list order
then
for each of six physical candidates:
    traverse physical mount(s)
```

With clean bootstrap success, archive order is highest numbered to zero.

## 7. Prefix table reconfirmed from raw pointers

Prefix pointer table VA: `0x14055AEF8`.

The six pointers resolve to exactly:

1. `GDataX360.afs/`
2. `GData.afs/`
3. `Video/`
4. `afs/sound/`
5. `SAVEDATA/`
6. empty string

No prefix-policy change is required.

## 8. Exact normalizer flag semantics

Normalizer: `0x140327160`.

Flag bits are instruction-backed as:

| Bit | Meaning |
| --- | --- |
| `0x01` | ASCII `a-z` -> `A-Z` |
| `0x02` | ASCII `A-Z` -> `a-z` |
| `0x04` | strip leading `/` or `\\` separators |
| `0x08` | strip trailing `/` or `\\` separators |

After the flag-controlled steps the helper always:

- converts `/` to `\\`;
- collapses repeated `\\` separators.

Therefore:

- archive `0x0E = 0x08 | 0x04 | 0x02`: trim ends + lowercase + canonical backslashes/collapse;
- physical `0x0C = 0x08 | 0x04`: trim ends + preserve case + canonical backslashes/collapse.

This reconfirms that physical lookup must not silently inherit archive lowercase semantics.

## 9. Archive hit still has a second success gate

Previous correction remains valid and is reconfirmed from the same raw EXE:

```text
0x140328160 normalized archive lookup -> entry pointer
0x140328290 wrapper/open creation -> usable stream wrapper
```

If the first succeeds but the second returns null, `0x140327430` exits through cleanup/null instead of continuing to a lower volume as an ordinary miss.

The R3 v1 `miss/selected` model therefore remains intentionally clean-path only. Provider/backend/wrapper failure is unsupported and fail-closed.

## 10. Reconciliation decisions

### Promote / retain

- keep three-direct-caller `flags=1` authority for the canonical direct-call surface;
- keep 12 attempts: six archive then six physical;
- keep higher-numbered clean archive precedence;
- strengthen its justification to linked-list construction + traversal;
- retain `0x0E` archive and `0x0C` physical normalization;
- retain terminal archive-wrapper failure distinction;
- phrase `0x140327800` as shared low-level open helper, with resolver semantics bound to caller `0x14032755C`;
- keep R3 omission of any expected higher discovered volume fail-closed.

### Correct wording

Replace wording equivalent to:

> `first_missing_archive_volume` means all lower numbered volumes are mounted

with:

> `first_missing_archive_volume` bounds the discovered existing filename range; clean-path R3 additionally requires the observed mount/probe topology to show that all claimed discovered volumes successfully participated.

### Do not claim

This pass does not establish:

- protected `81c7...` runtime addresses;
- real R2B mapping receipts;
- actual retail mount success for every numbered NBZ;
- retail normalized-key collision freedom;
- trusted original-process selected identity;
- absence of every possible runtime indirect call;
- global canonical/protected build equivalence;
- L1/L3 completion.

## 11. Next Layer 2 reverse/integration targets

1. Complete #229 process-instance binding before any real R2B/R3 promotion.
2. Extend real observation requirements to preserve the discovery-vs-mount distinction.
3. Trusted publisher must observe actual archive provider operations; it must not synthesize archive probes from `first_missing_archive_volume` alone.
4. Real retail execution should preserve archive mount outcome evidence if instrumentation can acquire it without altering resolver semantics.
5. Acquire exact retail member/central-directory surface for the outstanding `0x0E` collision census.
6. Only then promote bounded original-process selected identity and perform final L2 contradiction audit.
