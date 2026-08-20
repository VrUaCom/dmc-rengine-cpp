# GDSpaces Layer 1 — Current Status — 2026-08-21

## Current readiness

**Engineering readiness estimate: ~92% / NOT COMPLETE.**

This percentage is a planning/readiness estimate against the Layer-1 acceptance boundary, not a subsystem-completion claim and not original-game equivalence.

## Closed / strongly evidenced capabilities

- artifact-bound retail NBZ serialization authority;
- streaming retail NBZ repack with STORE and method-8 authoring, central/local/EOCD rebuild, temp-output validation and alias arbitration;
- size-changing PAC/PNST packed reflow preserving occupancy, aliases, protected prefix and untouched physical spans;
- synthetic full nested A-to-Z `NBZ -> PAC -> PNST -> edit -> PNST/PAC rebuild -> NBZ repack -> canonical reopen`;
- texture-specific full synthetic A-to-Z through nested PNST/PAC and NBZ repack;
- real v6 PAC/PNST structural corpus validation;
- real child-span authority including DMC3 texture wrappers/bundles;
- full structural authority for the evidenced 0x70 texture descriptor envelope;
- canonical DMC3 DDS standard-profile authority;
- safe same-layout intrinsic DDS reintegration;
- size-changing texture-slot authoring for the evidenced safe domain;
- current compiled C++ real-corpus execution: 112/112 successful edits over 45 real physical texture slots, zero source-slot SHA mismatches;
- Pass 83 fixed head `928cbf233216d738ea7c4ba1ea86db79a4825bd5`: Build #1159 SUCCESS;
- Pass 84 reconciled head `7b7092735ffbcd0de2f70851f05b2cc407511619`: Build #1164 SUCCESS on Windows + Ubuntu.

## Current real-corpus texture authoring envelope

Pass 84 compiled receipt:

- real physical slots: 45;
- compiled authoring cases: 112;
- successes: 112;
- failures: 0;
- DXT1: 47;
- DXT5: 65;
- secondary relation `same`: 12;
- secondary relation `half`: 100.

The remaining 42 descriptor/DDS relationships with non-zero unresolved auxiliary metadata remain read/preserve authority only; authoring is fail-closed.

## Mandatory remaining gates

1. compiled real PAC/PNST bottom-up rebuild receipt using edited real physical texture children;
2. representative real retail NBZ repack/reopen/materialization receipt;
3. raw real `.lst` corpus closure or an explicit evidence-backed replacement authority if `.lst` is not required for writeback;
4. resolve or formally bound/exclude the 42 non-zero auxiliary texture cases;
5. original DMC3 consumption of rebuilt PAC/PNST/NBZ artifacts;
6. final Layer-1 review and promotion into `main`.

## Active next pass

Pass 85 maps the 45 prepared real physical texture slots back to exact canonical slot identities across 6 preserved real PNST containers and executes compiled `RelativeSlotPackedReflowWriter` bottom-up reflow. All 112 Pass-84 physical offsets have exact PNST slot mappings; there are zero unresolved mappings.

## Acceptance boundary for 100%

Layer 1 may be marked COMPLETE only after an evidence-backed path exists for:

`retail source -> exact materialization -> supported edit/size change -> bottom-up nested rebuild -> retail archive repack -> canonical reopen/materialization compare -> original DMC3 consumption`

No resolver or Stage Ops work is counted toward this percentage.