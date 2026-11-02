# Why these five Phase-1 guides were selected

Decision date: **2026-11-02**.

## 1. How to Start Modding Devil May Cry 3 HD Collection

Route: `/guides/start-modding-dmc3/`

Why: broad entry intent exists across PCGamingWiki, Reddit, Steam and mod-framework documentation. The existing page was mostly conceptual. It now gives an edition check, backup rule, operation-to-tool routing, one reproducible first inspection and a clear boundary between DMC3 2006, HD Collection and PS2.

## 2. How to Extract Files from DMC3 NBZ Archives

Route: `/guides/extract-nbz/`

Why: archive access is the missing step between “I have the game” and “open MOD/SCM/DDS/PTX.” Public Steam/Nexus/Reddit evidence repeatedly asks how to open `dmc3-0.nbz` or find one resource. The guide describes a safe copy-first ZIP-compatible practical workflow while keeping the canonical DMC Rengine NBZ materialization model separate.

## 3. How to Open and Inspect DMC3 MOD Files

Route: `/guides/inspect-mod/`

Why: this matches a released first-party Windows tool and a strong standalone model-viewing task. The new version gives exact open/drag-drop steps, expected result, meaningful verification, texture troubleshooting and an explicit reader-versus-writer boundary.

## 4. How to Open and Inspect DMC3 SCM Files

Route: `/guides/inspect-scm/`

Why: SCM is a distinct scene-oriented task and must not be collapsed into MOD. The first-party Windows reader supports SCM inspection, while the core repository contains bounded SCM writer/rebuild evidence. The guide keeps those two maturity levels separate.

## 5. How to Open and Inspect DMC3 DDS and PTX Textures

Route: `/guides/extract-textures/`

Why: texture preview is supported by the Windows reader and active community texture-mod workflows demonstrate real user demand. The guide also solves an important technical misconception by separating model-facing slots, PTX framing, DDS-bearing payloads and runtime texture identity.

## Why Blender was not one of the five

The search intent is real and the existing `/guides/blender-import/` route remains useful. However, the first-party DMC Native Blender Reader currently has **no public add-on release** that can be promoted as a verified import/export workflow. Phase 1 therefore keeps Blender as a properly attributed discovery bridge to independent community projects rather than making it one of the five “completed first-party practical workflow” guides.
