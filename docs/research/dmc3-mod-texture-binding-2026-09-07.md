# DMC3 HD MOD texture binding — companion-authoritative validation

Date: 2026-09-07

## Purpose

This pass closes the analysis boundary between serialized MOD texture references and the recovered external texture-companion allocation.

It does not parse new binary data and does not mutate either resource. The output is a cross-resource validation result suitable for tooling, preview, and future authoring preflight.

## Canonical authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- shared manager initializer: `0x1402F9570`
- companion materialization helper: `0x140304B30`
- runtime descriptor-table builder: `0x14030D040`
- shared material consumer: `0x1402F9890`

Recovered source contract:

- MOD header `u8 +0x12` is a serialized texture-slot-domain mirror;
- MOD mesh `u16 +0x02` is the serialized texture slot selected by the material path;
- external companion `u32 +0x000` supplies texture payload count;
- runtime descriptor table is materialized from that companion with stride `0x40`;
- `0x1402F9890` selects the runtime descriptor using `texture_slot * 0x40`.

## Authority rule

The validator deliberately uses:

`companion.texture_count` = live/runtime binding authority.

Header `texture_slot_count` is compared only as a consistency mirror. A mismatch is observable but is not by itself a runtime binding failure.

This distinction follows the canonical manager path: runtime texture-table authority is taken from the external companion rather than blindly trusting the model header byte.

## Analysis result

`analysis::mod::TextureBindingAnalysis` reports:

- whether the companion parse result is valid;
- serialized header mirror count;
- companion-authoritative count;
- whether the header mirror matches the companion count;
- total mesh count examined;
- each mesh whose `texture_slot >= companion.texture_count`.

`runtime_bindings_valid()` is true only when:

1. the companion is valid; and
2. no mesh texture slot is outside the companion-authoritative domain.

A header mirror mismatch does not make `runtime_bindings_valid()` false when all actual mesh references remain valid.

## Fail-closed behavior

If the companion parse result is invalid, the analysis is runtime-invalid and does not generate per-mesh out-of-range conclusions from the companion's partial/untrusted count.

This is tooling safety behavior. It is not a claim that the original executable performs the same diagnostic procedure.

## Regression cases

The regression constructs a MOD document with two mesh references and a valid companion count of 2.

### Mirror mismatch, valid references

- header mirror: `3`
- companion count: `2`
- mesh slots: `0`, `1`

Result:

- mirror mismatch reported;
- zero out-of-range meshes;
- runtime bindings valid.

### Exact mirror

- header mirror: `2`
- companion count: `2`
- mesh slots: `0`, `1`

Result:

- mirror match;
- runtime bindings valid.

### Out-of-range mesh

- companion count: `2`
- second mesh slot: `2`

Result:

- one out-of-range mesh at outer `0`, mesh `1`;
- runtime bindings invalid.

### Invalid companion

A companion result carrying `tm2_magic_mismatch` is rejected as runtime-invalid without deriving mesh-range diagnostics from its count.

## Evidence status

- mesh texture slot `+0x02`: `EXE_CONFIRMED`;
- header texture-domain mirror `+0x12`: `EXE_CONFIRMED`;
- companion count `+0x000`: `EXE_CONFIRMED`;
- runtime descriptor stride `0x40`: `EXE_CONFIRMED`;
- companion authority over live runtime texture table: `EXE_CONFIRMED`;
- mirror-mismatch reporting policy: project/tooling behavior;
- fail-closed cross-resource validation: project/tooling behavior.

## Explicit non-claims

This pass does not establish:

- that the original game reports header/companion mismatches;
- a universal equality invariant between header mirror and companion count;
- production writer authority for MOD or companion resources;
- complete runtime descriptor semantics;
- complete TIM2 internal image semantics;
- automatic texture replacement or remapping behavior.
