# DMC3 Gemma 26B — Pass 2 / Math Batch 001 (2026-11-02)

> **NON-AUTHORITATIVE / PRELIMINARY TRIAGE ONLY.** This is a 40-function research queue prepared from the Gemma notes. No live Ghidra/REA output was accessed in this review, no independent binary-level verification was carried out, and **zero new functions are marked verified**. Working names are suggestions, not replacements of the verified names ledger.

**Target:** vanilla DMC3 HD `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Inputs:** 40 hypotheses from `DMC3-Reverse-Gemma/Functions/math/` in the connected Google Drive.  
**Authority policy:** [DMC3 reverse data authority](../architecture/dmc3-reverse-data-authority.md); do not promote any function to product code or `verified_names.json` without machine-backed evidence and test receipts.

## Independent analytical flags

1. `0x14032D950`: Gemma named it `Math_DegreesToRadians`, yet its own note describes `acosf`, multiplication by a global constant, and integer conversion. This requires a corrected semantic name after the global constant and actual machine operations are resolved.
2. `0x140057BE0`: Gemma's discussion of `(global-t)*A+t*B` is self-contradictory. If `global==1.0f`, the expression *is* linear interpolation; verify the global constant and array bounds.
3. `0x1403302A0`: the note documents only components 0 and 3 in the multiply-add; don't declare ordinary vec3/vec4 dot product.
4. `0x140320D20`: the conditional fallback and global subtraction do not support a generic `SubInt` name.
5. `0x140312CB0`: zeroing translation before affine inversion may not produce the complete inverse view transform; identify exact matrix layout.

## 40-function queue

| # | VA | Gemma name | Working name (unverified) | Review track |
|---:|---|---|---|---|
| 1 | `0x1402F2880` | `ColorList_UpdateAllColors` | `ColorList_UpdateAllColors` | reclassify |
| 2 | `0x14032D950` | `Math_DegreesToRadians` | `Math_AcosScaleToInteger` | rename |
| 3 | `0x140330090` | `Math_Vector4_SubtractAndNormalize` | `Math_SubtractVectorsNormalize3` | signature |
| 4 | `0x140330110` | `Math_Vector3_CrossProductAndNormalize` | `Math_Vector3_CrossProductAndNormalize` | keep |
| 5 | `0x14032E660` | `Math_Matrix4x4_CreateOrthonormalBasis` | `Math_Matrix4x4_CreateOrthonormalBasis` | keep |
| 6 | `0x1403301D0` | `Math_Vector3_Normalize` | `Math_Vector3_Normalize` | keep |
| 7 | `0x1400C6020` | `Math_UpdateCameraBasis` | `Transform_UpdateInterpolatedBasis_0C6020` | rename |
| 8 | `0x1400BFCC0` | `Math_UpdateOrthonormalBasisFromCamera` | `Transform_UpdateInterpolatedBasis_0BFCC0` | rename |
| 9 | `0x14032F7B0` | `Math_ComputeOrthonormalBasis` | `Math_ComputeOrthonormalBasis` | keep |
| 10 | `0x1402C7490` | `Math_ComputeAngleFromVector` | `Math_ComputeAngleFromVector_2C7490` | rename |
| 11 | `0x140061130` | `Math_CalculateAngleFromVectors` | `Math_CalculateAngleFromVectors` | keep |
| 12 | `0x140244A60` | `Math_CalculateDistanceRatioAndProjection` | `Math_CalculatePlanarDistanceRatioAndUpdateProjection` | rename |
| 13 | `0x1401FA680` | `Math_IsAngleWithinRange_fa680` | `Actor_CheckTargetAngleRange_1FA680` | signature |
| 14 | `0x1401FAC60` | `Math_GetAngleFromVector` | `Actor_GetBearingToTarget_1FAC60` | rename |
| 15 | `0x1401A3B30` | `Math_UpdateObjectTransformHierarchy` | `Math_UpdateObjectTransformHierarchy` | keep |
| 16 | `0x140089750` | `Math_ColorList_UpdateFromOffset` | `ColorList_UpdatePackedColorsAt80` | reclassify |
| 17 | `0x140063660` | `Math_Vector3_ScaleAndAccumulate` | `Math_ScaleAndAccumulateVectors_063660` | signature |
| 18 | `0x140061170` | `Math_ClampToPositive_61170` | `Math_ClampComputedValueToRange_61170` | rename |
| 19 | `0x140061160` | `Math_ShortSub` | `Math_ShortSub` | keep |
| 20 | `0x140057BE0` | `Math_LerpFloat` | `Math_InterpolateAdjacentTableFloats` | formula |
| 21 | `0x14005E350` | `Math_CompareFloats` | `CompareObjectsByFloatAtOffset8` | rename |
| 22 | `0x140055AC0` | `Math_GetScaleFactor` | `Object_GetNestedFloatOrDefaultOne` | rename |
| 23 | `0x14004EE40` | `Unknown_IdentityReturn` | `ReturnThirdArgument_04EE40` | reclassify |
| 24 | `0x14004CAF0` | `Math_SetNegativeOneF` | `Math_SetNegativeOneF` | keep |
| 25 | `0x14033B5F0` | `Math_GeneratePseudoRandomSequence_3b5f0` | `PRNG_InitializeState_33B5F0` | rename |
| 26 | `0x140332140` | `Math_ClampInt` | `Math_ClampIntToByteRangeAndStore` | rename |
| 27 | `0x1403302A0` | `Math_DotProduct_Partial` | `Math_DotSelectedComponents0And3` | rename |
| 28 | `0x140325B90` | `Math_TransformMatrixSet` | `Transform_SetProjectionLikeParameters_325B90` | rename |
| 29 | `0x140325D90` | `Res_UpdateDemoTransformMatrix` | `Demo_UpdateProjectionTransform_325D90` | reclassify |
| 30 | `0x1403396D0` | `Math_Transform_SetRotationAndPosition` | `Transform_InitializeAxesAndPosition_3396D0` | signature |
| 31 | `0x140339750` | `Math_SetTransformRecord` | `Transform_CopyRecordToField50` | rename |
| 32 | `0x140333A40` | `Math_ClampToNonNegative` | `Math_ClampToNonNegative` | keep |
| 33 | `0x140320D20` | `Math_SubInt` | `Math_GetGlobalRelativeIntegerOrFallback` | rename |
| 34 | `0x140316830` | `Math_InterpolateColorTable` | `Math_InterpolateColorTable` | keep |
| 35 | `0x14030DDD0` | `Math_Vec3_Add` | `Math_Vec3_Add` | keep |
| 36 | `0x140309AD0` | `Array_DispatchByBoundsCheck` | `Array_DispatchByBoundsCheck` | reclassify |
| 37 | `0x140326B30` | `Math_SetVector4One` | `Math_SetFourFloatsToOneAtOffset0C` | rename |
| 38 | `0x140325220` | `Math_TransformHierarchyMatrices` | `Math_TransformHierarchyMatrices` | keep |
| 39 | `0x1403148C0` | `Math_TransformAndProjectPoints` | `Math_TransformAndProjectPoints` | signature |
| 40 | `0x140312CB0` | `Math_GetInverseViewTransform` | `Math_InvertGlobalTransformWithoutTranslation` | rename |

## Mandatory evidence gate

For each item, record: (a) precise function entry, code bytes, Ghidra decompile and instruction-level path; (b) callers, callees, referenced strings/globals and layout; (c) ABI/prototype inferred from usage; (d) case-based tests with edge behavior (NaNs, zero vectors, collinear bases, row/column conventions); (e) independently reviewed verdict, corrected name, confidence and unresolved gaps. Reconcile local `reviews/codex-review-s*-*.json`, `reviews/codex-deep-*.json`, and `data/verified_names.json` before labeling any item independently reviewed.

The executable and local Ghidra/REA corpus are not available to this repository PR review session. This pretriage is deliberately **not a replacement** for the established Codex 40-function Ghidra/REA process. No source code, Ghidra symbol, verified ledger, or binary is changed. Do not merge this queue as a proof of 40 recovered functions.
