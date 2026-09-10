# Урок 1 — Що таке `.MOD` і де він живе

## 1. Не «просто 3D-модель»

У DMC3 HD `.MOD` — serialized MOD ресурс скінованої 3D-моделі. Він поєднує геометрію, object/mesh state, skeleton node domain, rest transforms, skin indices/weights, texture-slot selectors і legacy material state.

```text
MOD = model document
    + objects
    + meshes
    + vertex streams
    + skeleton/rest pose
    + skin binding
    + material selectors
    + runtime fixup contract
```

## 2. Поточний статус reverse

**MOD reverse complete for the canonical DMC3 HD scope.**

Це не означає, що кожен байт отримав красиву artistic/material назву. Це означає, що кожен важливий serialized/runtime domain має фінальний evidence-статус.

Можливі фінальні стани:

- typed semantic — пряме значення доведено;
- `PRESERVED_UNDECODED` — байт/поле і його flow відомі, але сильніша назва не доведена;
- `RESERVED_OBSERVED_ZERO` — bounded corpus zero, але universal padding не вигадується;
- `REJECTED` — стару гіпотезу спростовано;
- family-sensitive boundary — однаковий offset у MOD/EFM/SCM не отримує одну семантику автоматично.

Тому `PRESERVED_UNDECODED` тепер є terminal evidence classification, а не «ще треба дореверсити».

## 3. Ресурсний стек

```text
DMC3-*.nbz
 -> GData.afs/ logical namespace
 -> PAC або PNST
 -> physical slot / nested container
 -> MOD payload
```

- **NBZ** — volume/transport layer.
- **PAC/PNST** — container layer.
- **MOD** — resource payload.
- **`.index`** — extraction/naming metadata, не runtime MOD manifest.

Writer не закінчується на `.mod`: authored child треба коректно повернути в parent container, потім у NBZ/overlay chain.

Після PR #372 ми вже маємо provenance-bound **real retail PNST reintegration** для одного authored MOD child і synthetic MOD -> container -> NBZ overlay -> reopen. Але original-runtime selection/consumption цього overlay ще не доведені.

## 4. Що MOD дає runtime

Після завантаження гра:

1. читає document/object/mesh records;
2. relocates потрібні offsets;
3. будує runtime mesh state;
4. зв’язує texture slots із зовнішнім companion;
5. використовує skeleton runtime nodes;
6. будує current world matrices;
7. використовує inverse rest matrices;
8. формує skin palette;
9. подає skin indices/weights у vertex shader.

## 5. З чим MOD взаємодіє

### Texture companion
`mesh.texture_slot` посилається у зовнішню texture domain authority.

### MOT / CMotion
MOD дає hierarchy/rest pose/motion-group relationship. MOT/CMotion дає evaluated animation data. Serialized MOD rest pose не можна змішувати з animated pose.

### SCM
SCM і MOD частково ділять model-family infrastructure, але SCM — scene/stage geometry, MOD — skinned model pipeline.

### EFM
Спільні physical offsets не гарантують однакової семантики. `mesh+0x38` — хороший приклад: audited MOD path не forward-ить його, тоді як homologous EFM slot live.

### SHW
SHW має shadow-hull geometry і matrix selectors. Це сусідня resource-family authority і не є відкритим MOD reverse blocker.

## 6. Evidence baseline

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

Canonical reverse covers document/object/mesh ABI, hierarchy/order, transforms, texture slot + GS CLAMP, post-load topology, companion validation, inverse-rest skin palette, runtime texture descriptor ABI, direct skin ABI, motion-group relationship і source-byte preservation.

## 7. Ключові closure outcomes

- `BLENDINDICES.x` — preserved ABI byte; audited compiled shader reads Y/Z/W.
- source flag `0x00100000` — bounded EXE-confirmed legacy GS TEST/ZBUF selector.
- source flag `0x00200000` — runtime-carried/restored terminal preservation state; artistic label intentionally absent.
- header `+0x14` — raw runtime-carried `u32`; старий universal decimal interpretation rejected для MOD.
- mesh `+0x38` — family-sensitive terminal state: inactive в audited MOD path, homologous EFM slot live.
- mesh `+0x0C/+0x4C` і transform `+0x1C` — explicit source-preserved terminal states.

Це вже закриті reverse результати.

## 8. Що вже доведено як writer

- **Preserve-Layout Writer Gate 1** (#365): immutable source, no structural reflow, explicit fixed-size authorized spans.
- deterministic corpus runner (#367).
- provenance-bound no-op corpus (#368): **38/38**, 882,736 bytes, exact byte equality, canonical reopen.
- provenance-bound `em000_021.mod` controlled radius edit (#369): span `[124,128)`, exactly 3 changed bytes, all other bytes preserved.
- `ModAuthoredChildBridge` + synthetic PAC reintegration (#369).

## 9. Що вже доведено для container chain

### Real retail PNST — PR #372

```text
parent                   m20_s00_012.pac
representation           PNST
physical slots           33
target slot              23
target offset            129280
target size              1888
parent size              346272
```

Після authored MOD reintegration:

- parent size unchanged;
- slot table unchanged;
- у всьому parent змінені лише 3 expected child bytes;
- canonical reparse/re-expand повертає exact writer output;
- MOD reopen бачить requested edited radius.

### Synthetic NBZ overlay — PR #372

Authored MOD проходить через existing container writer/reintegrator та existing NBZ overlay writer/source, generated NBZ reopens, root member дорівнює rebuilt container, а MOD canonical reopen зберігає authored value.

Це **synthetic NBZ product gate**, не retail/original-runtime acceptance.

## 10. Що залишається після reverse completion

Не закриті **writer/integration/acceptance** gates:

- full production MOD writer authority;
- typed-IR-only layout synthesis/reflow;
- transform authoring;
- skin/blend-index authoring;
- source-flag/material/texture-binding authoring;
- texture-companion rewriting/coherence;
- broader mutation authority для preservation-only fields;
- provenance-bound retail NBZ overlay acceptance;
- original `dmc3.exe` no-op rebuilt-MOD acceptance;
- original `dmc3.exe` edited-MOD acceptance.

Сусідні MOT playback і TIM2 authoring мають власні authority і не рахуються як незакритий MOD reverse.

Правильна evidence chain така:

```text
MOD reverse complete
!= full writer complete
!= retail NBZ acceptance
!= original-game acceptance
```
