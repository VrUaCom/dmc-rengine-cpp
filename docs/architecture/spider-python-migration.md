# Spider C++20 — Python orchestration migration experiment

Status: **experimental / isolated branch only**

Branch: `experiment/spider-python-migration`

## Goal

Evaluate whether orchestration currently implemented as Python scripts can be
expressed as compact Spider Plans and executed directly by native C++20 modules,
without embedding Python and without duplicating canonical format, PE, evidence,
or GDSpaces logic.

Spider takes inspiration from Python's composability, not from Python's runtime.
The production rule for this experiment is:

> No embedded scripting runtime. No duplicated format logic. Pure C++20 native execution.

## Why `extract_exe_window_packet.py` is the first target

`scripts/reverse/extract_exe_window_packet.py` explicitly identifies itself as
orchestration-only. The script validates a packet plan, launches
`dmc-rengine extract-exe-window` once per requested window, validates the child
receipts, checks known-body hashes, and publishes the packet.

That is a strong Spider migration candidate because the authoritative work
already exists in C++:

- GDSpaces local resource acquisition;
- SHA-256 authority gate;
- PE parsing;
- VA/RVA/file-offset mapping;
- `ExeByteWindowExtractor`;
- `ExeByteWindowReceipt` validation.

The Python layer should not be replaced by a second implementation of those
algorithms. Spider should connect those existing nodes.

## First vertical slice

The branch introduces:

- `spider::Instruction` — compact 8-byte runtime instruction;
- `spider::Plan` — native execution plan;
- typed `ExeWindowPacketPlan` / `ExeWindowRequest` contracts;
- `compile_exe_window_packet()` — JSON plan validation and compilation to a
  Spider Plan;
- `execute_exe_window_packet()` — native orchestration through a function-pointer
  acquisition node;
- `NativeExeWindowSource` — a Spider anchor that loads the executable through
  GDSpaces, SHA-gates it, parses PE once, and reuses that immutable state for all
  requested byte windows;
- parity-oriented C++ tests mirroring the important Python guardrails.

The compiled plan for one window is conceptually:

```text
ValidatePlan [CPU]
    -> AcquireWindow[0] [IO]
    -> ValidateWindow[0] [CPU]
    -> PublishPacket [IO]
```

For N windows, the acquisition and validation node pair is repeated with the
window index carried as the compact instruction operand.

## Immediate performance hypothesis

The Python implementation starts a new `dmc-rengine` process for each window.
Each invocation reloads the executable and repeats the SHA/PE setup path.

The first Spider native anchor instead performs:

```text
open EXE once
 -> GDSpaces read once
 -> SHA gate once
 -> PE parse once
 -> acquire N windows from the same immutable session
```

This is a hypothesis until benchmarked. No performance claim should be promoted
without measurements.

## Safety / authority boundaries

Spider **does not own**:

- PE semantics;
- DMC executable identity;
- byte-window mapping rules;
- receipt schema semantics;
- format parsers.

Spider owns:

- execution order;
- dependencies;
- typed orchestration state;
- resource lifetime;
- future scheduling policy;
- future publication transaction.

Modules own algorithms. Spider owns execution.

## Python migration policy

Python files are not deleted during the experiment. Migration is staged:

1. Native Spider path exists beside Python.
2. C++ parity tests reproduce Python guardrails.
3. Run both implementations against the same plans/fixtures.
4. Compare semantic receipts and failure behavior.
5. Benchmark process startup, wall time, peak RAM, allocations, and output size.
6. Only after parity and performance evidence may a Python orchestration path be
   deprecated.

## Candidate queue discovered on `main`

Reverse-orchestration candidates currently include:

- `scripts/reverse/extract_exe_window_packet.py` — **in progress / first slice**;
- `scripts/reverse/verify_l2_runtime_mapping_packet.py`;
- `scripts/reverse/verify_l2_runtime_mapping_packet_v2.py`;
- `scripts/reverse/normalize_l2_original_selection_candidate.py`;
- `scripts/reverse/verify_l2_original_selection_evidence.py`.

`tools/build_discovery_site.py` is a different category: repository/discovery-site
build tooling. It should not be migrated merely to eliminate Python; it needs a
separate cost/benefit decision after the reverse-orchestration experiment.

## Next gates

### Gate A — compile

The new Spider sources are under `src/spider/`; the existing recursive core
source collection therefore compiles them as part of `DMCRengine::Core`.

### Gate B — parity

Wire `tests/spider_exe_window_packet_tests.cpp` into CTest and add exact plan
fixtures shared with the existing Python tests.

### Gate C — publication

Implement a Spider publication transaction matching the Python behavior:

- refuse replacement of an existing output directory;
- write exact plan bytes;
- write child receipts;
- write packet receipt last;
- remove a partial packet on failure.

### Gate D — CLI facade

Expose the Spider build through `dmc-rengine` without exposing Spider internals
or creating a second EXE authority path.

### Gate E — A/B benchmark

Compare Python orchestration vs Spider using the same executable and packet plan.
Record correctness, wall time, process count, peak memory, and binary-size delta.

## Non-goals for this branch

- no Python interpreter or bindings;
- no new format parser;
- no change to `main`;
- no replacement of existing evidence authority without parity proof;
- no Spider scripting language;
- no graphics/scheduler expansion until this first orchestration slice proves the
  architecture on real DMC Rengine work.
