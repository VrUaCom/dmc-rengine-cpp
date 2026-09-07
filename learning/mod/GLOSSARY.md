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

**Texture slot** — mesh `+0x02`, index у runtime texture descriptor domain.

**Texture companion** — external TM2-backed source texture envelope для model family.

**Runtime texture descriptor** — in-memory `0x40` descriptor, побудований із companion; не serialized MOD.

**GS CLAMP** — legacy PS2 GS clamp/region-repeat state у mesh.

**GS TEX0 / MIPTBP1** — legacy GS texture register images у runtime descriptor.

**BLENDINDICES** — per-vertex `u8x4` matrix-row selectors. Skin influences підтверджені для y/z/w.

**Packed weights** — three 5-bit quantized weights у lower 15 bits control word.

**Topology break** — high `0x8000` control bit, consumed/cleared CPU post-load.

**Generated topology workspace** — runtime workspace, адресований mesh `+0x40` relative to mesh record.

**PRESERVED_UNDECODED** — bytes/layout відомі й мають бути збережені, exact semantics не доведено.

**RESERVED_OBSERVED_ZERO** — zero observed у конкретному bound corpus; не global semantic claim.

**EXE_CONFIRMED** — direct canonical executable evidence.

**CORPUS_CONFIRMED / DATA_CONFIRMED** — hash-bound real data evidence.

**Writer authority** — доказ, що DMC Rengine може правильно serialized/rebuild/edit цей domain; reader support не дає writer authority автоматично.

**Fail closed** — при malformed/unsupported input система не вигадує fallback semantics і не робить partial unsafe mutation.

**Preservation authority** — canonical `Document::source_bytes`, джерело для exact unknown-byte retention.

**SCM** — споріднений model-family static/stage scene format; binary/semantic contract окремий.

**EFM** — effect-system model-family resource, який ділить частину runtime infrastructure з MOD.

**MOT/CMotion** — animation subsystem/payload path, що постачає evaluated pose.

**SHW** — self-contained shadow hull resource із per-vertex transform-matrix selectors; exact palette ownership open.

**SO** — project working family для окремих observed data structures; MOD binding analysis нині cardinality/correlation-level.

**PAC/PNST** — containers, у яких MOD може бути slot payload.

**NBZ** — volume/distribution layer над resource/container stack.
