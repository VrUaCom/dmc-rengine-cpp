# GDSpaces Layer 2 — raw EXE R3 identity / archive-selection pass — 2026-08-26

**Primary layer:** L2 — Resource Resolution.  
**Cross-layer seam:** L2 selected archive identity -> L1 archive materialization.  
**Canonical instruction authority:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Preferred image base:** `0x140000000`.  
**Review base:** `main@2ed43b438f1bf01638f3e56341e98f6085e5b0fd`.  
**Status:** RAW-EXE CONFIRMED / CORRECTS PART OF THE EARLIER R3 CHECKPOINT / L2 NOT COMPLETE.

This pass was performed from the canonical raw executable, not from the existing L2 documentation. It narrows the R3 observation contract and corrects one over-broad statement introduced during the #221 review. It does not promote any protected-process observation or retail-corpus result.

## 1. Whole-image resolver caller and mount-topology census

### 1.1 `OpenGameResource`

Canonical function: VA `0x14002FCA0`, RVA `0x2FCA0`.

The whole-image direct-call census reproduces exactly three direct callers:

- `0x14003340A`;
- `0x1403380C7`;
- `0x1403381F7`.

All three set `EDX = 1` immediately before the call. No additional direct call/jump to `OpenGameResource` was found in the full `.text` disassembly. A raw search for the absolute function address did not expose a callable absolute-pointer table; other numeric occurrences are consistent with metadata rather than an additional recovered dispatch surface.

**Promotion:** the existing `flags = 1` claim remains confirmed for the recovered direct-call surface.

### 1.2 Mount registration and list mutation

The recovered mount-list head is at `0x140CF3180`.

Direct static writes to this head are confined to the two registration paths:

- type-0 / physical registration `0x140326D20`;
- type-1 / archive registration `0x140326DA0`.

Each registration routine prepends its new node to the list. Each has one observed direct bootstrap caller in the canonical image.

Bootstrap performs the physical registration first and then registers contiguous numbered archives in increasing filename order:

```text
DMC3-0.nbz
DMC3-1.nbz
...
DMC3-(N-1).nbz
stop at first missing volume N
```

Because each archive is prepended, the resulting recovered traversal topology is:

```text
DMC3-(N-1) -> ... -> DMC3-1 -> DMC3-0 -> physical-root
```

No later direct write to the recovered mount-list head was found in the whole-image static census.

**Bounded claim:** this confirms the static canonical bootstrap/list topology for the recovered direct surface. It is not a proof against every theoretically possible computed memory write outside the recovered ownership path.

## 2. Path normalization is now bit-exact

`ResourcePathNormalize` at `0x140327160` implements:

- bit `0x01`: ASCII lowercase-to-uppercase;
- bit `0x02`: ASCII uppercase-to-lowercase, used only when uppercase mode is not active;
- bit `0x04`: strip all leading `/` and `\\` separators;
- bit `0x08`: strip all trailing `/` and `\\` separators;
- convert `/` to `\\`;
- collapse repeated separators.

Therefore the recovered DMC3 profiles are:

```text
physical: 0x0C = strip-leading | strip-trailing
archive:  0x0E = lowercase | strip-leading | strip-trailing
```

The current product `ResourcePathNormalizer` already implements this transformation order and flag meaning. No correction is required in the product normalizer from this pass.

## 3. Archive index construction and lookup

### 3.1 Parsed central-entry node

The archive central-directory parser around `0x1403289F0` allocates one `0x50`-byte entry node. The recovered layout is:

```text
+0x00 .. +0x2F  decoded fixed ZIP central-directory metadata
+0x30           filename heap pointer
+0x38           extra-field heap pointer
+0x40           comment heap pointer
+0x48           next entry pointer
```

The fixed-header decoder `0x140327E40` reads the complete 46-byte central-directory fixed record and stores its decoded fields into the first `0x30` bytes. It verifies central signature `0x02014B50`.

Important recovered fields include:

```text
entry +0x10  CRC32
entry +0x14  compressed size
entry +0x18  uncompressed size
entry +0x1C  filename length
entry +0x1E  extra length
entry +0x20  comment length
entry +0x28  external attributes
entry +0x2C  relative local-header offset
```

### 3.2 The runtime destroys original filename spelling in this node

Archive index construction at `0x140327CC0` traverses the parsed entry list and calls:

```text
ResourcePathNormalize(entry->filename, 0x0E)
```

on the `+0x30` filename buffer itself.

The normalized filename therefore replaces the original central-directory spelling in the recovered runtime entry node. In the bounded parser/index/wrapper path reviewed here, no second original-filename string field was found. Entry cleanup later independently frees only `+0x30`, `+0x38`, and `+0x40`, consistent with one filename allocation.

