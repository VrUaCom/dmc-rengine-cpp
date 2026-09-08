# DMC3 HD — em000 enemy effect graph integration (2026-09-08)

**Branch:** `research/enemy-architecture-em000-20260908`  
**Primary evidence branch:** `reverse/effect-pack-em000-20260908`  
**Corpus:** `em000-extract.zip`  
**SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

## Why this matters for enemy architecture

The enemy effect package is no longer best modeled as a flat list of opaque `G/V/E/P/T/A/M` records.

The em000 corpus now proves a typed logical resource graph whose nodes are manifest entries and whose edges are serialized inside the records themselves.

This is important for future `EnemyDefinition` authoring: creating or cloning an enemy effect set will require graph-consistent logical IDs and physical payload groups, not only copying raw record files.

## Bound graph

```text
G/V routing node
    -> G/V routing node (optional)
    -> P or E terminal

E subtype 1/2
    -> T
    -> optional A
          -> own T

E subtype 5
    -> M
         -> MOD primary resource
         -> optional raw companion

P primary variant 02 03
    -> T
    -> A
         -> T
```

## Shared G/V directed selector

Both G and V contain the same selector representation:

```text
u16 packed_selector
u16 target_manifest_id
```

Locations:

```text
V: +0x04 selector, +0x06 id
G: +0x24 selector, +0x26 id
```

The low selector byte chooses the target kind:

```text
0 -> P
1 -> E
2 -> G
3 -> V
```

The high byte remains raw/undecoded.

All 62 G/V selectors validate against the kind-specific manifest domain.

Observed edge counts:

```text
V -> P : 27
V -> E : 11
V -> G :  8
V -> V :  4
G -> P :  6
G -> E :  4
G -> V :  2
```

The bound G/V graph is acyclic. Every path terminates in P or E within at most two selector edges.

This makes G/V evidenced routing/control nodes at the structural graph level. Their historical artistic names remain unknown.

## A is texture-linked

Every one of the 11 A records contains a T-kind manifest selector at `A +0x01`.

The A body is a fixed-capacity sequence of 33 ten-byte entries with a 0..256 bounded, 32-quantized rectangle-like grid. This is strongly compatible with texture-atlas/UV-region state, but that high-level meaning still requires executable consumer confirmation.

For the 30 P records using primary variant bytes `02 03`:

```text
P -> T
P -> A
A -> T
```

and the T selected by P equals the T owned by A in **30/30** cases.

Therefore that P variant forms a coherent texture-linked subgraph rather than carrying coincidental numeric values.

## Consequence for enemy authoring

A future enemy effect authoring layer should not expose only independent records. It needs a logical graph IR approximately like:

```text
EnemyEffectGraph
    manifest entries
    typed logical ids
    directed selector edges
    grouped physical members
    texture bindings
    model bindings
```

Validation must include:

1. every selector kind code is recognized for the active profile;
2. every target id exists in the selected kind domain;
3. grouped M entries retain both physical slots even when the companion is empty;
4. P/A/T relationships remain internally coherent;
5. physical PNST slot identity is preserved independently from logical graph identity;
6. cycles are not rejected globally merely because em000 is acyclic — cycle legality requires broader corpus/runtime evidence.

## Current evidence boundary

This integration does not claim:

- historical expansions for G/V/E/P/T/A/M letters;
- that all enemy effect packs share exactly the em000 graph shape;
- that A is definitively named an atlas/animation record;
- writer authority for effect records;
- original-game acceptance of newly constructed effect graphs.

The next system-level gate is to bind this logical graph to the runtime effect manager and then repeat the graph census on at least one additional enemy family.
