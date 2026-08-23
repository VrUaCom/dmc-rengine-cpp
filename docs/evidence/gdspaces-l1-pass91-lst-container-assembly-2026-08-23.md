# GDSpaces Layer 1 — Pass 91 — `.lst` runtime container assembly

Date: 2026-08-23

Status: evidence checkpoint; Layer 1 remains **NOT COMPLETE**.

## Scope

Pass 90 separated two materialization modes for `kind16 == 0` resources:

1. exact resource exists → direct VFS/AsyncIO materialization;
2. exact resource absent, same-stem `.lst` exists → runtime container assembly.

Pass 91 isolates and reverses the second path. It does **not** claim that `.lst` is the retail storage representation, and it does not promote a production writer.

## Main functions

```text
0x1401B79E0  exact-resource / .lst source classifier
0x1401B7C70  .lst container-magic decoder
0x1401B7D10  manifest entry counter
0x1401B7E60  indexed manifest-entry/path resolver
0x1401B7FD0  recursive assembled-size calculator
0x1401B85C0  recursive runtime container assembler
0x1402EF620  VFS size probe, sector-padded result
0x1402EF920  synchronous VFS read into caller buffer
0x1402EF4D0  queued direct child read into selected destination
```

Fresh instruction re-dumps use the preserved Phase17 probe derivative and remain subject to the raw-canonical-byte promotion gate documented in Pass 90.

## 1. `.lst` is line-oriented manifest input

The runtime loads the `.lst` text through the same logical-resource VFS layer used by other resources.

The parser treats lines in three broad classes:

- resource entry line;
- control/directive line beginning with `#`;
- ignored/comment/empty line beginning with `/` or CR.

`0x1401B7D10` counts valid resource-entry lines. `0x1401B7E60` resolves the Nth valid resource line and joins it with the directory derived from the parent resource path.

The join operation uses the recovered format:

```text
%s%s
```

so manifest entries are resolved relative to the parent resource directory.

## 2. Magic directive and default identity

`0x1401B7C70` starts by zeroing the 4-byte output magic.

If no explicit directive is recovered, it writes:

```text
50 41 43 00
P  A  C \0
```

Therefore the default runtime-assembled container identity is `PAC\0`.

When a line begins with `#`, the parser captures up to four following bytes into the 4-byte magic field. The parser therefore supports a manifest-selected 4-byte container identity.

This is important for PNST-class resources: the mechanism is capable of producing a non-PAC 4-byte identity rather than hardcoding `PAC\0` for every `.lst` assembly.

### Promotion limit

The code accepts a four-byte directive syntactically. Pass 91 does not claim that every arbitrary four-byte value is semantically supported by downstream consumers. Known container identities remain evidence-scoped.

## 3. Synthesized header geometry

For `N` valid manifest entries, the builder reserves:

```text
4 bytes  magic
4 bytes  slotCount
N * 4    relative offset table
```

The raw header byte requirement is therefore:

```text
8 + 4*N
```

The runtime rounds the header allocation upward to a `0x40` boundary:

```text
headerSpan = align_up(8 + 4*N, 0x40)
```

Before writing payloads, `0x1401B85C0` constructs a temporary header containing:

```text
+0x00  magic[4]
+0x04  uint32 slotCount
+0x08  uint32 slotOffset[slotCount]
```

and copies the completed aligned header into the destination container.

This matches the same core relative-slot geometry observed by the PAC/PNST consumer:

```text
slot i -> containerBase + offset[i]
```

## 4. Sparse physical slots through `dummy`

Manifest entries equal to the literal token:

```text
dummy
```

are not assigned payload storage. Their physical table entry is written as:

```text
offset[i] = 0
```

Therefore `.lst` assembly explicitly preserves sparse physical slot identity rather than compacting non-empty children.

This is an important Layer-1 rule: empty slots are first-class container geometry.

## 5. Offset construction

The builder performs a sizing pass before population.

For each non-dummy slot:

1. current cumulative destination offset is written into the slot's offset-table entry;
2. the child's required span is determined;
3. cumulative offset advances by the child's aligned allocation span.

Conceptually:

```text
cursor = align_up(8 + 4*N, 0x40)

for each physical slot i:
    if entry[i] == "dummy":
        offset[i] = 0
        continue

    offset[i] = cursor
    cursor += allocation_span(entry[i])
```

The runtime does not renumber non-empty entries.

## 6. Direct child resource sizing

For a normal child resource, `0x1402EF620`:

1. opens the logical child through `0x1400333F0`;
2. obtains `ceil(logicalSize / 0x800)` through `0x1400333C0`;
3. returns:

```text
sectorCount * 0x800
```

The `.lst` sizing layer then applies an additional `0x40` align-up, which is a no-op for a direct child already rounded to an `0x800` sector.

Therefore direct child regions in this runtime assembly path reserve sector-padded storage, not merely compact raw child length.

### Important separation

This runtime `.lst` layout policy must not be silently promoted as the canonical compact retail PAC/PNST packing policy. Preserved retail-like PAC/PNST samples show tighter offset geometry. The two representations belong to separate provenance/layout domains until direct retail receipts close the gap.

