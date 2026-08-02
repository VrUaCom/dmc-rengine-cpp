# HITS$ Scanner CI Validation

Scope:

- HITS$ magic recognition;
- confirmed little-endian record marker `0x18060001`;
- 56-byte record validation;
- raw decoding of thirteen float32 values;
- unknown padding before records;
- wrong-magic, no-record, and truncated-record diagnostics;
- all previous foundation, Evidence, container, stage, and Binary Inspector tests.

The scanner does not claim a complete HITS file header schema or semantic names for the thirteen floats.
