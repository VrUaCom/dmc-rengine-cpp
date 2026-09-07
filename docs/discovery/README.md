# DMC Rengine Public Discovery Strategy

This document defines the evidence-bounded public discovery layer for **DMC Rengine**, the C++20 reverse-engineering and reconstruction framework for **Devil May Cry 3: Special Edition from the Devil May Cry HD Collection (DMC3 HD)**.

It does not replace implementation, reverse-evidence, roadmap, or status authority. `main`, the evidence records, and `docs/status/current.md` remain canonical technical truth.

## Objective

Make DMC Rengine easier to discover through GitHub Search, web search, and AI-assisted search without keyword stuffing and without publishing claims that are stronger than the evidence.

The public identity should consistently bind these concepts:

```text
DMC Rengine
 -> Devil May Cry 3 / DMC3 HD
 -> reverse engineering
 -> decompilation and long-term recompilation research
 -> binary and file-format research
 -> evidence-backed modding and authoring tools
```

## Research-backed findings

- GitHub Topics are an official repository-discovery surface and the repository currently has no topics configured.
- `DMC3` must be treated as an explicit identity alias alongside `Devil May Cry 3` and `Devil May Cry HD Collection`.
- The README already contains strong technical material; the discovery problem is primarily its information hierarchy and stale public status snapshot, not lack of substance.
- The format corpus is a major long-tail discovery asset: SCM, MOD, SHW, HITS, PAC, PNST, NBZ/GDSpaces, DDS/PTX and executable/runtime evidence.
- Stars and forks are useful social signals, but this project does not treat them as a proven primary GitHub ranking formula.
- A future crawlable documentation site can become a controlled discovery surface for Google and AI search while GitHub remains canonical source/evidence authority.

## Search-intent map

### Identity

- DMC Rengine
- DMC3
- Devil May Cry 3
- Devil May Cry 3 HD
- Devil May Cry HD Collection

### Reverse engineering and reconstruction

- DMC3 reverse engineering
- Devil May Cry 3 decompilation
- DMC3 recompilation
- DMC3 binary analysis
- DMC3 executable research

### File formats and archives

- DMC3 file formats
- DMC3 SCM format
- DMC3 MOD format
- DMC3 SHW format
- DMC3 PAC format
- DMC3 PNST
- DMC3 NBZ archive
- DMC3 texture formats

### Tooling

- DMC3 HD modding tools
- DMC3 resource extraction
- DMC3 archive inspection
- DMC3 model and texture tooling

Generic `best DMC3 mods` discovery is not the primary target; DMC Rengine is infrastructure and research rather than a gameplay overhaul.

## Repository metadata specification

### About description

Recommended stable description:

> DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing and progressively recompiling Devil May Cry 3 HD, with evidence-backed file-format research and safe modding tools.

### Topics

Recommended topic set, subject to GitHub's maximum topic count and final metadata review:

```text
devil-may-cry
devil-may-cry-3
dmc3
devil-may-cry-hd-collection
reverse-engineering
game-reverse-engineering
decompilation
recompilation
binary-analysis
binary-formats
file-formats
cpp20
cplusplus
cmake
game-modding
modding-tools
resource-extraction
game-reversing
windows
x86-64
```

2026-09-07 topic-budget review: `game-engine-research` was removed after a live GitHub topic-surface check returned no repositories for that topic. `game-reversing` has an active, semantically aligned game reverse-engineering surface and replaces that slot without increasing the 20-topic budget.

Do not add a topic that is semantically false merely because it has higher search volume.

## README information hierarchy

The top public surface should answer, in this order:

1. What is DMC Rengine?
2. Which game/version does it target?
3. What can the current repository actually do?
4. Which canonical Native Reader/container capabilities exist?
5. What is explicitly not claimed complete?
6. Where is the live canonical status?
7. How do contributors reach the deeper evidence-first architecture?

Existing evidence boundaries, architecture rules, build instructions, legal policy and public lore remain available below the public entry layer.

## Canonical public claims

Current public summaries may describe:

