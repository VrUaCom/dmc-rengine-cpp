# Face DNA v0 — universal identity foundation

Status: **STRUCTURAL_IMPLEMENTATION / UNIVERSAL_RUNTIME**

Goal: represent facial identity as compact deterministic Human DNA shared by every character profile.

## Ownership

Face identity belongs to `FaceGenomeV0`. `GeometryGenome` owns macro body and global head-to-body proportion only.

The old `geometry.jaw` and `geometry.facial_softness` binary slots are retained solely for revision-2 migration. Current revision-3 authoring requires both to be zero. Jaw, chin, cheek and soft-tissue identity are controlled by Face DNA.

## Schema

FaceGenomeV0 contains 20 signed normalized 16-bit controls:
skull_width, skull_height, face_length, forehead_height, brow_depth,
eye_spacing, eye_size, eye_tilt, nose_length, nose_width, nose_projection,
cheekbone_width, cheek_fullness, jaw_width, chin_width, chin_projection,
mouth_width, upper_lip_fullness, lower_lip_fullness, lip_projection.

Zero is the neutral identity offset for a carrier. Parameters are continuous, deterministic and character-agnostic.

## Canonical FaceField

The runtime evaluates Face DNA in a shared head-local metric frame derived from the canonical LSG carrier coordinate system. CPU reference implementation and GLSL implementation use the same region definitions and deformation coefficients.

The same FaceField drives:
- head/face surface deformation;
- eye socket placement, size and tilt;
- Male Base;
- Female Base;
- Ada;
- all future profiles using compatible carriers.

No FaceField function may branch on profile number, name, Ada, sex, or carrier identity.

## Binary compatibility

Generator revision 3 serializes FaceGenomeV0 after GeometryGenome.
Revision 2 remains decodable as an explicit migration input; missing Face DNA becomes neutral zeros.
All checked-in authoring profiles are revision 3 and contain an explicit Face DNA block.

## Source parity

JSON authoring profiles and built-in runtime profiles are temporarily dual representations. CI compares the exact encoded LSG0 bytes and fails if they diverge. A later data-driven profile-loading pass may remove the built-in representation entirely.

## Ada reference fit

Ada is a normal female-carrier profile with non-zero Face DNA. Her current values are a visual-fit candidate from the available reference image, not a biometric reconstruction.

Promotion remains:

`ADA_REFERENCE -> VISUAL_FIT_CANDIDATE -> DNA_FIT -> DEVICE_VISUAL_PASS -> ADA_CANONICAL`

Hair, glasses, jewellery and clothing remain external character assets.

## Acceptance

- Male Base, Female Base and Ada use generator revision 3;
- explicit Face DNA exists for all profiles;
- one character-agnostic FaceField implementation drives face and eye sockets;
- current authoring rejects legacy facial ownership in GeometryGenome;
- raw mesh diagnostic path remains untouched;
- source parity gate catches JSON/runtime drift;
- no baked face texture requirement;
- Android and Windows CI remain green.
