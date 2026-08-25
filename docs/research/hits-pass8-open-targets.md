# HITS Pass 8 — Open Reverse Targets

Date: 2026-08-14

This file is a compact execution boundary derived from the full Pass-8 review. It is not a replacement for the canonical Drive documents.

## Highest-value unresolved nodes

### 0x14005E7A0

Recover:
- exact input ABI;
- no-hit initialization;
- complete caller-visible write set;
- static candidate metric;
- dynamic candidate metric;
- winner selection;
- equality/tie-break rule;
- final output ownership/lifetime.

### 0x14005B460

Recover:
- internal body;
- category-list entry layout;
- list owner/lifetime;
- candidate production contract;
- return semantics;
- relationship to later category dispatch and query masks.

Known call-site ABI remains:
- RCX manager;
- RDX object;
- R8 category-list pointer;
- R9D object index/slot;
- fifth stack argument category ID.

### 0x14005FEC0

Recover:
- exact source-1 input/output ABI;
- written fields;
- validity/no-hit contract;
- relationship to raw upper flag bit `0x00080000`;
- caller consumption.

### 0x1400601E0

Recover:
- exact in/out structure;
- fourth-component semantics;
- per-contact accumulation;
- termination rule;
- caller-visible correction semantics.

### Source 2

Recover:
- concrete type at global collision root `+0x00`;
- backing resource/runtime state;
- constructor/destructor/lifetime;
- selection path;
- whether it participates in static, dynamic or another collision family.

Do not model it as a third stage-PAC HITS resource without direct evidence.

### General resource lifecycle -> HITS handoff

Recover the instruction-level path from the Wave-2 general resource manager state-3 ready object to:
- stage-specific PAC ownership;
- member-3/member-6 acquisition;
- `0x140245DE0`-side PAC consumer;
- `0x1402D3060` HITS initialization.

## Corpus expansion

Sweep all available entries in the 189-descriptor universe and record:
- descriptor/resource-set identity;
- numeric Stage identity when separately evidenced;
- resolved main/script PAC physical identity;
- PAC SHA-256;
- member-3 presence/hash;
- member-6 presence/hash;
- triangle counts;
- grid dimensions;
- flag distribution;
- duplicate payloads;
- cross-descriptor reuse.

This expands ecosystem coverage but does not reopen the already byte-verified Pass-5 spatial writer algorithm.

## Implementation gates

Do not implement speculative original-game fields or ownership until the corresponding reverse target is promoted through evidence.
