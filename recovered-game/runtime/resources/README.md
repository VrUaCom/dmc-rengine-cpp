# DMC3 Recovered Resource Runtime

This directory belongs to the **Recovered Game Source Tree**. It represents reconstructed DMC3 game-runtime code/data/types and is deliberately compiled/consumed separately from `dmc_rengine_core`.

## Executable recovered slice

The Wave-2 foundation is no longer only a table of evidence constants. `resource_manager.hpp` now provides an executable reconstruction of the directly evidenced resource-load lifecycle boundary:

- exact x64 `ResourceRuntimeEntry` ABI size `0x48`;
- exact known offsets `+0x00/+0x04/+0x08/+0x18/+0x20/+0x28` with unknown bytes preserved explicitly;
- all 363 runtime slots initialized according to the seven recovered partitions `[4, 136, 60, 28, 1, 128, 6]`;
- executable confirmed transitions:
  - `0 free -> 1 loading`;
  - `1 loading -> 2 I/O complete / post-load pending`;
  - `2 -> typed confirmed post-load -> 3 ready`;
  - observed cleanup `4 teardown/cancel pending -> 0 free`;
- state 3 is fail-closed: it is reached only when the payload has one of the directly confirmed MOD/EFM/SCM/SHW magics and the corresponding recovered post-load backend succeeds;
- unsupported magic, null payload, wrong-state reuse and out-of-range slots do not silently advance the lifecycle.

The manager intentionally does **not** invent slot-selection policy: callers provide the exact pool slot. It also does not invent the still-unrecovered source-state domain that enters state 4.

## Evidence foundation

Confirmed Wave-2 authorities retained by the executable slice:

- higher-level resource/load pool base `VA 0x140C99D30`;
- 363 entries with stride `0x48`;
- seven observed pool partitions `[4, 136, 60, 28, 1, 128, 6]`;
- observed successful lifecycle `0 -> 1 -> 2 -> 3`;
- state `4` as teardown/cancellation pending and observed cleanup `4 -> 0`;
- confirmed typed post-load helper identities:
  - MOD `0x1402FE3B0`;
  - EFM `0x1402F7A90`;
  - SCM `0x1403051B0`;
  - SHW `0x1403204C0`;
- typed normalization occurs after I/O completion and before ready state.

## Still not reconstructed

The executable manager stops exactly where evidence stops. These remain reverse targets:

- exact original slot-allocation/search policy;
- callback/completion fields;
- full per-group subtype ABI;
- cache keys, duplicate-request reuse and complete refcount/ownership behavior;
- exact source-state domain entering state 4;
- PAC/PNST dispatcher source reconstruction;
- actual MOD/EFM/SCM/SHW in-place pointer-fixup algorithms — the manager currently exposes the dispatch/backend boundary but does not fake the algorithms;
- higher-level factory/object construction after normalized bytes are ready;
- room/stage transition retention and unload behavior.

Those gaps must be closed by executable/disassembly evidence, not by importing product assumptions from GDSpaces.

Authority: `docs/research/dmc3-vanilla-deep-research-wave-2.md` and promoted packet `evidence/gdspaces/dmc3-stage-wave2.evidence.json`.
