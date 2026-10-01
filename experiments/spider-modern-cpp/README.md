# Spider modern C++ probes

These probes are intentionally outside the canonical build. The repository core
remains C++20; only the experiment executables use the selected C++23/C++26
language mode.

## Targets

### `dmc-rengine-spider-mod-skin-modern`

Performance/control probe over canonical `formats::mod::decode_vertex_skin()`:

- direct batch;
- Crusader batch;
- intentionally bad per-vertex Crusader dispatch;
- C++23 `std::expected` batch wrapper when available;
- C++26 `<simd>` feature detection when available.

The target fails if the tested paths do not preserve semantic parity.

### `dmc-rengine-spider-mod-skin-trifamily`

Architecture-control probe using all three Spider roles:

```text
Tarantula workflow
    -> Crusader native plan
    -> canonical MOD skin decoder
    -> workflow receipt
    -> Black Widow typed capability state
```

The Black Widow state deliberately reports skin editing as unavailable because
this experiment proves decode/orchestration only; it does not invent writer
authority.

## Build

```sh
cmake -S experiments/spider-modern-cpp \
      -B build/spider-modern-cpp-23 \
      -DSPIDER_EXPERIMENT_CXX=23 \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build/spider-modern-cpp-23 --config Release
```

Run:

```sh
./build/spider-modern-cpp-23/dmc-rengine-spider-mod-skin-modern
./build/spider-modern-cpp-23/dmc-rengine-spider-mod-skin-trifamily
```

For a C++26-capable compiler/library pair, configure another build directory
with `-DSPIDER_EXPERIMENT_CXX=26`.

See `docs/architecture/spider-modern-cpp-experiment.md` for research rationale,
acceptance gates and the initial exploratory benchmark result.
