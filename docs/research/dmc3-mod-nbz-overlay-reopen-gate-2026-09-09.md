# DMC3 MOD → NBZ overlay reopen gate — 2026-09-09

## Purpose

Advance the MOD authoring frontier from validated MOD writer output and provenance-bound retail PNST reintegration into the existing DMC3 next-volume NBZ overlay architecture without introducing another archive writer, another MOD branch, or a second container-reintegration path.

This gate is intentionally a **synthetic composition proof**. It proves that the already-validated components compose end to end. It does not claim that a provenance-bound retail NBZ overlay has already been generated or accepted by the original game.

## Prerequisites already established

Before this gate, the canonical MOD work had already established:

- preserve-layout MOD writer authority for bounded fixed-layout promoted fields;
- provenance-bound no-edit parity and canonical reopen for 38 retail MOD files;
- one independently verified real-retail bounding-radius edit;
- fail-closed `WriteReceipt → AuthoredChildImage` trust bridging;
- synthetic PAC/PNST child reintegration;
- provenance-bound retail PNST reintegration for `m20_s00_012.pac`, physical slot 23, with all parent bytes outside the controlled MOD edit preserved.

Retail PNST evidence is recorded in:

- `data/reverse/dmc3-mod-retail-pnst-reintegration-attestation-20260909.json`.

## Existing architecture reused

No new NBZ serializer was added.

The composition uses the existing canonical/product path:

```text
MOD typed edit
    ↓
formats::mod::Writer
    ↓
WriteReceipt
    ↓
ModAuthoredChildBridge
    ↓
AuthoredChildImage
    ↓
NestedRelativeSlotReintegrator
    ↓
rebuilt PAC/PNST root resource
    ↓
NbzStoreOverlayWriter
    ↓
next contiguous STORE-only DMC3-N.nbz
    ↓
NbzZipSource reopen
    ↓
container registry parse + expand
    ↓
MOD Parser reopen
```

The production `build-dmc3-overlay` command already performs staged `NbzZipSource` reopen and exact root-member byte verification before no-replace publication. The MOD-specific regression therefore extends the existing registered `mod_authored_child_bridge_tests` instead of adding another production command.

## Synthetic end-to-end regression

`tests/mod_authored_child_bridge_tests.cpp` now continues its existing controlled MOD edit after successful container reintegration.

The synthetic source MOD begins with:

```text
object[0].bounding_radius = 42.5
```

The canonical preserve-layout writer authors:

```text
object[0].bounding_radius = 50.0
```

The writer result must pass the existing MOD trust bridge and same-size container reintegration first.

The rebuilt root resource is then authored into a generated next-volume overlay with an observed product bootstrap input of synthetic present volumes:

```text
DMC3-0.nbz
DMC3-1.nbz
```

The expected generated artifact is therefore:

```text
DMC3-2.nbz
```

with root logical path:

```text
GData.afs/em000.pac
```

The regression requires all of the following:

1. `NbzStoreOverlayWriter::build()` succeeds;
2. receipt volume index is `2` and filename is `DMC3-2.nbz`;
3. the generated archive reopens through `NbzZipSource`;
4. central-directory offset/count receipts agree with the canonical walk;
5. the reopened root member bytes are exactly the reintegrated container bytes;
6. the root resource reparses through the DMC3 container registry;
7. the reopened root re-expands to the expected child;
8. the reopened child bytes are exactly the canonical MOD writer output;
9. the child reparses through the canonical MOD parser;
10. reopened `object[0].bounding_radius` is exactly `50.0`.

## Exact-head validation

Implementation commit:

```text
38c79902609c642e38d3472fbf5aa4fc59b8e1aa
```

GitHub Actions Build run:

```text
run id     : 34347990654
run number : 2099
Ubuntu     : PASS — build + tests
Windows    : PASS — build + tests
```

Discovery Site for the same head also passed:

```text
run id : 34347990668
result : PASS
```

This means the registered MOD test exercised the new NBZ composition path on both supported CI operating systems.

## Evidence promoted

This gate promotes:

- `syntheticModToNbzOverlayReopen = true`;
- `syntheticNbzOverlayAcceptanceForEditedMod = true`;
- `modWriterValidatedChildToNbzComposition = true`.

It also retains the already-established prerequisite:

- `provenanceBoundRetailPnstReintegration = true`.

## Evidence not promoted

This gate does **not** establish:

- a provenance-bound retail NBZ overlay run;
- original-runtime precedence/selection of the generated overlay in a real installation;
- original `dmc3.exe` acceptance;
- arbitrary MOD mutation authority;
- transform, skin, material, texture-companion, reflow, or rebuild-from-scratch MOD writer authority;
- Capcom archive-builder equivalence.

No copyrighted retail payload bytes are committed by this gate.

## Next frontier

Do not repeat synthetic MOD/PAC/PNST/NBZ composition work.

The next useful experiment is now concrete:

1. use the already-rebuilt provenance-bound retail PNST parent as the authored root resource;
2. run the existing `build-dmc3-overlay` against an external provenance-bound DMC3 executable/data directory so the next contiguous volume index is derived from the actual installation;
3. reopen the generated overlay and independently verify the root PNST hash/bytes;
4. re-expand the root and prove the edited MOD child hash, physical identity and authored scalar survive the NBZ boundary;
5. only then proceed to original `dmc3.exe` no-edit and controlled-edit acceptance.

That is the remaining MOD container frontier before game acceptance.
