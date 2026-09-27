# Rengine Graphics Engine — Architecture Baseline

Branch: `feature/rengine-graphics-engine`

## Mission

Build a renderer/runtime execution layer owned by Rengine rather than a thin abstraction over Vulkan, DirectX, Metal or OpenGL.

The first production-capable backend should be software/CPU based so the rendering algorithm, memory layout, scheduling and determinism remain under project control.

## Core modules

```text
rengine/gfx/
  vertex/
  assembly/
  clipping/
  raster/
  texture/
  depth/
  blend/
  framebuffer/
  command/
  tile/
  scene/

rengine/compute/
  task_graph/
  scheduler/
  numa/
  topology/
  compute_node/
  profiling/

rengine/memory/
  domains/
  mapping/
  residency/
  streaming/
  arenas/
  sparse/

rengine/platform/
  surface/
  input/
  filesystem/
  threading/
  timing/
```

## Renderer pipeline

```text
scene
 -> visibility
 -> geometry extraction
 -> vertex transform
 -> primitive assembly
 -> clipping
 -> binning
 -> tile rasterization
 -> sampling
 -> depth/stencil
 -> blend
 -> framebuffer
 -> present
```

## Multi-thread model

Use persistent workers and a dependency-driven Task Graph. Avoid per-frame thread creation.

The scheduler must support:
- local queues
- hierarchical scheduling
- locality-first work stealing
- NUMA domains
- device-specific queues
- dependency counters
- deterministic modes

## Multi-CPU

No one-socket assumption. Hardware topology must be explicit:
- package
- die/chiplet
- NUMA node
- core group
- core
- hardware thread
- cache hierarchy

Tasks should prefer the memory domain that already owns the required data.

## Multi-GPU / multi-device

Model devices as RComputeNode entries. A dual-GPU board exposes two compute nodes plus topology metadata. Four dual-GPU boards therefore expose eight GPU nodes without changing renderer architecture.

Do not equal-split by device count. Use measured capacity and transfer cost.

## Tile/bin renderer

The first scalable distribution primitive is tile/bin rendering.

Tiles should be small enough for dynamic balancing, but large enough to avoid scheduler overhead dominating work.

Potential scheduling dimensions:
- screen tile
- geometry bin
- material batch
- depth range
- post-process region
- independent scene partition

## Compute Fabric

RComputeNode should expose at least:

```cpp
struct RComputeNode {
    RNodeType type;
    RCapabilities capabilities;
    RMemoryDomainId memoryDomain;
    uint64_t localMemoryBytes;
    double computeScore;
    double memoryBandwidth;
    double dispatchLatency;
    std::vector<RLink> links;
};
```

Exact API is not fixed yet; the semantic contract is.

## Memory Fabric

Memory placement is first-class. Required concepts:
- storage-backed mapped ranges
- system memory
- NUMA-local memory
- device-local memory
- residency state
- migration cost
- replication state
- immutable/shared resource views
- hot/cold representations

## Hardware adaptation

Provide scalar reference kernels first, then architecture-specialized kernels.

Dispatch targets:
- x86-64 scalar/SSE/AVX2/AVX-512
- ARM64 scalar/NEON/SVE
- RISC-V scalar/RVV

The scalar path is the correctness oracle.

## Self-tuning

At startup and during safe runtime windows, collect throughput/latency measurements for:
- transforms
- clipping
- raster tiles
- texture sampling
- decompression
- memcpy/memory traversal
- NUMA transfer
- device-to-device transfer
- synchronization

Use these results to weight scheduling.

## DMC integration

Do not make the renderer understand PAC, SCM, MOD, PTX or MOT directly.

DMC formats are decoded into generic runtime structures, then submitted to Rengine GFX. Zero-copy views may be used when a DMC resource already matches a runtime-friendly layout.

## First milestones

1. RFrameBuffer + platform present surface.
2. Scalar triangle rasterizer.
3. Depth + texture sampling + blend.
4. Multithreaded tile binning/raster.
5. SIMD kernels.
6. Task Graph.
7. NUMA-aware scheduler.
8. RComputeNode + Compute Fabric.
9. Memory Fabric/residency.
10. Multi-device distribution.
11. Native Reader reference workload.