**Correction to the R3 data model:** the original central-directory pathname must not be required as a directly observed runtime text field unless separate runtime evidence proves another preserved copy. The current #221 candidate field `archive_member_path` is too strong if interpreted as original-spelling runtime authority.

## 4. `0x0E` collision semantics: no recovered secondary tie-break

Archive index construction creates an array of 16-byte lookup records:

```text
{ normalized_filename_pointer, central_entry_pointer }
```

and sorts it through CRT `qsort`. The comparator at `0x1403291D0` compares only the normalized filename strings, bytewise like `strcmp`.

Archive lookup at `0x140328160`:

1. bounded-copies the requested candidate;
2. normalizes it with `0x0E`;
3. calls CRT `bsearch` on that sorted array;
4. returns the central-entry pointer from the matched lookup record.

The same normalized-string comparator is used. No original pathname, central offset, local-header offset, insertion index, or other secondary field participates in the recovered comparison.

### Consequence

If two distinct retail central-directory entries collide to the same `0x0E` normalized key, this EXE path provides **no recovered semantic secondary tie-break rule** for choosing between them.

Do not encode a new deterministic GDSpaces tie-break as original behavior merely to make collisions convenient. The exact retail `0x0E` collision census is therefore a real correctness gate, not an optional confidence check.

If a collision exists, final parity requires direct evidence of the actually returned central-entry identity or a narrower declared product behavior. A library/CRT duplicate-key implementation detail must not be promoted as a game semantic unless independently demonstrated.

## 5. Selected archive identity is structural, not pathname-only

### 5.1 Lookup returns the exact central-entry node

On hit, `0x140328160` returns the exact central-entry pointer stored in the lookup array.

`ResourceMountResolve` archive branch at `0x140327490..0x1403274D9` then:

1. checks provider type/mask;
2. calls archive lookup `0x140328160`;
3. receives one exact central-entry pointer;
4. passes the mounted archive pathname plus that central-entry pointer to `0x140328290`;
5. stores the returned wrapper into the resolver result.

### 5.2 Wrapper copies the fixed central identity

`0x140328290` allocates `0x50` bytes and copies the selected entry's first `0x30` bytes into `wrapper +0x08`.

This preserves the fixed central metadata, including:

```text
wrapper +0x18  CRC32
wrapper +0x1C  compressed size
wrapper +0x20  uncompressed size
wrapper +0x34  relative local-header offset
```

The wrapper does **not** copy the runtime entry filename pointer.

The relative local-header offset is subsequently consumed by archive stream setup; therefore it is a direct L2 -> L1 structural bridge to the selected member bytes.

### Required R3 archive identity

A stronger selected archive identity is:

```text
exact numbered volume
+ normalized provider key
+ relative local-header offset
+ fixed central-entry metadata sufficient to reject ambiguous/mismatched entries
```

The artifact-backed binder should then parse the exact supplied NBZ and recover the original central-directory pathname from the matching central record. Runtime pathname spelling must not be invented or treated as preserved when the EXE normalizes the only recovered filename allocation in-place.

For fail-closed behavior, the binder should require exactly one matching central entry under the captured structural identity. If the structural key itself is ambiguous, the receipt is not sufficient for promotion.

## 6. Correction: `0x140328290` is not an archive backend open

The earlier `l2-exe-reconciliation-2026-08-26.md` described the post-lookup failure too broadly as a wrapper/open/backend failure.

Fresh import resolution and raw instructions narrow it:

`0x140328290` performs only:

1. allocation of the `0x50` wrapper;
2. copy of the selected fixed central-entry metadata;
3. CRT `_strdup` of the mounted archive path;
4. return of the wrapper or null.

The import used at `0x14034F610` is `_strdup` from the CRT string import surface.

Therefore a lookup hit followed by failure at `0x140328290` means **wrapper allocation or archive-path duplication failure**, not NBZ file-open/materialization failure.

The already-confirmed control-flow consequence remains:

- if archive lookup cleanly misses, traversal can continue;
- if lookup hits but wrapper construction returns null, `ResourceMountResolve` exits through null/cleanup;
- this state must not be laundered into a lower-volume clean miss.

## 7. Archive backend open is lazy and belongs downstream of L2 selection

The selected archive wrapper initially contains the duplicated archive path and central metadata but no opened archive handle.

At `0x140328540`, the later stream/materialization path:

1. sees the stored archive path;
2. calls `0x140327800` (`CreateFileA` final-open helper) on that path;
3. frees the duplicated path;
4. initializes the archive stream through `0x140328360`;
5. seeks using `wrapper +0x34`, the selected member's relative local-header offset;
6. validates the local ZIP header before member reading/decompression continues.

The helper at `0x1400629C0` simply returns `DWORD [wrapper+0x20]`, showing that archive uncompressed size is already available from copied central metadata before archive file open.

### Provider asymmetry

Physical provider:

```text
L2 resolver path -> construct normalized rooted path -> CreateFileA -> selected physical handle
```

