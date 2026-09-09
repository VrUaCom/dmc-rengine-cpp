# DMC Rengine Search Intent v2

Status: implementation specification for the public discovery layer.

This document extends the evidence-bounded discovery strategy from brand/technical discovery into user-problem discovery. It does not authorize capability claims beyond `docs/status/current.md`.

## Funnel

```text
Problem -> Guide -> Tool / capability -> Technical research -> Brand / ecosystem
```

A user does not need to know the names DMC Rengine, GDSpaces, MOD, SCM, PAC, PNST, NBZ, DDS or PTX before finding the project. Public pages should also answer the language people use when they are trying to solve a task.

## Intent families

### Unpack and extract

Primary phrases include:

- Devil May Cry 3 unpacker
- DMC3 unpacker
- DMC3 HD unpacker
- Devil May Cry 3 extractor
- DMC3 file extractor
- DMC3 archive extractor
- how to unpack DMC3 files
- how to extract Devil May Cry 3 HD files
- DMC3 PAC extractor / unpacker
- DMC3 NBZ extractor / unpacker

These phrases describe search intent. Public copy must distinguish a verified extraction/materialization path from unsupported universal repacking or editing claims.

### Models

- DMC3 model extractor
- DMC3 model viewer
- how to extract DMC3 models
- how to open DMC3 MOD files
- how to open DMC3 SCM files
- DMC3 Dante model
- DMC3 enemy models
- DMC3 stage models

### Textures

- DMC3 texture extractor
- DMC3 texture viewer
- how to extract DMC3 textures
- DMC3 DDS textures
- DMC3 PTX textures
- how to open DMC3 textures

### Browse and inspect

- DMC3 file viewer
- DMC3 resource browser
- DMC3 archive browser
- DMC3 binary inspector
- Devil May Cry 3 modding tools

### Authoring lifecycle

Search vocabulary also includes `convert`, `edit`, `replace`, `repack` and `rebuild`. Those words may be targeted only on pages where the corresponding operation has a current evidence-backed implementation. A reader or parser is not advertised as an editor/repacker merely for SEO.

## Guide architecture

First-wave guides:

1. **How to Unpack Devil May Cry 3 HD Collection Files** — cornerstone task guide; routes into NBZ, PAC, PNST and resource-specific paths.
2. **How to Extract Files from DMC3 NBZ Archives** — numbered archive/materialization path.
3. **How to Extract Models from Devil May Cry 3 HD** — routes model discovery into MOD/SCM evidence and reader capabilities.
4. **How to Extract Textures from Devil May Cry 3 HD** — routes texture discovery into PTX/DDS evidence and reader capabilities.
5. **How to Open and Inspect DMC3 MOD Files** — model inspection with explicit maturity boundaries.
6. **How to Open and Inspect DMC3 SCM Files** — scene/model inspection with explicit maturity boundaries.

Second-wave guides are justified after the first wave has useful canonical support and indexable public routes:

- How to Extract DMC3 PAC Files
- How to Open DMC3 PNST Containers
- How to Find DMC3 Character Models
- How to Find Dante's Model and Textures in DMC3 HD
- How to Extract Enemy Models from DMC3
- How to Extract Stage Models from DMC3
- How to Extract DMC3 DDS Textures
- How to Inspect DMC3 Animations
- How DMC3 HD Stores Game Resources

Do not create near-duplicate pages for `unpacker`, `extractor`, `DMC3 unpacker`, and `Devil May Cry 3 unpacker`. One strong page should satisfy closely related intent variants and link to narrower guides.

## Public hub targets

The discovery site should grow toward these intent hubs without replacing canonical technical documentation:

```text
/unpacker/
/guides/
/models/
/textures/
/modding/
/formats/
```

Existing `/formats/*`, `/archives/nbz/`, `/gdspaces/`, `/reverse-engineering/`, `/status/` and `/faq/` routes remain technical/evidence entry points.

## Ecosystem discovery

DMC Rengine is the canonical C++20 reverse-engineering/reconstruction core, but public discovery may route users to other project products when they solve the requested task.

### DMC Native Reader

DMC Native Reader should be presented as a user-facing reader/inspector product only to the extent supported by its own canonical repository/status. Useful intent families include:

- DMC3 file viewer
- DMC3 model viewer
- DMC3 texture viewer
- open DMC3 files on Android
- DMC3 viewer on Windows
- DMC3 viewer on Web
- DMC3 viewer on iOS

Platform names must not be advertised as released/supported merely because they are roadmap targets. Cross-platform wording must be tied to actual build/product evidence.

### GDSpaces / PocketGDS

Where current product evidence supports it, archive browsing, resource navigation and extraction intents may route to GDSpaces/PocketGDS rather than pretending every user workflow belongs to the C++ core repository UI.

## Internal-link contract

A guide should normally link in both directions:

```text
user problem
  -> guide
  -> capability/product
  -> format or architecture research
  -> current status / evidence
  -> DMC Rengine ecosystem
```

Technical format pages should link back to relevant practical guides so the graph works for both modders and reverse engineers.

## Measurement

Track a stable query set by intent rather than only branded queries:

- Brand: `DMC Rengine`, `dmc-rengine-cpp`
- Problem: `how to unpack DMC3`, `how to extract DMC3 textures`, `how to extract DMC3 models`
- Tool: `DMC3 unpacker`, `DMC3 extractor`, `DMC3 model viewer`, `DMC3 texture viewer`
- Technical: `DMC3 MOD format`, `DMC3 SCM format`, `DMC3 NBZ`, `DMC3 PAC format`
- Ecosystem: `DMC Native Reader`, plus platform-qualified viewer queries only when platform claims are current

Record separately: not surfaced, surfaced, top 20, top 10, top 3. Crawlability, indexing and ranking remain different evidence states.

## External-search observation — 2026-09-09

Current public search results show that `DMC3 unpacker` / `PAC extractor` intent is real and already contested by legacy/community tools. Results include a Nexus Mods DMC3 HDC PAC unpacker and a GitHub DMC3 PAC extractor/repacker. This strengthens the case for task-oriented pages, but DMC Rengine must compete on accurate modern capabilities and evidence rather than keyword duplication.

The controlled Pages site is still early in external discovery. Do not infer indexing from successful crawler fetches.
