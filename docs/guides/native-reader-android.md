# DMC Native Reader for Android — DMC3 Resource Inspection

DMC Native Reader inspects supported resources from **Devil May Cry 3: Special Edition** inside **Devil May Cry HD Collection** on Android. Start with MOD/SCM model and scene inspection or DDS/PTX texture inspection, then check the exact build before relying on a capability.

## Choose a version before following a workflow

The [Native Reader README](https://github.com/VrUaCom/DMC-Native-Reader/blob/52210874f74d5526c2f1bd9000aaeab4594a03de/README.md) identifies **v68 / 1.0.41** as its promoted Android baseline. The source registry lists additional bounded modules beyond MOD/SCM/DDS/PTX. That source list does not establish every module's behavior in a downloadable APK.

The [download catalog](https://vruacom.github.io/dmc-rengine-cpp/downloads/) separately selects a public **code-72 / 1.0.45** APK from revision `7d1188e`. It is a debug/device-test artifact. Its [pinned release notes](https://github.com/VrUaCom/DMC-Native-Reader/blob/7d1188e30b447213156fdd7bdcb66a290805eaee/docs/releases/android-v72-1.0.45.md) do not establish a new physical-device acceptance result.

Use the catalog for the exact asset, revision, size and SHA-256, and the [capability matrix](https://vruacom.github.io/dmc-rengine-cpp/capabilities/) for the operation and platform being compared. Do not substitute current source claims for artifact-specific evidence.

## Prepare the resource

- Confirm your edition using [Start modding DMC3](https://vruacom.github.io/dmc-rengine-cpp/guides/start-modding-dmc3/).
- Follow the [NBZ workflow](https://vruacom.github.io/dmc-rengine-cpp/guides/extract-nbz/) when the resource is still inside an archive. Preserve archive/member/slot provenance and work on copies.
- Hand a supported leaf resource to the selected reader build. Archive extraction is a separate workflow; these instructions do not add NBZ unpacking or repacking to Native Reader.
- Keep unknown or unsupported data unsupported rather than assigning a guessed parser.

The practical [MOD](https://vruacom.github.io/dmc-rengine-cpp/guides/inspect-mod/), [SCM](https://vruacom.github.io/dmc-rengine-cpp/guides/inspect-scm/) and [DDS/PTX](https://vruacom.github.io/dmc-rengine-cpp/guides/extract-textures/) walkthroughs explicitly identify **Windows v1.0.0 Preview**. Their UI steps are not an Android walkthrough or Android/Windows parity proof.

## Platform and product boundaries

A public Windows x64 v1.0.0 technical preview documents read-only MOD/SCM/DDS/PTX viewing. A separate public iOS IPA is unsigned and requires signing; its pinned evidence records build checks, not an app launched and accepted on iPhone. See Downloads and Capabilities for both.

Native Reader is read-only. Inspection does not establish editing, writing, arbitrary conversion, reintegration or original-game acceptance. DMC Rengine remains the canonical parser and evidence authority, with GDSpaces responsible for its resource-resolution/materialization contracts.

Evidence reviewed: **2026-11-02**, against the pinned Native Reader README and release documentation. No new Android device test was performed for this page.
