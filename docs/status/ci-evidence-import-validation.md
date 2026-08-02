# Evidence Import CI Validation — Final

This branch validates the complete strict JSON and Evidence Packet import layer.

Scope:

- bounded generic JSON parser;
- duplicate-key and trailing-content rejection;
- depth, input, value, array, object, and string limits;
- integer overflow and finite floating-point checks;
- Unicode escape and surrogate-pair handling;
- strict Evidence Packet field whitelist;
- artifact, record, confidence, location, and cross-reference validation;
- deterministic export/import/export round trip;
- uppercase SHA-256 normalization;
- CLI `validate-evidence`;
- CTest validation of the public DMC3 Phase 12 canonical Evidence Packet;
- all previous foundation and container tests.
