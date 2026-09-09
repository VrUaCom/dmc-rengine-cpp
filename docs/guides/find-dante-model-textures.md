# How to Find Dante Models and Textures in DMC3 HD

Users often search for a character by name rather than by binary format. This guide connects that practical intent to the evidence-aware DMC Rengine resource pipeline without assuming that every filename, slot or archive position is universal across all builds.

## Start from the resource hierarchy

Use GDSpaces to move from the numbered DMC3 archives into nested containers, then inspect materialized children by actual format and provenance:

```text
NBZ
  -> member
  -> PAC / PNST
  -> materialized resource
  -> MOD / texture resource / related data
```

For character-model work, MOD is the primary model research surface. Texture resources are handled separately and can appear as PTX or DDS-bearing representations depending on the physical container framing.

## What to inspect

For a candidate Dante model, verify the resource with the canonical MOD reader rather than relying only on a guessed filename. The current MOD reader exposes document structure, objects, meshes, hierarchy, transforms, texture-slot state and skinning-related data with evidence boundaries preserved.

For textures, use the canonical texture framing path to distinguish PTX bundles, descriptor-plus-DDS slots and directly recognizable DDS payloads. Model texture references and extracted image bytes are related but are not interchangeable identifiers.

## Why this guide avoids hard-coded one-file answers

DMC Rengine treats archive member identity, slot provenance and build-specific evidence as first-class data. A hard-coded "Dante is always file X" statement would be weaker than following the actual resource graph and validating the resulting MOD and texture payloads.

This page is therefore a practical search route into the canonical DMC3 model and texture research rather than a promise that one static path represents every installation or revision.
