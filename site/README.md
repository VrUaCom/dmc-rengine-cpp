# DMC Rengine public site scaffold

This directory contains the manifest and assets for the controlled DMC Rengine discovery/documentation site.

## Status

GitHub Pages is now enabled for the repository with **GitHub Actions** as the publishing source. The first public deployment is the remaining activation step; `.github/workflows/pages-deploy.yml` stays fail-closed and derives the canonical public `base_url` from GitHub Pages itself.

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

`.github/workflows/pages-deploy.yml` remains fail-closed without repository variables.

On every eligible run it first reads the repository's real Pages state through the GitHub Pages API using the workflow token:

- HTTP `404` means Pages is not enabled; the workflow records that state and performs no configure/build/upload/deploy steps;
- HTTP `200` means Pages is enabled; `actions/configure-pages@v5` becomes the canonical source of the actual Pages `base_url`;
- any unexpected API status fails the workflow instead of guessing whether publication is safe.

When Pages is enabled, the builder receives `steps.pages.outputs.base_url` directly. Canonical URLs, navigation, asset paths and `sitemap.xml` therefore follow the repository's actual GitHub Pages/custom-domain configuration rather than a duplicated repository variable.

No `DMC_RENGINE_ENABLE_PAGES` or `DMC_RENGINE_SITE_BASE_URL` repository variables are required.

Before enabling Pages:

1. P1 discovery docs must be in `main`.
2. Repository metadata issue #294 should be applied or remain explicitly tracked.
3. Pages must be enabled in repository **Settings → Pages** with **GitHub Actions** as the publishing source.
4. The first deployment must confirm the actual `base_url` reported by `actions/configure-pages` before indexing is encouraged.
5. Search Console is configured only for a property that can be verified/controlled.
6. For project Pages, verify the actual origin-root crawler policy before making any claim about bot access. For a controlled custom domain, ensure its root `/robots.txt` does not block `OAI-SearchBot` when ChatGPT Search discovery is desired.

If a custom domain is configured later in GitHub Pages, the deploy workflow consumes the new Pages `base_url` automatically on the next run. The public site still requires a fresh acceptance check after an origin change.

The generated site intentionally contains no proprietary game assets or binaries and does not require JavaScript or third-party analytics.
