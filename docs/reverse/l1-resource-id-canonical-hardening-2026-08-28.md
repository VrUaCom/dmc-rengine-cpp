# L1 ResourceId canonical-key hardening — 2026-08-28

## Scope

This pass closes P1 from the retained `gdspaces-l1-review-findings.md`: the legacy delimiter-only `ResourceId::canonical()` encoding was not injective.

It is integrated into the consolidated naming tree because all naming evidence and physical child identity ultimately bind to `ResourceId`; a non-injective machine key would undermine otherwise-correct naming authority separation.

## Defect

Legacy encoding:

`source_id:logical_path#container_chain@offset+size`

used delimiters without escaping or field lengths even though `source_id`, `logical_path` and `container_chain` are arbitrary strings. ZIP central-directory pathnames and virtual container paths can legally contain the same delimiter characters.

The retained review reproduced two distinct `ResourceId` values whose canonical strings collided, causing `ResourceGraph::add()` to drop the second identity.

## Correction

`ResourceId::canonical()` now emits a versioned machine key:

`rid2|<len>:source|<len>:logical_path|<len>:container_chain|offset|size`

All arbitrary string fields are byte-length bound. Decimal offset and size remain explicitly delimited after the final length-bound field.

`rid2` is a machine identity encoding version, not a display-path prefix.

## Acceptance boundary

The regression reproduces the exact collision class from the retained review:

- child identity with `logical_path = boot.pac::PAC/slot-0000/a.bin` and `container_chain = PAC[0]`;
- distinct loose identity whose `logical_path` itself ends in `#PAC[0]` and has no container chain.

Required result:

- `operator==` says the identities are different;
- `canonical()` strings are different;
- both resources can coexist in one `ResourceGraph`;
- source/path boundary collisions using `:` are also distinct;
- equal ResourceId values still produce deterministic equal keys.

## Compatibility note

The legacy canonical string was already used as an internal graph/index/session/manifest identity field. The previous encoding cannot be safely retained as an authoritative key because its ambiguity is the defect being fixed. The explicit `rid2` prefix makes the schema change visible rather than silently changing delimiter behavior.

This pass does not add a parser for historical canonical strings. Existing structured `ResourceId` fields remain the authority whenever available; any future importer that accepts persisted legacy canonical-only keys must classify them explicitly as legacy/non-injective rather than treating them as equivalent to `rid2`.

## Deferred related findings

- P6: `ContainerExpander::connect_graph` graph-mutation failure handling is a separate graph-status concern.
- P8: `NbzZipSource::find_entry` exact-comparison cleanup is separate from injectivity correctness.
- P9: `StageBundle::add` exact-comparison cleanup is the same low-risk class.

## Non-claims

- no Layer 1 completion claim;
- no NBZ serialization no-loss claim;
- no writer behavior change.
