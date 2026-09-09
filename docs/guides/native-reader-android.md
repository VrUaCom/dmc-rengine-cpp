# DMC Native Reader for Android — DMC3 Resource Inspection

DMC Native Reader is the current Android-facing reader for Devil May Cry 3 HD Collection resources in the DMC Rengine ecosystem.

The public search intent for this page includes phrases such as `DMC3 Android viewer`, `DMC3 model viewer Android`, `DMC3 texture viewer Android`, `open DMC3 files on Android` and `Devil May Cry 3 resource viewer Android`.

## Current Android baseline

The current `DMC-Native-Reader/main` application surface is intentionally narrow and capability-driven:

- **MOD** — structural model reading through the canonical DMC Rengine reader and Architecture v2 adapter;
- **SCM** — scene/model reading through the canonical DMC Rengine reader and Architecture v2 adapter;
- **DDS** — bounded DMC3 DDS validation with image preview support;
- **PTX** — bounded texture-bundle reading with DDS child resources and parent navigation.

Unknown or unpromoted formats fail closed instead of being exposed through guessed wildcard parsing.

## Relationship to DMC Rengine

DMC Native Reader is the Android application surface. DMC Rengine remains the canonical C++20 parser, evidence and integration authority.

The DMC Rengine Native Reader integration registry is broader than the Android app's currently promoted module set. That broader registry must not be confused with features already shipped in the Android application.

## What this page does not claim

This page does not claim that DMC Native Reader is currently a released all-platform application. Windows, Web and iOS are not advertised as current product support unless their own implementation and build evidence are promoted.

It also does not claim unrestricted MOD/SCM editing, arbitrary resource conversion, or original-game acceptance of rebuilt resources. Reading and inspection are separate capabilities from authoring and reintegration.

## Where to continue

For model structure, continue to the MOD and SCM research pages. For texture inspection, continue to the DDS/PTX pages. For archive extraction and nested-resource provenance, use the DMC Rengine / GDSpaces unpacking guides before handing a materialized leaf resource to the Android reader.