- C++20/CMake core and CI;
- GDSpaces resource resolution/materialization infrastructure;
- NBZ, PAC and PNST container/materialization work;
- canonical Native Reader modules listed by `docs/status/current.md`;
- executable and runtime reverse-engineering research;
- evidence-first format research;
- bounded guarded authoring/reintegration where explicitly supported.

Do not present the following as complete unless their formal completion gates later pass:

- full DMC3 decompilation;
- whole-game behavioral equivalence;
- Capcom offline-writer equivalence;
- complete desktop editor;
- behaviorally equivalent rebuilt executable.

Research that exists outside the canonical Native Reader set, including EFM/MOT work until promoted, must not be presented as canonical reader support.

## Public knowledge site

A future GitHub Pages or custom-domain site should act as a discovery/readability layer, not a second technical authority.

Suggested information architecture:

```text
/
/formats/
/formats/scm/
/formats/mod/
/formats/shw/
/formats/pac/
/formats/pnst/
/archives/nbz/
/reverse-engineering/
/gdspaces/
/faq/
/status/
```

The site should link back to exact GitHub source/evidence documents for canonical details.

## AI search boundary

For any future controlled documentation site:

- keep public technical pages crawlable unless there is a deliberate reason not to;
- do not block `OAI-SearchBot` if ChatGPT Search discovery is desired;
- treat `GPTBot`/training policy as separate from search discovery;
- expose stable page titles, headings, descriptions and canonical links;
- preserve evidence labels and unsupported boundaries in public summaries.

## External discovery priorities

1. Specialist reverse-engineering/file-format indexes, especially `awesome-game-file-format-reversing` under Capcom / Devil May Cry.
2. DMC modding communities and technical Discord/Reddit posts with reproducible findings.
3. Nexus Mods only when there is a user-facing release/tool workflow worth distributing.
4. Descriptive backlinks that bind `DMC3` with `reverse engineering`, `file formats`, or the exact tool capability.

Avoid backlink spam and generic SEO directories.

## Release policy

Do not create a fake `1.0` for discoverability.

A public release must correspond to a real promotion milestone and state:

- exact supported capabilities;
- unsupported/experimental boundaries;
- build/run instructions;
- downloadable artifacts where appropriate;
- evidence/status links.

Releases are a trust and distribution surface, not a ranking claim.

## Implementation phases

### P0 — repository discovery layer

- add this canonical discovery specification;
- bind `DMC3` directly in README identity/intro text;
- replace stale README snapshot data with live status authority;
- add a bounded current capability summary;
- link this specification from the documentation index;
- prepare the exact About/Topics metadata payload;
- validate via pull request and Windows + Ubuntu CI.

### P1 — public documentation surface

- add FAQ/search-intent documentation;
- improve the public format landing/index;
- create release-readiness guidance;
- create a repository social preview asset.

### P2 — controlled web surface

- publish GitHub Pages or a custom-domain documentation site;
- add canonical URLs, sitemap and robots policy;
- connect the controlled domain to Search Console;
- validate OAI-SearchBot accessibility where desired;
- add analytics.

### P3 — external authority

- contribute to relevant reverse-engineering indexes;
- publish technical research posts in DMC communities;
- distribute user-facing releases through appropriate modding channels.

## Acceptance criteria

- `DMC3` is explicit in the public identity layer.
- README does not freeze an obsolete canonical commit/status snapshot.
- Capability claims remain aligned with `docs/status/current.md`.
- Topics contain no semantically false classifications.
- Format documentation has stable, readable entry points.
- No keyword stuffing is introduced.
- No `complete` claim bypasses the applicable completion gate.
- Discovery changes pass the normal Windows + Ubuntu CI path.
- Any future website remains a discovery layer over canonical GitHub evidence, not a competing source of truth.

## External research basis

- GitHub Docs: repository Topics are explicitly intended to help people find and contribute to projects; GitHub allows up to 20 topics.
- Google Search Central: clear page titles/headings and useful page content help search systems understand and present pages.
- OpenAI publisher guidance: public sites can appear in ChatGPT Search; `OAI-SearchBot` access controls whether page content can be included in summaries/snippets.

Public-source URLs should be revalidated before a major website launch because search/crawler documentation can change over time.
