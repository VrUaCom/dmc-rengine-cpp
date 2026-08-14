# DMC3 Recovered Resource Runtime

This directory belongs to the **Recovered Game Source Tree**. It represents reconstructed DMC3 game-runtime code/data/types and is deliberately compiled/consumed separately from `dmc_rengine_core`.

## Executable recovered slice

The Wave-2 foundation is no longer only a table of evidence constants. `resource_manager.hpp` now provides a deliberately narrow executable reconstruction of the **directly evidenced lifecycle boundaries**, without claiming an original C++ manager class or unevidenced writer side effects:

- exact x64 `ResourceRuntimeEntry` ABI size `0x48`;
- exact known offsets `+0x00/+0x04/+0x08/+0x18/+0x20/+0x28` with unknown bytes preserved explicitly;
- seven recovered pool partitions `[4, 136, 60, 28, 1, 128, 6]` remain represented by the Wave-2 evidence model;
- executable confirmed state edges:
  - `0 free -> 1 loading`;
  - `1 loading -> 2 I/O complete / post-load pending`;
  - `2 -> explicitly selected confirmed typed post-load -> 3 ready`;
  - observed cleanup `4 teardown/cancel pending -> 0 free`;
- state 3 is reached only after a caller-selected confirmed MOD/EFM/SCM/SHW post-load backend succeeds.

The executable helpers intentionally do **not** claim which original transition writer assigns `subtype_index`, `source_descriptor`, `loaded_payload`, `owned_state`, callbacks, or ownership fields. Those field locations are known; their complete writer/ordering ownership is not.

Likewise the helper does not infer the original typed dispatcher key from three-byte file magic. Wave-2 confirms typed dispatch and the four helper identities, but the complete dispatcher ABI/selection mechanism remains a reverse target.

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

The executable slice stops exactly where evidence stops. These remain reverse targets:

- exact original slot-allocation/search policy;
- exact writers and ordering for the known non-state entry fields;
- callback/completion fields;
- full per-group subtype ABI;
- cache keys, duplicate-request reuse and complete refcount/ownership behavior;
- exact source-state domain entering state 4;
- PAC/PNST dispatcher source reconstruction and exact selection ABI;
- actual MOD/EFM/SCM/SHW in-place pointer-fixup algorithms — the slice exposes the backend boundary but does not fake the algorithms;
- higher-level factory/object construction after normalized bytes are ready;
- room/stage transition retention and unload behavior.

Those gaps must be closed by executable/disassembly evidence, not by importing product assumptions from GDSpaces.

Authority: `docs/research/dmc3-vanilla-deep-research-wave-2.md` and promoted packet `evidence/gdspaces/dmc3-stage-wave2.evidence.json`.
