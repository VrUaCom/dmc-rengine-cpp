# How to Extract Models from Devil May Cry 3 HD

This guide targets searches such as **DMC3 model extractor**, **Devil May Cry 3 model files**, **DMC3 MOD viewer**, and **how to find DMC3 character or stage models**.

DMC Rengine does not pretend that every model-like payload is one format. DMC3 HD model and scene data is reached through the resource hierarchy first, then interpreted by the appropriate canonical reader.

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

The canonical Native Reader includes both MOD and SCM modules. Current MOD evidence includes hierarchy, local transforms, world propagation, mesh texture slots, skinning-related data, inverse-rest ownership and runtime-facing model/texture relationships. This supports truthful read-only model inspection and skeleton/weight analysis.

SCM research exposes scene hierarchy, geometry relationships, stream data, transforms and texture-facing state with its own evidence boundaries.

## What this does not claim

A parsed or visualized model is not proof of unrestricted model editing. MOD writer authority, byte-identical no-edit rebuild, complete mutation rules, complete animation authoring, production texture replacement and original-game edited-MOD acceptance remain separate gates.

The same distinction applies to SCM: structural writer/rebuild work and original-game authoring equivalence are not synonyms.

## Which tool layer to use

Use **DMC Rengine** for the canonical C++20 parser, reverse evidence and resource/materialization architecture. Use **DMC Native Reader** as the user-facing reader/viewer layer where its current platform build exposes the relevant capability. Use **GDSpaces/PocketGDS** for archive/resource navigation and provenance-oriented browsing.

The ecosystem is intentionally layered so a user searching for a simple model viewer can reach the same canonical technical evidence used by reverse-engineering work.
