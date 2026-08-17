# Raw DEFLATE Product Primitive — Reconciled Boundary

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Purpose

Provide a bounded RFC1951 raw-DEFLATE materializer for GDSpaces/NBZ without treating this implementation as recovered original DMC3 inflater source or ABI.

## Product contract

`core::RawDeflate::inflate(compressed, expected_size)`:

- supports stored, fixed-Huffman and dynamic-Huffman blocks;
- rejects reserved `BTYPE=3`;
- bounds output to the caller-supplied expected materialized size;
- requires exact final output size;
- rejects truncated input, invalid trees/symbols/distances, malformed stored LEN/NLEN and extra trailing bytes;
- returns owned materialized bytes only on success.

This is an independent product implementation. Original `InflateRead`/zlib state layout, return/status ABI and internal ownership remain Recovered Game Source Tree/evidence concerns.

## Review correction against historical #65

The historical implementation rejected a valid RFC1951 dynamic-Huffman corner case: an all-literal dynamic block may declare one distance-code length of zero, meaning no distance codes occur in the block.

The shared Huffman builder now permits an empty tree only when the caller explicitly requests it, and that allowance is used only for the dynamic distance alphabet. Literal/length and code-length alphabets still require at least one symbol.

A regression vector (`05c081080000000020b6fda54e`) materializes exactly one byte, `A`. The vector was independently checked with a standard raw-DEFLATE decoder before being committed.

## Validation vectors

Regression covers:

- stored block;
- fixed Huffman;
- dynamic Huffman with length/distance back-references;
- dynamic all-literal block with no distance codes;
- output limit exceeded;
- output-size mismatch;
- truncation;
- trailing compressed bytes;
- reserved block type;
- malformed stored LEN/NLEN.

## NBZ boundary

The future NBZ source may use this primitive for the product-supported DEFLATE path. That does not convert a product `method == 8` whitelist into an original-game rule: current reverse evidence distinguishes original behavior from safe product validation.
