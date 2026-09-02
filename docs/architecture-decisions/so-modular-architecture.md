# SO modular architecture policy

**Status:** accepted for `research/so-cpp20`  
**Date:** 2026-09-02  
**Scope:** every current and future SO-family implementation added on this research branch.

## Decision

SO functionality must be built as reusable C++20 library modules. No parser, reverse-analysis component, writer, visualizer, GDSpaces bridge, Binary Inspector bridge, or editor may become the owner of unrelated SO behavior.

The dependency direction is one-way:

```text
binary primitives
      ↓
format parsers / writers
      ↓
semantic + cross-resource analysis
      ↓
tool adapters
      ↓
UI / workflow
```

Lower layers must never depend on higher layers.

## Current module layout

```text
include/dmc_rengine/
├─ binary/
│  └─ reader.hpp
│
├─ formats/
│  ├─ so/
│  │  ├─ graph.hpp
│  │  ├─ link_table.hpp
│  │  └─ volume_table.hpp
│  ├─ mod/
│  │  └─ transform_domain.hpp
│  └─ so.hpp                  # raw-format convenience umbrella only
│
├─ analysis/
│  ├─ so/
│  │  ├─ companion_correlation.hpp
│  │  └─ mod_binding.hpp
│  └─ so.hpp                  # analysis convenience umbrella
│
└─ so.hpp                     # optional full convenience umbrella

src/
├─ formats/so/
│  ├─ graph.cpp
│  ├─ link_table.cpp
│  └─ volume_table.cpp
├─ formats/mod/
│  └─ transform_domain.cpp
└─ analysis/so/
   ├─ companion_correlation.cpp
   └─ mod_binding.cpp
```

The previous monolithic `src/formats/so.cpp` implementation is removed.

## Module contracts

### Binary layer

Owns byte reading, bounds checks, endian conversion and other generic binary primitives.

It must not know SO, MOD, GDSpaces, ModViz or UI semantics.

### Format layer

Owns the bounded binary grammar of one resource surface.

Examples:

- `formats::so::graph` — type-6/type-8 indexed structure;
- `formats::so::link_table` — raw four-byte companion records;
- `formats::so::volume_table` — raw 0x50 spatial records;
- `formats::mod::transform_domain` — the reusable MOD transform-domain subset.

A format parser should normally accept `std::span<const std::byte>` and return typed data plus `ParseDiagnostic`.

It must not:

- create UI objects;
- know which editor invoked it;
- perform cross-file semantic guesses;
- mutate GDSpaces state;
- label unproven gameplay semantics as facts.

### Analysis layer

Consumes typed parser output and computes relationships without reparsing raw bytes when avoidable.

Examples:

- `analysis::so::correlate_companions`;
- `analysis::so::analyze_mod_binding`.

Cross-resource logic belongs here rather than inside either source format parser.

### Tool adapter layer

Future adapters convert reusable parsed/analysed models into a tool-specific view. Examples:

```text
SO graph data ───────────────→ Binary Inspector document adapter
SO volume data ──────────────→ ModViz overlay adapter
SO + MOD binding analysis ───→ SO Inspector skeleton-binding view
SO parsed model ─────────────→ GDSpaces metadata/resource adapter
```

Adapters may depend on format/analysis modules. Format/analysis modules may not depend on adapters.

## Future writer rule

Read and write paths remain separate modules.

When mutation becomes evidence-safe, use a structure such as:

```text
formats/so/graph_writer.hpp
formats/so/link_table_writer.hpp
formats/so/volume_table_writer.hpp
```

A writer consumes typed model data and explicit preservation metadata. It must not be hidden inside an editor command or parser.

Round-trip/rebuild policy belongs in dedicated integration modules/tests, not in the parser.

## Future semantic modules

If EXE reverse proves additional semantics, add narrowly scoped modules rather than expanding a generic SO class. Examples:

```text
analysis/so/joint_binding.hpp
analysis/so/volume_semantics.hpp
analysis/so/runtime_graph.hpp
analysis/so/animation_binding.hpp
```

A semantic promotion must preserve the raw field representation so unknown bytes and previous evidence remain inspectable.

## Public include policy

Reusable code should include the narrowest header it needs:

```cpp
#include "dmc_rengine/formats/so/volume_table.hpp"
```

Use `dmc_rengine/formats/so.hpp` when a component intentionally needs all raw SO parsers.

Use `dmc_rengine/analysis/so.hpp` when it needs all SO analysis modules.

Use `dmc_rengine/so.hpp` only for high-level applications that intentionally consume the full SO stack.

## Test policy

Each module gets its own regression source and the branch keeps an aggregate SO gate.

Current regression sources:

- `tests/so_graph_tests.cpp`
- `tests/so_link_table_tests.cpp`
- `tests/so_volume_table_tests.cpp`
- `tests/mod_transform_domain_tests.cpp`
- `tests/so_companion_correlation_tests.cpp`
- `tests/so_mod_binding_tests.cpp`
- `tests/so_tests.cpp` — aggregate modular-API gate

The isolated production-module compile harness passes with:

```text
-std=c++20
-Wall
-Wextra
-Wpedantic
-Wconversion
-Werror
```

This is not a substitute for repository Windows + Ubuntu CI; it is the local strict compile gate for the new modular slice.

## Non-negotiable boundaries

1. No new monolithic `SOParser`/`SOManager` owning every behavior.
2. No UI dependency in binary, format or analysis layers.
3. No cross-file MOD logic inside raw SO parsers.
4. No raw SO parsing duplicated independently inside ModViz, Binary Inspector or GDSpaces.
5. No writer hidden in editor code.
6. No semantic field rename without retaining the raw representation and evidence status.
7. New functionality must expose a reusable C++20 API before a tool-specific adapter uses it.

This structure is intended to let every DMC Rengine tool reuse the same canonical decoding and analysis code without copy/paste or tool-specific forks.
