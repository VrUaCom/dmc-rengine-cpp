# GDSpaces PAC/PNST relative-slot CLI

`rebuild-relative-slot` is a product-side authoring seam over the canonical DMC3 PAC/PNST parser/expander and `RelativeSlotPackedReflowWriter`.

```text
dmc-rengine rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>
```

The command reads source and replacement bytes through GDSpaces, preserves slot topology/aliases/protected prefix, permits size-changing replacement of one populated slot, validates the rebuilt topology, publishes atomically without replacement, and never modifies the source container in place.

This is a DMC Rengine product writer contract, not a claim about Capcom's original offline writer.
