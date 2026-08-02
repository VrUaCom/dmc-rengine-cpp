# StageBundle Assembly

`StageBundleAssembler` groups already resolved GDSpaces resources into a typed stage bundle. It does not search directories, open archives, or interpret EXE tables itself.

## Input

- `StageIdentity`;
- a sequence of `StageMemberCandidate` values.

Each candidate contains:

- stable `ResourceRef`;
- optional explicit category from stronger evidence;
- optional semantic role.

## Explicit evidence wins

When a candidate supplies a category, the assembler uses it. This supports evidence-backed interpretation for formats whose meaning is not safe to infer from extension alone.

Example: a DCA resource may be explicitly categorized as an event resource by a validated stage parser/evidence packet. The generic assembler does not assume that all DCA resources have that role.

## Conservative inference

Current inference rules:

- `txt` → scripts;
- `scm`, `mod` → models;
- `dds`, `ptx` → textures;
- `cam` → cameras;
- `lig`, `lig2` → lighting;
- `hits` → collision;
- `wav`, `ogg`, `snd`, `se` → sounds;
- paths containing the established `_effect`/effect directory pattern → effects;
- everything else → unknown.

Unknown preservation is required. Unsupported resources are never silently dropped.

## Diagnostics

- invalid candidates create an error diagnostic;
- duplicate canonical resource IDs are ignored with a warning;
- warnings do not invalidate otherwise usable bundles;
- errors make `StageBundle::valid()` false while preserving accepted members.

## Dependency direction

```text
EXE evidence / stage table
  + GDSpaces mounted sources
  + container expansion
  + resource classification
  → StageMemberCandidate[]
  → StageBundleAssembler
  → StageBundle
  → Stage Ops / ModViz / Binary Inspector
```

Stage Ops and ModViz may not reverse this flow by locating files independently.

## Next vertical target

The first game-backed local integration target remains `st001`, using the four known stage-table roles and user-supplied legal game files. Public tests will use synthetic identities and resources.
