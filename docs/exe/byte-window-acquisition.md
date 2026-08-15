# Executable Byte-Window Acquisition

## Purpose

DMC Rengine needs a reproducible way to reacquire exact executable bytes for a known reverse target without turning every reverse task into a custom file-reading script.

The byte-window acquisition primitive is intentionally small:

```text
local executable
  -> GDSpaces SourceRegistry / LocalDirectorySource
  -> exact artifact SHA-256 gate
  -> read-only PE parse
  -> checked VA -> RVA -> SizeOfImage + file-backed VA/raw mapping
  -> bounded byte window
  -> deterministic metadata receipt
  -> optional local-only raw hex
```

It is generic EXE/Reverse Core infrastructure. It does not know DMC3 function names, gameplay semantics, HITS, Stage IDs or recovered C++ identities.

## CLI

```text
dmc-rengine extract-exe-window <exe> <expected-sha256> <va> <size> [--hex]
```

`<va>` and `<size>` accept decimal values or `0x`-prefixed hexadecimal values.

Example shape:

```text
dmc-rengine extract-exe-window game.exe <64-hex-sha256> 0x140001000 0x100
```

The expected artifact SHA is mandatory. The command refuses to acquire a window when the local executable hash differs.

## Default receipt

Without `--hex`, the command emits deterministic JSON metadata only:

```json
{
  "schema": "dmc-rengine.exe-byte-window.v1",
  "artifact_sha256": "...",
  "artifact_size": 0,
  "image_base": "0x0",
  "va": "0x0",
  "rva": "0x0",
  "file_offset": "0x0",
  "size": 0,
  "section": ".text",
  "window_sha256": "..."
}
```

The example values above are schema illustrations only, not evidence for any game binary.

The metadata receipt binds:

- exact artifact SHA-256;
- artifact size;
- PE image base;
- requested VA;
- derived RVA;
- raw file offset;
- exact window size;
- section/header mapping identity;
- SHA-256 of the acquired byte window.

A receipt is not a semantic reverse claim. It establishes only exact artifact/range/byte identity.

## Optional raw bytes

`--hex` adds `bytes_hex` to the receipt and prints a warning to stderr.

This mode exists for local disassembly/reverse work. Raw executable bytes must not be committed to the public repository.

The receipt layer rejects raw hex unless:

- it is canonical lowercase hexadecimal;
- its byte count exactly equals the declared window size;
- decoding it and hashing the decoded bytes reproduces `window_sha256` exactly.

This prevents a receipt from claiming one byte-window hash while carrying different raw bytes.

## PE mapping safety

The extractor is deliberately stricter than a convenience RVA-to-file-offset converter because its input identity is a runtime-style **VA**.

An accepted section interval must be inside all applicable boundaries simultaneously:

1. the PE `SizeOfImage` runtime image boundary;
2. the section virtual extent (`VirtualSize`, with `SizeOfRawData` fallback when `VirtualSize == 0`);
3. the section raw file extent;
4. the supplied artifact byte span;
5. one unambiguous file-backed VA mapping authority across the whole requested interval.

For a section, the usable file-backed VA extent is therefore the intersection of virtual and raw coverage. This rejects both virtual-only tails and raw padding that lies beyond `VirtualSize`.

The extractor rejects:

- zero-sized windows;
- windows larger than `0x10000` bytes / 64 KiB;
- VA below the PE image base;
- RVA overflow;
- missing/zero `SizeOfImage` or RVA outside `SizeOfImage`;
- a window that crosses `SizeOfImage`;
- RVA not backed by PE headers or a section's file-backed VA extent;
- virtual-only section tails;
- raw-only section padding beyond `VirtualSize`;
- header-to-section or section file-backed-boundary crossing;
- file spans outside the supplied artifact;
- ambiguous file-backed VA mappings.

Ambiguity is checked across the **entire requested RVA interval**, not only the first byte. A malformed/odd PE whose requested window overlaps two section mappings is rejected instead of selecting the first section by header order.

A header-backed acquisition is also rejected if the same requested RVA interval overlaps section-backed VA data.

