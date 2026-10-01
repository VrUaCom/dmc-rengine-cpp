# Spider family architecture

Status: **experimental / branch-scoped architecture**  
Branch: `experiment/spider-python-migration`

Spider is one C++20 architecture split into three deliberately different roles.
The split exists to prevent one generic "Spider" layer from becoming a second
parser stack, a second UI framework, or an embedded scripting runtime.

## Canonical family names

### Spider Black Widow

**Purpose:** move application/business/UI decisions out of platform shells such
as Java/Kotlin, Swift, Win32 glue, or browser JavaScript and into typed native
C++ state.

Black Widow should own decisions such as:

- whether a resource action is available;
- whether a companion resource is already attached;
- whether render/wireframe/hierarchy/UV modes are valid for the current session;
- which presentation mode a shell should show;
- other platform-neutral session policy.

The platform shell should remain mechanical:

```text
Android Java/Kotlin / Swift / Win32 / JS
    -> open file / receive URI or FD
    -> query typed Black Widow state
    -> draw widgets
    -> forward user action
```

The shell must not reconstruct state from diagnostic strings, file extensions,
or duplicated combinations of native capability bits.

Black Widow **does not parse formats** and must not duplicate MOD/SCM/PTX/DDS or
other resource logic. It consumes typed native session state produced by
canonical modules.

## Spider Tarantula

**Purpose:** replace Python-like orchestration/scripting where a compact native
workflow is measurably cleaner and more reusable than a Python script.

Typical candidates:

```text
find resources
 -> filter
 -> acquire
 -> inspect
 -> choose operation
 -> transform
 -> validate
 -> publish/export
```

Tarantula takes inspiration from Python composability, not Python runtime
semantics. The intended direction remains pure C++20: no embedded Python VM is
required for the core design.

Migration policy is evidence-based:

1. keep the existing Python path while the experiment is young;
2. implement the equivalent typed native workflow;
3. run parity tests against the same fixtures/plans;
4. compare failure behavior and receipts;
5. benchmark wall time, process count, memory, allocations and binary-size cost;
6. deprecate Python only after parity and practical benefit are demonstrated.

Python should not be removed merely because Tarantula exists. Repository tooling
or one-off scripts may remain Python when migration has no clear benefit.

## Spider Crusader

**Purpose:** simplify orchestration between existing C++ modules.

Crusader owns:

- compact native execution plans;
- dependency ordering;
- operation bindings;
- fail-closed execution state;
- future scheduling metadata.

Crusader **does not own algorithms**. Format readers, DDS codecs, PE mapping,
geometry, UV projection, texture framing, archive code and evidence logic remain
inside their authoritative modules.

Current implementation rule:

```text
spider::crusader::Plan / execute()
          |
          v
existing spider::NativePlan / execute_native_plan()
```

`crusader.hpp` is therefore a zero-overhead facade over the existing native
executor. There must not be a second executor implementation.

## Shared Spider law: modules own algorithms, Spider owns coordination

The core rule for all three families is:

> Do not move an algorithm into Spider merely because Spider calls it.

Examples of good boundaries:

```text
PTX framing module ----\
DDS codec --------------> Crusader plan -> texture projection
UV module --------------/

canonical session state -> Black Widow -> typed UI/application decisions

resource acquisition ----\
validation ----------------> Tarantula workflow -> publication
export -------------------/
```

Examples that should stay direct C++ and should not be routed through Spider hot
paths:

- matrix math;
- triangle rasterization;
- barycentric interpolation;
- texture sampling;
- fixed-point UV conversion;
- DDS block decode inner loops;
- other tight numeric loops where orchestration adds no value.

Spider is not a performance feature by itself. It is used when it reduces
duplication, makes dependencies explicit, improves reuse, or eliminates expensive
process/script orchestration. Performance claims require measurement.

## Reuse and anti-duplication policy

Before adding new Spider code:

1. find the authoritative module that already owns the operation;
2. call or wrap that module rather than reimplementing it;
3. if two callers duplicate the same neutral validation/projection, extract a
   small reusable module and let both callers depend on it;
4. keep platform shells free of format offsets and DMC binary knowledge;
5. do not create parallel DDS/PTX/MOD/SCM readers for a Spider path;
6. prefer facades/aliases over cloned executors;
7. delete superseded duplicate policy code after the native replacement is
   tested.

## Companion-resource example

The Native Reader PTX-to-model work established a useful reference pattern:

```text
MOD / SCM
  -> geometry + UV + texture-slot bindings

PTX
  -> framing
  -> DDS entries

Black Widow
  -> decides whether Attach Texture Companion is valid for the session

Crusader
  -> orchestrates framing / decode / projection dependencies

neutral binding module
  -> validates model-required slots

renderer
  -> direct C++ UV sampling of ready RGBA textures
```

The important architectural result is that PTX does not belong to MOD or SCM.
The model exposes neutral texture-slot requirements; a texture set exposes slots;
a companion module binds them. This keeps both sides reusable for future formats.

## Product/build model

Spider does not imply dynamic plugins or many shared libraries. Source remains
modular while products link only the required slice:

```text
many C++20 modules
       |
       v
product-specific static/object targets
       |
       v
one small native core (.so / static lib / WASM module)
```

Dead-code elimination and LTO are preferred for release products.

## Family implementation status

- **Crusader:** native executor exists; `crusader.hpp` exposes the family name
  without duplicating execution code.
- **Tarantula:** native metadata EXE packet acquisition/publication and CLI exist,
  with synthetic Python/native parity tests. Raw-byte publication and the
  production benchmark remain open; `spider-python-migration.md` documents the
  implemented slice and its limits.
- **Black Widow:** architecture is now canonical; product-specific typed state
  evaluators should be implemented only where a platform shell currently owns
  business decisions. Do not add an empty generic framework merely to claim the
  family exists.

## Naming rule

Use the family name whenever the role matters:

- `Black Widow` = platform/application decision migration;
- `Tarantula` = Python-like workflow migration;
- `Crusader` = C++ module orchestration.

Avoid the ambiguous phrase "Spider does X" when the exact family is known.
