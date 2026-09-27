# RAPP / RIR — Universal Cross-Platform Application Format

Branch: `feature/universal-cross-platform-format`

## Mission

Define one portable application artifact that can be opened by Rengine Runtime on Windows, Android, iOS and later Linux/macOS without shipping a separate complete application build for each OS inside the package.

Working names:
- **RAPP** — Rengine Application Package
- **RIR** — Rengine Intermediate Representation

Names remain provisional until format freeze.

## High-level model

```text
Application source
 -> RIR
 -> RAPP
 -> Rengine Runtime
 -> platform host/HAL
 -> target OS
```

The portable package is the application. EXE/APK/IPA are only host/runtime delivery mechanisms where an OS requires them.

## Proposed package structure

```text
app.rapp
  header
  manifest
  code/
    app.rir
  ui/
  assets/
  resources/
  shaders-or-raster-programs/
  metadata/
  optional-native/
  signatures/
```

## Header

Minimum fields to define:
- magic
- format version
- minimum runtime version
- feature flags
- manifest offset/size
- directory/index offset
- signature metadata
- endianness/canonical encoding rules

## Manifest

Minimum semantic fields:
- application id
- display name
- application version
- entry point
- required runtime
- capability declarations
- permissions
- resource roots
- UI entry
- memory requirements/hints
- compute requirements/hints
- optional extension declarations

## RIR goals

RIR must be:
- architecture-neutral
- deterministic to parse
- versioned
- explicitly typed
- safe to validate before execution
- suitable for interpretation, AOT lowering and later JIT specialization
- independent from Windows/Android/iOS ABI details

Initial CPU targets:
- x86-64
- ARM64
- RISC-V

RIR is not required to expose native ISA directly.

## Runtime ABI

Define a stable Rengine ABI for application-facing services:

```text
filesystem
window/surface
input
audio
time
threads/tasks
memory
network
graphics
resource access
logging
capabilities
```

Applications target the Rengine ABI, not Win32/Android/iOS APIs.

## Platform hosts

Each OS requires a minimal host.

Host responsibilities:
- launch Rengine Runtime
- expose OS lifecycle
- create/present a surface
- provide filesystem/sandbox access
- map input/events
- provide audio/device access
- enforce OS permissions
- map runtime capabilities to legal platform mechanisms

The host must not contain application-specific logic.

## Security model

RAPP must be verifiable before execution.

Required design topics:
- package hash tree
- developer signature
- runtime trust policy
- capability/permission manifest
- immutable package sections
- optional encrypted sections
- extension signing
- version rollback rules
- deterministic parsing
- bounds-checked directory/index structures

## Native extensions

Native extensions are optional accelerators, not the default application representation.

A package should remain functional without native extensions unless the manifest explicitly declares a hard requirement.

Potential uses:
- architecture-specific hot kernels
- platform integrations impossible through generic ABI
- device-specific acceleration

## Resource model

RAPP should support:
- embedded immutable resources
- streamed resources
- external resource mounts
- content-addressed blobs
- compressed sections
- sparse/virtual resource ranges
- alignment metadata
- memory-domain hints

## UI

UI representation should be platform-neutral and rendered/managed by Rengine Runtime where possible.

Avoid embedding separate Android/iOS/Windows UI trees as the main model.

## Compatibility

Define compatibility separately for:
- RAPP container format
- RIR version
- Runtime ABI version
- capability set
- optional extension ABI

A new runtime should be able to reject unsupported features before attempting execution.

## DMC Native Reader reference package

Candidate proof-of-concept:

```text
NativeReader.rapp
  manifest
  reader.rir
  UI
  generic assets
  format modules
  runtime metadata
```

DMC resources such as PAC/NBZ/MOD/SCM/PTX/MOT remain user/game data, not baked into the generic format definition.

## First milestones

1. Freeze RAPP goals/non-goals.
2. Define canonical binary container.
3. Define manifest schema.
4. Define Runtime ABI v0.
5. Define RIR v0 instruction/type model.
6. Build validator/dumper.
7. Build minimal host loader.
8. Package and launch a hello-world RAPP.
9. Launch a simple Rengine-rendered application.
10. Package Native Reader as reference workload.
