# Face DNA v0 — identity fitting contract

Status: STRUCTURAL_IMPLEMENTATION / ADA_VISUAL_FIT_CANDIDATE

Goal: represent face identity as compact deterministic Human DNA rather than a baked character mesh or texture.

## Schema

FaceGenomeV0 contains 20 signed normalized 16-bit controls:
skull_width, skull_height, face_length, forehead_height, brow_depth,
eye_spacing, eye_size, eye_tilt, nose_length, nose_width, nose_projection,
cheekbone_width, cheek_fullness, jaw_width, chin_width, chin_projection,
mouth_width, upper_lip_fullness, lower_lip_fullness, lip_projection.

Zero means neutral carrier geometry. Values are continuous, deterministic and versioned.

## Binary compatibility

Generator revision 3 serializes FaceGenomeV0 after GeometryGenome.
Revision 2 remains decodable as an explicit migration input; its missing Face DNA is initialized to zero.
New authoring compiles to revision 3.

## Ada reference fit

Character 2 uses the current single portrait as the primary visual target. The initial numbers encode a narrower oval/soft-heart facial silhouette, relatively prominent cheekbones, narrower jaw/chin, moderately large slightly upturned eyes, narrow nose, and fuller lips with the lower lip slightly fuller.

This is a one-view visual fit, not biometric reconstruction. It must not be promoted to ADA_CANONICAL until multi-view/device comparison validates frontal silhouette, 3/4 silhouette and side projection.

Hair, glasses, jewellery and clothing remain external character assets.

## Acceptance

- deterministic encode/decode and revision-2 migration
- no baked face texture requirement
- Face DNA affects only genome-mode face/head deformation
- raw mesh diagnostic remains untouched
- body continuity remains unaffected outside the head field
- Ada can be selected independently as Character 2
- device portrait comparison required before FACE_DNA_FIT PASS
