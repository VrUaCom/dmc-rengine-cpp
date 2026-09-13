# DMC3 HD SCM — header +0x14 runtime-role refinement

**Date:** 2026-09-13  
**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Result

SCM header `+0x14` must be split into three independent evidence claims:

```text
serialized width/offset                  STRUCTURAL_CONFIRMED
SCM-corpus decimal component pattern     CORPUS_CONFIRMED
serialized +0x14 -> manager +0xE4        EXE_CONFIRMED
manager +0xE4 high-level runtime role    PRESERVED_UNDECODED
```

The current SCM corpus supports the structural decomposition:

```text
raw = family_class * 100000 + model_set * 100 + sub_index
```

Examples remain useful as **SCM structural/corpus descriptors**. They are not proof that the game interprets manager `+0xE4` as a resource class, stage identity, cache key, render family, routing key, or gameplay identity.

## Why the stronger runtime name is blocked

Canonical manager initialization proves:

```text
serialized +0x14
  -> 0x1402F9570
  -> manager +0xE4
```

But the type-aware consumer census does not establish a downstream read from that same manager instance.

Whole-image instructions using a raw `+0xE4` displacement are not sufficient evidence because multiple unrelated runtime structures have members at that displacement. Promotion requires pointer provenance from the manager initialized by `0x1402F9570` to the consuming instruction.

This is the same provenance rule that rejected unrelated `0x00200000` mask hits for SCM source object bit 21.

## Correct product contract

The public/IR name `LegacyResourceCode` remains acceptable only as a neutral **serialized SCM structural code** because its decimal component pattern is stable in the bound SCM corpus and already has encode/decode helpers.

It must not imply an EXE-confirmed high-level runtime meaning.

Writer policy:

```text
parsed resource -> preserve raw u32 exactly
explicit structural edit -> encode only the requested component change
no filename-derived regeneration
no inferred gameplay/material/resource-class label
```

## Next executable action

The validated SCM deep-reader v3 packet now treats `0x1402F9570` as a provenance root. The next fresh-byte pass must follow manager pointer escape from that root, then inspect only consumers proven to operate on the same manager instance. Raw `[reg+0xE4]` searches are regression hints, not evidence.

This note refines wording in earlier SCM completion/provenance documents; it does not invalidate the corpus decomposition or the confirmed serialized-to-manager transfer.
