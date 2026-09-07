# Урок 1 — Що таке `.MOD` і де він живе

## 1. Не «просто 3D-модель»

У DMC3 HD `.MOD` — це serialized ресурс скінованої 3D-моделі. Він поєднує геометрію, object/mesh state, skeleton node domain, rest transforms, skin indices/weights, texture-slot selectors і legacy material state.

Правильніше думати так:

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

Сам файл не містить усю систему рендеру. Частина даних приходить із сусідніх ресурсів та runtime.

## 2. Ресурсний стек

Типовий шлях до MOD:

```text
DMC3-*.nbz
 -> GData.afs/ logical namespace
 -> PAC або PNST
 -> slot / nested container
 -> MOD payload
```

- **NBZ** — volume/transport layer.
- **PAC/PNST** — container layer.
- **MOD** — resource payload.
- **`.index`** — extraction/naming metadata, не runtime MOD manifest.

Це важливо для майбутнього writer: правильний authoring не закінчується на записі `.mod`; треба reintegrate його назад у parent container і потім у NBZ/overlay chain.

## 3. Що MOD дає runtime

Після завантаження гра:

1. читає document/object/mesh records;
2. relocates потрібні offsets;
3. будує runtime mesh state;
4. зв’язує mesh texture slots із зовнішнім texture companion;
5. створює/використовує skeleton runtime nodes;
6. будує current world matrices;
7. використовує inverse rest matrices;
8. формує skin palette;
9. подає skin indices/weights у MOD vertex shader.

Тому MOD працює одночасно з CPU runtime і GPU shader ABI.

## 4. З чим MOD взаємодіє

### Texture companion
MOD має `mesh.texture_slot`, але сама texture domain authority приходить із зовнішнього TM2-backed companion.

### MOT / CMotion
MOD дає hierarchy, rest transforms і motion-group selector. MOT/CMotion дає evaluated animation pose. Не можна змішувати serialized MOD rest pose з animated pose.

### SCM
SCM і MOD мають частину спільного model-family shell, але SCM — static/stage scene geometry, MOD — skinned model pipeline.

### EFM
MOD та EFM ділять частину runtime model-family infrastructure, зокрема MOD/EFM transform initializer, але повний schema не можна переносити між ними автоматично.

### SHW
SHW має власну shadow-hull geometry і per-vertex matrix selectors. Точна ownership matrix palette щодо MOD skeleton ще відкрита.

### SO
DMC Rengine має окремий cross-resource MOD/SO cardinality analysis. Це correlation, не доказ semantic identity.

## 5. Що вже доведено

На current `main` канонізовані:

- document/object/mesh structural reader;
- node hierarchy;
- local transforms;
- world composition;
- texture slot + GS CLAMP;
- runtime post-load relocation/topology;
- texture companion envelope;
- companion-authoritative texture validation;
- inverse rest + skin palette;
- runtime texture descriptor ABI;
- object runtime flag projection;
- mesh unknown-field preservation;
- direct skin index/weight shader ABI;
- default joint index;
- motion-group selector;
- MOD-side animated pose -> world -> skin palette composition.

## 6. Що ще не доведено як writer

Не закриті production writer authority, broad no-edit byte parity, safe mutation ranges для unknown fields, complete MOT semantics, complete TIM2 authoring і original-game acceptance edited MOD.

Це не недолік parser. Це правильна evidence boundary.
