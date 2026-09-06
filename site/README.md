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

## Deployment gate

Before enabling Pages:

1. P1 discovery docs must be in `main`.
2. Repository metadata issue #294 should be applied or remain explicitly tracked.
3. Pages must be enabled through repository settings.
4. The final public base URL must be confirmed. If a custom domain is chosen, replace the planned base URL before deployment.
5. Search Console is configured only for a property that can be verified/controlled.
6. Crawl policy is reviewed; `OAI-SearchBot` should remain unblocked when ChatGPT Search discovery is desired.

The generated site intentionally contains no proprietary game assets or binaries and does not require JavaScript or third-party analytics.
