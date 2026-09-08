# Урок 2 — З яких ресурсів будується enemy

## 1. Resource graph на прикладі em000

```text
em000
├─ PTX                      textures
├─ MOD                      actor/model geometry + skeleton/skin
├─ CLT -> small MOD/EFM     cloth/deformation chain
├─ EFM                      related effect/model resource
├─ TSC                      texture/motion control text DSL
├─ PAC 035/036/037
│   └─ MOT x82              animation resources
├─ BIN 038/039/040
│   └─ SO working family    graph/link/spatial structures
└─ PNST 041                 effect pack
    ├─ G
    ├─ V
    ├─ E
    ├─ P
    ├─ T -> wrapped DDS
    ├─ A
    └─ M -> MOD + optional companion
```

## 2. MOD

MOD дає skinned geometry та node domain:

- objects / meshes;
- positions / normals / UV;
- blend indices / weights;
- hierarchy;
- local transforms;
- motion groups;
- texture slots;
- rest/world/skin matrix contract.

## 3. PTX

PTX є texture bundle. MOD вибирає texture через slot, але texture image фізично живе поза MOD.

## 4. MOT

MOT зберігає animation channel domain і compressed key tracks.

Current em000 grammar уже показує:

```text
header channel masks
record_count = sum(popcount(mask))
compression 2 / 3
quantized key tracks
```

MOT не повинен бути частиною MOD parser.

## 5. CLT

CLT — окремий text-serialized cloth/deformation format.

У em000:

```text
CLT Bone -> local small MOD/EFM chain
WindParent -> external actor-domain candidate
```

## 6. TSC

TSC — теж окремий text-serialized format.

Bound em000 sample:

```text
TexNo 2
```

корелює з texture slot 2 у EFM і PTX entry 2.

## 7. Effect pack

Effect pack — semantic layer над PNST.

Важлива correction:

```text
one manifest line != always one physical payload
```

`M` має primary MOD + optional companion.

Також E-records уже дають reference graph до T/A/M.

## 8. SO working family

038/039/040 утворюють пов'язану graph/link/volume family, але історичне ім'я Capcom ще не доведене.

Тому UI може показувати:

```text
SO (working identity)
```

але не повинен підміняти source identity фальшивим оригінальним `.so` extension.

## 9. Контейнери

PAC/PNST — не gameplay formats. Це physical slot containers.

Enemy composition має стояти над ними:

```text
container tree
 -> semantic resources
 -> cross-resource graph
 -> enemy definition
```
