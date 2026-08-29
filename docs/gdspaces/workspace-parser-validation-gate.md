# GDSpaces workspace parser-validation gate

Status: product integration safety boundary for evidence-gated editable formats.

## Problem

A format label is not structural proof. After PAC/PNST parsers and bounded writers were promoted, changing their integration descriptor from `read_only` to editable without another gate would allow malformed bytes to acquire a `WorkingCopy` merely because `ResourceRef.format` says `pac` or `pnst`.

The safe path is:

```text
immutable ResourcePayload
 -> canonical parser for this format
 -> source-bound parser validation state
 -> WorkingCopy
 -> bounded writer
```

## Parser validation state

`ResourceWorkspaceSession` retains:

- exact parser ID;
- SHA-256 of the immutable workspace source bytes at parser completion;
- recognized/not-recognized result;
- parser-error diagnostic state.

For descriptors with `parser_validation_required=true`, `enable_working_copy()` requires all of:

- validation state exists and is internally valid;
- parser ID exactly equals the descriptor's canonical parser authority;
- parser recognized the source;
- no parser error diagnostic invalidates the session;
- current immutable source SHA still equals the SHA recorded with parser completion.

A wrong parser completion fails closed and does not create a validation state.

Parser errors arriving after a successful completion also invalidate the stored state before editing is enabled.

## PAC / PNST

Current canonical parser authorities:

- PAC: `dmc3-pac-structural-v1`;
- PNST: `dmc3-pnst-structural-v1`.

Both are `structural`, `working_copy_only`, parser-gated, and expose the bounded writer mode:

`layout-preserving-packed`.

This does not advertise runtime-synth size-changing authoring or subsystem equivalence.

## Source-backed formats

`FormatIntegrationDescriptor` now separates parser and source/materializer authorities.

A structurally usable format may be backed by either:

- `parser_id`, or
- `source_adapter_id`.

NBZ is the important current example:

- no fake parser ID;
- source adapter: `gdspaces.nbz-zip-source-v1`;
- structural product maturity;
- read-only as a workspace resource;
- bounded output capability `store-overlay-nbz` recorded separately.

The generated overlay writer is an authoring output mode; it does not imply arbitrary editing or lossless retail-volume repack of an NBZ source resource.

## Writer modes

`writer_modes` records explicit bounded product authoring capabilities instead of allowing one generic `exportable` bit to imply every possible writer.

Current modes introduced by the Layer-1 reconciliation:

- PAC/PNST: `layout-preserving-packed`;
- NBZ family: `store-overlay-nbz`.

Future modes such as runtime-synth relative-slot output or full metadata-preserving retail repack must be added only when their own implementation/evidence gates are satisfied.

## AFS correction

DMC3 HD `.afs/` strings remain confirmed logical namespaces. The integration registry now separates `afs-namespace` from non-expandable `afs-binary-candidate` and `pack-binary-candidate` identities. A binary parser remains evidence-gated until raw/profile/backend authority exists; it is not a pending namespace parser.

## Compatibility boundary

Parser gating is opt-in per descriptor. Existing HITS/TXT/other workflows are not silently forced through the new PAC/PNST gate in this migration.

This is product integration safety, not original-game behavior.
