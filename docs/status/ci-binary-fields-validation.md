# Binary Inspector Fields CI Validation

Scope:

- typed fields and field kinds;
- parent-child containment rules;
- deterministic field ordering;
- annotations with normalized tags and evidence IDs;
- owner lookup by byte offset;
- field, region, owner, and annotation selection context;
- structural region conflicts;
- ownership conflicts with exact intersections;
- all previous foundation, container, Evidence, and stage tests.

This validation covers the domain model only. It does not add a GUI, source resolver, or byte editor.
