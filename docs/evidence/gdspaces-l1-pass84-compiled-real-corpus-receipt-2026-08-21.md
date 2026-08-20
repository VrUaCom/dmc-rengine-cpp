# GDSpaces L1 Pass 84 — Compiled Real-Corpus Texture Reflow Receipt — 2026-08-21

## Scope

Layer 1 only. This receipt upgrades the preserved v6 texture evidence from independent-algorithm validation to execution of the exact compiled current C++ Pass 82 writer on real physical texture-slot bytes.

## Source corpus

- source package: `DMC 3 RENGINE (6).zip`
- source package SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`
- prepared physical slots: 45
- authoring cases: 112
- source-slot SHA mismatches before execution: 0

## Compiled runner identity

- source head: `2bf1c9a975167cb55bd327d3740c49963d10d834`
- GitHub Actions run: `32425047839` / Build #1158
- Ubuntu job: SUCCESS, including build, tests and runner upload
- runner artifact: `dmc-rengine-texture-corpus-reflow-linux`
- artifact SHA-256: `0fb417ba0f4251bb4fc4da32f873d84ece5e079fbd313bcb53aeaa954e3bf33a`

The Windows job on this historical runner head failed only because Pass 83 still contained the already-localized MSVC `char -> std::byte` test-helper narrowing issue. Pass 82 itself had already passed whole-head Ubuntu + Windows before this corpus run. Pass 84 subsequently inherited the Pass 83 portability correction and requires a fresh whole-head gate on its new head.

## Exact current-writer corpus execution

The compiled runner uses the repository implementations of:

`TextureSlotFramingParser -> Dmc3DdsProfile -> TextureSlotPackedReflowWriter -> TextureSlotFramingParser`

Results across the prepared real-v6 matrix:

- attempted edits: 112
- successful edits: 112
- failed edits: 0
- unique physical slots covered: 45 / 45
- source-slot SHA mismatches: 0
- DXT1 edits: 47
- DXT5 edits: 65
- source secondary relation `same`: 12
- source secondary relation `half`: 100

Observed dimension transitions:

- 256x256 -> 512x512: 43
- 512x512 -> 1024x1024: 41
- 256x128 -> 512x256: 13
- 128x128 -> 256x256: 5
- 256x512 -> 512x1024: 4
- 512x256 -> 1024x512: 4
- 1024x1024 -> 512x512: 2

Every successful case was reparsed by the compiled runner after rebuild and returned an `ok` receipt with the expected authored dimensions and a new output-slot SHA-256.

## Meaning

This closes the prior `current_compiled_cpp_writer_execution = false` gap for the evidenced safe texture-authoring domain. The size-changing texture writer is no longer supported only by synthetic tests and an independent corpus algorithm; the exact compiled C++ implementation has now executed successfully across the full prepared 112-case real-corpus matrix.

## Non-claims / remaining L1 gates

This receipt does not establish:

- authoring of the 42 descriptor relationships with unresolved non-zero auxiliary metadata;
- a full compiled real PAC/PNST bottom-up rebuild receipt using these edited physical children;
- a real retail `dmc3-0.nbz` repack receipt;
- raw real `.lst` corpus closure;
- original DMC3 consumption of a rebuilt PAC/PNST/NBZ artifact;
- Layer 1 COMPLETE.

Current engineering readiness estimate remains approximately 92% until these real-artifact and game-consumption gates close.
