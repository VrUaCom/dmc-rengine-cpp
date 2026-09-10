# DMC3 SCM — hash-bound real node-translation edit and parent PAC reintegration

Date: 2026-09-10  
Project: DMC Rengine  
Branch: `reverse/mod-completion-20260907`  
Evidence class: `REAL_CORPUS_BOUND_SCM_TRANSLATION_AND_PARENT_PAC_REINTEGRATION`

## Purpose

Advance SCM authoring beyond the already-proved bounded `alpha_control` scalar edit with a second independent semantic edit class:

```text
hash-bound real SCM payload
 -> typed node translation edit
 -> writer-derived translation_magnitude
 -> preserve-layout writer
 -> exact-span byte guard
 -> parent PAC reintegration
 -> canonical reopen/extract
 -> inverse edit restoring exact child and parent bytes
```

This is not a no-op corpus pass and not a synthetic writer fixture.

## Source authority

The child is the hash-bound real SCM payload already used by the canonical SCM corpus:

```text
label   st001.scm
size    887,760
sha256  3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

Parent binding used for reintegration:

```text
parent            st001.pac
parent size       4,632,960
parent SHA-256    43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
physical slot     2
slot offset       0x353060
slot size         887,760
slot SHA-256      3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

The SCM child is hash-bound real-corpus authority. The complete parent PAC is not promoted here to protected-install provenance.

## Canonical typed path

The command is a thin harness over the existing canonical implementation:

```text
Parser
 -> set_node_translation
 -> Writer(WriteMode::preserve_layout)
 -> writer-owned reparse
 -> independent exact-span guard
 -> staged no-replace publication
```

`set_node_translation` owns the derived magnitude:

```text
translation_magnitude = sqrt(x*x + y*y + z*z)
```

The writer's source-bound guard still owns preservation of transform `+0x1C`.

## Selected transform

Node 0:

```text
scene block            0xB6A30
transform_rel          0xC0
transform record       0xB6AF0

source translation     (0, -27.5, 0)
source magnitude       27.5
rotation               (0, 0, 0)
reserved +0x1C         0
```

Selected bounded edit:

```text
(0, -27.5, 0)
 ->
(1, -27.5, 0)
```

Canonical derived magnitude:

```text
27.51817512512207
```

## Exact CI-built runner

PR: `#379`  
Branch head: `2f8d6477fbadc54bcc78447a2b1b46ab685732ef`  
Workflow run: `34441590590`  
Ubuntu artifact ID: `10138153714`  
Artifact digest: `sha256:2015498c13afd82067922adece2a07ea2daebd5bb98b203fec084c336ed56424`  
Executable SHA-256: `918c43004dfae25776a7b3ed2bfa0daf97977d83229ccac801f22f91f3a64f35`

The artifact name uses the pull-request merge-ref `${{ github.sha }}` label. The branch-head identity is recorded separately.

CI result for this code head:

```text
Ubuntu Build + full CTest   PASS
Windows Build + full CTest  PASS
```

## Canonical writer result

```text
SCM_TRANSLATION_EDIT_OK
node=0
transformOffset=748272
oldX=0
oldY=-27.5
oldZ=0
newX=1
newY=-27.5
newZ=0
oldMagnitude=27.5
newMagnitude=27.5182
changedBytes=4
sourceSize=887760
outputSize=887760
reparse=PASS
exactSpanGuard=PASS
publication=NO_REPLACE_PASS
```

Authored child:

```text
sha256  5d552c1144f2a17513455938afe988c6aa82727f50377141dd18c009b4a9dcd6
size    887,760
```

Independent raw diff found exactly four bytes:

```text
0xB6AF2: 00 -> 80
0xB6AF3: 00 -> 3F
0xB6AFC: 00 -> 39
0xB6AFD: 00 -> 25
```

All four bytes lie inside transform `+0x00..+0x0F`.

No byte in:

```text
rotation +0x10..+0x18
reserved +0x1C
```

changed.

## No-replace publication

A second attempt to publish a different translation into the already-existing output returned:

```text
exit code 12
destination-exists
```

The existing output SHA remained unchanged.

## Parent PAC reintegration

The authored SCM was reinserted through canonical `rebuild-relative-slot` into physical slot 2.

Result:

```text
source parent SHA-256  43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
output parent SHA-256  f0f7b90536cc8d05b1cf361baf83c9a554f75349796bcaf45f66d65a9502c91f
source size            4,632,960
output size            4,632,960
```

Independent parent diff:

```text
0x409B52: 00 -> 80
0x409B53: 00 -> 3F
0x409B5C: 00 -> 39
0x409B5D: 00 -> 25
```

These are exactly:

```text
slot 2 base 0x353060 + child offsets 0xB6AF2/3/C/D
```

Every non-target physical PAC slot is byte-identical to source.

## Canonical reopen and extraction

`list-container` successfully reopened and expanded the authored parent.

`extract-slot 2` produced:

```text
size    887,760
sha256  5d552c1144f2a17513455938afe988c6aa82727f50377141dd18c009b4a9dcd6
format  SCM
```

The extracted bytes are exactly equal to the authored child.

## Exact reversibility

Inverse edit:

```text
node[0] translation
(1, -27.5, 0)
 ->
(0, -27.5, 0)
```

restored the exact original SCM SHA:

```text
3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

Reintegrating that restored child back into the authored parent restored the exact original parent SHA:

```text
43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
```

Thus both child and parent images are byte-for-byte reversible for this bounded edit.

## Real transform-magnitude census boundary

The same hash-bound `st001.scm` contains 50 serialized scene transforms. A bounded census found:

```text
transforms                         50
non-zero translations             45
non-zero rotations                  0
non-zero transform +0x1C            0
stored magnitude bit-exact to
current C++ float derivation       46 / 50
stored magnitude within <= 1 ULP   50 / 50
1-ULP mismatch node indices         4, 12, 14, 44
```

This supports the Euclidean-length interpretation numerically for this sample, but it also establishes an important writer boundary: recomputing the magnitude with the current floating-point expression is **not universally byte-identical** to every stored retail magnitude. Therefore this receipt does not claim generic byte-exact magnitude reconstruction for all nodes. Exact inverse-byte restoration is proven for the selected node 0 edit because its original translation/magnitude pair round-trips exactly. Generic rollback remains source-image/preservation based.

## Promoted claims

Supported:

- hash-bound real SCM node-translation edit;
- writer-derived translation magnitude serialization;
- preserve-layout exact-span enforcement;
- rotation and transform `+0x1C` preservation;
- parent PAC physical-slot reintegration;
- canonical parent reopen and child extraction;
- exact child and parent reversibility;
- no-replace output publication.

Not supported:

- universal byte-exact translation-magnitude reconstruction for every SCM node;
- generic SCM transform authoring;
- rotation authoring acceptance;
- generic Native Reader/ModViz SCM write authority;
- protected-install provenance for the complete parent PAC;
- retail NBZ acceptance;
- original `dmc3.exe` acceptance;
- full SCM writer or Capcom-builder equivalence.

## Machine receipt

`data/reverse/dmc3-scm-real-translation-pac-reintegration-attestation-20260910.json`

No proprietary SCM or PAC payload bytes are committed.
