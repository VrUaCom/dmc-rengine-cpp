# Pass 2 — Universal Identity Foundation

Status: **IMPLEMENTED / CI_REQUIRED**

This pass makes Face DNA a foundation feature rather than an Ada-specific path.

## Invariants

1. Every current profile is revision 3 with explicit Face DNA.
2. Male Base and Female Base use neutral Face DNA offsets; Ada uses non-zero fit values.
3. Face identity is independent of carrier allocation.
4. GeometryGenome legacy jaw/facial-softness slots are zero in current authoring.
5. CPU and GLSL expose the same canonical FaceField semantics.
6. Human face deformation and eye socket deformation consume the same Face DNA.
7. Runtime code contains no identity branch for Ada or profile 2.
8. JSON authoring and built-in profiles must encode byte-identically.
9. Raw diagnostic geometry bypasses genome deformation.
10. Shared carrier residency from Pass 1 is unchanged.

## Next gate

Pass 3 may begin only after Android + Windows CI PASS and storage/source-parity gates remain green.
