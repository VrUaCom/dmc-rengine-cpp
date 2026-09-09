# Spider Tarantula — Python orchestration migration experiment

Status: **experimental / isolated branch only**

Branch: `experiment/spider-python-migration`

Canonical Spider family architecture:
`docs/architecture/spider-family.md`

## Scope

This document now describes the **Tarantula** family only.

Spider has three distinct roles:

- **Black Widow** — move platform/application business decisions out of Java,
  Swift, Win32 glue or browser JS into typed native C++ state;
- **Tarantula** — replace Python-like orchestration when parity and practical
  benefit are demonstrated;
- **Crusader** — orchestrate dependencies between existing C++ modules through
  compact native plans.

The generic `native_executor` is the current execution kernel used by Crusader
and may also execute compiled Tarantula workflows. It is not itself a format
parser or scripting runtime.

## Goal

Evaluate whether orchestration currently implemented as Python scripts can be
expressed as compact Tarantula/Crusader plans and executed directly by native
C++20 modules, without embedding Python and without duplicating canonical format,
PE, evidence, or GDSpaces logic.

Tarantula takes inspiration from Python's composability, not from Python's
runtime. The production rule for this experiment is:

> No embedded scripting runtime. No duplicated format logic. Pure C++20 native execution.

## Why `extract_exe_window_packet.py` is the first target

`scripts/reverse/extract_exe_window_packet.py` explicitly identifies itself as
orchestration-only. The script validates a packet plan, launches
`dmc-rengine extract-exe-window` once per requested window, validates the child
receipts, checks known-body hashes, and publishes the packet.

That is a strong Tarantula migration candidate because the authoritative work
already exists in C++:

- GDSpaces local resource acquisition;
- SHA-256 authority gate;
- PE parsing;
- VA/RVA/file-offset mapping;
- `ExeByteWindowExtractor`;
- `ExeByteWindowReceipt` validation.

Tarantula must not replace those algorithms. It connects the existing nodes.

## First vertical slice

The branch already contains:

- `spider::Instruction` — compact 8-byte runtime instruction used by the older
  packet-plan slice;
- `spider::Plan` — native execution plan;
- typed `ExeWindowPacketPlan` / `ExeWindowRequest` contracts;
- `compile_exe_window_packet()` — JSON plan validation and compilation;
- `execute_exe_window_packet()` — native orchestration through a function-pointer
  acquisition node;
- `NativeExeWindowSource` — a Spider anchor that loads the executable through
  GDSpaces, SHA-gates it, parses PE once, and reuses that immutable state for all
  requested byte windows;
- parity-oriented C++ tests mirroring important Python guardrails;
- the newer generic 16-byte `NativeInstruction` executor, which is now exposed
  through the **Crusader** facade for reusable C++ module orchestration.

The compiled packet plan for one window is conceptually:

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

The first native Tarantula anchor instead performs:

```text
open EXE once
 -> GDSpaces read once
 -> SHA gate once
 -> PE parse once
 -> acquire N windows from the same immutable session
```

This remains a hypothesis until benchmarked. Spider is not automatically faster
because it is Spider; performance claims require measurements.

## Safety / authority boundaries

Tarantula **does not own**:

- PE semantics;
- DMC executable identity;
- byte-window mapping rules;
- receipt schema semantics;
- format parsers;
- hot numeric loops.

Tarantula owns high-level workflow composition:

- execution order;
- typed workflow state;
- resource lifetime across steps;
- conditional/iterative orchestration when justified;
- publication/export workflow.

Crusader owns the lower-level reusable C++ dependency-plan execution mechanism.
Modules own algorithms. Spider families own coordination/state at their respective
levels.

## Python migration policy

Python files are not deleted merely because Tarantula exists. Migration is staged:

1. native Tarantula path exists beside Python;
2. C++ parity tests reproduce Python guardrails;
3. run both implementations against the same plans/fixtures;
4. compare semantic receipts and failure behavior;
5. benchmark process startup, wall time, peak RAM, allocations, and output size;
6. only after parity and practical benefit may a Python orchestration path be
   deprecated.

Repository/discovery tooling can remain Python when there is no measurable
benefit from migration.

## Candidate queue discovered on `main`

Reverse-orchestration candidates currently include:

- `scripts/reverse/extract_exe_window_packet.py` — **in progress / first slice**;
- `scripts/reverse/verify_l2_runtime_mapping_packet.py`;
- `scripts/reverse/verify_l2_runtime_mapping_packet_v2.py`;
- `scripts/reverse/normalize_l2_original_selection_candidate.py`;
- `scripts/reverse/verify_l2_original_selection_evidence.py`.

`tools/build_discovery_site.py` is a separate category and should not be migrated
merely to eliminate Python.

## Reuse / anti-duplication rule

Before a Tarantula step is implemented, identify the authoritative C++ module
that already performs the operation. Tarantula should bind that operation, not
reimplement it. If two workflows duplicate neutral validation/projection, extract
one reusable module and use it from both.

Never create a second DDS/PTX/MOD/SCM/PE reader solely for a Spider workflow.

## Next gates

### Gate A — compile

The Spider sources are under `src/spider/`; the existing recursive core source
collection therefore compiles them as part of `DMCRengine::Core`.

### Gate B — parity

Wire `tests/spider_exe_window_packet_tests.cpp` into CTest and add exact plan
fixtures shared with the existing Python tests.

### Gate C — publication

Implement a Tarantula publication transaction matching Python behavior:

- refuse replacement of an existing output directory;
- write exact plan bytes;
- write child receipts;
- write packet receipt last;
- remove a partial packet on failure.

### Gate D — CLI facade

Expose the native workflow through `dmc-rengine` without exposing unnecessary
Spider internals or creating a second EXE authority path.

### Gate E — A/B benchmark

Compare Python orchestration vs Tarantula using the same executable and packet
plan. Record correctness, wall time, process count, peak memory, and binary-size
delta.

## Non-goals for this branch

- no Python interpreter or bindings;
- no new format parser;
- no change to `main` unless separately approved;
- no replacement of existing evidence authority without parity proof;
- no requirement to route tight math/codec/render loops through Spider;
- no empty generic Black Widow or Tarantula framework merely for naming;
- no dynamic-plugin requirement: modular source can still link into one compact
  product binary.
