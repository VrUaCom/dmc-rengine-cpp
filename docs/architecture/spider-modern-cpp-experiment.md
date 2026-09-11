# Spider modern C++ experiment — MOD skin control case

Status: **experimental / branch-scoped**  
Branch: `experiment/spider-python-migration`  
Date: 2026-09-11

## Question

Can newer C++ improve Spider orchestration without forcing the canonical DMC
Rengine modules away from the current C++20 baseline?

The first control case is the EXE/corpus-confirmed MOD skin contract because the
expected semantics are already known:

```text
BLENDINDICES.x = preserved/undecoded lane
BLENDINDICES.y = influence 0 matrix-row start
BLENDINDICES.z = influence 1 matrix-row start
BLENDINDICES.w = influence 2 matrix-row start
bone index = active lane / 4
packed q0/q1/q2 = three 5-bit weights, sum 31
bit 15 = independent topology-break state
```

The experiment deliberately reuses the canonical
`formats::mod::decode_vertex_skin()` implementation. Spider does not get a second
MOD parser or a second skinning algorithm.

## Architecture under test

All three Spider families participate, but at different levels:

```text
Tarantula
    workflow / resource lifetime / evidence experiment
        |
        v
Crusader
    compact native dependency plan
        |
        v
canonical MOD skin module
    direct C++ numeric work
        |
        v
Black Widow
    future typed product/session capability state
```

The benchmark focuses on the Crusader boundary because it is the easiest place
to accidentally put orchestration inside a hot loop.

## Modern-C++ policy under test

The repository remains C++20. The experiment executable is compiled separately
at either C++23 or C++26 and links to the unchanged C++20 core.

This tests a useful compatibility property:

> newer Spider frontends may consume a stable C++20 native core without making
> the whole engine adopt the newest language mode.

### C++23 candidates

`std::expected` is the strongest immediate candidate for Tarantula/Black Widow
workflow results. It gives a typed success/error channel without requiring a
scripting runtime or exception-driven control flow.

The experiment therefore compares a `std::expected` batch wrapper against the
existing direct result path when the library exposes `__cpp_lib_expected >=
202202L`.

We intentionally retain Crusader's current function-pointer binding model.
Replacing it with `std::function`/`std::move_only_function` would add abstraction
without a demonstrated need.

### C++26 candidates

C++26 was technically completed by WG21 in March 2026, while the ISO edition is
still progressing through the formal DIS/approval/publication process as of this
experiment date. Compiler/library support remains uneven.

Candidates worth tracking:

- `<inplace_vector>` — potentially useful for small bounded Spider plans and
  dependency lists because it can remove heap allocation from plan storage;
- `<simd>` — potentially useful inside authoritative numeric modules (matrix,
  skin, UV, corpus transforms), **not** inside Spider coordination itself;
- execution control / sender-receiver facilities — architecturally relevant to
  future scheduling, but not ready to replace the existing executor until
  compiler/library support and measured benefit are sufficient;
- contracts/reflection — potentially useful later for validation and generated
  registration, but too immature for a canonical dependency in this branch.

Current public compiler-support tables indicate GCC 16 as the first libstdc++
release with `std::inplace_vector` and partial `<simd>` support; execution-control
support is still not broadly implemented. Therefore the probe only reports
C++26 feature availability instead of making those facilities mandatory.

## Benchmark cases

`experiments/spider-modern-cpp/spider_mod_skin_modern.cpp` executes the same
synthetic valid MOD skin corpus through four paths:

1. **direct batch** — canonical `decode_vertex_skin()` called directly;
2. **Crusader batch** — one compact plan validates the batch then calls one
   native batch kernel;
3. **Crusader per vertex** — intentionally bad control case that executes a
   Spider plan for every vertex;
4. **C++23 expected batch** — fail-closed typed result wrapper, when available.

Every path must produce the same decoded-count and checksum. Performance numbers
are rejected as useful if semantic parity fails.

## Build

C++23:

```sh
cmake -S experiments/spider-modern-cpp \
      -B build/spider-modern-cpp-23 \
      -DSPIDER_EXPERIMENT_CXX=23 \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build/spider-modern-cpp-23 --config Release
./build/spider-modern-cpp-23/dmc-rengine-spider-mod-skin-modern
```

C++26-capable toolchain:

```sh
cmake -S experiments/spider-modern-cpp \
      -B build/spider-modern-cpp-26 \
      -DSPIDER_EXPERIMENT_CXX=26 \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build/spider-modern-cpp-26 --config Release
./build/spider-modern-cpp-26/dmc-rengine-spider-mod-skin-modern
```

Optional arguments are `vertex_count` and `repeats`:

```sh
./dmc-rengine-spider-mod-skin-modern 1000000 10
```

The executable prints feature-test values, timings, ratios and
`semantic_parity=ok` on success.

## Initial exploratory result

A standalone prototype of the same boundary was run before committing this
probe with GCC 14.2.0, `-O3 -march=native`, on the available Linux environment.
This is **not a canonical product benchmark** and must not be used as a release
performance claim.

Observed behavior:

- batch-level Crusader dispatch stayed close to direct batch execution, with
  run-to-run noise dominating a small difference;
- a C++23 `std::expected` batch wrapper also stayed close to the direct path;
- intentionally executing Spider per vertex was about **2.3x–2.6x slower** in
  the small synthetic dispatch control.

The important result is architectural, not the exact number:

> Spider should schedule **batch/native kernels**, never individual vertices or
> other hot-loop elements.

## Acceptance gates

Before promoting any newer language feature into a non-experimental target:

1. semantic parity with the C++20 authority must pass;
2. benchmark at least GCC, Clang and MSVC toolchains used by project products;
3. record wall time, allocations, peak memory and binary-size delta;
4. keep Android/NDK and WASM portability in the matrix where relevant;
5. require a measurable simplification or performance/robustness gain;
6. do not move format or numeric authority into Spider;
7. keep a C++20-compatible core boundary unless a separate migration is
   explicitly justified.

## Current decision

Use **C++23 selectively now** in Spider experiments, especially typed workflow
results such as `std::expected`.

Treat **C++26 as feature-probed experimental acceleration**. The most promising
near-term candidates are `std::inplace_vector` for compact plan storage and
`std::simd` for optional backends inside authoritative numeric modules once the
actual target toolchains support them.

Do not raise the repository-wide language standard yet.

## External references

- WG21 C++ committee: https://www.open-std.org/JTC1/SC22/WG21/
- ISO DIS 14882 project: https://www.iso.org/standard/91179.html
- C++23 compiler support: https://en.cppreference.com/cpp/compiler_support/23
- C++26 compiler support: https://en.cppreference.com/cpp/compiler_support/26
- C++ feature-test macros: https://en.cppreference.com/cpp/feature_test
