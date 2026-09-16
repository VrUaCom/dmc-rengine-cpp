# LSG Base Human source

The Phase 1 macro-geometry source is the official MakeHuman HM08 base mesh.

- Repository: `makehumancommunity/makehuman`
- Source path: `makehuman/data/3dobjs/base.obj`
- Pinned source commit: `a8bc2d54ff0ac92e78ff71431b1023eda42bf482`
- Pinned Git blob: `d26635e9326e3cca30778fd7b9c00062b03cce09`
- License: CC0 1.0 for this asset. The OBJ itself contains the explicit CC0 release notice.

The MakeHuman application source code is **not** imported into DMC Rengine. CI fetches only the pinned CC0 OBJ asset, verifies its Git blob hash with `git hash-object`, and converts it with `lsg_prepare_mesh` to the experimental `RMS0` runtime format. This keeps the LSG runtime independent from MakeHuman software licensing and removes Blender/manual authoring from the build path.

Canonical source URL used by CI:

`https://raw.githubusercontent.com/makehumancommunity/makehuman/a8bc2d54ff0ac92e78ff71431b1023eda42bf482/makehuman/data/3dobjs/base.obj`
