# How to Find Vergil Models and Textures in DMC3 HD

Users searching for a DMC3 Vergil model, Vergil textures or Vergil HD Collection assets should enter through the same evidence-aware resource pipeline used for other character resources. DMC Rengine does not treat one guessed filename or archive slot as universal unless corpus evidence proves it across the intended scope.

## Resource workflow

```text
DMC3 HD archive
  -> NBZ member
  -> PAC / PNST container path
  -> materialized candidate resource
  -> MOD verification
  -> related PTX / DDS texture resources
```

Keep archive, member and slot provenance attached to every candidate. Then verify a model candidate with the canonical MOD reader before describing it as a structured Vergil model resource. The read-side model surface exposes hierarchy, transforms, meshes, texture-facing state and skinning-related data strongly enough for inspection without upgrading that evidence into unrestricted authoring support.

## Texture workflow

Do not collapse every texture-bearing payload into one generic image identity. Follow model-facing texture relationships into the texture layer and distinguish PTX bundles, descriptor-plus-DDS slots and directly recognizable DDS resources according to the actual bytes and parser evidence.

This lets a user searching for Vergil textures move from a character-level question into the canonical DDS/PTX research without publishing an unverified universal file path.

## Capability boundary

Finding and inspecting a Vergil model does not prove that DMC Rengine can universally export, replace or rebuild every character resource for original-game acceptance. Extraction, read-side inspection, authoring and runtime acceptance remain separate proof gates.

For the broader character workflow see `character-models.md`; for the model structure see `inspect-mod.md`; for texture extraction see `extract-textures.md`.
