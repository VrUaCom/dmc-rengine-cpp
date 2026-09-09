# How to Inspect DMC3 Animations and MOT Research

Animation work in DMC Rengine crosses model hierarchy, evaluated transforms and the separate MOT research frontier. The safest public workflow is therefore inspection-oriented: identify the model node domain first, then relate animation data without pretending MOD itself owns every motion semantic.

## Model-side context

The canonical MOD reader already exposes hierarchy, local/world transform relationships, motion-group values and skin-palette relationships strongly enough for pose-aware inspection. This establishes the model-side node domain that animation data must target.

## MOT boundary

MOT reverse/parser work exists and the canonical DMC Rengine Native Reader integration registry includes a MOT module. That is a C++ integration fact.

The current DMC Native Reader Android application is a different product surface: its promoted `main` module set is intentionally narrower and currently includes MOD, SCM, DDS and PTX rather than MOT. Public wording must therefore distinguish **DMC Rengine registry presence** from **Android application promotion**.

A practical research path is:

```text
MOD hierarchy / node domain
  -> motion-group relationship
  -> MOT research data
  -> evaluated local transforms
  -> world pose / skin palette analysis
```

This is useful for reverse engineering and debugging animation binding, but it is not a claim of complete animation authoring, universal MOT revision coverage, Android MOT viewing, or original-game acceptance of edited motion data.

## What to use today

Use the canonical DMC Rengine MOD and MOT inspection/research surfaces for hierarchy and motion evidence. Use the Android DMC Native Reader only for capabilities promoted by its current application `main`. Check `docs/status/current.md` and the relevant application baseline before presenting a parser or integration module as a shipped user-facing feature.
