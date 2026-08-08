# ModViz Menu Editor

**Status:** canonical product direction / complete UI pending  
**Snapshot date:** 2026-08-08

ModViz has two top-level modes: the Scene/Model Editor and the Menu Editor. The Menu Editor is a consumer of GDSpaces resource identity, shared working-copy/export contracts, and EXE evidence/guarded patch planning. It must not resolve or rewrite archive/container topology itself.

## Scope

The Menu Editor should support:

- HUD/menu model meshes and object hierarchy;
- 2D/orthographic screen canvas;
- transforms and scale;
- texture/UV atlas regions;
- draw order and visibility;
- safe-area guides;
- counter and icon layouts;
- digit/icon slots;
- clone/insert/remove operations for supported UI meshes;
- icon remapping;
- preview of representative runtime values;
- linked executable evidence and guarded patch requests where runtime limits live outside the resource.

## First vertical slice: Red Orb counter

The first complete Menu Editor milestone is the DMC3 Red Orb HUD counter. It is deliberately narrow because it exercises multiple architecture boundaries at once:

```text
GDSpaces resource identity
  -> menu/HUD resource
  -> model/mesh hierarchy
  -> digit layout and UV/atlas data
  -> working-copy edit
  -> runtime formatting/limit evidence in EXE Editor
  -> guarded patch request when required
  -> preview and validation
  -> explicit export/build artifact
```

Acceptance should cover editing the counter model and extending/reconfiguring digit meshes while keeping executable-side formatting/render limits in EXE/Reverse Core evidence rather than embedding them in ModViz.

## Non-goals

- direct PAC/NBZ/AFS resolution or repacking in ModViz;
- an independent resource identity system;
- direct unguarded EXE writes;
- broad Menu Editor feature expansion before the Red Orb slice proves the end-to-end contracts.
