# DMC Modding Hub — First-party tool directory

This canonical guide records the intended division of responsibility between the DMC Rengine ecosystem's first four named tools. It is not an assertion that every feature is currently shipped.

## DMC Rengine C++

The canonical, evidence-backed C++ research framework for Devil May Cry 3: Special Edition from Devil May Cry HD Collection. It maintains format documentation, resource materialization and resolver architecture, guarded writers for specifically proven cases, reconstruction research and validation. It does not claim full whole-game decompilation or universal original-game writer equivalence.

Source: https://github.com/VrUaCom/dmc-rengine-cpp

## DMC Native Reader

The user-facing native resource viewer, with supported visual inspection of selected DMC3 models, scenes, textures, motions and other native modules. Operation and support differ by **published platform release**. Android's public GitHub release on the review date is v72 / 1.0.45; Windows is v1.0.0 Preview, and cross-platform feature parity is a planned milestone. Do not advertise iOS/macOS/Linux parity or a feature exclusively present in an unmerged branch.

Source and releases: https://github.com/VrUaCom/DMC-Native-Reader

## GDS / GDSpaces / Pocket GDS

The dataspace and resource-resolution layer for NBZ archive members, PAC/PNST containers and byte-provenance-aware navigation. Pocket GDS is the Android application; its source repository is private on the review date. It is a separate tool from format-specific Native Reader. A protected original archive must not be silently overwritten.

Canonical technical context: https://github.com/VrUaCom/dmc-rengine-cpp

## DMC Native Blender Reader

The planned Blender integration surface for canonical native format reading and relevant art workflows. The public repository currently has no releasable add-on documented on its default branch; do not promise one-click Blender import/export or release downloads.

Project repository: https://github.com/VrUaCom/DMC-Native-Blender-Reader

## Tool discovery rules

Every public tool card should eventually carry platform, release tag, supported operations, version-specific status, documentation and a trustworthy download URL. New ecosystem tools are added as entries to a catalog rather than being hard-coded into each article. Individual guides should show the *most relevant* first-party tool and provide an honest answer even when no first-party implementation is available.

See [website strategy](modding-hub-strategy.md) and [canonical implementation status](../status/current.md).
