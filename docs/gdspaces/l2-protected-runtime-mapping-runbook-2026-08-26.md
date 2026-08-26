# GDSpaces Layer 2 — protected-runtime RVA mapping runbook — 2026-08-26

**Primary layer:** L2 / Resource Resolution evidence  
**Implementation slice:** PR #219  
**Purpose:** prove bounded address mapping for selected resolver anchors before any original-process selected-provider instrumentation.

This runbook does **not** prove retail NBZ collision freedom, selected-provider identity, Layer 1 completion or Layer 3 lifecycle semantics.

## 1. Authority split

Machine-readable authority: `data/registries/dmc3-reverse-authorities.v1.json`.

### Canonical instruction-reverse artifact

- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- size: `6,356,432`
- preferred image base: `0x140000000`
- role: instruction-level reverse authority

### Protected original-execution candidate

- SHA-256: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`
- size: `6,567,320`
- role: protected distribution provenance + original execution candidate
- **not** instruction-level reverse authority

Do not copy canonical VAs into the protected process. Runtime capture always uses `actual module base + RVA`.

## 2. Approved initial mapping anchors

All initial windows are exactly `0x40` bytes and must resolve to `.text` in the protected PE headers.

| Anchor | Canonical VA | RVA | Role |
|---|---:|---:|---|
| `OpenGameResource` | `0x14002FCA0` | `0x0002FCA0` | generic resolver entry / six-prefix branch authority |
| type-0 mount registration | `0x140326D20` | `0x00326D20` | physical provider registration / `0x0C` normalization flags |
| type-0 mount resolve | `0x140327430` | `0x00327430` | physical provider candidate resolve path |
| type-0 final open | `0x140327800` | `0x00327800` | exact Win32 final-open edge |

A valid mapping packet requires:

- `OpenGameResource`;
- at least two of the three physical anchors;
- all receipts from one PID / one module base / one image path;
- exact protected artifact SHA/size;
- exact canonical artifact SHA on every expectation;
- exact runtime-window SHA == canonical-window SHA for every included anchor.

One matching child window is deliberately insufficient.

## 3. Acquire canonical expected window hashes

Run locally against the exact canonical analysis executable. Raw bytes are not required.

```text
dmc-rengine extract-exe-window <canonical-dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x14002FCA0 0x40
dmc-rengine extract-exe-window <canonical-dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x140326D20 0x40
dmc-rengine extract-exe-window <canonical-dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x140327430 0x40
dmc-rengine extract-exe-window <canonical-dmc3.exe> e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 0x140327800 0x40
```

For each receipt preserve only the metadata needed for mapping, especially `window_sha256`.

Do not use a canonical receipt whose `artifact_sha256`, `artifact_size`, VA/RVA or size differs from this runbook.

## 4. Launch the protected game and identify the PID

Use the exact protected distribution build. The capture command independently reads the running process image path and requires its on-disk SHA/size to match the supplied protected authority.

PID must be supplied explicitly. The tool does not attach to the first process with a matching name.

## 5. Capture live windows by RVA

For each approved anchor run:

```text
dmc-rengine capture-exe-process-window <PID> 81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6 6567320 <RVA> 0x40 --expect-window e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 <CANONICAL_WINDOW_SHA256>
```

Example shape only:

```text
dmc-rengine capture-exe-process-window 1234 81c7... 6567320 0x2FCA0 0x40 --expect-window e454272e... <64-hex-window-sha>
```

Do **not** use `--hex` for mapping-packet inputs. The multi-anchor validator rejects child receipts containing `bytes_hex` so proprietary process bytes cannot accidentally enter the metadata packet.

### Successful child receipt

Exit `0` plus `matches_expected_window: true` proves only that exact live range at that RVA matched the exact canonical range hash.

### Mismatch

Exit `8` plus `matches_expected_window: false` is negative evidence. Do not edit the JSON or force promotion.

A mismatch does not by itself prove that the RVA is wrong. Possible causes include protection/runtime transformation, loader relocation affecting the selected bytes, a different code build or an incorrect mapping hypothesis. The next reverse step must distinguish those causes with narrower signatures/relocation-aware evidence rather than weakening the hash gate.

## 6. Build the bounded multi-anchor packet

Supply at least three metadata-only child receipts:

```text
python scripts/reverse/verify_l2_runtime_mapping_packet.py \
  --receipt open-game.json \
  --receipt type0-registration.json \
  --receipt type0-resolve.json \
  --output l2-runtime-mapping.packet.json
```

The validator fails closed when:

- artifact SHA/size differs;
- canonical expectation authority differs;
- a child has `bytes_hex`;
- a child mismatch is present;
- PID/module base/image path changes between receipts;
- an RVA is duplicated or not approved;
- `OpenGameResource` is missing;
- fewer than two physical anchors are present;
- window size/section/base/range metadata is inconsistent;
- output already exists.

The output is created no-replace.

## 7. Promotion boundary

A valid packet may promote only:

```text
protected runtime process
 -> listed L2 RVAs
 -> exact live byte matches to listed canonical-analysis windows
 -> bounded address mapping for those listed anchors
```

It does not promote:

- global byte/build equivalence;
- unlisted function addresses;
- original resolver selected-provider identity;
- retail DMC3 `0x0E` collision status;
- L1 or L3 completion.

Only after this packet is valid may the next L2 slice attach selection instrumentation to the mapped resolver anchors.

## 8. Public evidence hygiene

Child process receipts contain the local executable path and should be treated as local/raw evidence unless explicitly sanitized through a future public-export contract. Do not manually edit a child receipt and then call it canonical evidence.

The final mapping packet contains hashes and bounded address metadata; review it for local path disclosure before publishing outside the controlled evidence store.

## 9. Current environment boundary

The connected automation environment cannot execute this real protected-process capture because it does not currently expose/rerun the exact protected original game process. CI therefore validates only:

- Win32 self-process read mechanics;
- non-Windows fail-closed behavior;
- receipt structural authority guards;
- multi-anchor validator positive/negative paths;
- no-replace preservation.

A green CI run makes the acquisition tooling promotion-ready; it does not close the real original-process mapping gate.
