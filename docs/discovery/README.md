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

## Current production discovery state

As of 2026-09-08, the controlled GitHub Pages discovery site is live at:

```text
https://vruacom.github.io/dmc-rengine-cpp/
```

Canonical production evidence:

- GitHub repository `main` remains the technical/evidence authority;
- GitHub Pages is the public discovery/readability layer;
- the repository About website points to the Pages origin;
- the site exposes 12 controlled HTML routes plus `sitemap.xml`;
- canonical URLs resolve under the Pages project path;
- the approved image-backed OG/Twitter card asset is served from the same controlled HTTPS origin;
- `Deploy Discovery Site` run #8 on merge `22cfb1073f73d8290a04968dbf0f038048f01990` passed build, deployment, and independent post-deploy HTTP acceptance;
- the actual host-level `https://vruacom.github.io/robots.txt` returned HTTP 404 during that acceptance run;
- Googlebot and OAI-SearchBot both successfully fetched the live project root;
- no analytics or external scripts are present in the production discovery site.

Crawlability is therefore evidenced independently from indexing. Public web-search checks on 2026-09-08 did not yet surface the Pages site; Search Console/indexing work is tracked separately in issue #338 and must not be represented as already indexed.

Repository Topics are only partially owner-applied and remain intentionally deferred in issue #294. Do not treat partial Topic state as the reviewed final metadata set.

## Research-backed findings

- GitHub Topics are an official repository-discovery surface; the reviewed final set is tracked in #294 and must be applied deliberately rather than replaced with generic high-volume tags.
- `DMC3` must be treated as an explicit identity alias alongside `Devil May Cry 3` and `Devil May Cry HD Collection`.
- The README already contains strong technical material; the discovery problem is primarily information hierarchy, stable public entry points, metadata consistency, and external indexing rather than lack of substance.
- The format corpus is a major long-tail discovery asset: SCM, MOD, SHW, HITS, PAC, PNST, NBZ/GDSpaces, DDS/PTX and executable/runtime evidence.
- Stars and forks are useful social signals, but this project does not treat them as a proven primary GitHub ranking formula.
- The controlled GitHub Pages site provides a crawlable discovery surface while GitHub remains canonical source/evidence authority.

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

Reviewed 20-topic set:

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

Do not add a topic that is semantically false merely because it has higher search volume. The repository settings state may lag this specification while #294 remains open; this list is the reviewed target, not a claim that all 20 are already persisted.

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

The production GitHub Pages site acts as a discovery/readability layer, not a second technical authority.

Current information architecture:

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

The site links back to exact GitHub source/evidence documents for canonical details. Its generated canonical URLs, sitemap, breadcrumb metadata, image-backed social cards, and source links are validation surfaces; they do not promote technical maturity beyond repository evidence.

The project-path `robots.txt` artifact documents intended project policy but is **not** host-level crawler authority. Production acceptance must always inspect the actual origin-root `https://vruacom.github.io/robots.txt` before making a host-level crawler-policy statement.

## Search Console and indexing boundary

Search Console activation is tracked in #338.

Use a URL-prefix property for exactly:

```text
https://vruacom.github.io/dmc-rengine-cpp/
```

Do not claim ownership of `github.com`, `github.io`, or the entire `vruacom.github.io` host without actual authority.

After real property verification:

- submit `https://vruacom.github.io/dmc-rengine-cpp/sitemap.xml`;
- inspect representative public URLs;
- record Google crawl/index state as external evidence;
- keep crawlability, indexing, ranking, and attribution as separate claims.

Do not commit a placeholder verification token or claim Search Console ownership before Google confirms it.

## AI search boundary

For the controlled documentation site:

- keep public technical pages crawlable unless there is a deliberate reason not to;
- do not block `OAI-SearchBot` if ChatGPT Search discovery is desired;
- treat `GPTBot`/training policy as separate from search discovery;
- expose stable page titles, headings, descriptions and canonical links;
- preserve evidence labels and unsupported boundaries in public summaries;
- distinguish successful OAI-SearchBot fetch from actual search indexing/discovery.

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

- [x] add this canonical discovery specification;
- [x] bind `DMC3` directly in README identity/intro text;
- [x] replace stale README snapshot data with live status authority;
- [x] add a bounded current capability summary;
- [x] link this specification from the documentation index;
- [x] apply the approved About description;
- [ ] finish the reviewed Topics payload in #294;
- [x] validate promoted discovery code/docs through the normal CI path.

### P1 — public documentation surface

- [x] add FAQ/search-intent documentation;
- [x] improve the public format landing/index;
- [x] create release-readiness guidance;
- [x] create and owner-apply the repository social preview asset;
- [ ] finish independent repository shared-link render acceptance in #301.

### P2 — controlled web surface

- [x] publish GitHub Pages;
- [x] add canonical URLs, sitemap and reviewed project-path robots policy;
- [x] validate live public routes after deployment;
- [x] validate actual host-level robots response;
- [x] validate Googlebot and OAI-SearchBot live fetchability;
- [x] serve the approved image-backed OG/Twitter card from the controlled origin;
- [ ] connect the exact URL-prefix property to Search Console and submit the sitemap in #338;
- [ ] analytics remain deliberately deferred pending privacy/hosting review; analytics are not required for discovery acceptance.

### P3 — external authority and measurement

- [ ] complete Search Console/indexing measurement in #338;
- [ ] contribute to relevant reverse-engineering indexes (#300);
- [ ] publish technical research posts in DMC communities when a reproducible finding warrants one;
- [ ] distribute user-facing releases through appropriate modding channels when release gates are satisfied.

## Acceptance criteria

- `DMC3` is explicit in the public identity layer.
- README does not freeze an obsolete canonical commit/status snapshot.
- Capability claims remain aligned with `docs/status/current.md`.
- Topics contain no semantically false classifications.
- Format documentation has stable, readable entry points.
- Production Pages remains a discovery layer over canonical GitHub evidence.
- Crawlability claims require real live-origin evidence.
- Indexing/ranking claims require separate external evidence.
- No keyword stuffing is introduced.
- No `complete` claim bypasses the applicable completion gate.
- Discovery code changes pass the normal Windows + Ubuntu CI path before promotion.

## External research basis

- GitHub Docs: repository Topics are explicitly intended to help people find and contribute to projects; GitHub allows up to 20 topics.
- Google Search Central: clear page titles/headings and useful page content help search systems understand and present pages; URL-prefix Search Console properties can be scoped to a path the owner can actually verify.
- OpenAI publisher guidance: public sites can appear in ChatGPT Search; `OAI-SearchBot` access controls whether page content can be included in search summaries/snippets.

Public-source URLs and crawler behavior should be revalidated before major discovery-policy changes because search/crawler documentation can change over time.
