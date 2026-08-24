# GDSpaces PAC/PNST relative-slot CLI

`rebuild-relative-slot` is a product-side authoring seam over the canonical DMC3 PAC/PNST relative-slot parser/expander and `RelativeSlotPackedReflowWriter`.

```text
dmc-rengine rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>
```

The command:

- reads the parent and replacement through GDSpaces local sources;
- requires a canonical PAC/PNST parse;
- preserves declared slot count, empty slots, alias partition and protected prefix;
- permits size-changing replacement of one populated slot;
- leaves unrelated physical spans byte-exact;
- validates the rebuilt topology before publication;
- publishes through the shared atomic/no-replace primitive;
- never modifies the source container in place.

This is a DMC Rengine product writer contract. It is not a claim about Capcom's original offline PAC/PNST authoring tools.
