# GDSpaces Layer 2 — archive normalized-key duplicate semantics — 2026-08-26

**Scope:** canonical DMC3-HD archive search index only.  
**Artifact:** `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.  
**Status:** STATIC REVERSE CONFIRMED / RETAIL COLLISION CENSUS STILL REQUIRED.

Companion census: `../../data/reverse/dmc3-gdspaces-l2-resolver-static-census-2026-08-26.v1.json`.

## 1. Index construction

Archive index builder `0x140327CC0` walks the archive entry list and for every entry:

1. takes the entry pathname at `entry+0x30`;
2. normalizes it through `0x140327160` with literal flags `0x0E`;
3. builds a flat search array with `0x10`-byte elements;
4. stores normalized pathname pointer plus exact entry pointer;
5. sorts the array through CRT `qsort` at `0x140327D6D`.

Comparator: `0x1403291D0`.

## 2. Lookup uses the same equivalence relation

Archive lookup `0x140328160` copies the requested candidate into a bounded buffer, normalizes it with `0x0E`, calls CRT `bsearch` at `0x1403281E0`, and uses the same comparator `0x1403291D0`.

The comparator compares only the normalized NUL-terminated strings. It has no secondary comparison of archive entry address, central-directory order, offset, size, original case, or insertion index.

Therefore two distinct central entries that collapse to one `0x0E` normalized key are comparator-equal.

## 3. No recovered deterministic duplicate winner

The original path relies on CRT `qsort` + `bsearch` with no game-defined tie-break. The reverse therefore does **not** support claims such as:

- first central entry wins;
- last central entry wins;
- lowest/highest offset wins;
- qsort preserves archive order.

`qsort` is not a stable-order contract for equal elements, and `bsearch` does not define which equal element is returned when several compare equal.

## 4. Layer 2 consequence

The exact real-retail `0x0E` collision census remains mandatory.

Desired clean closure:

```text
for every exact retail archive member surface inside the claimed resolver scope:
normalized_key_count == unique_normalized_key_count
```

If zero collisions are proven, duplicate-winner semantics are irrelevant for that corpus.

If any collision exists, Layer 2 must fail closed or narrow the parity claim until the actual original-runtime duplicate behavior is measured. A deterministic product-side tie-break is allowed only as product policy; it must not be authority-laundered into recovered-original semantics.

Synthetic or DMCL zero-collision results do not close the DMC3 retail gate.

## 5. Non-claims

This checkpoint does not prove:

- that retail `dmc3-0.nbz` contains duplicate normalized keys;
- that the retail corpus is collision-free;
- which equal entry a particular CRT/run would return;
- protected-build address equivalence;
- trusted selected-provider identity;
- Layer 1 or Layer 3 completion.
