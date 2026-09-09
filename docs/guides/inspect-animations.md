# How to Inspect DMC3 Animations and MOT Research

Animation work in DMC Rengine crosses model hierarchy, evaluated transforms and the separate MOT research frontier. The safest public workflow is therefore inspection-oriented: identify the model node domain first, then relate animation data without pretending MOD itself owns every motion semantic.

## Model-side context

The canonical MOD reader already exposes hierarchy, local/world transform relationships, motion-group values and skin-palette relationships strongly enough for pose-aware inspection. This establishes the model-side node domain that animation data must target.

## MOT boundary

MOT reverse/parser work exists, but MOT is not yet part of the canonical built-in Native Reader module set. Public wording must therefore distinguish active animation research from promoted reader support.

A practical research path is:

```text
MOD hierarchy / node domain
  -> motion-group relationship
  -> MOT research data
  -> evaluated local transforms
  -> world pose / skin palette analysis
```

This is useful for reverse engineering and debugging animation binding, but it is not a claim of complete animation authoring, universal MOT revision coverage or original-game acceptance of edited motion data.

## What to use today

Use the canonical MOD inspection surfaces for hierarchy and pose-related context, then follow MOT research notes and parser evidence where available. Check `docs/status/current.md` before presenting MOT as a supported Native Reader format because promotion status can change independently from research progress.
