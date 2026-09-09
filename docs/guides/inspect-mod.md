# How to Open and Inspect DMC3 MOD Files

This guide targets searches such as **open DMC3 MOD file**, **DMC3 MOD viewer**, **DMC3 model viewer**, and **Devil May Cry 3 MOD format**.

DMC Rengine treats MOD as a model-format research surface with a canonical read-only Native Reader path. The goal is to expose what the evidence supports while preserving unresolved bytes and relationships instead of inventing editor-friendly semantics.

## What the MOD reader can expose

The promoted MOD work covers document/object/mesh structure, hierarchy and ordering, local transforms and world propagation, mesh texture slots, legacy texture-facing state, runtime topology generation, object controls and bounds, model texture companion relationships, inverse-rest ownership and packed skinning data.

That is enough for meaningful structural inspection, skeleton and weight visualization, texture-binding analysis and pose-aware research. It also lets tools present typed fields while retaining raw source/control bytes for unresolved variants.

## Hierarchy and skinning are not unrestricted authoring

A visible hierarchy does not mean every node has a fully decoded animation role. A correct-looking skinned model does not prove that every bind/inverse-bind relationship, mutation quantization rule or animation ownership rule is closed.

Current canonical status explicitly keeps MOD writer authority, byte-identical no-edit rebuild, complete TIM2 semantics, production texture replacement, complete animation authoring and original-game edited-MOD acceptance outside the promoted read-only claim.

## Related resources

MOD model data can refer into a separate texture domain, and animation behavior intersects with MOT research. Those neighboring resources must remain separate authorities rather than being flattened into a single "MOD contains everything" model.

Use the dedicated texture guide for DDS/PTX extraction and the canonical MOD research pages for field-level evidence. Use DMC Native Reader where the current build exposes model inspection, and use GDSpaces/PocketGDS to reach the physical archive/container slot that supplied the file.

## Evidence-first inspection workflow

When inspecting a MOD, keep four questions separate: what bytes are structurally present, what semantics are corpus-confirmed, what semantics are executable-confirmed, and what remains preserved but undecoded. DMC Rengine's evidence labels exist specifically to prevent a successful render from silently upgrading a hypothesis into format truth.
