# DMC3 HD CLT ↔ model binding — em000 corpus pass (2026-09-08)

**Branch:** `reverse/clt-em000-20260908`  
**Corpus:** `em000-extract.zip` SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Scope:** all eight CLT resources in the supplied actor corpus.

## Primary result

All eight CLT resources are immediately followed in the top-level physical slot domain by a mesh-bearing model resource whose node hierarchy exactly fits the CLT `Bone` list.

```text
slot 002  em000_01.clt -> slot 003 MOD, 5 nodes
slot 006  em001_01.clt -> slot 007 MOD, 5 nodes
slot 009  em002_01.clt -> slot 010 MOD, 6 nodes
slot 011  em002_02.clt -> slot 012 MOD, 6 nodes
slot 014  em003_01.clt -> slot 015 MOD, 5 nodes
slot 016  em003_02.clt -> slot 017 MOD, 5 nodes
slot 020  em005_01.clt -> slot 021 MOD, 5 nodes
slot 022  em005_02.clt -> slot 023 EFM, 5 nodes
```

This is **8 / 8** physical adjacency matches. The last case is especially important: CLT is not MOD-only; its target can be the related EFM model family as well.

## Target hierarchy shape

Every one of the eight following model resources has a simple linear node chain.

Five-node targets:

```text
node 0 parent -1
node 1 parent  0
node 2 parent  1
node 3 parent  2
node 4 parent  3
```

Six-node targets:

```text
node 0 parent -1
node 1 parent  0
node 2 parent  1
node 3 parent  2
node 4 parent  3
node 5 parent  4
```

The CLT `Bone` rows select the tail of those exact chains:

```text
5-node target -> Bone 2, 3, 4
6-node target -> Bone 2, 3, 4, 5
```

No CLT bone index is out of range for its immediately following model, and every listed index is a direct parent-child continuation of the previous listed index.

Safe corpus conclusion:

> `Bone` is a node-index list for the associated cloth/deformation model resource, and in every em000 sample it describes the simulated tail of a linear local node chain.

This is now **CORPUS_CONFIRMED** at the association/structural level. Exact solver behavior for each selected node still requires executable consumer recovery.

## Two distinct node domains inside CLT

The two `em002` CLTs provide a critical distinction:

```text
em002_01.clt
  target model: 6 nodes, valid local indices 0..5
  Bone:         2,3,4,5
  WindParent:   9

em002_02.clt
  target model: 6 nodes, valid local indices 0..5
  Bone:         2,3,4,5
  WindParent:   13
```

`WindParent` therefore **cannot** be an index into the same local cloth-model node domain: values 9 and 13 exceed the target model's six-node range.

The surrounding `em002` actor model (`em000_008.mod`) has a 22-node main skeleton, where both 9 and 13 are valid node indices.

This strongly supports two separate domains:

```text
Bone       -> associated cloth-model local node domain
WindParent -> external/owning actor skeleton node domain
```

The latter high-level name is already present literally in the source text, but its exact attachment/wind-space runtime behavior remains to be traced. The cross-domain distinction itself is corpus-proven.

## Actor-cluster pattern

The top-level layout repeatedly forms a model/cloth cluster:

```text
main actor model
    -> CLT
    -> small chain model
```

or multiple CLT/model pairs under one actor family.

Examples:

```text
em002:
  MOD 008 : 22-node actor model
  CLT 009 : em002_01.clt, WindParent 9
  MOD 010 : 6-node chain
  CLT 011 : em002_02.clt, WindParent 13
  MOD 012 : 6-node chain

em003:
  MOD 013 : 22-node actor model
  CLT 014 : em003_01.clt
  MOD 015 : 5-node chain
  CLT 016 : em003_02.clt
  MOD 017 : 5-node chain
```

The `em005_02.clt -> EFM 023` pair proves the chain resource need not have MOD identity as long as it participates in the related model/node runtime family.

## Axis-token observations

Observed per-chain axis tokens:

```text
Y  -> five CLTs
NY -> two CLTs
Z  -> one CLT
```

Each CLT uses one axis token consistently for all of its `Bone` rows.

Do not yet promote:

```text
Y  = positive local Y constraint
NY = negative local Y constraint
Z  = positive local Z constraint
```

although those are natural candidates. The token-to-solver-axis mapping needs the CLT consumer or an independent test corpus.

## `LimitLength`

Only `em005_02.clt` contains:

```text
LimitLength 0
```

and that CLT targets the five-node EFM chain. One occurrence is insufficient to determine whether the key is EFM-specific, axis-specific, optional generally, or a solver mode. It remains a recognized optional raw key.

## Architectural consequence

CLT analysis should explicitly model two references without merging them:

```text
CltDocument
  cloth_local_bones[]       # source `Bone` rows
  wind_parent_raw           # source `WindParent`

analysis/clt/model_binding
  local_model               # immediately associated MOD/EFM chain candidate
  owning_actor_model        # external skeleton candidate
```

The raw parser must not perform container-neighbor binding. Physical/contextual association belongs in an analysis/profile layer so a standalone CLT remains parseable without inventing its owner.

## Direct EXE gate

The next canonical proof should locate the parser/consumer that stores:

```text
Bone
WindParent
WindLocal
WindType
```

and trace both node indices to their runtime model-manager arrays. That will determine whether `WindParent` is a joint index, transform selector, parent object index, or another external domain. Until then the corpus establishes the two-domain relationship but not the exact pointer chain.
