# SCM occurrence census — direct container method and closed retail statistic, 2026-09-13

**Canonical branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Why the per-file sweep was insufficient

`verify-scm-corpus` walks extracted `.scm` files. Retail stage containers hold
occurrences, and extraction/naming metadata can omit unnamed payloads. A direct
scan of `st000.pac`–`st003.pac` therefore remains a separate authority.

Matching `SCM ` magic is not enough. The occurrence scanner validates object and
mesh tables, the `0x50` continuation chain, vertex sums, stream bounds, scene
arrays and index-workspace footprint through the canonical SCM grammar.

## Exact-span method

The canonical parser rejects a container tail because the serialized SCM must end
at its own exact layout extent. The scanner therefore works in two passes:

1. probe the candidate tail to recover object/mesh shape;
2. compute the serialized extent with `build_serialized_layout`;
3. parse exactly that span and keep it only when the canonical reader accepts it;
4. hash that exact span for payload identity.

Hashing to end-of-container would make identical embedded payloads appear unique,
so the exact extent is part of the evidence contract.

The CLI is:

```text
dmc-rengine census-scm-occurrences <container>... [--json <report.json>]
```

## Closed st000..st003 result

Fresh retail authority for the four stage PACs is now:

```text
st000.pac   16 structurally valid SCM occurrences
st001.pac    3 structurally valid SCM occurrences
st002.pac    5 structurally valid SCM occurrences
st003.pac   27 structurally valid SCM occurrences
--------------------------------------------------
total       51 occurrences
unique      51 SHA-256-distinct payloads
```

Version distribution:

```text
0.83   x1
0.90  x16
1.00   x1
1.01  x33
```

Aggregate structure:

```text
objects       345
meshes        506
scene nodes   408
vertices      107233
topology      {0, 2}
```

Observed `header +0x14` family classes are `3 / 4 / 7 / 8`. The structural code
is not a unique payload identifier: the same raw code can occur in multiple
SHA-distinct SCM payloads.

## Preservation and lighting observations

Across these 51 payloads the checked preservation domains remain zero:

- header reserved lanes except typed `+0x13`;
- object `+0x04` and `+0x14..+0x2F`;
- mesh `+0x0C`, `+0x30`, serialized `+0x48`, `+0x4C`;
- scene shell `+0x10..+0x1F`;
- transform `+0x1C`;
- GS CLAMP REGION_REPEAT fields.

Topology uses only `0` and confirmed break bit `0x02` over all 107233 vertices.

Header `+0x13` is no longer a preservation field. Canonical EXE evidence closes
its technical role as `lighting_reference_node_index`. The current 51-payload
retail census observes value `0` in every payload, so authored non-zero values
remain a synthetic structural authoring result until a retail specimen or
vanilla-game acceptance test extends that evidence.

## Evidence boundary

This census closes occurrence-vs-unique accounting for `st000.pac`–`st003.pac`.
It does not claim that every game container has been scanned, that every SCM
revision is supported, or that the full SCM writer is game-validated.

Exact SCM render-command VM to D3D11 shader/resource binding remains a separate
reverse frontier.
