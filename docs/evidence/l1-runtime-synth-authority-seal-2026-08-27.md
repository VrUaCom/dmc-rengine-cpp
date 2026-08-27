# GDSpaces L1 — runtime-synth writer authority seal

**Date:** 2026-08-27  
**Base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Scope:** L1 product authoring provenance hardening; no original-runtime semantic promotion.

## Defect

`ExactChildImage::from_verified_runtime_synth_result()` is intended to accept only output produced by `RuntimeSynthRelativeSlotWriter`. However `RuntimeSynthResult` exposed mutable output bytes and a mutable public receipt/output hash. A caller could copy a genuine result, mutate bytes outside intrinsic child spans (for example alignment padding), recompute the public `output_sha256`, and retain a result that could satisfy the previous `RuntimeSynthResult::ok()` checks because child receipts bind only intrinsic child spans.

That allowed writer authority to be laundered onto a modified complete image without re-running the writer.

## Fix

`RuntimeSynthResult` now carries a private writer-only output SHA-256 seal. `RuntimeSynthRelativeSlotWriter::rebuild()` binds that seal to the exact output hash before the result is returned. `RuntimeSynthResult::ok()` requires the public receipt hash to equal the private writer seal in addition to the existing whole-output SHA and child-span checks.

The result type is no longer an aggregate, while remaining default-constructible for invalid/failure values. Callers cannot set the private seal.

## Regression

`runtime_synth_result_integrity_tests` now mutates the first alignment-padding byte, recomputes the public receipt output hash, and requires both:

- `RuntimeSynthResult::ok()` to reject the modified result;
- `ExactChildImage::from_verified_runtime_synth_result()` to refuse promotion to `format_writer_receipt` authority.

This case specifically covers bytes not protected by intrinsic child hashes.

## Evidence boundary

This fix does **not** claim that zero-filled alignment padding matches original DMC3 runtime synthesis. Product zero-fill remains a deterministic product policy until the open #244 byte-exactness reverse establishes original allocation/write/padding semantics.

This fix does **not** close L1-M, #209, #244, original terminal materialization semantics, or L1 COMPLETE/100%.
