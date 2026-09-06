# DMC Rengine FAQ — DMC3 HD Reverse Engineering and Modding

This FAQ is the public entry point for common questions about **DMC Rengine**, **Devil May Cry 3: Special Edition**, and the **Devil May Cry HD Collection (DMC3 HD)**.

It is a discovery/readability layer, not a second technical authority. For live implementation status and evidence boundaries, use [Current Project Status](../status/current.md).

## What is DMC Rengine?

DMC Rengine is an open-source C++20 framework for reverse engineering, decompiling, editing and progressively reconstructing/recompiling Devil May Cry 3 HD.

The project combines executable research, binary/file-format documentation, resource materialization, archive/container handling, guarded authoring and validation under an evidence-first architecture.

## Which version of Devil May Cry 3 does it target?

The project targets **Devil May Cry 3: Special Edition from the Devil May Cry HD Collection on PC**. Exact executable or corpus claims are bound to the artifact identities documented in the repository; the project does not assume that every DMC3 executable build is byte-identical.

## Is DMC Rengine a complete DMC3 decompilation?

No.

DMC Rengine does **not** currently claim full DMC3 decompilation, whole-game behavioral equivalence, Capcom offline-writer equivalence, a complete desktop editor, or a behaviorally equivalent rebuilt executable.

Completion is gate-based. See [Current Project Status](../status/current.md) and the project roadmaps for the open proof requirements.

## Can DMC Rengine be used for DMC3 HD modding?

It already provides infrastructure relevant to safe modding workflows: resource discovery/materialization, archive and container handling, binary inspection, selected format readers, guarded working-copy changes and bounded reintegration/authoring paths.

That does **not** mean every DMC3 asset or format is editable today. Each format and authoring path has its own evidence and support boundary.

## What DMC3 formats does the Native Reader currently support?

The current canonical Native Reader set is documented in [Current Project Status](../status/current.md). At the current promoted status it includes readers for:

- DDS;
- PTX;
- HITS;
- DCA;
- LIG / LIG2;
- Stage TXT;
- SCM;
- MOD;
- SHW;
- PE / EXE.

PAC and PNST are handled as container parsers, while NBZ is handled as a source/materialization layer. Research existing outside that canonical reader set must not be presented as promoted reader support.

## Where can I find DMC3 file-format documentation?

Start with the [DMC3 HD Format Documentation Index](../formats/README.md).

The project maintains separate views for:

- format/resource purpose;
- physical/corpus presence;
- schema maturity;
- original-runtime evidence;
- current DMC Rengine product support.

These axes are intentionally not collapsed into one “supported/unsupported” label.

## What are NBZ, PAC and PNST in DMC Rengine?

In the current architecture:

- **NBZ** is treated as a source/materialization layer for numbered DMC3 archive volumes;
- **PAC** and **PNST** use relative-slot container handling with sparse/empty/alias identity preservation in the supported product paths.

For exact layout, resolver and authoring boundaries, use the GDSpaces and format documentation rather than this summary.

## What are SCM, MOD and SHW?

They are DMC3 resource families with dedicated reverse-engineering and parser work in the repository. Their exact semantics and maturity differ, so the project avoids reducing them to one generic “3D format” label.

Use the [format catalog](../formats/dmc3-hd-format-catalog.md), [format presence census](../formats/dmc3-hd-format-presence-census.md), and linked deep-reverse documents for current evidence.

## Does DMC Rengine support DMC3 texture research?

Yes, the repository contains canonical reader/integration work for DDS and PTX plus broader texture/resource framing research. Support must still be interpreted per format and operation: recognizing or reading a texture family is not automatically proof of universal editing or game-accepted writing.

## Can DMC Rengine rebuild DMC3 archives?

The repository contains bounded NBZ/PAC/PNST rebuild and reintegration capabilities, including protected original-file policies and reopen/rematerialization validation in supported paths.

However, full original-game acceptance is not inferred from synthetic rebuild success. The applicable original-process proof gates remain authoritative.

## Does the repository include Capcom game files or extracted assets?

No.

DMC Rengine does not distribute Capcom executables, proprietary game assets, extracted archives, leaked source code or unauthorized game data. Users must provide legally obtained game files locally where a workflow requires them.

## How do I build DMC Rengine?

The standard CMake path is:

```bash
cmake -S . -B build -DDMC_RENGINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

See the root [README](../../README.md) for presets and platform-specific commands.

## Where should I check whether a feature is really complete?

Use these authorities in order:

1. current `main` implementation;
2. [Current Project Status](../status/current.md);
3. evidence/reverse documentation linked from that status;
4. format-specific documentation and registries;
5. roadmap gates.

A README summary, FAQ answer, parser success, green synthetic test or historical branch must never override a stronger current authority.

## How can I contribute DMC3 reverse-engineering findings?

Read [CONTRIBUTING.md](../../CONTRIBUTING.md), the [reverse-engineering rules](../reverse-engineering-rules.md), and the [clean-room policy](../legal/clean-room-policy.md).

Useful contributions include reproducible binary observations, hash-bound corpus evidence, exact offsets/RVAs, corrected hypotheses, tests, synthetic fixtures, documentation and implementation work that respects the canonical GDSpaces/resource architecture.

## Is DMC Rengine affiliated with Capcom?

No. DMC Rengine is an independent community research/modding project and is not affiliated with or endorsed by Capcom.
