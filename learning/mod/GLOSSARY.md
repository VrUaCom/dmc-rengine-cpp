# MOD Glossary

**MOD** — serialized DMC3 HD skinned model resource.

**Outer object / model** — `0x40` object record, що групує meshes, flags, bounds і object-level runtime state.

**Inner mesh** — `0x50` mesh record із geometry streams, texture selector, skin/control streams і topology workspace contract.

**Node domain** — serialized hierarchy/order/motion-group/transform domain.

**Node index** — canonical identity node у domain.

**Order position** — позиція node у topological evaluation order. Це не те саме, що node index.

**parentByOrderPosition** — parent node index для кожної order position.

**nodeAtOrderPosition** — permutation, що каже, який node evaluation відбувається на цій position.

**Rest local** — serialized local transform із MOD.

**Animated local** — evaluated runtime animation transform, який приходить із animation layer; не serialized MOD rest record.

**rootBase** — зовнішня/root matrix, з якою множиться root local.

**currentWorld** — поточні hierarchical world matrices.

**inverseRestWorld** — inverse model-space rest/world-at-load matrix per node.

**Skin palette** — `inverseRestWorld * currentWorld`.

**Motion group** — EXE-confirmed MOD per-node selector, який runtime переносить у `CMotionJoint +0xF8`.

**default_joint_index** — MOD header `+0x13`, `JntNo` fallback і currentWorld selector. Не доведено як обов’язковий root.

**runtime_metadata_u32** — MOD header `+0x14`; raw `u32`, EXE-confirmed transfer до manager `+0xE4`. Старий universal decimal family/model/sub-index interpretation відхилений multi-corpus evidence; high-level semantic лишається `PRESERVED_UNDECODED`.

**Texture slot** — mesh `+0x02`, index у runtime texture descriptor domain.

**Texture companion** — external TM2-backed source texture envelope для model family.

**Runtime texture descriptor** — in-memory `0x40` descriptor, побудований із companion; не serialized MOD.

**GS CLAMP** — legacy PS2 GS clamp/region-repeat state у mesh.

**GS TEX0 / MIPTBP1** — legacy GS texture register images у runtime descriptor.

**GS TEST_1 / ZBUF_1 selector (`0x00100000`)** — EXE-confirmed MOD object source-flag semantic у active low-mode path: перемикає `TEST_1.AREF 0↔16` і `ZBUF_1.ZMSK 1↔0`. Це technical renderer semantic, не artistic/material category.

**Source flag `0x00200000`** — real runtime-carried baseline/effective state. Confirmed local GS packet helper його не інтерпретує; high-level semantic open, writer preserve.

**BLENDINDICES** — per-vertex `u8x4` matrix-row selector stream. Skin influences підтверджені для Y/Z/W. Canonical compiled DXBC має `ReadWriteMask 0xE`, тобто X не читається; raw X усе одно залишається source-preserved ABI byte.

**Packed weights** — three 5-bit quantized weights у lower 15 bits control word.

**Topology break** — high `0x8000` control bit, consumed/cleared CPU post-load.

**Generated topology workspace** — runtime workspace, адресований mesh `+0x40` relative to mesh record.

**Mesh auxiliary slot `+0x38`** — family-sensitive physical slot. Canonical MOD path його не forward-ить і current MOD corpus має zero; homologous EFM slot live як COLOR0. Не переносити EFM semantic у MOD.

**Transform `+0x1C`** — fourth scalar in 0x20 local-transform record. Bound MOD/EFM corpus zero; canonical local rotation helper не читає його. Global semantic `PRESERVED_UNDECODED`, writer preserve.

**PRESERVED_UNDECODED** — bytes/layout відомі й мають бути збережені, exact semantics не доведено.

**RESERVED_OBSERVED_ZERO / multi-corpus observed zero** — zero observed у конкретному bound corpus; не global semantic claim і не writer permission.

**EXE_CONFIRMED** — direct canonical executable evidence.

**CORPUS_CONFIRMED / DATA_CONFIRMED** — hash-bound real data evidence.

**EXE_AND_CORPUS_CONFIRMED** — independent executable і corpus evidence підтримують один bounded claim.

**REJECTED** — hypothesis прямо спростована evidence і не повинна повертатися в canonical naming без нового доказу.

**Writer authority** — доказ, що DMC Rengine може правильно serialize/rebuild/edit цей domain; reader support не дає writer authority автоматично.

**Fail closed** — при malformed/unsupported input система не вигадує fallback semantics і не робить partial unsafe mutation.

**Preservation authority** — canonical `Document::source_bytes`, джерело для exact unknown-byte retention.

**SCM** — споріднений model-family static/stage scene format; binary/semantic contract окремий.

**EFM** — effect-system model-family resource, який ділить частину runtime infrastructure з MOD, але може використовувати homologous mesh slots інакше.

**MOT/CMotion** — animation subsystem/payload path, що постачає evaluated pose.

**SHW** — self-contained shadow hull resource із per-vertex transform-matrix selectors; exact palette ownership open.

**SO** — project working family для observed data structures. Latest classifier work confirms MOD transform-domain binding for the SO link-table node selector, але SO physical records лишаються окремим resource family.

**PAC/PNST** — containers, у яких MOD може бути slot payload.

**NBZ** — volume/distribution layer над resource/container stack.
