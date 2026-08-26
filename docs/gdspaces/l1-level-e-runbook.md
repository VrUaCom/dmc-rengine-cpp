# GDSpaces L1 Level-E Operator Runbook

**Purpose:** produce the final protected-DMC3 original-game consumption + rollback receipt required for `L1 COMPLETE / 100%`.

**Canonical tracking:** issue #209  
**Prerequisite code:** current `main` after #208 and #213  
**Safety rule:** DMC Rengine tooling must not automatically overwrite the retail installation. The only retail-tree mutation in this run is the explicitly controlled temporary copy of one generated next-volume NBZ, followed by verified rollback.

## 1. Preconditions

Prepare:

- path to the real protected DMC3 executable directory (`<exe-dir>`);
- a representative game request (`<game-request>`);
- a supported PAC/PNST target slot or a previously rebuilt supported nested target;
- replacement bytes with a deterministic expected consumer-visible effect;
- an empty workspace **outside the complete retail executable tree**;
- a way to record the game observation (notes/screenshot/video/instrumentation as appropriate to the claim).

Do not proceed if:

- `dmc3.exe` does not pass the protected-distribution preflight;
- the numbered retail volume topology is ambiguous for next-volume authoring;
- the expected destination `DMC3-N.nbz` already exists;
- the exact retail representation is outside the writer domain being used;
- the consumer effect would be ambiguous without instrumentation.

## 2. Protected executable preflight

Run:

```text
dmc-rengine preflight-dmc3-game-test <exe-dir>
```

Require success as the protected distribution execution authority.

Preserve the executable SHA/size/authority output in the evidence bundle.

## 3. Product-side L1 closure

For a top-level supported PAC/PNST slot:

```text
dmc-rengine verify-dmc3-l1-authoring <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>
```

Require:

- direct-retail acquisition success;
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

## 4. Record the generated overlay identity

From the closure workspace record:

- generated volume filename, e.g. `DMC3-1.nbz`;
- full source path;
- file size;
- SHA-256 from the closure receipt;
- authored member/request identity;
- replacement SHA-256.

The generated filename must equal the first missing contiguous runtime volume index.

## 5. Verify destination is absent

Destination is:

```text
<exe-dir>/data/dmc3/<generated-DMC3-N.nbz>
```

Require that this destination does **not** exist.

If it exists, stop. Do not overwrite, rename, merge with or delete an unknown pre-existing artifact as part of this run.

## 6. Controlled publication for Level-E only

Copy the exact generated NBZ to the absent destination.

Immediately compute SHA-256 of the destination and require:

```text
post-copy destination SHA == closure receipt overlay SHA
```

If the hash differs, remove only the copied artifact if ownership is certain, record failure, and stop.

Do not modify `DMC3-0.nbz` or any other original retail archive.

## 7. Original-game observation

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
- unrelated visual difference occurred.

If the visible effect is ambiguous, obtain dynamic selected-resource / typed-consumer evidence before claiming success.

## 8. Clean exit / transition

Exit or transition through the relevant normal game path.

Record whether the game exited/transitioned cleanly and any load/reload behavior relevant to the acceptance claim.

Broader L3 lifecycle equivalence is separate; only the lifecycle evidence needed to bind this consumer observation is required for L1.

## 9. Rollback

After the game is no longer using the resource:

1. verify the temporary destination still has the expected overlay SHA;
2. remove **only** that known test overlay;
3. require the path no longer exists;
4. verify no original retail numbered archive was rewritten by the procedure;
5. preserve a rollback receipt/note with the overlay SHA and removal result.

If any original retail archive changed, the Level-E run fails even if the game showed the desired effect.

## 10. Required evidence bundle

A valid L1 Level-E bundle contains:

- protected executable SHA/size/authority role;
- direct-retail acquisition receipt;
- exact selected volume/archive/member identity;
- retail materialized SHA/size/ByteProvenance;
- explicit retail representation classification;
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

## 11. Final acceptance decision

After the evidence bundle exists, run the final cross-stack audit in `l1-final-audit-2026-08-25.md`.

Mark `L1 COMPLETE / 100%` only when:

- all mandatory L1-C..G receipts are valid;
- #209 acceptance is satisfied;
- final canonical code/docs have exact-head Windows + Ubuntu green CI;
- no contradictory evidence changes the claimed representation/materialization scope.

If any item fails, record the exact failing gate. Do not lower the acceptance criterion to reach 100%.
