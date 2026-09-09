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
- Vergil model / Vergil textures;
- DMC3 character models;
- DMC3 enemy models;
- DMC3 weapon models / Yamato / Rebellion / Beowulf / Force Edge;
- DMC3 stage models / stage resources;
- DMC3 animations / MOT inspection;
- DMC3 Blender import / DMC3HDC Import Tools / MOD Blender / SCM Blender / MOT Blender;
- DMC3 Android viewer / model viewer Android / texture viewer Android;
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
- finding Vergil-related models and textures;
- finding character models;
- finding and inspecting enemy models;
- finding named weapon resources such as Yamato, Rebellion, Beowulf and Force Edge;
- extracting stage-related models and scene resources;
- opening and inspecting MOD;
- opening and inspecting SCM;
- inspecting animation relationships and MOT research;
- bridging extracted/verified DMC3 resources into independent Blender community tooling;
- using the current DMC Native Reader Android application surface for promoted MOD, SCM, DDS and PTX inspection.

Guide wording is evidence-bounded. `extract`, `browse`, `open`, `view`, `inspect`, `import`, `export`, `edit`, `replace`, `repack` and `rebuild` are different operations and must not be used as synonyms.

## Tool / capability layer

### DMC Rengine

Canonical C++20 parser, reverse-evidence, integration and reconstruction authority. Public claims should point back to `main`, format specifications, current status and evidence records.

The DMC Rengine Native Reader integration registry is a C++ integration surface and is broader than any one user-facing application. Registry presence is evidence that a module is integrated in the core, not proof that every consuming product exposes that module.

### DMC Native Reader

Current Android-facing reader/viewer application. Its promoted `main` surface is intentionally narrower than the DMC Rengine integration registry and currently includes:

- MOD;
- SCM;
- DDS;
- PTX.

Platform and format claims must follow the actual application build state, not roadmap intent. Windows, Web and iOS are not advertised as shipped Native Reader platforms until their own implementation and build evidence are promoted.

### GDSpaces / PocketGDS

Archive and resource navigation, provenance-aware materialization and nested container browsing. They provide the path from NBZ/PAC/PNST resources into typed model, texture, scene and other readers.

### Community Blender tooling

Independent Blender/DCC projects such as `deshayu/DMC3HDC-Import-Tools` and `HansLichtner/DMC3-Blender-Import-Addon` can be referenced when they directly answer Blender-import search intent. They remain external projects with their own compatibility and support contracts.

DMC Rengine must not claim a native Blender importer/exporter merely because another project consumes DMC3 MOD, SCM, MOT or texture resources. The discovery layer can bridge the workflows while keeping product ownership explicit.

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
- MOT and other active research frontiers where parser, registry, DCC-tool and application-promotion states are kept distinct.

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

The ecosystem can mention DMC Native Reader, GDSpaces/PocketGDS and relevant independent community tools when the route genuinely benefits from them, but each project keeps a distinct product role.

The public wording must distinguish:

```text
DMC Rengine Native Reader integration registry
    !=
DMC Native Reader Android application module set
    !=
independent Blender import/export tooling
```

This prevents a canonical C++ integration module or an external DCC addon from being misreported as an already-shipped DMC Rengine or Android application feature.

## Capability boundaries

Current public wording may describe canonical NBZ materialization, recursive PAC/PNST expansion, supported typed readers and read-side model/texture/scene inspection where those capabilities are promoted.

Do not infer unrestricted writer authority, universal conversion, Blender import/export, original-game acceptance or whole-game recompilation from those capabilities. Stronger operations require their own writer, DCC-tool, rebuild and original-runtime proof gates.

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
/guides/vergil-model-textures/
/guides/character-models/
/guides/enemy-models/
/guides/weapon-models/
/guides/stage-models/
/guides/inspect-mod/
/guides/inspect-scm/
/guides/animations/
/guides/blender-import/
/native-reader/android/
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
- Ecosystem / platform / community-tool.

Track transitions such as `not found -> indexed -> top 20 -> top 10 -> top 3` instead of treating page publication as ranking proof.

## Acceptance rules

- Every practical route answers a distinct user problem.
- Every page has a canonical technical source.
- Search wording never upgrades reader support into authoring support.
- Core registry coverage and application feature coverage are reported separately.
- Community-tool compatibility is not reported as native DMC Rengine ownership.
- Near-duplicate keyword pages are rejected.
- DMC Native Reader and GDSpaces/PocketGDS are mentioned only for real ecosystem roles.
- New guide waves must be added to the same manifest-driven Pages build and sitemap.
- Public routes must pass the discovery-site builder and exact-head CI before promotion.
