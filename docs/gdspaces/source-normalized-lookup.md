# GDSpaces Normalized Source Lookup

**Status:** implementation prepared; current-main whole-head promotion pending.

`ISource::lookup(provider_key, normalization_flags)` is the generic GDSpaces seam between provider-specific normalized keys and stable source `ResourceRef` identities.

## Contract

- the caller supplies an already-normalized provider key;
- the key must be non-empty and C-string-compatible;
- a key that changes when normalized with the supplied flags is invalid at this layer;
- source resources with non-C-string-compatible logical paths do not participate in normalized-key matching;
- every valid matching physical/resource identity is returned;
- duplicate normalized keys are preserved as ambiguity and no winner is selected here.

`SourceLookupReport` distinguishes invalid key, missing source, available/no-hit, unique hit and ambiguous hit.

The default `ISource` implementation may enumerate and normalize resource paths. A source such as NBZ may later override this with an indexed implementation if it preserves the same observable lookup/ambiguity contract.

## Evidence boundary

DMC3 archive provider flags `0x0E` and physical flags `0x0C` are supplied by the profile layer. This generic lookup is product infrastructure; it does not claim that the original runtime enumerates resources this way or uses this data structure.

Duplicate-key winner semantics remain deliberately unresolved. The future DMC3 resolver must stop/report ambiguity rather than silently choosing enumeration order until stronger evidence is promoted.
