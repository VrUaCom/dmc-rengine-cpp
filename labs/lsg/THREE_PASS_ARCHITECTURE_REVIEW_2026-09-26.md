# Three-Pass Architecture Review — Pass 1 + Pass 2 + Pass 3

Date: 2026-09-26  
Reviewed baseline: `110cf0e68ffe51c7c5b3360b86615af63010b42f`  
Current branch also contains the Windows character-picker UI fix after that baseline.

Overall status: **CONDITIONAL PASS / PASS 4 BLOCKED ON FOUNDATION CLEANUP**

The first three passes established the intended direction correctly:

- shared carrier geometry instead of one mesh copy per character;
- universal Face DNA / FaceField rather than Ada-specific identity code;
- shared SkinPhenotype / material library rather than character-specific skin shaders or baked skin textures.

The review found no requirement to roll back any of those three decisions. The blockers are scalability and ownership boundaries that must be corrected before the runtime becomes data-driven.

## PASS 1 — Shared Carrier / Character Profile

### PASS

- Ada references the female carrier rather than packaging duplicate female body/eye geometry.
- Profile identity and carrier geometry are separate concepts.
- Storage contract reports zero profile-specific carrier duplicate bytes.
- Male and female carriers remain independently addressable.

### BLOCKERS

#### R1 — Compile-time registry

`kBuiltinCarrierCount = 2`, `kBuiltinProfileCount = 3`, fixed `std::array` registries and the `CarrierId` enum make the prototype finite at compile time.

Required Pass 4 action:

- data-driven carrier/profile registry;
- stable profile/carrier handles or IDs;
- explicit lookup failure instead of modulo normalization.

#### R2 — Silent ID aliasing

`normalize_character_profile_index(index)` uses modulo and `carrier_slot()` maps every non-female enum value to male. Invalid IDs can silently select a valid but wrong resource.

Required:

- checked lookup;
- invalid handle/status;
- no modulo fallback in runtime resource resolution.

#### R3 — Eager carrier residency

`create_mesh_buffers()` uploads every built-in carrier during renderer initialization. Current two-carrier payload is approximately:

- one carrier body + eyes: 1,576,664 B;
- both carriers body + eyes: 3,153,328 B.

The single-character viewer therefore keeps roughly one extra carrier payload resident before allocator overhead.

Required:

- carrier cache;
- load on first use;
- reference counting / last-use tracking;
- eviction policy and residency telemetry.

#### R4 — Geometry buffers use host-visible memory

Static carrier vertex/index buffers are created directly in `HOST_VISIBLE | HOST_COHERENT` memory. This is convenient for the prototype but is not the final GPU residency path.

Required:

- staging upload;
- device-local static geometry when supported;
- allocation-size accounting from Vulkan memory requirements;
- host-visible fallback only when required by device architecture.

## PASS 2 — Universal Identity / Face DNA

### PASS

- Male Base, Female Base and Ada use generator revision 3.
- Face identity ownership moved out of legacy GeometryGenome jaw/facial-softness slots.
- Face and eye sockets use the same Face DNA parameter set.
- No FaceField branch depends on Ada, profile number, sex or character name.
- Raw diagnostic geometry remains outside genome deformation.
- JSON/runtime source parity gate currently prevents silent profile drift.

### BLOCKERS

#### R5 — Face data is stored inside FrameLightingGpu

The 20 Face DNA floats are packed into the same 192-byte UBO as scene lighting. This is semantically wrong for multiple characters: lighting is frame/global state, Face DNA is character-instance state.

Required:

- separate character identity/anatomy GPU block;
- per-character or dynamic-offset binding;
- lighting buffer must remain independent of selected identity.

#### R6 — Hard-coded canonical face basis

FaceField assumes a fixed MakeHuman-derived metric basis:

- head pivot Y;
- canonical face half-width;
- face half-height;
- face depth;
- fixed eye socket centres.

This is valid only for compatible carriers.

Required:

- carrier metadata describing canonical head/face basis;
- compatibility/version field for FaceField;
- reject incompatible carriers instead of applying the field blindly.

#### R7 — CPU and GLSL FaceField are manually duplicated

The current formulas match by inspection and tests protect CPU behavior, but the GPU implementation is a separate handwritten copy. CI cannot numerically execute the GLSL implementation and compare it with CPU reference output.

Required:

- one declarative FaceField specification or generated constants/functions;
- generated C++ + GLSL, or a shared generated coefficient table;
- parity vectors stored as test data.

## PASS 3 — Shared Skin Material Library

### PASS

