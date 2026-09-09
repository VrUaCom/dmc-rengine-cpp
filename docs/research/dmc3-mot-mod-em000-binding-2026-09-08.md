# DMC3 HD MOT ↔ MOD — em000 channel-domain correlation (2026-09-08)

**Branch:** `reverse/mot-em000-20260908`  
**Corpus:** `em000-extract.zip` SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Scope:** corpus correlation only; direct canonical-EXE binding remains the promotion gate.

## Primary result

The MOT `channel_domain_count` values in em000 are not arbitrary table lengths. They align exactly with real MOD node domains present in the same actor bundle:

```text
MOT channel_domain_count histogram
22 -> 77 motions
 3 ->  4 motions
 4 ->  1 motion
```

Top-level MOD node domains include exact corresponding model domains:

```text
22 nodes -> em000_008.mod, em000_013.mod, em000_018.mod, em000_019.mod
 3 nodes -> em000_033.mod
 4 nodes -> em000_034.mod
```

The 3-node and 4-node MOT domains therefore each have a unique same-sized top-level MOD candidate in this corpus. The 22-node domain has a still stronger family relationship described below.

## Shared 22-node core across large MOD variants

The following seven top-level MODs form one structural skeleton family:

```text
23 nodes: em000_001.mod, em000_004.mod, em000_005.mod
22 nodes: em000_008.mod, em000_013.mod, em000_018.mod, em000_019.mod
```

Across node indices `0..21`, all seven have:

- identical parent-by-node topology;
- identical motion-group projection by node;
- the same 22-node index domain;
- identical local transforms in six of seven files;
- in `em000_004.mod`, only node 0 rest translation differs; nodes 1..21 remain identical.

The common 22-node motion-group distribution is:

```text
group 0 ->  9 nodes
group 1 -> 13 nodes
```

The three 23-node MOD variants add exactly one extra node:

```text
node index 22 -> motion_group 2
```

while preserving the same core node indices `0..21`.

This means the 77 MOT resources with `channel_domain_count = 22` line up exactly with the stable core node-index domain shared by both the 22-node and 23-node model variants. The extra group-2 node in 23-node models lies immediately outside that MOT table domain.

Safe corpus conclusion:

> `MOT.channel_domain_count` is strongly correlated with the model animation/node index domain, and the 22-entry MOT table covers the common node indices 0..21 of the large em000 MOD skeleton family.

Direct `MOT table index == MOD node index` remains a semantic promotion target for canonical machine-code confirmation, even though the corpus fit is now very strong.

## 22-domain channel-mask stability

Across all 77 MOT resources with 22 entries, mask usage by table index is highly stable:

```text
index 0:
    0x1C0 -> 72
    0x000 ->  5

index 1:
    0x1C0 -> 76
    0x1F8 ->  1

indices 2..20:
    0x038 -> 77/77 at every index

index 21:
    0x038 -> 68
    0x1F8 ->  8
    0x03F ->  1
```

This is inconsistent with a random opaque table and consistent with a fixed per-node channel-selection domain.

The independent DMC3 importer maps these masks as translation/rotation/scale channel bits, which makes the pattern intuitive, but those high-level names remain externally corroborated semantic candidates until the canonical executable channel switch is rebound.

## Small-domain motions

### 3-entry domain

Four MOT resources use three table entries:

```text
[0x000, 0x000, 0x1C0]
[0x000, 0x1C0, 0x000]
[0x000, 0x000, 0x1C0]
[0x000, 0x000, 0x1C0]
```

Each therefore contains exactly three tracks, matching the popcount invariant. `em000_033.mod` is the only top-level MOD with a three-node domain.

### 4-entry domain

The single four-entry MOT is:

```text
[0x000, 0x007, 0x007, 0x007]
```

and therefore contains nine tracks. `em000_034.mod` is the only top-level MOD with a four-node domain.

These are strong same-bundle correlations, not yet path/name proof that each PAC is attached to that exact MOD.

## Important motion-group boundary

The 23-node large-model variants show an especially useful negative boundary:

```text
MOD node domain: 0..22
MOT 22-domain:   0..21
extra MOD node:  22, motion_group 2
```

No 22-domain MOT has an entry for that extra node.

Do **not** yet conclude that `motion_group 2` universally means “unanimated”, “weapon”, or any gameplay label. The safe statement is narrower:

> In this em000 skeleton family, the additional node carrying motion-group value 2 is outside the 22-entry MOT channel domain observed in the actor's main motion corpus.

That is a high-value target for direct `CMotionJoint.motion_group` and requested-group tracing once the canonical EXE bytes are available again.

## Next proof gate

The required direct executable chain is now precise:

```text
MOT channel-mask table index
    -> track group construction
    -> CMotionJoint / model node index
    -> motion_group selection
    -> evaluated local matrix
    -> MOD currentWorld[index]
```

Finding this chain will either promote the corpus model or show the exact remapping layer. No node-index semantic should be hard-coded before that pass.
