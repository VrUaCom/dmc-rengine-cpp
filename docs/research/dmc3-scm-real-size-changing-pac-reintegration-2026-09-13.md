# DMC3 SCM — real size-changing canonical rebuild and parent PAC reintegration

Date: 2026-09-13  
Project: DMC Rengine  
Branch: `reverse/mod-completion-20260907`  
PR: #386  
Evidence class: `REAL_CORPUS_BOUND_SCM_SIZE_CHANGING_CANONICAL_REBUILD_AND_PARENT_PAC_REINTEGRATION`

## Purpose

Close the explicit SCM writer/integration blocker that remained after the preserve-layout alpha/translation/rotation receipts:

```text
hash-bound real SCM payload
 -> typed size-changing structural edit
 -> preserve-layout rejection
 -> deterministic canonical_rebuild
 -> canonical SCM reparse
 -> real parent PAC packed reflow
 -> staged reopen
 -> exact authored-child rematerialization
 -> exact preservation of every non-target physical slot
```

This proves a bounded real-retail size-changing SCM reflow path. It does **not** prove NBZ delivery or original `dmc3.exe` acceptance.

## Source authority

Real hash-bound child:

```text
resource       st001.scm
size           887,760 bytes
SHA-256        3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
parent slot    PAC physical slot 2
slot offset    3,485,792 / 0x353060
```

Real parent:

```text
resource       st001.pac
size           4,632,960 bytes
SHA-256        43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
format         PAC
slots          8
```

The child and parent are hash-bound real corpus evidence. The parent is not promoted to protected-install/original-resolver-selected provenance.

## Exact CI-built runner

The acceptance run used the Ubuntu artifact from PR #386 exact branch head:

```text
branch head SHA   08d9c7935316d9afa2991e3210a02561099a9011
workflow run      34759963639
Ubuntu            Build + full CTest PASS
Windows           Build + full CTest PASS
artifact id       10318353331
artifact name     dmc-rengine-linux-fcc9df0cd25addcc288c2f0ed36f59d37fba1b06
artifact digest   sha256:77b106594215287e9d7514910b016c1094288b944cd2d9d709dcf3d8c5d97c02
executable SHA    e22f7ee8cf31e26758c207658e20416abcfdbef6b83ea15681c5e920b2997a86
```

The artifact filename is workflow-generated and carries the PR merge-ref SHA label; the artifact metadata separately binds it to branch head `08d9c793...`.

## Controlled size-changing edit

Command class: `scm-append-break-vertex-copy` / typed API `append_break_vertex_copy()`.

Selected target:

```text
object             0
mesh               0
source vertex      166
source mesh count  167
output mesh count  168
```

The authoring operation copies one existing vertex across position/normal/UV/RGB data and appends the confirmed topology break bit `0x02`. This grows the serialized stream count while deliberately not creating an additional non-degenerate triangle.

Exact observed result:

```text
SCM_SIZE_CHANGING_REBUILD_OK
sourceVertices=167
outputVertices=168
sourceObjectVertices=167
outputObjectVertices=168
sourceSize=887760
outputSize=887776
sizeDelta=16
trianglesBefore=118
trianglesAfter=118
appendedTopology=2
reparse=PASS
publication=NO_REPLACE_PASS
```

`preserve_layout` rejected the stream-count change as required. `canonical_rebuild` succeeded and produced:

```text
size       887,776 bytes
SHA-256    467734f48d96c76eb44abb395280a8743fac0ce959dbd26c5ce226f928cb9cc2
```

A second independent run with the same exact CI binary produced byte-identical authored SCM output.

## Real parent PAC packed reflow

The authored SCM was passed to the provenance-bound `verify-scm-reintegration` path against physical slot 2 of the source parent.

Observed result:

```text
SCM_REINTEGRATION_VERIFIED
format=PAC
slot=2
targetAliases=1
sourceScmBytes=887760
authoredScmBytes=887776
sourceParentBytes=4632960
outputParentBytes=4632976
targetAliasRematerialization=PASS
nonTargetPhysicalSpansExact=PASS
authoredScmReparse=PASS
publication=NO_REPLACE_PASS
```

Rebuilt parent:

```text
size       4,632,976 bytes
size delta +16
SHA-256    adb978f58926f0f04cd2d72fe3fc12a410d9dd9c2277819f9637cb71d4b9c5f6
```

A repeated run produced the same parent bytes and SHA-256.

## Independent slot-preservation check

After rebuilding, the output PAC was reopened independently and every top-level physical slot was extracted and hashed.

| Slot | Source offset | Output offset | Size | SHA-256 / result |
| ---: | ---: | ---: | ---: | --- |
| 0 | 48 | 48 | 48 | `7efcf182f28135a3d694324ba06715ca6f6b075a028d035c89c69809df23faa5` — exact |
| 1 | 96 | 96 | 3,485,696 | `976e6a6961686572c66cb6935f7e44dd450990016071604d99ff831ed9d36d5a` — exact |
| 2 | 3,485,792 | 3,485,792 | 887,760 -> 887,776 | source `3ed787...` -> authored `467734...` |
| 3 | 4,373,552 | 4,373,568 | 26,160 | `80b4b64318d84748c34a9e4f08065e35998e2e14e3766affb653e0f26adab77a` — exact |
| 4 | 4,399,712 | 4,399,728 | 256 | `36d40827c90e7ce0750a56c0f267be6721dd2b2e4bf30f4100a0cd6ecdabc07e` — exact |
| 5 | 4,399,968 | 4,399,984 | 167,888 | `500dadbc8e0db8d8d232fa04c49db437d41cbe6c50e555e24d91e758c6bcc4b0` — exact |
| 6 | 4,567,856 | 4,567,872 | 1,648 | `0f3d9952b61e3d1346dc9910c9c401fc0214d0d6fb2593791e0c000cc4599f64` — exact |
| 7 | 4,569,504 | 4,569,520 | 63,456 | `f0c0a2252f26d8e6f1538bc307b9c2ebf9a785e5d11b1fbe4ffc45672a3a5487` — exact |

All seven non-target top-level slots are byte-identical. Slots after the changed SCM shift by exactly `+16` bytes, matching the child growth. The nested PNST in slot 5 and nested PAC in slot 7 still expand through the canonical container path.

## Independent authored-child rematerialization

Physical slot 2 was re-extracted from the rebuilt parent after reopen:

```text
size       887,776 bytes
SHA-256    467734f48d96c76eb44abb395280a8743fac0ce959dbd26c5ce226f928cb9cc2
```

It is byte-identical to the authored SCM emitted before reintegration, and canonical SCM parse succeeds.

## Promotion result

The previous blocker **`real-retail size-changing canonical rebuild` is closed for this explicit bounded class**.

The evidence now supports:

- a typed size-changing SCM edit on a hash-bound real payload;
- mandatory preserve-layout rejection for that stream-count change;
- deterministic canonical layout reflow;
- canonical reparse of the larger SCM;
- real parent PAC packed reflow;
- exact authored-child rematerialization after parent reopen;
- byte-exact preservation of every non-target top-level physical slot.

## Evidence boundary

Still open and not promoted by this receipt:

- provenance-bound retail texture rewrite;
- broader PNST authored-delivery coverage for SCM;
- retail NBZ authored-resource delivery;
- original resolver selection of the authored overlay;
- original `dmc3.exe` acceptance and consumer-visible effect;
- universal/100% production SCM writer authority;
- Capcom offline-builder equivalence.

Machine receipt: `data/reverse/dmc3-scm-real-size-changing-pac-reintegration-attestation-20260913.json`.

No proprietary SCM or PAC payload bytes are committed.