- one shared material family is used by Male/Female/Ada;
- no Ada/Male/Female-specific skin shader path;
- all current SkinGenome/MicroDetail channels enter SkinPhenotype;
- `SkinMaterialGpuV0` is bounded at 80 bytes;
- generated skin microdetail stored on disk remains 0 B;
- semantic BodyRegion no longer phase-shifts procedural skin noise;
- deterministic CPU cell hashing was aligned to the GLSL cell hash;
- Detail-on-Demand has Macro/Meso/Micro/MicroHigh bands.

### BLOCKERS

#### R8 — Duplicate legacy skin state remains in push constants

Human push constants still contain:

- `skin0[4]`;
- `micro0[4]`.

The renderer still fills them from `DerivedCharacterParameters`, while the material shader now reads `SkinMaterialGpuV0`. These fields are dead/duplicated material state and consume 32 bytes of the guaranteed 128-byte Vulkan push-constant budget.

Required:

- remove skin/micro values from `DerivedCharacterParameters` when no longer required elsewhere;
- remove the two push-constant vec4s;
- preserve one authoritative SkinPhenotype -> SkinMaterialGpu path.

#### R9 — Skin CPU/GLSL model still has two handwritten implementations

Base reflectance, roughness, detail thresholds, anatomical pore scaling and procedural feature formulas are mirrored manually between C++ and GLSL.

Required:

- generated/shared skin constants;
- parity vectors for pigment/roughness/feature thresholds;
- one source for DetailThresholds used by both CPU and shader build generation.

#### R10 — SkinMaterial UBO is single-active-character state

The current renderer updates one 80-byte material UBO for the selected character. Correct for the prototype viewer, but insufficient for drawing multiple independently configured humans in one frame.

Required:

- character-instance material buffer/ring;
- dynamic UBO/SSBO/descriptor-indexing strategy selected from target-device capability;
- no material state keyed only by current UI selection.

## Cross-pass ownership findings

#### R11 — EyeRuntimeState is profile-owned, not instance-owned

`std::array<EyeRuntimeState, kBuiltinProfileCount>` ties dynamic pupil adaptation to the profile. Two simultaneous instances of the same profile would share runtime adaptation conceptually.

Dynamic state must belong to a character instance, not an immutable profile.

The same ownership rule applies to future per-person:

- physiology;
- blinking;
- gaze;
- wetness;
- damage;
- animation state.

#### R12 — Dual profile source of truth remains

Checked-in JSON profiles and `builtin_profile()` both encode the same identities. The byte-parity CI gate is good protection but not the final architecture.

Pass 4 must load compiled profile data from the registry and remove built-in hard-coded identity values from runtime authority.

#### R13 — UI/shader still assumes exactly three profiles

The prototype selector contains three rows and `human.frag` still reduces profile display state with `% 3u`.

This is UI/demo code, not identity-core logic, but it will immediately contradict a data-driven registry.

Required:

- profile count supplied by UI model;
- selector entries generated from registry;
- shader must receive selected-state/index data without a literal 3-profile assumption.

## Memory / performance evidence

### Confirmed

- carrier duplication on disk: 0 B;
- generated skin texture storage: 0 B;
- current shared skin material state: 80 B;
- current profile genome: 138 B each;
- Android and Windows CI passed on the Pass 3 baseline.

### Not yet evidence

`estimated_gpu_bytes` is only an estimate. It is not a Vulkan memory-budget/residency measurement.

Before claiming production-scale memory efficiency, add:

- actual VkMemoryRequirements allocation accounting;
- optional `VK_EXT_memory_budget` telemetry where available;
- resident carrier count/bytes from the cache;
- peak residency;
- upload bytes/time;
- GPU timestamps for human passes;
- CPU update cost per visible character.

## Required order before Pass 4 feature expansion

### Foundation cleanup A

Remove dead skin/micro push-constant state and make the SkinMaterial path single-source.

### Foundation cleanup B

Split global frame lighting from per-character identity/material GPU state.

### Foundation cleanup C

Replace compile-time profile/carrier resolution with checked data-driven registry handles.

### Foundation cleanup D

Introduce lazy carrier cache + device-local static geometry upload and real residency telemetry.

### Foundation cleanup E

Move dynamic eye/physiology state to CharacterInstance ownership.

### Foundation cleanup F

Introduce carrier FaceField metadata and CPU/GLSL generated parity contracts.

Only after A–F should Pass 4 be considered structurally complete.

## Review verdict

Pass 1, Pass 2 and Pass 3 are **accepted as prototype foundations**.

They are **not yet accepted as scalable runtime architecture**.

No redesign of the core idea is required. The next work should consolidate the successful prototypes into:

`DataDrivenRegistry + CharacterInstance + CarrierCache + CharacterGpuState + generated CPU/GLSL contracts`

before adding large numbers of characters or further character-specific fidelity work.
