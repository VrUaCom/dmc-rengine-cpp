# GDSpaces L1 Level-E Operator Runbook

**Purpose:** produce the final protected-DMC3 original-game consumption + rollback receipt required for `L1 COMPLETE / 100%`.

**Canonical tracking:** issue #209  
**Canonical desktop implementation:** current `dmc-rengine-cpp` `main` after #208/#213 and subsequent evidence reconciliations  
**Real-device evidence bridge:** Pocket GDS PR #2 / `gdspaces.l1.member-acquisition-receipt.v1`  
**Safety rule:** DMC Rengine tooling must not automatically overwrite the retail installation. The only retail-tree mutation in this run is the explicitly controlled temporary copy of one generated next-volume NBZ, followed by verified rollback.

A Pocket/mobile receipt can provide exact member materialization evidence when the full retail NBZ is local to the device. It does **not** replace the protected executable authority, selected-provider binding, original-game consumption or rollback portions of this run.

## 1. Preconditions

Prepare:

- path to the real protected DMC3 executable directory (`<exe-dir>`);
- a representative game request (`<game-request>`);
- a supported PAC/PNST target slot or a previously rebuilt supported nested target;
- replacement bytes with a deterministic expected consumer-visible effect;
- an empty workspace **outside the complete retail executable tree**;
- a way to record the game observation (notes/screenshot/video/instrumentation as appropriate to the claim).

Optional but useful when the 960 MB archive cannot be moved into the desktop automation environment:

- GDSpace Manager/Pocket GDS with PR #2 or later evidence support;
- the actual source NBZ already local on the device;
- exported representative member + its generated JSON member receipt.

Do not proceed to Level-E if:

- `dmc3.exe` does not pass the protected-distribution preflight;
- the numbered retail volume topology is ambiguous for next-volume authoring;
- the expected destination `DMC3-N.nbz` already exists;
- the exact retail representation is outside the writer domain being used;
- the consumer effect would be ambiguous without instrumentation.

## 2. Optional real-device member acquisition receipt

This step narrows L1-C/L1-D when the actual NBZ is available on the phone but cannot be transferred through the connected large-file channel.

In GDSpace Manager:

```text
Open actual NBZ
 -> search/navigate representative member
 -> select member
 -> Export
```

A successful export must create `gdspaces.l1.member-acquisition-receipt.v1` and preserve:

- archive snapshot SHA-256 + size;
- producer build/source/core revision metadata;
- canonical ResourceIdentity;
- exact logical path;
- central index or nested slot identity where available;
- format/profile/container classification;
- compression method;
- ByteProvenance authority/offset/stored size/materialized size/transform/CRC32 where available;
- exact exported member SHA-256 + size;
- `original_game_consumption = not-claimed`.

For `obj/em000.pac` or another target, preserve both the exported bytes and JSON receipt outside Git. Do not commit proprietary game bytes.

### Mobile receipt acceptance boundary

The receipt proves the recorded **local NBZ snapshot/member materialization**. Before it participates in the final Level-E lineage, independently bind that archive/member identity to the same selected source used by the protected-install request path.

A filename match is insufficient. An unrelated Pocket receipt must not be combined with an unrelated desktop/original-process run.

## 3. Protected executable preflight

Run:

```text
dmc-rengine preflight-dmc3-game-test <exe-dir>
```

Require success as the protected distribution execution authority.

Preserve the executable SHA/size/authority output in the evidence bundle.

If a Pocket receipt is being used, this step still remains mandatory.

## 4. Bind the representative source/request lineage

The final acceptance resource must have one coherent identity chain:

```text
protected game request
 -> selected provider/volume/archive/member
 -> exact L1 materialized member bytes
```

Preferred desktop acquisition path:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

If a Pocket member receipt supplies the materialized bytes instead, require the desktop/protected selection evidence to identify the same archive/member lineage before treating the mobile SHA as the L1 materialized identity.

Do not promote a Pocket archive SHA into protected-selection authority merely because the archive is named `DMC3-0.nbz`.

## 5. Representation classification

Classify the exact materialized member from the accepted source lineage.

For a Pocket receipt, use its canonical node fields and ByteProvenance as direct evidence; for the desktop path, use the acquisition receipt/materialized artifact.

Proceed only if the observed representation is inside the evidenced writer domain used next. In particular:

- PAC/PNST relative-slot authoring must be based on an actually parsed PAC/PNST representation;
- do not infer writer authority from filename alone;
- do not infer a DDS/TM2 conversion step unless direct evidence requires it.

If the representation is unsupported, stop and create a bounded evidence gate instead of forcing it through an existing writer.

## 6. Product-side L1 closure

For a top-level supported PAC/PNST slot:

```text
dmc-rengine verify-dmc3-l1-authoring <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>
```

Require:

- protected executable preflight success;
- direct-retail/selected-source acquisition success;
- artifact-bound acquisition receipt sidecar;
- successful supported representation rebuild;
- generated next-contiguous `DMC3-N.nbz` outside the retail tree;
- canonical higher-volume resolver win;
- exact rebuilt-member rematerialization;
- exact authored target-slot equality;
- `l1-closure.receipt.json`;
- `original_game_consumption = pending-external-level-e-receipt`.