## 7. Nested `.lst` recursion and sibling `.pac` preference

If a manifest child itself has extension `.lst`, the builder derives the same-stem `.pac` path.

The recovered decision is:

```text
child X.lst
    |
    +-- if X.pac exists -> use/read X.pac directly
    |
    +-- else             -> recursively size/assemble X.lst
```

Size calculation uses `0x1401B7FD0` recursively; population uses `0x1401B85C0` recursively.

This establishes a runtime preference for an already-materialized container over reconstructing the same subtree from a manifest.

## 8. Population pass

After the header and offsets are finalized, the assembler walks the manifest again.

For each physical slot:

- `dummy` → no read, zero offset remains;
- ordinary child → queue direct VFS read to `destinationBase + offset[i]` via `0x1402EF4D0`;
- nested `.lst` with sibling `.pac` → queue the sibling `.pac` directly;
- nested `.lst` without sibling `.pac` → recurse into `0x1401B85C0` at `destinationBase + offset[i]`.

Therefore the parent container is materialized **in place** in one destination allocation.

## 9. Recovered assembly model

The strongest supported pseudocode is:

```text
assemble_lst(parentLogicalPath, destination):
    lstPath = replace_extension(parentLogicalPath, ".lst")
    baseDir = directory_of(parentLogicalPath)
    text = vfs_read(lstPath)

    magic = decode_magic(text)        // default PAC\0, or #xxxx directive
    entries = parse_resource_lines(text)

    headerSpan = align_up(8 + 4*len(entries), 0x40)
    cursor = headerSpan

    for i, entry in physical order:
        if entry == "dummy":
            offsets[i] = 0
            continue

        offsets[i] = cursor

        if extension(entry) == ".lst":
            siblingPac = replace_extension(entry, ".pac")
            if vfs_exists(siblingPac):
                childSpan = sector_padded_size(siblingPac)
            else:
                childSpan = recursive_lst_size(entry)
        else:
            childSpan = sector_padded_size(entry)

        cursor += align_up(childSpan, 0x40)

    write magic, slotCount, offsets[] into aligned header

    for i, entry in physical order:
        if offsets[i] == 0:
            continue

        if extension(entry) == ".lst" and no sibling .pac:
            assemble_lst(entry, destination + offsets[i])
        else:
            queue_vfs_read(resolved_child_path, destination + offsets[i])
```

## 10. What this does and does not explain

### It explains

- how a missing exact PAC/PNST-class resource can be reconstructed at runtime from a manifest;
- how physical slot order and zero slots are preserved;
- how relative offsets are synthesized;
- how nested container trees can be recursively materialized;
- how a manifest can select a non-default 4-byte container identity;
- why exact child PACs take precedence over nested manifest reconstruction.

### It does not explain

- the TM2↔DDS transformed corpus;
- the producer of the preserved DDS-bearing PAC samples;
- canonical retail packing/alignment policy;
- all permitted manifest directives;
- exact post-EOF/padding byte contents;
- whether any retail shipping path actually relies on `.lst` instead of exact PAC files for the targeted resource.

## 11. Product consequences

GDSpaces should model at least two distinct container materialization recipes:

```text
A. exact stored container
   source bytes are authoritative

B. runtime .lst-assembled container
   manifest + child resources + assembly receipt are authoritative
```

For a `.lst`-assembled resource, provenance should retain:

- parent logical path;
- `.lst` logical path;
- decoded 4-byte magic;
- physical slot count;
- each manifest line in physical order;
- `dummy`/empty-slot markers;
- resolved child path;
- direct-child vs sibling-PAC vs recursive-LST source mode;
- computed offset;
- reserved span;
- source hashes when available.

A runtime-assembled container must not be labeled byte-identical to a retail stored container unless independently proven.

## 12. Promotion boundary

### Confirmed/high

- `.lst` fallback is a recursive relative-slot container assembly mechanism;
- default magic is `PAC\0`;
- `#` can supply up to four magic bytes;
- valid manifest resource lines define physical slot order;
- `dummy` produces offset zero;
- header geometry is `magic + count + uint32 offsets[]`;
- synthesized header span is `0x40` aligned;
- direct child size comes from VFS sector-padded size (`0x800` units);
- nested `.lst` prefers same-stem `.pac` when that direct child exists;
- otherwise nested manifests recurse;
- child population occurs directly into `destinationBase + offset[i]`.

### Still blocked

1. Raw canonical EXE byte re-dump of the Pass-91 functions.
2. One or more pristine real `.lst` fixtures with source lineage.
3. Confirmation of concrete `#PNST` or other directive text on a real manifest.
4. Exact permitted comment/control grammar beyond the recovered parser mechanics.
5. Byte-exact padding initialization and unused-header bytes.
6. Canonical retail PAC/PNST writer equivalence — not established.
7. Rebuilt-container game-consumption round trip.

## Layer-1 status

**NOT COMPLETE.**

Pass 91 substantially closes the runtime `.lst` assembly branch, but it remains a separate runtime-materialization recipe, not writer authority for pristine retail PAC/PNST/NBZ reconstruction.