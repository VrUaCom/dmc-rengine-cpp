# GDSpaces Layer 1 — archive entry read branch recovery — 2026-08-26

**Scope:** Layer 1 / member materialization only.

**Authority:** canonical unpacked analysis executable, size `6,356,432` bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Why this document exists

`ZipEntryRead 0x140328F50` has been listed under *strong / do not restart* in the layer classification matrix, and the repository held no recovered detail for it beyond that one line. A boundary asserted as strong with nothing written down cannot be checked, reused, or contradicted. This recovers the body and states it.

## Acquisition

Every window below came through the canonical path — `dmc-rengine extract-exe-window` against the analysis image — so each is bound to that artifact and to its own bytes:

| VA | Size | Section | Window SHA-256 (first 16) |
|---|---|---|---|
| `0x140328F50` | 144 | `.text` | `382c09eaf41035c4` |
| `0x140328540` | 320 | `.text` | `9f1b5660d6beea5a` |
| `0x140328820` | 544 | `.text` | `5b579efe58fea1b2` |
| `0x140327910` | 64 (probe) | `.text` | `c2f62c72cc683968` |

`0x140328F50 + 0x90` lands exactly on `0x140328FE0`, so the entry-read body is bounded rather than a window guessed around an anchor.

## Recovered behavior

```text
ZipEntryRead(entry, destination, size, flags) -> signed count

prepare(entry)                       ; 0x140328540
  false -> return -1

inflater = [entry + 0x40]
  non-null -> return InflateRead(inflater, destination, size, flags)   ; 0x140328820

stream    = [entry + 0x38]
remaining = [stream + 0x10] - [stream + 0x0C]
size      = min(size, remaining)
  size == 0 -> return 0

read = backend_read([stream + 0x00], destination, size, flags)         ; 0x140327910
  read >= 0 -> [stream + 0x0C] += read
return read
```

### The branch key is the context, not the method

The decision is made on whether an inflater context is attached at `entry + 0x40`, not by re-reading the member's compression method. A product reader that branches on `method == 8` is making the same decision by a different route and must not describe itself as reproducing this one.

### Cursor discipline is shared

`InflateRead` refills its input window from the same stream object with the same arithmetic — remaining is `[+0x10] - [+0x0C]`, clamped, read through `0x140327910`, and `[+0x0C]` advanced by the result. The input window lives at `context + 0x10` and refills `0x1000` bytes at a time. One layout therefore serves both branches, which is why the direct path and the inflater cannot disagree about position.

### Failure and exhaustion are different answers

A failed preparation returns `-1`. An exhausted stream returns `0` without reaching the backend at all. A negative backend result is returned unchanged — not translated — and the consumed cursor does not advance. A short read advances by the short count, so the next request sees the real position.

## Product boundary

`ArchiveEntryReadContract` states these facts and `ArchiveDirectReadModel` reproduces the direct branch as portable arithmetic, in the same spirit as `PhysicalProviderModel`: product behavior can be compared against the recovered path without the original object layout entering portable code.

This does **not** promote `core::RawDeflate` to recovered status. The original inflater's state layout, return ABI and internal ownership remain Recovered Game Source Tree concerns, exactly as `raw-deflate-boundary.md` states. What is recovered here is the *dispatch and cursor* boundary around it.

## Still open

`0x140328540` preparation and `0x140328FE0` compressed seek/reset have bodies acquired but not yet stated as contracts; their allocation, lifetime and error behavior remain the bounded open targets the classification matrix already names.
