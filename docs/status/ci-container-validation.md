# Container Foundation CI Validation

This validation branch tests the first 0.3 generic read-only container foundation.

Scope:

- generic container document/entry/diagnostic contracts;
- read-only parser interface and parser registry;
- invented `SLTC` slot-container parser used only as a legal synthetic fixture;
- empty slot preservation and stable slot identity;
- fallback names;
- truncation, out-of-range, residue, and overlap diagnostics;
- GDSpaces child ResourceRef/ResourcePayload expansion;
- parent/child ResourceGraph connections;
- child magic reclassification;
- protection against parent-payload range changes;
- all existing foundation tests.

The synthetic format does not claim compatibility with PAC, PNST, AFS, NBZ, or any proprietary format. No writer is included.
