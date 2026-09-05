# DMC3 Resource Vertical Proof Reconciliation — 2026-09-05

**Canonical analysis executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Reviewed canonical implementation:** `main@ee08b388cbc5448a0e1a5d02231d9aaf7e01587d`  
**Purpose:** reconcile already recovered static authority into one exact L2 -> L1 -> L3 proof chain and isolate the smallest remaining proof gaps.

This document is a reconciliation of existing canonical reverse evidence plus the 2026-09-03 fresh `.index` whole-image pass. It does **not** claim a new raw-byte disassembly of the unresolved scheduler cluster because a raw canonical executable blob was not available in the connected file surface during this pass.

## 1. Static chain now supported

### A. Runtime bootstrap

`0x14002E930`:

```text
GetModuleFileNameA
 -> derive <exe-dir>\\data\\dmc3\\
 -> register physical root with flags 0x0C
 -> probe DMC3-0.nbz, DMC3-1.nbz, ...
 -> stop discovery at first missing filename
 -> call archive registration for every existing numbered filename
```

Strong bounded conclusions:

- numbered discovery begins at zero;
- first missing filename terminates discovery;
- discovery existence does not by itself prove successful mounting.

### B. Successful mount topology

`0x140326D20` physical registration and `0x140326DA0` archive registration both prepend successful nodes to global head `0x140CF3180`.

Clean successful archive order therefore becomes:

```text
DMC3-(N-1)
 -> ...
 -> DMC3-1
 -> DMC3-0
 -> physical root
```

A discovered archive whose archive registration fails is absent from the linked topology. It is not a clean lookup miss.

### C. Request construction and provider phases

`OpenGameResource 0x14002FCA0` has three recovered direct callers:

- `0x14003340A`;
- `0x1403380C7`;
- `0x1403381F7`.

Each loads `EDX=1` immediately before the call.

Prefix table `0x14055AEF8`:

1. `GDataX360.afs/`
2. `GData.afs/`
3. `Video/`
4. `afs/sound/`
5. `SAVEDATA/`
6. empty prefix

Recovered direct-call policy:

```text
for each of six candidates:
    ResourceMountResolve(candidate, provider_mask=1)  // archives only

if none selected:
    for each of six candidates:
        ResourceMountResolve(candidate, provider_mask=2)  // physical only
```

### D. Normalization

Normalizer `0x140327160`:

```text
0x01 ASCII a-z -> A-Z
0x02 ASCII A-Z -> a-z
0x04 strip leading separators
0x08 strip trailing separators
always '/' -> '\\'
always collapse repeated '\\'
```

Therefore:

```text
archive = 0x0E = lowercase + trim ends + canonical separators
physical = 0x0C = preserve case + trim ends + canonical separators
```

### E. Archive selection has two success gates

```text
0x140328160  normalized archive lookup -> entry pointer
0x140328290  wrapper/open construction -> usable stream
```

If lookup finds an entry but wrapper/open construction fails, `0x140327430` returns through failure cleanup; it does not reinterpret the event as a lower-volume clean miss.

### F. Retail `dmc3-0.nbz` collision receipt

Bound artifact result:

```text
files-only          4333 keys / 4333 unique / 0 collisions
all central entries 4334 keys / 4334 unique / 0 collisions
```

This removes normalized-key ambiguity for that exact archive only.

### G. Original representation choice after resource acquisition

Fresh 2026-09-03 whole-image pass recovered `0x1401B79E0`:

```text
if packed path exists:
    mode = 1
else if same path rewritten to .lst exists:
    mode = 2
else:
    mode = 0
```

`0x1401B8CA0`:

```text
container-kind:
    mode 0 -> acquisition failure
    mode 1 -> 0x1402EF4D0 packed whole-file acquisition
    mode 2 -> 0x1401B85C0 .lst in-memory synthesis

non-container:
    -> 0x1402EF4D0 packed/direct whole-file acquisition
```

No `.index` resolver/materialization branch was recovered. `.index` remains extraction/naming metadata, not runtime materialization authority on this recovered path.

### H. L1/L3 acquisition seam

`0x1401B84E0`:

```text
record +0x18 <- descriptor/type authority
 -> prepare +0x28 backing
 -> record +0x20 <- loaded/materialized payload handle/pointer
 -> 0x1401B8CA0 materialization dispatcher
 -> only on success: record state <- 1
 -> schedule normal completion callback 0x1401B8DC0
```

This closes the bounded statement:

> LoadedResource state1 is not published before materialization-dispatch success.

### I. Normal completion and ready visibility

`0x1401B8DC0` normal callback:

```text
state 1 -> state 2
```

