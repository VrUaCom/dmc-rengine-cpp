# Working Copy Contract

`WorkingCopy` separates immutable source bytes from editable state.

## Construction

A working copy is created from a resolved `ResourcePayload` and records:

- stable `ResourceRef`;
- immutable original bytes;
- current editable bytes;
- SHA-256 of the original bytes;
- current revision;
- edit history.

The original `ResourcePayload` is not modified.

## Edit operation

An `EditOperation` contains:

- stable operation ID;
- base revision;
- byte offset;
- expected current bytes;
- replacement bytes;
- semantic description.

Replacement size may differ from expected size. This allows format editors to express structural edits in a private working copy without implying that the underlying archive can already be exported safely.

## Guards

An operation is rejected when:

- ID or description is empty;
- base revision is stale;
- operation ID already exists in active history;
- offset/range is outside current bytes;
- expected bytes do not match;
- resulting allocation would exceed vector limits.

## Revision model

- accepted edit increments revision;
- undo increments revision again because state changed;
- reset restores original bytes and revision zero;
- stale operations cannot apply after another accepted edit.

## Undo

The working copy stores the previous byte range for every accepted operation. `undo_last()` validates that the current replacement bytes still match before restoring the previous range.

## Export boundary

`WorkingCopy` does not write files, archives, or executables. Future exporters consume validated working-copy state and produce explicit output plans, manifests, backups, and rollback information.

## Relation to GuardedPatchPlan

- `WorkingCopy`: general editor state, revisioned operations, variable-size local edits.
- `GuardedPatchPlan`: fixed-size, hash-gated, atomic byte patches for known artifact versions.

They may interoperate later, but they are not the same responsibility.
