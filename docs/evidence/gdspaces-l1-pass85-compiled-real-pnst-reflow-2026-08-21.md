# GDSpaces L1 Pass 85 — Compiled Real PNST Bottom-Up Reflow — 2026-08-21

## Scope

Layer 1 only. This pass lifts the Pass-84 exact compiled texture-slot edits into real parent PNST containers using the current compiled `RelativeSlotPackedReflowWriter` through the canonical DMC3 parser and GDSpaces `ContainerExpander`.

## Source authority

Source package: `DMC 3 RENGINE (6).zip`

SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

The 45 distinct physical texture slots selected by Pass 84 map exactly to canonical slot identities in six real structurally-valid PNST containers. There are zero unresolved offset-to-slot mappings.

Parent containers represented by the matrix:

- `em035_057_001.pac` — PNST, 128 slots, SHA-256 `bb0b36d3daf7ffa5953f07380cff68b7059aa13ab8ddd52071f88c4ef83b843c`, 2 cases;
- `m20_b00_004.pac` — PNST, 36 slots, SHA-256 `d4e0b014e27286c6159eac2caef24b8836df293ce0384ca668fa111c2db4946c`, 13 cases;
- `m20_b00_012.pac` — PNST, 42 slots, SHA-256 `77ec1717cc6294244b21b830182d4ba64893b801690a4f0092f4123007a39deb`, 1 case;
- `m20_c00_004.pac` — PNST, 37 slots, SHA-256 `6a44e8a813b4da80edb6f9601be8af38fd335f0d0fc3942a3cf0624d2b6fa9bb`, 15 cases;
- `m20_c00_012.pac` — PNST, 47 slots, SHA-256 `99b28593fc70efc166d97ca939d6487c74f0cfcd7ba8b8d2544e10e622f14b25`, 1 case;
- `m20_s00_004.pac` — PNST, 45 slots, SHA-256 `63ff1b987a475990a4bdc450a84834c8400c7653fedc144330a3400d818cf863`, 13 cases.

Example exact mapping: in `em035_057_001.pac`, physical offsets `61488` and `139024` are canonical PNST slots `90` and `94` respectively.

## Compiled runner authority

Pass-85 source head used to build the evidence runner: `e34e52fe5008bc650d2391d0fa6ecf4fce5e7104`.

GitHub Actions Build #1165 / run `32426516193`:

- Ubuntu configure/build/test — SUCCESS;
- Ubuntu exact runner compile — SUCCESS;
- runner artifact upload — SUCCESS.

Artifact:

- name: `dmc-rengine-relative-slot-corpus-reflow-linux`;
- artifact ZIP SHA-256: `c08285efb997c4f6213d3dfcaac540d68a5c6d352c2d5348f09d879eb69358fe`;
- extracted runner SHA-256: `7bbc633e17e3b5306ae0490d3be826b1bf0baf8761d749ea5a1a40b98b2e80de`.

The runner executes:

`real PNST bytes -> make_container_parser_registry -> canonical parse -> ContainerExpander -> exact target slot -> SHA-bound AuthoredChildImage -> RelativeSlotPackedReflowWriter -> canonical output reparse/expand -> exact target-child compare -> byte-exact untouched-sibling compare`.

No game bytes are committed or uploaded to GitHub.

## Compiled real-corpus result

All 45 distinct real physical texture slots were executed through the compiled parent writer.

- attempted cases: **45**;
- successful cases: **45/45**;
- failures: **0**;
- source parent SHA mismatches: **0**;
- output external-SHA vs runner-receipt mismatches: **0**;
- parent format: PNST in **45/45**;
- unique parent containers: **6**;
- grow cases: **43**;
- shrink cases: **2**;
- same-size cases: **0**;
- minimum child/parent size delta: **-524,288 bytes**;
- maximum child/parent size delta: **+1,048,576 bytes**;
- largest observed rebuilt PNST in the matrix: approximately **9.66 MB**.

For every successful case the compiled runner independently required after output reparse:

1. the same declared slot space / valid PNST topology;
2. the target canonical slot to contain exactly the compiled Pass-84 replacement physical child;
3. every other populated sibling to remain byte-exact;
4. every empty slot to preserve occupancy;
5. the emitted parent SHA-256 to match the bytes written to disk.

## Meaning

This closes the prior mandatory gate **“compiled real PAC/PNST bottom-up rebuild receipt using edited real physical texture children”** for the represented PNST families and safe texture-authoring domain.

It is materially stronger than the previous synthetic composition proof: the exact current compiled C++ writer has now performed size-changing bottom-up reflow on real DMC3 PNST byte images with large positive and negative child-size deltas while preserving every untouched sibling.

## Remaining Layer-1 gates

Layer 1 remains NOT COMPLETE. Mandatory remaining work is:

1. representative real retail NBZ repack/reopen/materialization receipt carrying a real rebuilt nested child;
2. raw real `.lst` corpus closure or evidence-backed proof that `.lst` is not required for writeback authority;
3. resolve or formally bound/exclude the 42 non-zero auxiliary texture relationships that remain read/preserve-only;
4. original DMC3 consumption of rebuilt PAC/PNST/NBZ artifacts;
5. final cross-stack review and promotion to `main`.

The engineering readiness estimate remains approximately 92% until a real retail NBZ and original-game consumption receipt are closed.