`0x1401B92D0` state2 finalizer:

```text
loaded payload
 -> PAC: visit non-zero children and dispatch each
 -> otherwise dispatch root
 -> optional callback
 -> state = 3
```

Typed dispatcher `0x1401B9FA0` bounded paths:

- MOD -> `0x1402FE3B0`;
- EFM -> `0x1402F7A90`;
- SCM -> `0x1403051B0`;
- SHW -> `0x1403204C0`;
- PNST -> recursive non-zero child traversal;
- unrecognized/default -> return/no-op at this central boundary.

Therefore the static vertical can currently be stated as:

```text
logical request
 -> OpenGameResource candidate policy
 -> mounted provider traversal
 -> selected archive/physical stream
 -> packed/.lst representation choice
 -> materialization dispatcher success
 -> LoadedResource state1
 -> normal completion state2
 -> typed post-load
 -> optional callback
 -> state3 ready visibility
```

## 2. What this static chain still does not prove

The following are separate and still open:

1. which provider/volume/member a **real protected process** selected for a representative request;
2. whether every discovered retail numbered archive successfully mounted in that process;
3. protected-build RVA mapping for the recovered canonical anchors;
4. exact runtime bytes tied to the selected entry and independent materialized SHA;
5. exact terminal scheduler dependency between lower I/O completion and normal `0x1401B8DC0` dispatch;
6. deterministic consumer-visible effect of one selected resource;
7. original selection/consumption of a DMC Rengine-authored higher-numbered overlay;
8. rollback proof.

## 3. Highest-priority unresolved static reverse

The narrow open materialization dependency remains:

> what terminal condition keeps/releases the materialization scheduler work and prevents normal state2 publication after failed or incomplete transport?

Fresh raw canonical-byte targets:

1. `0x1402EF4D0` — queued materialization job identity/type and load-context consumer;
2. `0x1402EF790` — corresponding dispatch case and persistence/re-poll/retirement behavior;
3. `0x1400333E0` — pending/success/error status domain;
4. `0x140033390` — terminal cleanup/release point;
5. `0x1400335A0` — lower transport completion/status writer;
6. `0x1402EF460` — queued higher-work clear/rollback;
7. regression anchor `0x1401B8DC0` — normal state2 publication.

Do not promote historical helper labels for `0x1400333E0` or `0x140033390` without fresh canonical bytes.

## 4. Highest-priority product correction

Static reverse already proves:

```text
filename discovery / registration attempt
!=
successful linked mount topology
```

Current main must represent those domains separately. The historical #246 implementation is evidence-aligned but stale/diverged; do not merge it wholesale. Semantically port only the correction onto current architecture and preserve current-main capabilities.

Required product acceptance for that port:

- discovery plan contains discovered existing numbered filenames only;
- successful topology contains explicitly successful physical/archive registrations only;
- sparse successful archive topology is representable;
- resolver traverses only successful topology;
- discovered-but-failed archive is not converted into a clean lookup miss;
- higher successful archives preserve prepend-derived precedence;
- exact-head Ubuntu + Windows CI green.

## 5. Required original-process proof packet

The first useful runtime receipt should bind one representative request and contain:

```text
protected executable SHA/size
process id + module base
mapped approved resolver anchors
exact requested logical name
observed successful mount topology
observed provider/volume probe order
selected provider + numbered volume + member identity
selected archive SHA/size
selected central entry metadata
independent materialized SHA/size
trace completeness + dropped-event count
observer artifact SHA
```

A later vertical receipt extends the same identity with:

```text
LoadedResource record identity
state1 observation
state2 observation
typed post-load family/child identity
state3 observation
deterministic consumer-visible effect
```

## 6. Authored overlay proof

After the vanilla vertical is accepted, repeat with a DMC Rengine-generated next-contiguous `DMC3-N.nbz` containing one bounded edited member.

Required result:

```text
original resolver selects authored higher-numbered volume
 -> exact authored member bytes are materialized
 -> typed post-load reaches the expected resource
 -> state3/consumer path is reached
 -> deterministic intended effect is observed
 -> test overlay is removed
 -> original retail archive hashes remain unchanged
```

Only this can promote the representative L1 Level-E acceptance claim.

## 7. Current decision

The static request->ready spine is strong enough that another broad whole-EXE reverse is not justified. Work should stay narrow:

1. semantic mount-topology product correction;
2. fresh raw pass only on the scheduler terminal cluster when canonical bytes are available;
3. protected-process R2B/R3 instrumentation;
4. same-lineage vanilla vertical receipt;
5. authored-overlay vertical receipt;
6. independent final audits.
