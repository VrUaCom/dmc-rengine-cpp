# GDSpaces Layer 2 — archive normalized-key duplicate semantics — 2026-08-26

**Scope:** canonical DMC3-HD archive search index only.  
**Artifact:** `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.  
**Status:** STATIC REVERSE CONFIRMED / RETAIL COLLISION CENSUS STILL REQUIRED.

Companion census: `data/reverse/dmc3-gdspaces-l2-resolver-static-census-2026-08-26.v1.json`.

## 1. Index construction

Archive index builder: `0x140327CC0`.

The builder walks the archive entry linked list and, for every entry:

1. takes the entry pathname at entry `+0x30`;
2. normalizes that pathname in place through `0x140327160` with literal flags `0x0E`;
3. counts entries;
4. allocates a flat search array with element width `0x10`;
5. stores normalized pathname pointer plus exact entry pointer in each element;
6. sorts the array with CRT `qsort` at call site `0x140327D6D`.

The comparator passed to `qsort` is `0x1403291D0`.

## 2. Lookup uses the same comparator

Archive lookup helper `0x140328160`:

1. copies the requested candidate into a bounded buffer;
2. normalizes it with `0x0E`;
3. invokes CRT `bsearch` at `0x1403281E0`;
4. uses the same comparator `0x1403291D0`;
5. returns the exact stored central-entry pointer from the matched search-array element.

Thus sorting and lookup use one identical equivalence relation.

## 3. Comparator semantics

Comparator `0x1403291D0` dereferences the first field of both `0x10` search elements and compares the two normalized NUL-terminated strings byte by byte.

It returns:

- `0` when the two normalized strings are byte-identical through the terminating NUL;
- negative/positive only at the first differing byte.

There is **no** secondary comparison of:

- archive entry address;
- central-directory order;
- compressed/local-header offset;
- uncompressed/compressed size;
- original casing;
- insertion index.

Therefore two distinct archive entries whose names collapse to the same `0x0E` normalized key are comparator-equal.

## 4. No recovered deterministic duplicate winner

The original code then relies on CRT `qsort` and CRT `bsearch`.

This gives no recovered game-defined deterministic winner for comparator-equal entries:

- C/C++ `qsort` does not promise stable ordering among equal elements;
- `bsearch` may return an unspecified matching element when multiple equal elements exist;
- the game comparator adds no tie-break that would recover a semantic first/last/lowest-offset/highest-offset policy.

Therefore statements such as these are **not evidence-backed**:

> first central entry wins

> last central entry wins

> lowest offset wins

> highest offset wins

> qsort preserves original archive order

No such rule is present in the recovered comparator/index path.

## 5. Consequence for Layer 2 acceptance

The real-retail `0x0E` collision census remains mandatory.

The desired clean result is:

```text
for the exact retail archive member surface used for the claimed resolver scope:
normalized_key_count == unique_normalized_key_count
```

If zero collisions are proven, duplicate-winner semantics are irrelevant for that corpus.

If any collision exists, Layer 2 must not invent a portable deterministic winner from the static reverse. The collision becomes a separate original-runtime evidence problem and product parity claim must be narrowed until that behavior is measured.

Synthetic or DMCL zero-collision results do not close the DMC3 retail gate.

## 6. Relationship to current ResourceKeyIndex

A product-side deterministic index may choose to reject/diagnose duplicate normalized keys for safety. That is a product policy, not recovered evidence that the original game deterministically selects the same duplicate.

For recovered-equivalence claims:

- unique key -> normal bounded comparison is possible;
- duplicate key -> fail closed or classify as ambiguous until original runtime evidence exists.

Do not authority-launder a deterministic product tie-break into `recovered_original` status.

## 7. Non-claims

This checkpoint does not prove:

- that retail `dmc3-0.nbz` actually contains duplicate normalized keys;
- that retail `dmc3-0.nbz` is collision-free;
- which duplicate would be returned by a particular CRT implementation/run;
- protected-build address equivalence;
- trusted selected-provider identity;
- Layer 1 or Layer 3 completion.
