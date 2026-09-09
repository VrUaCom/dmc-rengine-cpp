# How to Extract Models from Devil May Cry 3 HD

This guide targets searches such as **DMC3 model extractor**, **Devil May Cry 3 model files**, **DMC3 MOD viewer**, **DMC3 MOD editor**, and **how to find DMC3 character or stage models**.

DMC Rengine does not pretend that every model-like payload is one format. DMC3 HD model and scene data is reached through the resource hierarchy first, then interpreted by the appropriate canonical reader and, where separately proved, a bounded writer.

## Model extraction path

A practical discovery path is:

```text
DMC3-*.nbz
  -> PAC / PNST or direct member
  -> resource slot
  -> MOD or SCM when identified
```

MOD is the primary model-family research surface for character/enemy-style model documents. SCM is scene-oriented and covers scene nodes, objects, meshes, streams and transforms. They share some concepts but are not interchangeable binary formats.

## What DMC Rengine can inspect

The canonical Native Reader includes both MOD and SCM modules. Current MOD evidence includes hierarchy, local transforms, world propagation, mesh texture slots, skinning-related data, inverse-rest ownership and runtime-facing model/texture relationships. This supports structural model inspection, skeleton/weight analysis and pose-aware research.

SCM research exposes scene hierarchy, geometry relationships, stream data, transforms and texture-facing state with its own evidence boundaries.

## Bounded MOD authoring now exists

The promoted MOD Preserve-Layout Writer Gate 1 is stronger than read-only inspection but narrower than a general model editor. It can preserve the original physical layout while applying fixed-size edits to the already-typed object bounds, existing mesh positions, existing normals and existing UV values.

Its no-edit retail corpus gate passes 38/38 MOD files with exact source/output hashes, zero modified bytes across 882,736 source bytes and successful canonical reopen. Unauthorized byte changes fail closed.

This does **not** prove arbitrary geometry topology changes, transform or skin authoring, stream/cardinality changes, material/texture binding authoring, texture-companion rewriting, archive reintegration or original-game acceptance of edited MOD files.

## What this does not claim

A parsed, visualized or preserve-layout-written model is not proof of unrestricted model editing. Canonical layout planning from typed IR alone, complete mutation rules, complete animation authoring, production texture replacement, PAC/PNST/NBZ reintegration and original `dmc3.exe` edited-MOD acceptance remain separate gates.

The same distinction applies to SCM: structural writer/rebuild work and original-game authoring equivalence are not synonyms.

## Which tool layer to use

Use **DMC Rengine** for the canonical C++20 parser, reverse evidence, bounded writer gates and resource/materialization architecture. Use **DMC Native Reader** as the user-facing reader/viewer layer where its current platform build exposes the relevant capability. Use **GDSpaces/PocketGDS** for archive/resource navigation and provenance-oriented browsing.

The ecosystem is intentionally layered so a user searching for a simple model viewer or editor can reach the same canonical technical evidence used by reverse-engineering work without being promised capabilities that have not passed their own proof gates.
