# How to Open and Inspect DMC3 MOD Files

This guide targets searches such as **open DMC3 MOD file**, **DMC3 MOD viewer**, **DMC3 model viewer**, **DMC3 MOD editor**, and **Devil May Cry 3 MOD format**.

DMC Rengine treats MOD as a model-format research surface with a canonical reader plus a deliberately bounded preserve-layout writer. The goal is to expose what the evidence supports while preserving unresolved bytes and relationships instead of turning the first writer gate into an unsupported "full MOD editor" claim.

## What the MOD reader can expose

The promoted MOD work covers document/object/mesh structure, hierarchy and ordering, local transforms and world propagation, mesh texture slots, legacy texture-facing state, runtime topology generation, object controls and bounds, model texture companion relationships, inverse-rest ownership and packed skinning data.

That is enough for meaningful structural inspection, skeleton and weight visualization, texture-binding analysis and pose-aware research. It also lets tools present typed fields while retaining raw source/control bytes for unresolved variants.

## What the preserve-layout writer can do

MOD Preserve-Layout Writer Gate 1 starts from an immutable original serialized image and allows only fixed-size edits whose serialized spans are already typed. The current bounded edit set includes object bounding center/radius, existing mesh positions, existing mesh normals and existing UV values. It does not synthesize a new physical layout or change stream/table cardinality.

The writer independently protects every byte outside the authorized edit spans and reparses output through the canonical MOD parser before success. Its provenance-bound retail no-edit corpus gate passes **38/38 MOD files** with exact source/output SHA equality, zero modified bytes across **882,736 source bytes**, and **38/38 canonical reopen**.

This is real writer evidence, but it is specifically **preserve-layout Gate 1** rather than unrestricted authoring.

## What is still outside the claim

A visible hierarchy or a successful preserve-layout write does not mean every node has a fully decoded animation role or that arbitrary model authoring is closed. Current evidence still blocks transform authoring, skin/blend-index authoring, texture-slot/material-state authoring, stream/cardinality changes, layout synthesis, texture-companion rewriting, PAC/PNST/NBZ reintegration of MOD writer output, and original `dmc3.exe` acceptance of either a rebuilt or edited MOD.

The project therefore does **not** claim a complete MOD editor, a canonical rebuild-from-typed-IR writer, production texture replacement authority, Capcom authoring-tool equivalence or a 100% MOD writer.

## Related resources

MOD model data can refer into a separate texture domain, and animation behavior intersects with MOT research. Those neighboring resources must remain separate authorities rather than being flattened into a single "MOD contains everything" model.

Use the dedicated texture guide for DDS/PTX extraction and the canonical MOD research pages for field-level evidence. Use DMC Native Reader where the current build exposes model inspection, and use GDSpaces/PocketGDS to reach the physical archive/container slot that supplied the file.

For writer-specific evidence, see `docs/research/dmc3-mod-preserve-layout-writer-gate-2026-09-09.md` in the canonical repository.

## Evidence-first workflow

When inspecting or authoring a MOD, keep five questions separate: what bytes are structurally present, what semantics are corpus-confirmed, what semantics are executable-confirmed, which exact byte spans are currently authorized to change, and what remains preserved but undecoded. DMC Rengine's evidence labels and writer receipts exist specifically to prevent a successful render or write from silently upgrading a bounded capability into complete format authority.
