# HITS / PR #26 overlap audit

Draft PR #26 changes:

- `CMakeLists.txt`
- Project Graph and ProjectWorkspace headers
- Custom Build Identity headers and implementations
- Custom Build tests

The HITS migration changes:

- `include/dmc_rengine/formats/hits.hpp`
- `src/formats/hits.cpp`
- `src/formats/hits_binary.cpp`
- `tests/hits_tests.cpp`
- `tests/hits_binary_tests.cpp`
- `tests/resource_analyzer_tests.cpp`
- HITS research documentation

There is no direct path overlap. Both branches share the same base commit `d72d8887b987ac464ac52034170b7bbdb933ce5e`.

The later editor/build integration must wait until PR #26 is merged and must use the resulting BuildRecord and IntegrationProject APIs rather than duplicating them.
