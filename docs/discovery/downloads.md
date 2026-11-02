# DMC Rengine — Public downloads

Verified on 2026-11-02. The structured source for packages and checksums is
`site/tool-catalog.json` (v2). Only public Native Reader release assets appear
as binary downloads. The three selected files were downloaded independently;
their byte sizes and SHA-256 values matched GitHub's release metadata.

## Android ARM64 — test APK

The selected file is `DMC-Native-Reader-v72-1.0.45-7d1188e.apk`, versionName
1.0.45 / versionCode 72. Its release tag is `android-v72-1.0.45`, whose notes
explicitly call this an NR Luna v73 debug/device-test build, not a production
release or physical-device acceptance. The same release also contains the
older 55f874d APK; the website selects the newer asset uploaded on 2026-09-30
and pins its exact source revision and checksum. The tag itself still resolves
to 55f874d; it must not be used as the source identity of the 7d1188e asset.

The pinned source README separately identifies v68 as the promoted Android
baseline. That README is not proof that every newer test-APK feature has been
accepted on a physical device. Download stage and operation evidence are
therefore shown separately.

## Windows x64 — technical preview

`DMC-Native-Reader-Windows-v1.0.0-preview.zip` is the published portable preview.
Its release notes describe read-only MOD/SCM viewing, DDS/PTX preview, overlays
and optional per-user Open-With registration. The site does not claim parity
with the newer Android source or equivalent physical-device acceptance.

## iOS ARM64 — unsigned developer artifact

`DMCReader-unsigned.ipa` is a public developer artifact, not a signed installer.
It cannot be installed by tapping it. Users need their own signing setup.
The pinned status records iOS compilation/build checks and explicitly says
the app has not been launched. No on-device preview acceptance is promoted.

## Other products and platforms

No public binary release was present in the checked DMC Rengine C++ or DMC
Native Blender Reader release listings. Pocket GDS remains outside public
binary distribution in this catalog. macOS and Linux have no verified public
Native Reader package in the checked listing. Source/documentation links are
provided separately; no guessed installer or private repository URL is shown.

Checksums verify artifact identity, not correctness, acceptance or trust in
all behavior. This website distributes no game files and does not rebuild or
re-sign the application packages.
