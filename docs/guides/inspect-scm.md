# How to Open and Inspect DMC3 SCM Files

This guide targets searches such as **open DMC3 SCM file**, **DMC3 SCM viewer**, **DMC3 stage model format**, and **Devil May Cry 3 SCM format**.

SCM is a scene-oriented resource family. DMC Rengine's canonical research models scene nodes, object bindings, meshes, streams, transforms and texture-facing state without collapsing those domains into a simplified editor-only representation.

## What SCM inspection covers

The current research exposes the scene hierarchy, object/mesh relationships, positions, normals, UV data, topology reconstruction, transform relationships and legacy rendering-facing fields that have enough evidence to be typed.

This makes SCM useful for stage/world inspection and for understanding how scene structure reaches geometry and texture-facing state.

## Why SCM is different from MOD

MOD and SCM both contain geometry-related information, but they serve different resource roles and have different binary layouts. MOD is the main model-family surface for character/enemy-style documents; SCM is scene-oriented and carries its own node/object/mesh organization.

A tool should therefore route each file through its canonical parser instead of treating SCM as simply another MOD variant.

## Writer and authoring boundary

DMC Rengine has substantial SCM writer/rebuild research, but structural serialization alone does not prove equivalence with Capcom's original offline tooling or acceptance by every original-game path.

For SEO and user documentation we therefore distinguish **open**, **inspect**, **parse** and evidence-backed **rebuild research** from any stronger claim of universal stage editing or production reintegration.

## Ecosystem workflow

Use GDSpaces/PocketGDS to locate and materialize the containing resource, DMC Rengine for canonical SCM parsing/evidence, and DMC Native Reader where the current client exposes scene/model inspection capabilities.

For exact field layouts, writer modes, proof gates and current maturity, follow the canonical SCM format documentation and `docs/status/current.md`.
