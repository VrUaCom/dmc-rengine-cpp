# GDSpaces L3 R1 — higher-level state-machine exclusion pass — 2026-08-26

## Authority

Canonical analysis executable:

- `dmc3.exe`
- size `6,356,432` bytes
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- preferred image base `0x140000000`

Primary layer: **[L3] Original Runtime / Resource Lifecycle**.

This pass continues R1 after direct-base, leaf/no-unwind and derived-record alias analysis. It targets a large false-positive family: objects near `0x1401C83xx..0x1401C98xx` whose own field `+0x04` uses small integer states that numerically overlap LoadedResource states.

The question is not whether those functions interact with LoadedResource. Many do. The question is whether their own `object+0x04` field **is** LoadedResource state.

## Candidate population

Within the bounded `0x1401C8300..0x1401C9900` range, whole-image disassembly contains:

- **31** exact immediate writes of `1..4` to `object+0x04`;
- distribution: state1 ×6, state2 ×12, state3 ×7, state4 ×6;
- at least **12** additional exact `object+0x04 = 5` writes in the same orchestration family.

The existence of state5 already distinguishes this object-state domain from canonical LoadedResource states `0..4`, but this pass also requires direct pointer provenance rather than relying on numeric range alone.

## Representative object/record separation

### Variant using `object+0x58`

The state machine beginning around `0x1401C839C` does both of the following:

```text
record = qword(object+0x58)
read record.state from [record+0x04]
```

and independently:

```text
object+0x04 = 1
object+0x04 = 2
object+0x04 = 3
object+0x04 = 4
```

Its release-side helper around `0x1401C84F6` loads:

```text
RDX = qword(object+0x58)
RCX = 0x140C99D30
call 0x1401B9530
object+0x58 = null
```

while separately updating the higher-level `object+0x04` state.

The same orchestration family also writes `object+0x04 = 5` around `0x1401C84AE/0x1401C84E4`.

Therefore `object+0x04` and `record+0x04` are distinct fields in distinct objects even though both use small integers.

### Variant using `object+0x68`

The state machine beginning around `0x1401C85BC` loads:

```text
record = qword(object+0x68)
read [record+0x04]
```

then independently advances its own `object+0x04` through `1/2/3/4`.

Its release path around `0x1401C8717` passes `object+0x68` as the record to `0x1401B9530`, clears `object+0x68`, and continues its own orchestration state machine. State5 is also present in the same family around `0x1401C86CF/0x1401C8705`.

### Variant using `object+0x60`

The state machine around `0x1401C962C` reads LoadedResource state through:

```text
record = qword(object+0x60)
read [record+0x04]
```

while writing its own `object+0x04 = 1/2/3/4`.

Release path around `0x1401C9787` passes `object+0x60` to `0x1401B9530`, clears that pointer and independently mutates `object+0x04`. State5 writes exist around `0x1401C973F/0x1401C9775`.

### Variant using `object+0x70`

The broader orchestration path around `0x1401C8FA0` reads canonical LoadedResource state from pointers stored at `object+0x70` and `object+0x40`, while its own `object+0x04` dispatches a separate multi-state machine.

The initializer around `0x1401C9830` makes the provenance especially explicit:

```text
RCX = 0x140C99D30
call 0x1401B8DF0
object+0x70 = returned group5 LoadedResource*
```

and separately initializes/updates:

```text
object+0x04 = 0
...
object+0x04 = 2
```

Other paths in the same higher-level family drive `object+0x04` through states 1,2,3,4,5.

## Exact bounded candidate list

The 31 exact immediate `1..4` writes in this range are:

```text
1C83D3=1  1C83F2=2  1C8440=3  1C8482=4
1C8556=2
1C85F3=1  1C8612=2  1C8660=3  1C86A3=4
1C8777=2
1C89A3=1  1C89C2=2  1C8A1E=3  1C8A61=4
1C8B35=2
1C8BED=1  1C8C0C=2  1C8C5A=3  1C8CA4=4
1C90AC=1  1C90CB=2  1C9154=3  1C91A0=3  1C91DE=4
1C949C=2
1C9663=1  1C9682=2  1C96D0=3  1C9713=4
1C97E7=2
1C98DD=2
```

Representative state5 writes in the same object-state family include:

```text
1C84AE 1C84E4
1C86CF 1C8705
1C8A8D 1C8AC3
1C8CD0 1C8D06
1C920F 1C9254
1C973F 1C9775
```

## Promotion

All 31 exact immediate `object+0x04 = 1..4` candidates in the bounded `0x1401C8300..0x1401C9900` family are **rejected as LoadedResource state writers**.

Evidence basis:

1. the same higher-level objects hold separate exact LoadedResource pointers at other fields;
2. those pointers are dereferenced to read canonical `record+0x04` state;
3. those pointers are passed to canonical LoadedResource release/acquisition helpers;
4. the higher-level object pointers are nulled independently from `object+0x04`;
5. the higher-level `+0x04` state domain includes state5, outside canonical LoadedResource `0..4`.

This is stronger than a naming or layout guess: the executable simultaneously manipulates both state domains as separate objects.

## Architecture implication

The numeric overlap `0/1/2/3/4` is not sufficient to identify LoadedResource lifecycle state.

The recovered runtime contains higher-level orchestration/load/task state machines that:

```text
own/observe LoadedResource* in another field
while
maintaining their own independent state field at +0x04
```

This reinforces the canonical architecture rule:

- LoadedResource record state is one L3 lifecycle authority;
- higher-level loader/orchestration state is a separate L3-adjacent mechanism;
- shared numeric state values must not be authority-laundered across object types.

No stronger semantic class name is promoted for the `0x1401C83xx..0x1401C98xx` object family by this pass.

## R1 effect

This removes the largest concentrated false-positive block from the exact-immediate state1/2/3/4 whole-image census.

Remaining exact/non-immediate work is now outside this rejected family and should be reviewed by exact destination provenance rather than by numeric state coincidence.

### Still open

1. remaining exact-immediate state1/2/3/4 candidates outside the canonical LoadedResource cluster and outside this `0x1401C83xx..0x1401C98xx` higher-level family;
2. non-immediate state values carried through registers/tables into proven record aliases;
3. indirect callback/function-pointer registrations;
4. record aliases outside the reviewed lifecycle/orchestration clusters;
5. final whole-image contradiction sweep.

## Completion claim

**R1 narrowed materially again; not complete. L3 is not complete.**