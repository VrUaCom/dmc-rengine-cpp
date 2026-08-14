# DMC3 Recovered Resource Runtime

This directory belongs to the **Recovered Game Source Tree**. It represents reconstructed DMC3 game-runtime code/data/types and is deliberately compiled as a separate target from `dmc_rengine_core`.

Current Wave-2 foundation is evidence-only and intentionally narrow:

- higher-level resource/load pool base `VA 0x140C99D30`;
- 363 entries with stride `0x48`;
- seven observed pool partitions `[4, 136, 60, 28, 1, 128, 6]`;
- minimum stable entry field offsets `+0x00/+0x04/+0x08/+0x18/+0x20/+0x28`;
- observed successful lifecycle `0 -> 1 -> 2 -> 3`;
- state `4` as teardown/cancellation pending and observed cleanup `4 -> 0`;
- confirmed typed post-load helper identities for MOD, EFM, SCM and SHW;
- Wave-2 evidence that typed normalization occurs after I/O completion and before ready state.

Not reconstructed here yet:

- exact callback/completion fields;
- full per-group subtype ABI;
- cache keys or duplicate-request reuse;
- reference counting or equivalent ownership;
- exact source-state domain entering state 4;
- PAC/PNST dispatcher source reconstruction;
- the actual MOD/EFM/SCM/SHW pointer-fixup algorithms;
- higher-level factory/object construction after normalized bytes are ready;
- room/stage transition retention and unload behavior.

Those remain reverse targets and must not be filled with product assumptions from GDSpaces.

Authority: `docs/research/dmc3-vanilla-deep-research-wave-2.md` and promoted packet `evidence/gdspaces/dmc3-stage-wave2.evidence.json`.