These rules are important for evidence work: the same artifact/range request must have one deterministic runtime-to-file byte authority.

## GDSpaces boundary

The CLI does not open the executable through a private loader.

It uses:

```text
SourceRegistry
  -> LocalDirectorySource
  -> ResourceId
  -> ResourcePayload
```

The PE reader and byte-window extractor consume the supplied immutable bytes.

This keeps local reverse acquisition inside the same no-second-resolver architecture as the rest of DMC Rengine.

## Artifact hash policy

The extractor accepts an explicit expected SHA rather than hardcoding the DMC3 known-target registry.

That is intentional:

- the primitive remains reusable for other DMC builds and other games;
- every acquisition run is still fail-closed against one exact caller-selected artifact identity;
- game/profile-specific code can supply its canonical expected SHA from its own Evidence/known-target authority.

The extractor does not decide whether an artifact is canonical for a subsystem. It only verifies that the bytes presented to the run match the exact SHA requested by the caller.

## Reverse Core / evidence relationship

The acquisition primitive can support a later pipeline such as:

```text
BinaryArtifact
  -> exact VA/range request
  -> ExeByteWindowReceipt
  -> disassembly / structural observation
  -> EvidenceRecord
  -> Function / DataObject reconstruction
  -> recovered source
  -> behavioral validation
```

But acquisition is not Evidence promotion by itself.

A matching window hash may verify that a previously recorded body was reacquired exactly. It does not prove that a function name, ABI, ownership model or semantic interpretation is correct unless those claims have their own evidence.

## Known-body verification pattern

When an existing Evidence Packet already contains:

- exact VA;
- exact body size;
- exact body SHA-256;

run the extractor first **without** `--hex` and compare the returned `window_sha256` against the recorded body hash.

Only after that identity check should raw bytes be emitted locally for re-disassembly when needed.

This gives a clean distinction:

```text
known-body verification
    exact size + expected body hash exist

probe/discovery acquisition
    body boundary/hash is not yet known
```

A probe window must never be described as an exact function body merely because it starts at a known VA.

## HITS Pass-10 Slice-16 application

The current Stage-CFG transform-source investigation already has exact body windows for several supporting functions:

| Target | VA | Size | Recorded body SHA-256 |
|---|---:|---:|---|
| C260 manager source initializer | `0x14005C260` | 184 | `9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7` |
| C630 indirect-transform builder | `0x14005C630` | 257 | `97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a` |
| C740 direct-table transform builder | `0x14005C740` | 253 | `0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6` |
| C8D0 runtime-object initializer | `0x14005C8D0` | 205 | `f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe` |

Those values come from the existing HITS Slice-9 Evidence Packet. They can be used as exact known-body verification targets when the matching canonical executable is available locally.

`0x1400594B0` is different: the current Slice-16 plan does **not** have a canonical exact body size/body hash for it. It remains a discovery/reacquisition target. Do not invent a body length merely to fit this command.

Likewise the Stage-CFG setup anchors around `0x14009823F` and `0x1400B6483` require caller-context acquisition/disassembly; they are not automatically function-body windows.

## Tests

Synthetic tests cover:

- PE-header and section-backed extraction;
- `SizeOfImage` boundaries;
- section virtual/raw intersection behavior;
- virtual-only section tails;
- raw-only padding beyond `VirtualSize`;
- file-backed boundary crossing;
- truncated files;
- ambiguous mappings at the first RVA and later inside the requested interval;
- deterministic receipt generation;
- receipt mapping invariants;
- canonical SHA/hex formatting;
- optional raw-byte SHA binding.

No proprietary game bytes are required by these tests.

## Completion boundary

The byte-window acquisition primitive may be considered complete only at its bounded responsibility:

> exact-hash-gated, deterministic, read-only extraction of one unambiguously file-backed PE VA window and production of a self-consistent acquisition receipt.

It does **not** complete Reverse Core, EXE Editor, decompilation, HITS Slice 16 or any original-game behavioral-equivalence gate.
