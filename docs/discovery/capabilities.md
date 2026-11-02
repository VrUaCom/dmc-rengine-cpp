# DMC Rengine — Capability matrix

Reviewed 2026-11-02. `site/tool-catalog.json` v2 records each matrix entry's
product, game, platform, version/source snapshot, formats, six independent
operations, release identity and evidence links. The web renderer reads this
single catalog for both the capability matrix and downloads.

## Evidence levels

- Release documented: stated by the exact published release; this review adds no device tests.
- Source documented: described at a pinned source revision, without promoting runtime acceptance.
- Build only: compilation or artifact evidence; execution on the target device remains unestablished.
- Tested development: bounded canonical development tests/receipts.
- Bounded support: only the edit/rebuild cases named by the cited receipts.
- Not supported: outside the reader's product boundary or the stated build.
- Not established: no adequate verification is promoted in this snapshot.
- Not applicable: the source layer is not itself a consumer preview/editing app.
- Planned: future work, not current released support.

## Scope

Native Reader entries distinguish the Android test APK, Windows technical
preview and unsigned iOS artifact. They never advertise editing, rebuilding or
original-game acceptance. iOS recognition/parsing/preview remain build-only.
Android source documentation is distinguished from test-APK device acceptance.

Canonical C++ MOD and SCM entries are separate: preserve-layout MOD authoring
and bounded PNST delivery do not imply the same authoring capability as SCM's
selected semantic edits and size-changing PAC rebuild/reopen. Neither entry
promotes universal original-game acceptance.

The GDSpaces entry describes the canonical source resource layer, not a public
Pocket GDS package. Original NBZ source archives remain read-only; bounded
container/overlay writers have separate evidence.

Blender, macOS and Linux availability stays explicit. An empty public Blender
project or a roadmap intention is not a working importer or an installer.

## Refresh contract

On a release update, re-read the public release metadata and the relevant
source/release evidence, pin the selected asset/source identity, download and
check the exact bytes, and update the review date. Never select an APK merely
by an ambiguous release label. Validate the catalog, build, check generated
links/SEO and complete publication acceptance before calling the site updated.
