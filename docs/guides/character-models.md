# How to Find DMC3 Character Models

Character-model searches such as Dante, Vergil, Lady or other playable and NPC resources should enter through the same archive-to-model pipeline and then rely on typed format evidence rather than filename guesses alone.

## Workflow

```text
NBZ
  -> member
  -> PAC / PNST
  -> materialized resource
  -> MOD or another confirmed model-related family
```

Keep archive/member/slot provenance attached to the materialized resource. Then verify the candidate with the canonical model reader before presenting it as a character model.

## Reader boundary

For MOD resources, current canonical read-side support includes hierarchy, transforms, meshes, texture-facing state and skinning-related data. That is enough for strong model inspection and viewer workflows while preserving unknown fields and unresolved semantics.

Character discovery does not imply unrestricted editing, conversion or original-game acceptance of rebuilt model files. Those remain separate authoring and runtime proof gates.

## Ecosystem path

GDSpaces/PocketGDS handles resource navigation, DMC Native Reader provides user-facing inspection where the current build exposes the relevant capability, and DMC Rengine remains the canonical C++20 parser and evidence authority.
