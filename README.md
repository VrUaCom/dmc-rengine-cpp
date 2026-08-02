# DMC Rengine

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing, and eventually recompiling Devil May Cry 3 HD.

The project is built around a strict architecture:

- **GDSpaces** is the only resource access API.
- **EXE Editor** owns executable analysis, decompilation evidence, and guarded patch planning.
- **Binary Inspector** consumes bytes, regions, diagnostics, and evidence; it does not resolve game sources independently.
- **Stage Ops** receives typed `GDStageBundle` objects.
- **ModViz** contains a 3D Scene/Model Editor and a Menu/HUD Editor.
- Format editors, including Item Editor, are clients of GDSpaces.
- PAC, PNST, NBZ, and AFS are internal container layers, not top-level product architecture.

## Status

This repository is the clean C++ foundation for the next generation of DMC Rengine. It intentionally does not import the legacy project wholesale. Existing knowledge and proven behavior will be migrated through documented contracts, evidence, tests, and reviewed implementation phases.

## Initial scope

1. Establish the C++20/CMake foundation.
2. Define stable resource identities and GDSpaces contracts.
3. Build read-only source mounting and typed resource discovery.
4. Integrate Binary Inspector, Stage Ops, ModViz, and EXE evidence as clients.
5. Add safe working-copy, validation, patch-plan, and export pipelines.
6. Progress from evidence-backed decompilation units toward recompilable engine modules.

## Legal and repository policy

This repository does not contain Capcom game binaries, `dmc3.exe`, proprietary game assets, extracted archives, copyrighted resource blobs, or redistributed reverse-engineered binary data. Users must provide legally obtained game files locally.

DMC Rengine is an independent, community-driven research and modding project and is not affiliated with or endorsed by Capcom.

See [Architecture](docs/architecture.md), [Roadmap](docs/roadmap.md), and [Reverse Engineering Rules](docs/reverse-engineering-rules.md).
