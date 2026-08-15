# StageSet token-to-factory recovery packet

## Purpose

Close Knowledge Base gap **G-P16-0039** without inferring concrete runtime
classes from Stage TXT token names.

The unresolved transition is:

```text
Stage TXT bytes
  -> tokenizer / parser helpers
  -> StageSet token classifier
  -> classifier result
  -> factory / constructor dispatch       <-- G-P16-0039
  -> concrete CStageSet-derived object
  -> registration / lifetime
  -> gameplay consumers
```

This packet is intentionally narrower than general StageSet reverse engineering.
Repeated token census, RTTI family enumeration, or filename-based inference does
not close the gap.

## Canonical target

- executable SHA-256:
  `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- image base: `0x140000000`
- StageSet token classifier: `0x140246680` — **EXE CONFIRMED**
- TXT tokenizer/helper family: **EXE CONFIRMED**
- Knowledge Base gap: `G-P16-0039` — **RESEARCH REQUIRED**

## Existing evidence boundary

The current corpus proves that StageSet lexical/runtime families exist, but it
does **not** prove a universal mapping such as:

```text
"STAY" -> CStageSetStay constructor
"SEAL" -> CStageSetSeal constructor
```

merely because the names resemble one another.

The hierarchy itself is heterogeneous. For example, current RTTI/inheritance
recovery shows:

- `CStageSetStay -> CStageSetStayDemo`
- `CStageSetStayDemo -> CActor`
- `CStageSetStayDemo -> CStageSet`
- `CStageSetSeal -> CActor`
- `CStageSetSeal -> CStageSet`

Representative recovered COL navigation anchors include:

- `CStageSetSeal`: RVA `0x51DC28`, `0x51DCC8`, `0x51DCF0`, `0x51DD18`
- `CStageSetStayDemo`: RVA `0x51DE58`, `0x51DEF8`, `0x51DF20`, `0x51DF48`
- `CStageSetStay`: RVA `0x51DF70`, `0x51E018`, `0x51E040`, `0x51E068`

These are navigation anchors, not constructor addresses.

## Required two-sided proof

### Route A — classifier forward data flow

Start at logical function `0x140246680` and recover every caller that consumes
its return value.

For each caller record:

1. exact logical function / runtime ranges;
2. incoming parser/cursor state;
3. classifier argument origin;
4. return-value compare/switch/table use;
5. allocation or object-storage destination;
6. constructor/factory call target;
7. registration/storage target after construction;
8. cleanup/destructor path if visible.

The acceptance point is a direct data-flow chain from classifier result to a
constructor/factory decision, not proximity in the call graph.

### Route B — concrete type reverse XREF

Start from representative concrete RTTI/COL/vfptr families such as
`CStageSetStay` and `CStageSetSeal`.

For each candidate constructor-like function recover:

1. vfptr installation writes;
2. base/subobject initialization;
3. allocation size/source where visible;
4. immediate callers;
5. registration target;
6. whether a caller is reached from the classifier-consumer chain.

### Intersection rule

A token/classifier-result -> concrete runtime type mapping may be promoted only
when Route A and Route B intersect through exact data flow, or when an equally
strong independent direct proof exists.

RTTI name similarity alone is insufficient.

## Promotion statuses

Use the following Stage Ops recovered-runtime link authority:

- `direct_reconstructed` only after the relevant recovered function/contract is
  physically represented in `recovered-game` and the data-flow mapping is
  closed;
- `disassembly_complete_corpus_pending` when the full logical disassembly and
  mapping are closed but recovered source/evidence promotion is still pending;
- `executable_candidate` for incomplete XREF/factory hypotheses.

A Stage TXT `stage_set_value_token` remains a structural lexical fact regardless
of runtime-link authority.

## Prohibited shortcuts

Do not:

- create a generic `StageSetObject` ABI from lexical token text;
- assign a concrete `CStageSet*` class by matching names;
- treat all StageSet-derived types as one inheritance/subobject layout;
- let Semantic Graph infer constructor/factory relationships;
- put recovered Capcom factory behavior inside generic Stage Ops;
- mark a mapping confirmed because a test fixture uses a synthetic runtime link.

## Required artifacts for closure

A closure packet for `G-P16-0039` must contain:

- raw/citable disassembly for `0x140246680` and all relevant callers;
- caller/callee XREF table;
- classifier-result dispatch table or comparison matrix;
- concrete constructor/vfptr-install evidence;
- classifier-result -> factory/constructor mapping table;
- registration/lifetime observations;
- evidence IDs and canonical executable identity;
- recovered-game source unit(s) for promoted contracts;
- Stage Ops profile runtime-link regression;
- Windows + Ubuntu CI receipt.

## Stage Ops integration after closure

The intended product-side relationship is:

```text
StageDomainObject(stage_set_value_token)
  -- classified-by-runtime-function --> recovered classifier
  -- constructs-runtime-object ------> recovered factory/type
```

These links are supplied explicitly by the DMC3 profile/reverse integration
layer to `StageSceneController`. They remain evidence-bearing links in the
`StageDomainKnowledgeWorkspace`; Stage Semantic Graph only projects them.