Archive provider:

```text
L2 resolver path -> normalized index lookup -> selected central-entry wrapper
later L1/materialization -> CreateFileA archive -> seek local header -> read/decompress member
```

Therefore a global R3 rule such as "selected means backend successfully opened" is incorrect.

**Correct bounded R3 meaning:** selected means the original resolver accepted a provider identity and returned its provider result object/wrapper. Archive materialization/open/read success is a separate downstream L1/L3 validation question.

## 8. R2B mapping coverage is insufficient for a trusted R3 observer

Current #219 R2B tooling was intentionally bounded to `0x40` windows such as:

- `OpenGameResource` RVA `0x2FCA0`;
- `ResourceMountResolve` entry RVA `0x327430`;
- type-0 registration/final-open anchors.

A `0x40` window beginning at `0x327430` ends before the actual archive selection call region.

The canonical archive observation region is:

```text
RVA 0x327490 .. approximately 0x3274CD
```

It contains:

```text
provider type-1 test
archive-mask gate
call 0x328160 lookup
exact entry-pointer hit/miss result
archive-path load from mount node
call 0x328290 wrapper creation
terminal wrapper-null vs selected-wrapper branch
```

A single `0x40` canonical window beginning at RVA `0x327490` covers this bounded observation site.

Likewise the physical selected/open observation site is around RVA `0x327530..0x327565`; it should receive its own bounded protected-process mapping rather than inheriting address authority from a different matched range.

### Promotion rule

A trusted publisher must not install a breakpoint/hook at a canonical-analysis address merely because another range in the same function matched the protected build.

Before protected R3 instrumentation, the exact observation sites used by the publisher must themselves be validated by bounded canonical-window equality in the same protected process instance.

This requirement is additive to #229 process-instance binding:

```text
#229: same real Windows process instance
+ observation-site mapping: exact code region authority
= acceptable address provenance for trusted R3 instrumentation
```

Neither substitutes for the other.

## 9. Updated R3 acquisition design

For one real protected-process selected archive receipt, the minimum evidence chain should now be:

```text
1. process-instance-bound R2B mapping
2. exact mapped OpenGameResource observation site
3. exact mapped archive-selection observation site
4. zero-loss request/provider event capture
5. capture selected numbered archive volume
6. capture normalized provider key
7. capture selected central fixed identity, including local_header_offset
8. artifact-bind every mounted numbered NBZ
9. parse exact selected NBZ in the binder
10. recover and verify the original central-directory pathname offline
11. compare product selection without relabeling product evidence as original evidence
```

For physical selection, capture the exact mapped physical call region and the mounted-root-relative identity while keeping private absolute workstation paths out of public evidence.

## 10. Corrections to existing status

### Still confirmed

- exactly three observed direct `OpenGameResource` callers, all `flags=1`;
- six-prefix direct-call policy;
- archive-before-physical traversal under that mode;
- physical `0x0C` normalization;
- archive `0x0E` normalization;
- highest-contiguous-numbered-volume precedence from prepend topology;
- physical `CreateFileA` contract from #215;
- archive lookup-hit + wrapper-construction failure is terminal null, not a lower-volume clean miss.

### Corrected

- `0x140328290` is **wrapper allocation + `_strdup`**, not archive backend open/materialization;
- archive original pathname spelling is not retained in the recovered selected wrapper and should not be claimed directly from that runtime text path;
- archive selected identity should be bound structurally through central metadata/local-header offset and then resolved against the exact NBZ artifact;
- trusted R3 breakpoints require exact observation-site mapping, not function-entry mapping by extrapolation.

### Still open

- real protected process-instance-bound R2B packet;
- protected-build exact mapping for R3 observation sites;
- trusted zero-loss R3 publisher execution;
- exact retail DMC3 `0x0E` collision census;
- real selected-entry structural receipt;
- final product/original comparison and L2 promotion.

## 11. Non-claims

This pass does not prove:

- global canonical/protected executable equivalence;
- any unlisted protected-process RVA;
- that the retail DMC3 corpus is collision-free under `0x0E`;
- which equal-key central entry `bsearch` would return if an actual collision exists;
- a real original-process selected provider;
- archive materialization success from L2 selection alone;
- Layer 1 or Layer 3 completion.

## 12. Next implementation order

1. keep #220 open until its original real-evidence acceptance is genuinely satisfied;
2. complete #229 process-instance identity hardening;
3. add exact R2B v2 observation-site anchors for archive and physical R3 capture;
4. revise the #221 archive selected-identity candidate from pathname-authority to structural central-entry identity;
5. make the artifact binder recover/verify the original central pathname from the exact NBZ;
6. run the real retail `0x0E` collision census;
7. implement/execute trusted zero-loss publisher only on independently mapped observation sites;
8. reconcile final L2 evidence before promotion.
