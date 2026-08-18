# DMC3 Resource Lookup Candidate Plan

**Status:** bounded implementation; whole-head CI required before promotion.

## Recovered caller-side order

For the evidenced `OpenGameResource` branch, the request is reduced to the basename after the last `/` or `\\`, then six namespace prefixes are tried in this exact order:

1. `GDataX360.afs/`
2. `GData.afs/`
3. `Video/`
4. `afs/sound/`
5. `SAVEDATA/`
6. empty prefix

All six archive/provider-mask `1` attempts precede all six physical/provider-mask `2` attempts. The `.afs/` strings are logical namespaces; they are not evidence for a binary AFS container backend.

Candidate construction uses a recovered `0x400`-byte destination bound. `ResourceLookupPolicy` keeps one byte for terminating NUL.

## Product safety boundaries

The recovered code consumes C strings. Product `string_view` requests containing embedded NUL fail closed and cannot produce a lookup plan.

The current representation is complete-or-invalid when any candidate exceeds the bounded destination. The `0x400` bound is recovered; exact original continuation behavior after one oversized candidate is not separately claimed and this conservative rule is product behavior.

Candidate construction preserves basename bytes/case. Provider normalization (`0x0E` archive, `0x0C` physical) remains the next stage and is not performed by this planner.

## Not owned here

This planner does not select sources, normalize archive keys, choose duplicate-key winners, mount `DMC3-N.nbz`, synthesize `.lst`, parse AFS binaries, or implement original FileSlot/cache/lifetime semantics.
