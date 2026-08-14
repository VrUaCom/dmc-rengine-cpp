# DMC3 Resource Runtime State Model — Wave 2

This note belongs to the Recovered Game Source Tree and mirrors only promoted executable evidence.

## Confirmed successful path

```text
state 0
  free / not started
    |
    v
state 1
  I/O scheduled / loading
    |
    v
state 2
  I/O complete / typed post-load pending
    |
    | MOD / EFM / SCM / SHW and recursive PAC/PNST post-load traversal
    v
state 3
  ready / postprocessed
```

The important boundary is **2 -> typed post-load normalization -> 3**. Product-side byte materialization or container expansion alone must not be labeled equivalent to game-ready state 3.

## Confirmed cleanup edge

```text
state 4
  teardown / cancellation pending
    |
    v
state 0
  free
```

Wave 2 confirms cleanup `4 -> 0`. It does **not** yet close the exact set of active source states that are permitted to enter state 4. The compiled evidence model therefore records `teardown_source_state_domain_complete = false` rather than inventing a transition table.

## Still unresolved

- exact scheduler/completion function ABI;
- complete set of state-transition writers;
- callback/completion fields in the `0x48` entry;
- cache/reuse semantics;
- ownership/refcount or equivalent lifetime mechanism;
- group-specific subtype contracts;
- actual MOD/EFM/SCM/SHW pointer-fixup algorithms;
- factory/object construction after normalization;
- stage/room transition retention and unload behavior.

These are reverse targets, not placeholders to be filled from GDSpaces assumptions.
