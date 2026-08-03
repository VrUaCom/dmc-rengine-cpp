# HITS Local Corpus Comparison CLI — 2026-08-03

## Command

```text
dmc-rengine compare-hits-spatial <original> <candidate> [report.json]
```

When the output path is omitted, the report is written beside the candidate as:

```text
<candidate>.hits-spatial-comparison.json
```

## Data path

- both files are read through mounted GDSpaces `LocalDirectorySource` instances;
- SHA-256 is calculated for both exact input byte streams;
- both resources are parsed by the canonical HITS `RecordScanner`;
- triangle identity is matched by bit-exact raw flags plus ordered A/B/C coordinates;
- spatial ownership is compared through the stable-ID differential validator;
- the deterministic JSON report is written as a research artifact, not as a game-resource mutation.

## Automatic matching boundary

Automatic matching is accepted only when each `flags+A/B/C` geometry key is unique and appears exactly once in both files. This supports unchanged geometry after writer serialization or record reorder.

The command rejects:

- different triangle counts;
- edited or unmatched geometry;
- duplicate identical geometry that makes identity ambiguous;
- structurally invalid HITS input;
- incompatible spatial grids;
- malformed or duplicate cell references.

Edited geometry requires an explicit authoring/stable-ID manifest and is not silently guessed.

## Output

The terminal summary includes:

- exact/different state;
- both SHA-256 identities;
- mapped triangle count;
- original/candidate/shared/missing/extra references;
- exact cell and surface counts;
- precision, recall and Jaccard similarity;
- final report path.

The JSON report contains full per-cell and per-surface deltas and can be attached to an Evidence Packet or synchronized to Drive/Knowledge Base records.

## Evidence status

The command is a measurement and corpus-research tool. An exact report for one file does not prove general equivalence with Capcom's unknown offline builder. Promotion requires consistent results across the complete source-0/source-1 corpus and controlled runtime validation.