For a nested slot path, use the canonical nested authoring command where needed:

```text
dmc-rengine rebuild-relative-slot-path <container-file> <slot/path> <replacement-file> <output-file>
```

Then build/verify the authored overlay through the canonical overlay/reopen path. Do not invent nested offsets in archive coordinates.

If a real-device member receipt was used as supporting evidence, preserve its member SHA alongside the desktop closure receipt and show where the closure path binds to the same source/member identity.

## 7. Record the generated overlay identity

From the closure workspace record:

- generated volume filename, e.g. `DMC3-1.nbz`;
- full source path;
- file size;
- SHA-256 from the closure receipt;
- authored member/request identity;
- replacement SHA-256.

The generated filename must equal the first missing contiguous runtime volume index.

## 8. Verify destination is absent

Destination is:

```text
<exe-dir>/data/dmc3/<generated-DMC3-N.nbz>
```

Require that this destination does **not** exist.

If it exists, stop. Do not overwrite, rename, merge with or delete an unknown pre-existing artifact as part of this run.

## 9. Controlled publication for Level-E only

Copy the exact generated NBZ to the absent destination.

Immediately compute SHA-256 of the destination and require:

```text
post-copy destination SHA == closure receipt overlay SHA
```

If the hash differs, remove only the copied artifact if ownership is certain, record failure, and stop.

Do not modify `DMC3-0.nbz` or any other original retail archive.

## 10. Original-game observation

Launch the protected distribution executable from the same installation whose preflight identity was recorded.

Enter the deterministic gameplay/menu/scene path that requests `<game-request>`.

Record:

- exact action/path used to trigger the resource;
- expected visible/consumer effect;
- observed effect;
- whether the observation is initial load, reload, restart or transition;
- any instrumentation showing selected higher-volume/member identity when available.

Acceptance requires an effect attributable to the authored bytes.

The following do **not** prove consumption by themselves:

- process launched;
- game reached menu;
- stage loaded without crash;
- no error dialog appeared;
- unrelated visual difference occurred;
- Pocket GDS opened/exported the source archive successfully.

If the visible effect is ambiguous, obtain dynamic selected-resource / typed-consumer evidence before claiming success.

For the first full vertical proof preserve, where available:

```text
exact L2 selected identity
 -> exact L1 materialized byte identity
 -> L3 acquisition/state1
 -> completion/state2
 -> typed-ready/state3
 -> deterministic consumer effect
```

Do not invent lifecycle fan-in fields that are not evidenced by the raw EXE.

## 11. Clean exit / transition

Exit or transition through the relevant normal game path.

Record whether the game exited/transitioned cleanly and any load/reload behavior relevant to the acceptance claim.

Broader L3 lifecycle equivalence is separate; only the lifecycle evidence needed to bind this consumer observation is required for L1 acceptance.

## 12. Rollback

After the game is no longer using the resource:

1. verify the temporary destination still has the expected overlay SHA;
2. remove **only** that known test overlay;
3. require the path no longer exists;
4. verify no original retail numbered archive was rewritten by the procedure;
5. preserve a rollback receipt/note with the overlay SHA and removal result.

If any original retail archive changed, the Level-E run fails even if the game showed the desired effect.

## 13. Required evidence bundle

A valid L1 Level-E bundle contains:

- protected executable SHA/size/authority role;
- exact selected-provider/source/archive/member identity;
- direct-retail acquisition receipt or an equivalently bound member acquisition chain;
- optional Pocket `gdspaces.l1.member-acquisition-receipt.v1` when used;
- exact materialized SHA/size/ByteProvenance;
- explicit representation classification;
- replacement file SHA/size;
- rebuilt container/member SHA;
- generated overlay filename/index/SHA/size;
- closure receipt;
- post-copy destination SHA;
- game request + authored slot/path identity;
- deterministic original-game consumer observation;
- instrumentation if visible attribution is ambiguous;
- clean transition/exit note relevant to the test;
- rollback receipt proving test overlay removal;
- evidence that original retail archives remained unchanged.

Every receipt used in the completion claim must belong to one cryptographically/structurally reconciled lineage. Do not compose independent evidence packets by filename alone.

## 14. Final acceptance decision

After the evidence bundle exists, run the final cross-stack audit using `l1-final-audit-2026-08-25.md` together with `l1-real-device-member-evidence-reconciliation-2026-08-26.md` and any later completion audit.

Mark `L1 COMPLETE / 100%` only when:

- all mandatory L1-C..G receipts are valid and same-lineage;
- #209 acceptance is satisfied;
- final canonical code/docs have exact-head Windows + Ubuntu green CI;
- no contradictory evidence changes the claimed representation/materialization scope;
- V issues the final L1 original-equivalence verdict.

If any item fails, record the exact failing gate. Do not lower the acceptance criterion to reach 100%.
