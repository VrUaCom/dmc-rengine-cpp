# Generic Container Foundation

The container foundation exposes nested resources through GDSpaces without making PAC, PNST or NBZ a top-level product architecture. DMC3-HD AFS-named paths are logical namespaces; binary AFS/PACK candidates are not registered with this foundation until parser authority is promoted.

## Domain contracts

### `ContainerEntry`

Represents one declared slot:

- slot index;
- byte offset and size;
- logical/display name;
- populated flag;
- synthetic-name flag.

Empty slots are preserved as structural entries with zero offset/size. Slot identity is independent from filenames.

### `ContainerDocument`

Contains:

- format identifier;
- parser schema version;
- declared slot count;
- source container size;
- one entry per declared slot.

A valid document requires deterministic slot ordering and structurally valid populated ranges.

### `ContainerParseResult`

Contains:

- recognized flag;
- document;
- structured diagnostics with severity, code, message, and offset.

`ok()` requires a recognized valid document and no error diagnostics. A parser may still preserve a valid partial document with error diagnostics by normalizing a malformed slot into an invalid/empty structural placeholder.

### `IContainerParser`

A read-only parser:

- probes supplied bytes/path hints;
- consumes a byte span only;
- returns a container document and diagnostics;
- never opens files;
- never writes bytes.

### `ContainerParserRegistry`

- rejects duplicate parser IDs;
- selects the highest positive probe score;
- reports when no parser recognizes a resource;
- does not own source acquisition.

## GDSpaces expansion

`ContainerExpander` converts a parsed document and parent `ResourcePayload` into child payloads.

Each child receives:

- same source ID;
- deterministic logical path containing format and slot index;
- container chain with slot identity;
- physical offset relative to the parent source identity;
- child byte size;
- display/fallback name;
- inherited game profile;
- centralized magic/extension classification;
- child diagnostics when the current payload no longer matches parsed ranges.

Empty slots remain visible as `empty-slot` child resources.

## Resource graph

`ContainerExpander::connect_graph()` creates:

```text
parent --contains--> child slot resource
```

No tool has to parse the container again to discover the same child.

## Safety rules

- parser counts and ranges are bounded;
- integer/range checks precede slicing;
- the expander rechecks ranges against the current parent payload;
- unsafe child names are sanitized for virtual logical paths;
- malformed children do not erase unrelated valid children;
- unknown child formats remain visible;
- no writer exists in this phase.

## Real format migration

Real parsers must be added as narrow evidence-backed subsets:

1. generic contracts and synthetic malformed corpus;
2. independently confirmed PAC structural subset;
3. PNST metadata/linkage;
4. AFS source layer;
5. NBZ volume source;
6. nested classification/resource graph;
7. game-backed `st001` StageBundle.

The synthetic test format is not a template to be relabeled as PAC. Real layouts require separate evidence and tests.
