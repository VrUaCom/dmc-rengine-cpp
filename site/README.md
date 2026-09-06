# DMC Rengine public site scaffold

This directory contains the manifest and assets for the controlled DMC Rengine discovery/documentation site.

## Status

The site is **scaffolded but not enabled for public deployment**. Repository Pages/settings remain a separate gate because the current authenticated connector cannot safely modify those settings.

## Authority rule

The site is not a second technical source of truth. Every generated page points back to one canonical repository document through `source_path` in `manifest.json`.

Current implementation/status/evidence in `main` always overrides generated public summaries.

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

`.github/workflows/discovery-site.yml` builds the site on relevant pull requests and `main` changes, validates the expected public surfaces, project-base URLs, canonical tags, `robots.txt` and sitemap, and uploads the generated `_site` as a preview artifact.

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
7. Crawl policy is reviewed; `OAI-SearchBot` should remain unblocked when ChatGPT Search discovery is desired.

The generated site intentionally contains no proprietary game assets or binaries and does not require JavaScript or third-party analytics.
