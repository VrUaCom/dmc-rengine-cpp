# DMC Rengine public site scaffold

This directory contains the manifest and assets for the controlled DMC Rengine discovery/documentation site.

## Status

The site is **scaffolded but not enabled for public deployment**. Repository Pages/settings remain a separate gate because the current authenticated connector cannot safely modify those settings.

## Authority rule

The site is not a second technical source of truth. Every generated page points back to one canonical repository document through `source_path` in `manifest.json`.

Current implementation/status/evidence in `main` always overrides generated public summaries.

## Public content-quality contract

`site/manifest.json` is intentionally more than a title/meta registry. Every indexable page must provide a useful, distinct entry point while remaining weaker than its canonical technical source.

Each page therefore requires:

- a unique `summary`;
- at least two page-specific `sections`, each with substantive explanatory items;
- at least two descriptive `related_links` to other manifest routes;
- one valid canonical repository `source_path`.

The builder fails closed when primary content is too thin, two pages have identical primary-content signatures, a related route does not exist, a page links to itself, a related route is duplicated, or a canonical source path is invalid.

Durable public explanations may describe what a research area covers, what questions it helps answer, architecture relationships and where canonical evidence lives. Do **not** duplicate volatile support matrices, exact offsets/addresses, unpromoted reverse findings or completion claims into the manifest. Current capability and maturity remain governed by `docs/status/current.md` and the canonical format/evidence documents.

Internal-link labels should be descriptive enough to make sense outside surrounding prose. Do not add links or repeated terms solely to manufacture search signals.

## Build locally

```bash
python tools/build_discovery_site.py --output _site
```

To generate canonical tags and a sitemap for the planned default GitHub Pages URL:

```bash
python tools/build_discovery_site.py \
  --output _site \
  --base-url https://vruacom.github.io/dmc-rengine-cpp
```

The builder is aware that this is a GitHub **project Pages** site under `/dmc-rengine-cpp/`; generated navigation and asset links must retain that base path.

## Continuous validation

`.github/workflows/discovery-site.yml` builds the site on relevant pull requests and `main` changes, runs the builder regression tests, validates the expected public surfaces, project-base URLs, canonical tags, generated crawl-policy artifact and sitemap, and uploads the generated `_site` as a preview artifact.

## robots.txt scope

The builder emits a `robots.txt` alongside the generated site so the intended crawl policy is explicit and can be used directly if the site is later served at a controlled origin root.

For the planned **project Pages** URL `https://vruacom.github.io/dmc-rengine-cpp/`, that generated file would live at `/dmc-rengine-cpp/robots.txt`. Standard robots exclusion rules are fetched from the origin root (`https://vruacom.github.io/robots.txt`), not from a project subdirectory. Therefore the generated project-path `robots.txt` must **not** be treated as effective host-level crawler control or as proof that `OAI-SearchBot` is allowed by the origin.

If crawler policy needs to be controlled directly, use a root-controlled origin/custom domain and verify its actual `/robots.txt` response. The generated sitemap remains a normal site artifact and can be submitted directly through a verified Search Console property.

## Deployment gate

`.github/workflows/pages-deploy.yml` is fail-closed. The build/deploy jobs are skipped unless repository settings explicitly provide:

```text
DMC_RENGINE_ENABLE_PAGES=true
DMC_RENGINE_SITE_BASE_URL=https://vruacom.github.io/dmc-rengine-cpp
```

If a custom domain is selected later, `DMC_RENGINE_SITE_BASE_URL` must be changed before deployment so canonical tags and sitemap URLs do not point at the old Pages URL.

Before enabling Pages:

1. P1 discovery docs must be in `main`.
2. Repository metadata issue #294 should be applied or remain explicitly tracked.
3. Pages must be enabled in repository Settings → Pages with GitHub Actions as the publishing source.
4. `DMC_RENGINE_ENABLE_PAGES` and `DMC_RENGINE_SITE_BASE_URL` must be set deliberately.
5. The final public base URL must be confirmed before indexing is encouraged.
6. Search Console is configured only for a property that can be verified/controlled.
7. For project Pages, verify the actual origin-root crawler policy before making any claim about bot access. For a controlled custom domain, ensure its root `/robots.txt` does not block `OAI-SearchBot` when ChatGPT Search discovery is desired.

The generated site intentionally contains no proprietary game assets or binaries and does not require JavaScript or third-party analytics.
