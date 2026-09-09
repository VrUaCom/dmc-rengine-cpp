# DMC3 HD MOD — controlled PAC reintegration gate

Date: 2026-09-09  
Branch: `reverse/mod-completion-20260907`  
Evidence class: `WRITER_GATE / CONTAINER_REINTEGRATION_GATE_1`

## Context

The preserve-layout MOD writer is already promoted, and the provenance-bound 38-file retail corpus has independently passed no-edit parse/write/byte-parity/reopen. This pass does not repeat those gates. It addresses the next missing boundary: moving a writer-validated, same-size MOD child back into an existing PAC/PNST materialized parent without inventing another container writer.

## New trust boundary

`ModAuthoredChildBridge` converts a canonical MOD `WriteResult` into the existing generic `AuthoredChildImage` only after independently checking:

- readable non-container source payload;
- exact `ResourceId.size == source.bytes.size()` binding;
- canonical source MOD parse;
- successful canonical MOD writer result;
- valid writer receipt;
- immutable-source/document binding;
- unauthorized-byte preservation;
- unchanged byte count;
- independently recomputed source SHA-256;
- independently recomputed output SHA-256;
- independent canonical output MOD reopen.

The bridge deliberately does not infer PAC slot identity from a filename or from a free-form `writer_mode` string. It carries the exact child `ResourceId` supplied by `ContainerExpander` into `AuthoredChildImage`.

## Controlled mutation used by the gate

The existing bounded `mod-writer-set-bounding-radius` command is retained as the mutation producer for this pass. It edits only one promoted `object.bounding_radius` field in place, requires all changed bytes to remain inside the exact four-byte serialized radius span, rereads the emitted file, verifies the writer output SHA-256 and canonical reopen, forbids source overwrite, and fails closed on output/receipt path collisions.

This does not grant arbitrary MOD mutation authority.

## End-to-end regression

`dmc_rengine_mod_authored_child_bridge_tests` constructs:

```text
synthetic PAC
  -> one canonical MOD child
  -> canonical MOD parse
  -> bounding-radius edit
  -> preserve-layout MOD Writer
  -> ModAuthoredChildBridge
  -> AuthoredChildImage
  -> existing NestedRelativeSlotReintegrator
  -> rebuilt materialized PAC
  -> canonical PAC reparse
  -> ContainerExpander reopen
  -> canonical MOD reparse
```

The regression verifies:

- the authored child keeps the exact expanded child `ResourceId`;
- source/output hashes equal the canonical MOD writer receipt;
- reintegration changes exactly one bounded parent child span;
- the rebuilt PAC preserves parent byte count and reparses;
- reopening the PAC reproduces the exact MOD writer bytes;
- the reopened MOD contains the requested bounding radius;
- tampering with writer output bytes after the writer returns is rejected by the bridge through independent output hashing;
- a container-marked source cannot be laundered through the MOD child bridge.

## Reused architecture

No new PAC writer, PNST writer, NBZ packer or alternate MOD serialization path is added. The gate reuses:

- `formats::mod::Writer`;
- `gdspaces::ContainerExpander`;
- `profiles::dmc3::AuthoredChildImage`;
- `profiles::dmc3::NestedRelativeSlotReintegrator`;
- canonical PAC/PNST parser registry.

## Claims promoted by this pass

Promoted only when exact-head CI succeeds:

- a canonical MOD writer result can be converted into the generic authored-child envelope through a format-specific fail-closed bridge;
- a controlled same-size MOD edit can be reintegrated into a synthetic PAC through the existing Layer-1 container writer path;
- the rebuilt synthetic PAC can be reparsed/re-expanded and the authored MOD can be reopened with exact emitted child bytes.

## Claims explicitly not promoted

This pass does **not** establish:

- retail PAC/PNST corpus reintegration acceptance;
- texture-companion rewriting or coherence after texture-domain edits;
- arbitrary MOD field editing;
- transform, skin, material or source-flag writer authority;
- layout reflow or rebuild-from-scratch;
- NBZ overlay acceptance for the edited MOD/PAC chain;
- original `dmc3.exe` acceptance;
- full MOD writer authority.

## Next evidence frontier

After this gate is promoted, the next non-repeating authoring evidence should be one of:

1. provenance-bound retail PAC/PNST reintegration of a writer-validated same-size MOD child;
2. root-resource emission through the existing `NbzStoreOverlayWriter` followed by canonical NBZ reopen;
3. original `dmc3.exe` acceptance of a no-edit or tightly controlled edited resource chain.

Texture-companion writer authority remains a separate gate and is not required for a bounding-radius edit because this mutation does not alter texture-slot/domain state.
