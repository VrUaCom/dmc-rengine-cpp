# Reverse Decompilation Depth Layers (DL0–DL8)

This scale tracks **depth of understanding**, not confidence. Evidence labels such as `EXE CONFIRMED`, `DERIVED FROM VERIFIED RUNTIME`, `GAME VERIFIED`, `RESEARCH REQUIRED`, etc. remain a separate axis.

| Layer | Canonical name | Promotion gate |
|---|---|---|
| DL0 | Raw Binary Evidence | Exact bytes/location reproducible against a canonical file/hash. |
| DL1 | Machine Decode & References | Valid instruction/data decode and stable xrefs/code boundaries. |
| DL2 | ABI, Layout & Type Recovery | Widths, offsets, domains and call/layout contracts backed by repeated accesses or exact layout tests. |
| DL3 | Local Algorithm Semantics | Function/parser/evaluator/writer behavior reconstructed as equivalent pseudocode/C++ and locally verified. |
| DL4 | Object, Class & Ownership Semantics | State and methods tied to an owning object/component using RTTI, vtables, lifecycle and caller evidence. |
| DL5 | Subsystem Architecture | Producers, consumers, APIs, state model and lifecycle connected into one coherent subsystem. |
| DL6 | Runtime Integration & End-to-End Flow | Source/trigger, transformations and cross-system sink/persistence/render path are traced end to end. |
| DL7 | Game-Domain Semantics | Exact game meaning/name is independently bound to the recovered mechanism; convention-based naming is forbidden. |
| DL8 | In-Game Application & Behavioral Parity | Controlled runtime/game test reproduces the expected behavior and side effects; round-trip/parity is checked where relevant. |

## Pass fields

Every substantial reverse pass should track:

- **Target Layer** — intended depth for the pass.
- **Closure Layer** — deepest layer consistently closed across the declared pass scope.
- **Peak Layer** — deepest isolated finding reached by any result in the pass.

A single DL7 finding does not make an entire pass DL7.

## Promotion chain rule

Higher-layer claims must retain traceable provenance to the lower-layer evidence that supports them. Semantic promotion must never erase byte/ABI provenance.

Examples:

- A structure can be `DL2 + EXE CONFIRMED`.
- A suspected gameplay label can be a `DL7 target + RESEARCH REQUIRED`.
- Clean C++ parity without a game/runtime test cannot be promoted to DL8.

This taxonomy is format-agnostic and is intended for DMC Rengine reverse work beyond DMC3 as well.
