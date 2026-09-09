# DMC3 HD MOD — container reintegration gate

Date: 2026-09-09  
Branch: `reverse/mod-completion-20260907`  
Evidence class: `WRITER_GATE / CONTAINER_REINTEGRATION_GATE_2`

## Context

The MOD preserve-layout writer has already passed the provenance-bound 38-file retail no-edit corpus. A separate real-retail controlled edit also proved one bounded `object.bounding_radius` mutation with exact changed-span preservation and canonical disk reopen.

PR #369 then introduced the fail-closed `ModAuthoredChildBridge` and a synthetic PAC regression through the existing generic `NestedRelativeSlotReintegrator`. This pass advances the next non-repeating frontier: **provenance-bound real retail PNST reintegration**. No new PAC/PNST writer was added.

## Trust boundary retained

`ModAuthoredChildBridge` accepts only a canonical same-size MOD writer result whose source/output hashes, source-image binding, unauthorized-byte preservation and independent output reparse are valid. It carries the exact expanded child `ResourceId` into the generic authored-child envelope rather than inferring slot identity from a filename.

The real-retail reintegration experiment below additionally binds physical slot identity through canonical container expansion plus exact child SHA-256.

## Provenance-bound retail source

The retained source package used for this gate is:

```text
DMC_Rengine_Item_Editor_Phase4_source.zip
SHA-256: 400954e637342f8036879120d1a3845f3d74077a9844fcfad643421180602d57
```

Relevant internal paths:

```text
DMC 3 RENGINE/analysis_inputs/stage_drops/m20_s00/m20_s00_012.pac
DMC 3 RENGINE/analysis_inputs/stage_drops/m20_s00/m20_s00_012/m20_s00_012_023.mod
```

No source-package or retail payload bytes are committed to the repository.

## Canonical parent discovery

The existing `list-container` command opened the raw `m20_s00_012.pac` bytes and detected:

```text
extension             : .pac
magic / detected type : PNST
source size           : 346272
source SHA-256        : a09898bbf73d944f9f52a1de13a3bce0eebb90726947248bf35085f52df15be8
slots                 : 33
fully expanded        : yes
```

This again demonstrates that extension is not format authority.

The exact extracted MOD source SHA-256 is:

```text
096f2e8b81dd55a450b15defeb52345626156010c8fb70f3ab5f540d52b447ce
```

Canonical expansion located those exact bytes at:

```text
physical slot : 23
slot offset   : 129280
slot size     : 1888
format        : MOD
```

The filename suffix `_023` was not trusted as slot authority. The physical slot binding comes from canonical container expansion plus exact SHA-256 equality.

## Controlled child edit

A GitHub-Actions-built Linux runner from source commit:

```text
8ddd5c2acd455b12ed1f67b9af30571d5e9634be
```

with runner SHA-256:

```text
a308a428b0d4e6ac54080a6d58cf9e98f2f6e303b7849c22370c7671cd1d7eb8
```

performed:

```text
object[0].bounding_radius: 4.321839332580566 -> 4.5
```

The authored child result was:

```text
source child SHA-256 : 096f2e8b81dd55a450b15defeb52345626156010c8fb70f3ab5f540d52b447ce
authored SHA-256     : 94bcb6189435ca283c58be698ba1f4caa68663af46f6dc42c96c89db5c003f33
source/output size   : 1888 / 1888
local field span     : [124, 128)
changed local bytes  : [124, 125, 126]
writer disk reopen   : PASS
receipt SHA-256      : 9c0e33f3aabdc611c0b8c93b1aef6f531c0804db9d72ed28e3cb8989034ae07e
```

## Real retail PNST reintegration

The existing command:

```text
dmc-rengine rebuild-relative-slot <parent> 23 <authored-child> <rebuilt-parent>
```

returned `VERIFIED` and produced:

```text
source parent SHA-256 : a09898bbf73d944f9f52a1de13a3bce0eebb90726947248bf35085f52df15be8
rebuilt parent SHA-256: 78c002cc62dd235a7f95f16b3e53fd1588d8f4e8b917400e8f4471eff48d9e8e
source parent bytes   : 346272
rebuilt parent bytes  : 346272
physical slot         : 23
```

The rebuilt parent was then independently passed through `list-container` and `extract-slot`:

```text
rebuilt format        : PNST
fully expanded        : yes
slot 23 offset        : 129280
slot 23 size          : 1888
slot 23 format        : MOD
reopened child SHA-256: 94bcb6189435ca283c58be698ba1f4caa68663af46f6dc42c96c89db5c003f33
```

The extracted reopened child is byte-for-byte identical to the authored MOD and reparses with `bounding_radius = 4.5`.

## Independent full-parent byte diff

A separate raw byte comparison, independent of the reintegrator receipt, found the only changed parent offsets to be:

```text
[129404, 129405, 129406]
```

The expected offsets from physical slot placement are:

```text
slot offset 129280 + local child changes [124,125,126]
= [129404,129405,129406]
```

They match exactly.

Therefore:

- PNST header and slot table are unchanged;
- parent byte count is unchanged;
- slot 23 physical identity, offset and size are unchanged;
- all other slots are byte-identical;
- every parent byte outside the three controlled authored-child bytes is identical to the retail source parent.

Machine-readable attestation:

- `data/reverse/dmc3-mod-retail-pnst-reintegration-attestation-20260909.json`.

## Architecture reused

No duplicate archive architecture was introduced. This evidence uses existing canonical components/commands:

- `formats::mod::Writer`;
- `ModAuthoredChildBridge` trust model;
- PAC/PNST parser registry and `ContainerExpander`;
- `NestedRelativeSlotReintegrator` / relative-slot writer path;
- `list-container`;
- `extract-slot`;
- `rebuild-relative-slot`.

## Claims promoted by this pass

This gate promotes:

- provenance-bound retail PNST reintegration of a same-size, writer-validated MOD child;
- canonical physical slot identification independent of filename suffix;
- exact slot identity/offset/size preservation;
- canonical rebuilt-parent reopen and re-expansion;
- exact authored-child recovery after reopen;
- full-parent byte preservation outside the controlled three edited child bytes.

## Claims explicitly not promoted

This pass does **not** establish:

- PAC0-family retail MOD reintegration coverage;
- NBZ overlay/reopen acceptance for this edited PNST root;
- original `dmc3.exe` acceptance;
- arbitrary MOD field editing;
- transform, skin, material or texture-companion writer authority;
- layout rebuild from typed IR alone;
- full MOD writer authority.

## Next evidence frontier

The non-repeating next steps are now:

1. feed the verified rebuilt retail root resource into the existing NBZ overlay path and require canonical NBZ reopen/rematerialization;
2. where useful for container-family coverage, repeat the same bounded evidence on a provenance-bound **PAC0** parent containing a MOD child;
3. run original `dmc3.exe` acceptance on a no-edit chain and then the tightly controlled edited chain.

Texture-companion writer authority remains a separate gate and is not required by this bounding-radius-only edit because no texture-slot or texture-domain state changed.
