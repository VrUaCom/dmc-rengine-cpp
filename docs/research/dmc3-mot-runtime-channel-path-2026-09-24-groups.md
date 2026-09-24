# DMC3 MOT binding across motion groups (domain shorter than the model)

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Sample (analysed only): em000.pac, SHA-256
`10ab4cd0cc83abe4ca4b5ee98e9cc97f953db6f86dac7fddcb4aed540ba1a87c`.

Extends `dmc3-mot-runtime-channel-path-2026-09-11.md`. That note codified the
normal binding `0x140310A61` as requiring MOT channel-domain count = model
node count.

## 1. What the loop does — EXE_CONFIRMED

```text
for joint j in 0 .. CMotion+0x20 (joints, array +0x28):   ; every joint
    mask = u16[MOT + 0x1E + 2·j]; advance the mask pointer
    if joint.group(+0xF8) != evaluated group:              ; 0x140310A97
        count the set channel bits, skip that many tracks   ; 0x140310C5C
        continue
    bind the set channels to +0x120..+0x220 in table order
```

- One mask is consumed for every joint, whatever its group.
- Joints of other groups consume their tracks without binding them.

## 2. em000

The CEm000-family init (`0x140097C05` / `0x140097C8D`) takes:
- the body from slot 1 (23 nodes; variants 2 and 3 use slot 5);
- motion PAC group 0 from slot 35, whose MOTs have a channel domain of 22.

The body's motion groups, by node:

```text
0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 1 2
```

Nodes 0–21 use groups 0 and 1, which are the ones the MOT drives. Node 22 is
the only node of group 2. The group-0/1 evaluators therefore bind the 22
covered joints and skip joint 22. Joint 22 would read one mask past the
table, but its group differs, so nothing from that read is bound. CEm004
(slot 8) has the same 22 nodes without the extra one.

## 3. Rule

A MOT with channel domain *d* smaller than the model node count *n* drives
the model when:
- every node at index *d* or above belongs to a motion group;
- no node below *d* uses that group.

The trailing nodes keep their rest locals. When *d* > *n*, or when a trailing
node shares a group with a covered node, the MOT is still rejected.

Native Reader implements this rule in `MotionClip::bind` and
`motion::motion_can_drive`.
