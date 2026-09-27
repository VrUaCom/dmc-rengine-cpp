# Rengine Runtime / Graphics / Universal Format — Session Capture

This document preserves the architectural decisions from the design session that established the next two Rengine workstreams.

## Core decisions

Rengine will evolve beyond a DMC-specific toolchain into a reusable runtime platform. DMC Rengine and DMC Native Reader are reference workloads on top of that platform, not the platform itself.

The project will pursue two independent branches:

- `feature/rengine-graphics-engine` — renderer, task graph, heterogeneous compute, memory topology, multi-CPU, multi-GPU and hardware adaptation.
- `feature/universal-cross-platform-format` — one portable application container, RIR, runtime ABI, manifest/capabilities, signing and minimal platform hosts.

## Universal application goal

The target is not merely one C++ codebase compiled into EXE/APK/IPA. The goal is one portable application artifact, provisionally called **RAPP**, that can be consumed by Rengine Runtime across Windows, Android, iOS and later Linux/macOS.

Concept:

```text
NativeReader.rapp
  -> Rengine Runtime
  -> Rengine Core / Rengine VM / Rengine GFX
  -> minimal platform HAL
  -> OS
```

The portable payload should avoid embedding a full OS-specific executable per platform. The primary execution representation is intended to be a portable Rengine IR (RIR). Optional native extensions may exist as accelerators, not as the architectural foundation.

## Graphics engine goal

Rengine GFX is not designed as a wrapper over Vulkan, DirectX, Metal or OpenGL. The core rendering path is owned by Rengine.

Initial software pipeline:

```text
Scene
 -> Geometry
 -> Vertex Processing
 -> Triangle Assembly
 -> Clipping
 -> Rasterization
 -> Texture Sampling
 -> Depth Test
 -> Blending
 -> RFrameBuffer
 -> minimal OS present path
```

Proposed core components:

- RVertexProcessor
- RTriangleAssembler
- RClipper
- RRasterizer
- RDepthBuffer
- RTextureSampler
- RBlendUnit
- RCommandQueue
- RFrameBuffer

The OS-specific layer should remain a minimal host/HAL for window/surface creation, present, files, input, audio, memory, threads and networking.

## Architecture adaptation

Rengine Execution Core should support multi-versioned implementations and runtime dispatch for:

- scalar generic
- x86-64
- SSE
- AVX2
- AVX-512
- ARM64
- NEON
- SVE
- RISC-V / RVV

A portable RIR is intended to decouple application logic from any one CPU ISA.

## Task Graph

Do not build around a permanent “Main Thread + Render Thread + Workers” worldview.

Tasks should carry dependencies, memory locality, execution requirements and preferred compute domains.

Example frame graph:

```text
Animation
 -> Transforms
 -> Visibility
 -> LOD / Culling
 -> Geometry Preparation
 -> parallel compute/raster
 -> Composition
 -> Present
```

## Multi-CPU / NUMA

There must be no single-socket assumption.

Target model:

```text
1..N CPU packages
1..N dies/chiplets
1..N cores
1..N hardware threads
```

The architecture explicitly includes 2x, 4x, 6x CPU systems and larger systems without special-case rewrites.

Scheduling must be NUMA-aware. Work should move toward local data whenever possible. Cross-node transfer cost is part of scheduling.

Hierarchical scheduling:

```text
Global Scheduler
 -> Package Scheduler
 -> Core-group Scheduler
 -> Worker
```

Work stealing is locality-first.

## Multi-GPU / multi-die

There must be no single-GPU assumption.

Target model:

```text
1..N GPU boards
1..N GPU dies/devices per board
heterogeneous device sets
integrated + discrete devices
other accelerators
```

A system with 4 boards and 2 GPU dies per board is modeled as 8 execution nodes plus physical board/link/memory topology.

Do not statically split load equally. Dynamic distribution should account for compute throughput, memory bandwidth, link cost, synchronization, load, thermal throttling and local memory.

## RComputeNode / Compute Fabric

Introduce a generalized execution entity: **RComputeNode (RCN)**.

An RCN describes:

- node type
- capabilities
- compute score
- memory domain
- local memory
- bandwidth
- latency
- links to other nodes
- supported execution classes

CPU core groups, GPU dies, NPUs and future accelerators are nodes inside one Compute Fabric.

## Memory Fabric

Treat storage, system RAM, NUMA-local RAM and device-local memory as a topology rather than isolated buckets.

A resource may have representations in:

- storage
- mapped file ranges
- system RAM
- NUMA-local RAM
- GPU/device memory
- cache-friendly CPU layout

The runtime should choose placement based on measured cost and expected use.

## Data-oriented and underused techniques to investigate

- memory mapping
- zero-copy pipelines
- Structure of Arrays
- hot/cold data splitting
- cache-aware layouts
- large/huge pages where beneficial
- compressed execution data
- async decompression
- sparse resources / virtual memory
- predictive streaming
- persistent workers
- lock-free queues/pools where appropriate
- frame pipelining
- double/triple-buffered world state
- runtime profiling and self-tuning
- JIT specialization / multi-versioning
- deterministic task graphs

## Self-profiling

Do not assume the “best” hardware path statically. Rengine should build a machine-specific profile using measured latency, bandwidth, transform throughput, raster throughput, decompression throughput, transfer cost and thermal behavior.

Scheduler decisions should be based on the measured machine, not only on vendor/model tables.

## Initial implementation order

1. Rengine CPU renderer.
2. x86/ARM/SIMD specialization.
3. Multithreaded tile/bin renderer.
4. Task Graph + Compute Fabric + Memory Fabric.
5. RIR + runtime/VM.
6. RAPP universal container.
7. Multi-device execution.
8. Optional hardware acceleration/native extension backends.

## Reference workload

DMC Native Reader is the first candidate reference application.

```text
PAC/NBZ
 -> MOD / SCM / PTX / MOT
 -> Rengine runtime representation
 -> Rengine renderer
 -> Windows / Android / iOS hosts
```

The DMC format logic stays outside the generic runtime boundary.

## Non-negotiable design principles

- No hardcoded single-device assumptions.
- No fixed requirement that CPU is the central execution authority.
- No fixed requirement that one GPU owns rendering.
- Task Graph is the execution model.
- Compute Fabric + Memory Fabric describe hardware.
- RAPP is the portable application container.
- RIR is the portable execution representation.
- Platform-specific code is isolated to a minimal HAL/host.
- DMC-specific code is not part of generic Rengine Runtime.
