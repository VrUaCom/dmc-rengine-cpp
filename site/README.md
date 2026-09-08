# DMC Rengine public discovery site

This directory contains the manifest and assets for the controlled DMC Rengine discovery/documentation site.

## Status

GitHub Pages is enabled for the repository with **GitHub Actions** as the publishing source. Production is live at `https://vruacom.github.io/dmc-rengine-cpp/`. `.github/workflows/pages-deploy.yml` remains fail-closed, derives the canonical public `base_url` from GitHub Pages itself, deploys the generated artifact, and then performs independent live-origin HTTP acceptance.

The current manifest defines 14 evidence-bounded public HTML routes. The expansion from the original 12-route surface adds dedicated HITS collision and DDS/PTX texture research entry points without changing repository technical authority or claiming Google indexing/ranking.

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

Internal-link labels should be descriptive enough to make sense outside surrounding prose. Do not add links, pages or repeated terms solely to manufacture search signals. A dedicated landing page is justified only when a canonical evidence source supports a useful standalone search intent.

## Approved social-preview delivery

`site/assets/social-preview.png` is the exact project-approved 1280×640 RGB PNG used for image-backed share metadata. The builder validates its SHA-256 (`a81a726bcff355cad5a6b25ecc7d35570d178ffdc3f1dddce8750e45f17bf21f`) and dimensions before publishing it.

When an authoritative HTTPS `base_url` is available, every generated page points `og:image`, `og:image:secure_url` and `twitter:image` to `<base_url>/assets/social-preview.png`, emits the matching PNG type/dimensions/alt metadata, and uses `summary_large_image`. Builds without a `base_url` intentionally omit image URLs and retain the non-image `summary` card so no unstable or guessed public origin is published.

Drive, sandbox, temporary artifact and expiring URLs are not valid share-card image sources. The public raster is served from the same controlled Pages origin as the generated site.

## Build locally

```bash
python tools/build_discovery_site.py --output _site
```

To generate the production-style canonical tags and sitemap for the GitHub Pages URL:

```bash
python tools/build_discovery_site.py \
  --output _site \
  --base-url https://vruacom.github.io/dmc-rengine-cpp
```

The builder is aware that this is a GitHub **project Pages** site under `/dmc-rengine-cpp/`; generated navigation and asset links must retain that base path.

## Continuous validation

`.github/workflows/discovery-site.yml` builds the site on relevant pull requests and `main` changes, runs the builder regression tests, validates the expected public surfaces, project-base URLs, canonical tags, generated crawl-policy artifact and sitemap, and uploads the generated `_site` as a preview artifact.

The builder tests require unique titles, descriptions, summaries and substantive primary-content signatures across the manifest. They also validate breadcrumb hierarchy, descriptive internal links, canonical source links, image-backed social metadata, project-base handling and the exact sitemap route set.

## robots.txt scope

The builder emits a `robots.txt` alongside the generated site so the intended project crawl policy is explicit.

For the production **project Pages** URL `https://vruacom.github.io/dmc-rengine-cpp/`, that generated file lives at `/dmc-rengine-cpp/robots.txt`. Standard robots exclusion rules are fetched from the origin root (`https://vruacom.github.io/robots.txt`), not from a project subdirectory. Therefore the generated project-path `robots.txt` must **not** be treated as effective host-level crawler control or as proof that `OAI-SearchBot` is allowed by the origin.

Production run #8 independently observed the origin-root response as HTTP 404 and successfully fetched the project root as Googlebot and OAI-SearchBot. That observation is external state, not a permanent configuration guarantee, so the production acceptance job rechecks the actual host-level response and crawler fetchability after eligible deployments.

If crawler policy needs to be controlled directly, use a root-controlled origin/custom domain and verify its actual `/robots.txt` response. The generated sitemap remains a normal site artifact and can be submitted directly through a verified Search Console URL-prefix property.

## Deployment gate

`.github/workflows/pages-deploy.yml` remains fail-closed and uses GitHub's real Pages state rather than repository variables.

On every eligible run it first reads the repository's Pages state through the GitHub Pages API using the workflow token:

- HTTP `404` means Pages is not enabled; the workflow records that state and performs no configure/build/upload/deploy steps;
- HTTP `200` means Pages is enabled; `actions/configure-pages@v5` becomes the canonical source of the actual Pages `base_url`;
- any unexpected API status fails the workflow instead of guessing whether publication is safe.

When Pages is enabled, the builder receives `steps.pages.outputs.base_url` directly. Canonical URLs, navigation, asset paths and `sitemap.xml` therefore follow the repository's actual GitHub Pages/custom-domain configuration rather than a duplicated repository variable.

No `DMC_RENGINE_ENABLE_PAGES` or `DMC_RENGINE_SITE_BASE_URL` repository variables are required.

After deployment, `production-http-acceptance` verifies the live representative routes, exact approved social raster, OG/Twitter image contract, expected sitemap URL count, actual origin-root robots response, Googlebot/OAI-SearchBot root fetches and Twitterbot image-backed metadata. Expansion of the manifest must update this acceptance contract in the same change so CI cannot silently validate an obsolete public surface.

Repository metadata issue #294 may remain separately open while Topics are owner-deferred. Search Console property verification/indexing work is tracked separately in #338; successful HTTP acceptance must not be translated into an indexing or ranking claim.

If a custom domain is configured later in GitHub Pages, the deploy workflow consumes the new Pages `base_url` automatically on the next run. The public site still requires a fresh acceptance check after an origin change.

The generated site intentionally contains no proprietary game assets or binaries and does not require JavaScript or third-party analytics.
