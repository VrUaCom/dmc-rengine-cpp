# How to Find and Inspect DMC3 Enemy Models

Enemy-model searches are another practical entry point that should route users into the same evidence-aware model pipeline instead of relying on guessed filenames or one-off extraction recipes.

## Resource path

Use GDSpaces to locate the relevant archive member, preserve PAC/PNST slot provenance, then classify the leaf payload before inspection. For enemy model resources, MOD is the main model-family surface when the payload actually matches that format.

```text
NBZ
  -> member
  -> PAC / PNST
  -> candidate model resource
  -> MOD inspection
```

## What the canonical reader can expose

The current MOD reader can represent document, object and mesh structure, hierarchy, transforms, texture-slot relationships and skinning-related data strongly enough for read-only structural and visual inspection.

That supports an enemy-model viewer or research workflow, but it does not authorize unrestricted model editing, universal animation binding or original-game acceptance of rebuilt files.

## Why provenance matters

Different enemies, variants or stages can reuse neighboring resource families or container layouts. Keeping archive/member/slot provenance attached to each model is more reliable than publishing one guessed path as a universal answer.
