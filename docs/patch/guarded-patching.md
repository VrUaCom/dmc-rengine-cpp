# Guarded Patching

The current Patch Engine seed applies fixed-size byte replacements to an in-memory copy only after all guards pass.

## Required plan identity

A `GuardedPatchPlan` contains:

- plan ID;
- target SHA-256;
- ordered byte patches.

Each `BytePatch` contains:

- byte offset;
- expected source bytes;
- replacement bytes of the same size;
- human-readable semantic description.

## Atomic validation

Before modifying output, the plan validates:

1. non-empty plan identity;
2. valid target SHA-256 length;
3. actual source SHA-256;
4. every patch range;
5. every expected-byte sequence;
6. patch overlap during plan construction.

If any guard fails, no output is returned and no partial change is applied.

## Current limits

- fixed-size replacement only;
- in-memory application only;
- no direct file output;
- no patch dependency graph;
- no RVA/VA resolver integration;
- no manifest serialization;
- no runtime test runner.

These limits are intentional. Variable-size archive edits belong to future working-copy/export systems, not the EXE byte-patch primitive.

## Intended integration

```text
User-supplied artifact
  → GDSpaces ResourcePayload
  → SHA-256 identity
  → EXE/Binary evidence
  → GuardedPatchPlan
  → atomic patched output bytes
  → validation/report
  → explicit export by a higher-level service
```

## Historical migration

Legacy item/inventory patches must be migrated as guarded plans only after their addresses, expected bytes, supported hashes, semantics, conflicts, and runtime tests are represented in public/sanitized evidence.

## Security

The current API never writes to the source file. A future exporter must create a new file or controlled backup/replace transaction and record rollback metadata.
