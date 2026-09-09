# DMC Rengine — IndexNow discovery path

## Purpose

DMC Rengine does not use Google Search Console. Public discovery is measured with independently observable search results, GitHub-native metadata, crawler/indexability receipts, external authority links and IndexNow notifications to participating search engines.

IndexNow is used only as a discovery notification mechanism. A successful submission means the endpoint received the URL batch; it does **not** prove crawling, indexing or ranking.

## Canonical production origin

```text
https://vruacom.github.io/dmc-rengine-cpp/
```

The site is hosted under a path prefix on the `vruacom.github.io` host. The IndexNow verification file is therefore published inside that same controlled prefix and submitted through `keyLocation`.

Current verification file:

```text
https://vruacom.github.io/dmc-rengine-cpp/6ef2bc73288e1e2b580119a4b1bfc2fa.txt
```

The file contains only the IndexNow protocol key. It is a public ownership-verification token required by the protocol, not an application credential and not a secret used to access repository or user data.

## Submission architecture

```text
main push affecting discovery surface
        ↓
Deploy Discovery Site
        ↓
build
        ↓
deploy
        ↓
production-http-acceptance
        ↓ success only
Notify IndexNow
        ↓
verify live key file
        ↓
fetch live site-index.json
        ↓
build bounded URL batch
        ↓
POST https://api.indexnow.org/indexnow
```

The notification workflow does not run after a failed Pages deployment. It builds the URL list from the live controlled `site-index.json`, requires `page_count == len(paths)`, rejects duplicate paths, enforces the 10,000-URL protocol ceiling, and ensures every submitted URL stays below the canonical `/dmc-rengine-cpp/` site prefix.

## Response evidence

Accepted protocol responses:

- `HTTP 200` — URL batch accepted;
- `HTTP 202` — URL batch received and key validation pending.

Other responses fail the notification job and are recorded as an IndexNow submission failure, not as a Pages deployment failure.

## Evidence boundary

```text
IndexNow accepted != crawled
crawled != indexed
indexed != ranking
ranking != proof that IndexNow caused the ranking
```

Public SERP checks remain the external visibility measure. IndexNow complements the sitemap and normal crawler discovery; it does not replace either.

## Related

- `docs/discovery/README.md`
- `docs/discovery/search-intent-v2.md`
- issue #294 — repository Topics
- issue #300 — specialist external authority/backlink
- issue #338 — public indexing/discovery measurement without Google Search Console
