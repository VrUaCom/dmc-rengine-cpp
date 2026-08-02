# Binary Inspector Manifest CI Validation

Scope:

- deterministic Binary Inspector document manifest JSON;
- stable resource identity export;
- regions, fields, ownership, annotations, unknown ranges, region conflicts, and ownership conflicts;
- escaped text and annotation tags;
- parsing the exported manifest through the bounded JSON parser;
- all previous foundation, container, Evidence, stage, and Binary Inspector tests.

The manifest is read-only metadata. It does not contain source bytes and does not write game files.
