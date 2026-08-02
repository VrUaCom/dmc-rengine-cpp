# Reverse Engineering Rules

Every technical claim must carry a status:

- `hypothesis` — plausible but not tested
- `candidate` — supported by evidence, still incomplete
- `low` / `medium` / `high` — confidence grades where useful
- `confirmed` — independently verified against bytes, behavior, or repeatable analysis
- `corrected` — replaces an older conclusion
- `rejected` — disproven and retained to prevent regression

## Required evidence

Reverse-engineering records should include, where applicable:

- exact input hash and version
- file offset and RVA/VA
- original bytes
- disassembly or structural context
- callers and callees
- inferred ABI and ownership
- test procedure and result
- unresolved alternatives
- links to implementation and tests

## Implementation rule

Readable C++ is not considered recovered behavior by itself. A reconstructed function must remain tied to its evidence, assumptions, ABI, lifetime model, and behavioral tests.

## Patch rule

An executable patch requires expected source bytes, target bytes, semantic purpose, dependencies, conflicts, rollback data, and a repeatable runtime test.

## Public repository rule

Do not commit game executables, proprietary assets, extracted game archives, copyrighted binary blobs, credentials, private paths, or user-specific game installations.
