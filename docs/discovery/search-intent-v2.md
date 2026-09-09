# DMC Rengine Search Intent v2

## Canonical funnel

```text
Problem -> Guide -> Tool / Capability -> Technical Research -> Brand / Ecosystem
```

The discovery layer should answer the practical question a user types first, then route toward the exact DMC Rengine capability and finally into canonical technical evidence.

## Problem language

Primary practical intents include:

- DMC3 unpacker / Devil May Cry 3 unpacker;
- DMC3 extractor / archive extractor;
- DMC3 NBZ extractor;
- DMC3 PAC extractor;
- DMC3 PNST browser;
- DMC3 model extractor / model viewer;
- DMC3 texture extractor / texture viewer;
- Dante model / Dante textures;
- DMC3 stage models / stage resources;
- DMC3 animations / MOT inspection;
- open or inspect DMC3 MOD;
- open or inspect DMC3 SCM;
- DMC3 modding tools;
- DMC3 reverse engineering.

One strong page should cover close spelling or phrasing variants of the same intent. Do not create thin near-duplicate pages for every keyword permutation.

## Guide layer

Guides translate real project capability into workflows ordinary users can follow. The current guide families are:

- unpacking DMC3 HD resources;
- extracting NBZ members;
- extracting PAC children;
- browsing PNST containers;
- extracting models;
- extracting textures;
- finding Dante-related models and textures;
- extracting stage-related models and scene resources;
- opening and inspecting MOD;
- opening and inspecting SCM;
- inspecting animation relationships and MOT research.

Guide wording is evidence-bounded. `extract`, `browse`, `open`, `view`, `inspect`, `edit`, `replace`, `repack` and `rebuild` are different operations and must not be used as synonyms.

## Tool / capability layer

### DMC Rengine

Canonical C++20 parser, reverse-evidence and reconstruction authority. Public claims should point back to `main`, format specifications, current status and evidence records.

### DMC Native Reader

User-facing reader/viewer product direction that consumes canonical native parser contracts. Platform and format claims must follow actual current build state, not roadmap intent.

### GDSpaces / PocketGDS

Archive and resource navigation, provenance-aware materialization and nested container browsing. They provide the path from NBZ/PAC/PNST resources into typed model, texture, scene and other readers.

## Technical research layer

Technical pages cover the underlying resource families and architecture:

- NBZ;
- PAC;
- PNST;
- MOD;
- SCM;
- DDS;
- PTX;
- HITS;
- SHW;
- Stage TXT;
- LIG/LIG2;
- DCA;
- executable/runtime evidence;
- MOT and other active research frontiers where promotion state is explicit.

The technical layer must remain stronger than the SEO wording: search pages summarize evidence, they do not replace it.

## Brand / ecosystem layer

The public entity should consistently bind:

```text
DMC Rengine
 -> Devil May Cry 3 / DMC3 HD
 -> C++20
 -> reverse engineering
 -> file-format research
 -> resource extraction and inspection
 -> evidence-backed modding and reconstruction work
```

The ecosystem can mention DMC Native Reader and GDSpaces/PocketGDS when the route genuinely benefits from them, but each project keeps a distinct product role.

## Capability boundaries

Current public wording may describe canonical NBZ materialization, recursive PAC/PNST expansion, supported typed readers and read-side model/texture/scene inspection where those capabilities are promoted.

Do not infer unrestricted writer authority, universal conversion, original-game acceptance or whole-game recompilation from those capabilities. Stronger operations require their own writer, rebuild and original-runtime proof gates.

## Content architecture

Current user-intent routes include:

```text
/unpacker/
/guides/
/guides/unpack-dmc3-hd/
/guides/extract-nbz/
/guides/extract-pac/
/guides/browse-pnst/
/guides/extract-models/
/guides/extract-textures/
/guides/dante-model-textures/
/guides/stage-models/
/guides/inspect-mod/
/guides/inspect-scm/
/guides/animations/
/models/
/textures/
/modding/
```

These routes should cross-link with canonical `/formats/*`, `/archives/nbz/`, `/gdspaces/`, `/reverse-engineering/` and `/status/` pages so the site forms one semantic graph rather than separate SEO and research silos.

## Indexing and measurement

Crawlability, indexing and ranking remain separate evidence states. Search Console activation and sitemap submission must be recorded only after the exact URL-prefix property is verified.

Measure the funnel with a stable keyword set grouped by intent:

- Brand;
- Problem;
- Guide;
- Tool / capability;
- Technical format;
- Ecosystem / platform.

Track transitions such as `not found -> indexed -> top 20 -> top 10 -> top 3` instead of treating page publication as ranking proof.

## Acceptance rules

- Every practical route answers a distinct user problem.
- Every page has a canonical technical source.
- Search wording never upgrades reader support into authoring support.
- Near-duplicate keyword pages are rejected.
- DMC Native Reader and GDSpaces/PocketGDS are mentioned only for real ecosystem roles.
- New guide waves must be added to the same manifest-driven Pages build and sitemap.
- Public routes must pass the discovery-site builder and exact-head CI before promotion.
