# Phase 1 distribution integration — 2026-11-02

## Native Reader → Hub

[Native Reader PR #121](https://github.com/VrUaCom/DMC-Native-Reader/pull/121) adds the Hub, five practical workflows, Downloads and Capabilities to its README, links its documentation index to the guide funnel, and adds a Windows v1.0.0 Preview overview. Nine unique Hub destinations returned HTTP 200 with canonical metadata before publication; local documentation links were checked.

The Windows overview is repository documentation. The existing GitHub release body has **not** been edited. No binary, release tag, package promotion or device-acceptance result was changed.

## Consistent platform guidance

The legacy Android landing page and canonical guide previously described an obsolete four-module-only `main` and did not account for the Windows preview or unsigned iOS artifact. They now distinguish:

- the README's promoted Android v68 / 1.0.41 baseline;
- the selected code-72 / 1.0.45 device-test APK from revision `7d1188e`;
- current source-module descriptions;
- Windows v1.0.0 Preview;
- the unsigned iOS developer artifact with build-only evidence.

The page points to the existing version-bound Downloads and Capabilities surfaces. Windows task-guide UI steps remain explicitly Windows-scoped. No Android device test was performed.

## Legacy unpacker reference audit

The current `site/` and `docs/guides/` sources contain no link to Nexus mod `/mods/438` or `rusakVi/DMC-3-HDC-.pac-Unpacker`. The historical 50-query evidence map retains its dated PAC-unpacking article reference (`/articles/35`); it is an observation record, not a current product recommendation. No ownership relationship or deletion outcome was independently established.

The project's `/unpacker/` route is a separate existing archive/extraction entry point. It was preserved. Removal of an external tool is not evidence of improved search indexing or ranking.

## Remaining owner actions

- **Nexus:** owner postponed the link insertion until account access is available. The prepared [owner block](nexus-owner-block.md) remains ready; no Nexus account action was taken.
- **GitHub Topics (#294):** the reviewed payload uses `asset-extraction`, replacing the older `resource-extraction` proposal. The GitHub connector provides no Topics/settings write operation; the fallback browser is signed out. The actual Topics remain `devil-may-cry` and `dmc`.
- **Windows GitHub release description:** no release-update connector is exposed, and the browser is signed out. The repository Windows overview supplies the guide links meanwhile, but is not a release-body edit.
- **External reference (#300):** no message or community submission was sent.

These are distribution/admin gaps, not reasons to fabricate search positions or declare indexing complete.
