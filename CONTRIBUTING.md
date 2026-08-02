# Contributing to DMC Rengine

DMC Rengine accepts contributions to code, documentation, tests, synthetic fixtures, reverse-engineering evidence, visual design, and project infrastructure.

## Before contributing

Read:

- `docs/architecture.md`
- `docs/reverse-engineering-rules.md`
- `docs/legal/clean-room-policy.md`
- `docs/brand/README.md`

The central architectural rule is non-negotiable:

> All tools receive resources through GDSpaces.

Editors must not introduce private loaders, competing resource identities, or direct container resolution.

## Contribution categories

### Engineering

- C++20 libraries and applications
- CMake and CI
- parsers and validators
- tests and synthetic fixtures
- diagnostics and developer tooling

### Reverse engineering

- evidence records
- reproducible runtime observations
- hashes, offsets, RVA/VA references, and parser traces
- corrected hypotheses
- independent clean-room interfaces

### Documentation and design

- specifications
- architecture diagrams
- tutorials
- UI concepts and original brand assets
- translations

## Evidence requirements

Reverse-engineering claims must state a confidence level:

`hypothesis`, `candidate`, `low`, `medium`, `high`, `confirmed`, `corrected`, or `rejected`.

A claim should include enough information for another contributor to reproduce or challenge it. Prefer small evidence packets over unsupported summaries.

## Prohibited repository content

Do not commit:

- Capcom executables or DLLs;
- extracted game archives or assets;
- copyrighted textures, models, audio, scripts, or binary blobs;
- leaked source code or confidential material;
- credentials, tokens, personal data, or machine-specific secrets;
- generated decompiler output copied wholesale without review and provenance.

Use synthetic fixtures and locally supplied legally obtained game files.

## Development workflow

1. Open or reference an issue/specification for non-trivial work.
2. Create a focused branch.
3. Add or update tests.
4. Keep architecture and status documents synchronized.
5. Run:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

6. Submit a pull request using the repository template.

## C++ rules

- C++20, no compiler extensions.
- Prefer value types, RAII, explicit ownership, and deterministic behavior.
- Avoid global mutable state.
- Avoid throwing across subsystem boundaries unless the contract explicitly permits it.
- Parse untrusted binary data with bounds checks and diagnostics.
- Do not hide source resolution inside UI code.
- Public APIs live under `include/dmc_rengine/`.
- Every new subsystem requires tests and a short architecture note.

## Commit guidance

Use clear imperative messages, for example:

- `feat(gdspaces): add local directory source`
- `docs(canon): record stage table evidence`
- `fix(exe): reject overflowing RVA conversion`
- `test(binary): add malformed region fixture`

## Brand terminology

Lore names are welcome in community pages and UI presentation, but technical APIs must use their canonical engineering names. The fictional Sect of Neuroslop and Order of the Inverted Triangle are satire/brand identity, not real organizations.
