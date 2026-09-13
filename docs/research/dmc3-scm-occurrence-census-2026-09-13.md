# SCM occurrence census — method and open statistic, 2026-09-13

**Branch:** `main`
**Canonical executable:** `dmc3.exe`
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
**Scope:** how SCM payloads are counted inside retail containers. No new field semantics.

## Why a per-file sweep was not enough

`verify-scm-corpus` walks a directory of extracted `.scm` files. That answers
questions about files, and the containers hold occurrences: the same scene
model appears more than once across a stage set, and extraction tooling
silently drops payloads it does not have a slot name for. A direct scan of
`st000.pac`–`st003.pac` found 51 structurally valid SCM occurrences — 16 in
st000, 3 in st001, 5 in st002, 27 in st003 — against a version distribution of
0.83 ×1, 0.90 ×16, 1.00 ×1, 1.01 ×33.

Those are occurrence counts. How many of the 51 are distinct payloads was not
determined in that pass, and the distinction changes what every derived
statement means: "16 at version 0.90" is a claim about how widespread a legacy
revision is only if the 16 are 16 different files.

## The method

Matching `SCM ` is not the test. The magic appears inside unrelated payloads,
and the reader is the authority on whether bytes are a document. But the reader
refuses a span that does not terminate exactly where the canonical layout ends,
which is a condition no payload embedded in a container can meet when handed
the rest of the file — every embedded SCM fails with
`scm.serialized-size-mismatch`.

So the scan is two passes:

1. parse the tail to learn the object/mesh shape, then compute the serialized
   extent with `build_serialized_layout` — the same layout authority the reader
   and the writer both use;
2. parse exactly that extent and keep the occurrence only if `ok()`.

The extent is also what makes the digest identify a payload. Hashing to
end-of-file gives every occurrence in a container a different hash and the
deduplication reports nothing; the regression test for this holds the
difference.

## Reproducing it

```
dmc-rengine census-scm-occurrences st000.pac st001.pac st002.pac st003.pac \
    --json scm-occurrence-census.json
```

The report gives, per occurrence: container, offset, serialized size, SHA-256,
version, resource code with its decimal decomposition, the lighting reference
node against the scene-node count, and object/mesh/vertex counts. The summary
gives occurrences against unique payloads overall and per version, the family
class histogram, the topology flag union, how many occurrences revive a
preservation-only domain, and how many carry a lighting reference outside the
serialized scene-node domain.

## What has been run here

Only `st001.pac` and `st114.pac` are present in this working environment.

| Container | Occurrences | Unique payloads | Versions | Family classes |
|---|---:|---:|---|---|
| `st001.pac` | 3 | 3 | 1.01 ×3 | 3 ×1, 4 ×2 |
| `st114.pac` | 4 | 4 | 1.01 ×4 | 3 ×4 |

The st001 figure reproduces the independent scan exactly, which is what
qualifies the method rather than the numbers.

Two observations from st114, which the earlier pass did not cover:

- its four occurrences carry the same resource code `311400` and are four
  distinct payloads, so the code is not a payload identity;
- all seven occurrences have a lighting reference node of 0, so the
  out-of-range check has no positive case in this sample and is unproven
  against retail data.

Across both containers: 54,544 vertices, topology flag union `0x02`, and no
occurrence reviving any preservation-only domain — consistent with the
st000–st003 result and on a much smaller sample.

## Open

**The 51-occurrence deduplication is not finished here.** `st000.pac`,
`st002.pac` and `st003.pac` are not in this environment; the command above run
over all four containers closes it. Until then, the version distribution
recorded in `formats/scm_version.hpp` is stated as occurrence counts, and says
so.
