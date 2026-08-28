# DMC3 `.index` extracted-ordinal recovery — L1 Pass 7

Status: **CORRECTED / corpus-confirmed extraction naming behavior**  
Scope: Layer 1 resource materialization and naming provenance only.  
Target profile: `dmc3-hd`.

## Why this correction exists

The earlier recovery model treated ordinary `.index` manifests as physical-position maps and switched to non-empty sequential mapping only when the manifest contained the standalone `PNST` directive.

A deeper retained-corpus pass disproves that distinction.

The `.index` numeric suffix is an **extracted ordinal**, not a physical PAC/PNST slot number. Sparse `PAC\0` containers without a `PNST` directive use the same dense sequence across populated payloads as sparse PNST containers.

## Retained-corpus evidence

The audit covered the retained stage-drop material used by the GDSpaces v6 review:

- 121 `.index` manifests;
- 42 raw PAC/PNST containers with a matching manifest and recoverable physical slot table;
- all 42 have `manifest entry count == populated payload count`;
- 8 of the 42 are sparse containers: 6 PAC and 2 PNST;
- zero sparse cases contain a manifest entry for an empty physical slot;
- zero ordinal/prefix anomalies were observed across the 121 manifests;
- every observed `<stem>_NNN` entry uses `NNN` as the zero-based extracted ordinal.

Representative sparse PAC evidence:

```text
em035_037.pac
physical slots: 95
populated slots: 5, 6, 7, 21, 50, 80, 89, 94
.index entries: _000 .. _007

mapping:
_000 -> physical slot 5
_001 -> physical slot 6
_002 -> physical slot 7
_003 -> physical slot 21
_004 -> physical slot 50
_005 -> physical slot 80
_006 -> physical slot 89
_007 -> physical slot 94
```

Representative sparse PNST evidence:

```text
st001_005.pac
physical slots: 11
populated slots: 0, 10
extracted ordinals: 0, 1

mapping:
_000 -> physical slot 0
_001 -> physical slot 10
```

## Corrected invariant

For the recovered legacy DMC3 extraction grammar:

```text
physical slot identity != extracted ordinal

manifest entry 0 -> first populated payload
manifest entry 1 -> second populated payload
...
```

Empty physical slots remain in the container topology and retain their physical indices. They do not consume an extracted ordinal.

The `PNST` directive remains parsed and retained as manifest/container evidence, but it is **not the authority that creates dense non-empty numbering**.

For a dense container, physical position and populated sequence are observationally identical. The implementation may retain `physical_position` as the mapping-mode label when no empty slot exists, while still recording the explicit extracted ordinal for every authority entry.

## Provenance model

Every external `.index` name authority now keeps both coordinates explicitly:

```text
physical_slot_index
extracted_ordinal
```

It also retains:

- companion manifest resource identity;
- manifest SHA-256;
- raw index label;
- normalized index name;
- manifest line;
- mapping mode.

Neither coordinate is allowed to replace the other.

`ResourceId` continues to be based on physical source/container/slot lineage. The extracted ordinal and `.index` label are naming provenance only and are never write authority.

## `.index` runtime boundary

This pass does **not** promote `.index` into a game runtime manifest.

The canonical DMC3 executable evidence currently contains runtime resource-path vocabulary such as `GData.afs/`, `GDataX360.afs/`, and `%sDMC3-%d.nbz`, while no ASCII literal `.index`, `PNST`, `folder`, or `.lst` was found in the canonical executable. Combined with the sparse-slot mismatch above, the retained `.index` files are treated as extraction/naming receipts unless future runtime evidence proves a stronger role.

## `folder` boundary

The same corpus contains 71 `folder` entries. All 71 materialize as nested directories with their own `.index`; all inspected materialized directories contain DDS children, and the 58 cases recoverable back to parent raw payload bytes also contain DDS data.

This is strong evidence that `folder` in this retained extraction corpus marks a recursively materialized texture-bundle representation. It is not part of the filename and is not a physical slot locator. This semantic is documented here as corpus evidence but is not used by Pass 7 to redirect physical identity.

## Exact extractor provenance

The public DMC3 HDC modding trail strongly points to the legacy Python 2 `ex.py` / “Pac tool update 7” workflow as the producer family for this extraction convention. The exact script revision that produced the retained corpus has not yet been code-level matched.

Status: **STRONG PROVENANCE CANDIDATE, NOT CONFIRMED**.

Do not turn that attribution into a canonical fact until the exact extractor implementation is recovered and its numbering / `.index` / `folder` behavior is matched against the corpus.

## Pass 7 implementation consequence

Pass 7 corrects the C++ authority chain so that:

1. sparse PAC without a `PNST` directive skips empty physical slots when consuming `.index` entries;
2. sparse PNST keeps the same populated-sequence behavior;
3. dense containers remain behaviorally unchanged;
4. `IndexSlotNameAuthority` records `extracted_ordinal`;
5. `IndexNameOverlayEntry` carries it without changing physical identity;
6. sealed `ResourceNameEvidence` persists it alongside `physical_slot_index`;
7. regression coverage proves `extracted ordinal 1 -> physical slot 2` for a synthetic sparse PAC without a directive.

## Superseded rule

The following older rule is superseded for the recovered DMC3 extraction grammar:

```text
normal manifest: manifest entry i -> physical container.entries[i]
PNST manifest:   names advance only over populated entries
```

Corrected form:

```text
dense container: positions coincide, preserve both coordinates
sparse container: manifest entries advance only over populated entries
PNST directive: metadata, not the cause of numbering
```

Any future corpus that demonstrates a physically indexed sparse manifest must be represented by an explicit grammar/evidence mode rather than silently reusing this recovered legacy convention.
