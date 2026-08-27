# L1 `.index` authority status — 2026-08-27

## Completed in stacked implementation

### Parent PAC/PNST member naming pass

- physical PAC/PNST child identity is independent from presentation/index labels;
- `.index` is parsed from exact observed bytes and SHA-256 sealed;
- manifest entries bind to exact already-materialized physical child IDs;
- sparse PNST uses populated-slot sequence without renumbering physical slots;
- index/container count mismatch fails closed;
- display reconciliation cannot mutate bytes or write authority;
- nested PAC/PNST rebuild -> reopen -> rename-only -> second edit regression is covered;
- whole-head Ubuntu and Windows CI for PR #251 is green.

### Nested texture/DDS pass

- validated texture slots expand to physical `TEXTURE[n]` DDS children;
- DDS children retain parent/container lineage and byte provenance;
- the same sealed `.index` parser/binder names DDS children;
- retained audit evidence that `st001_001.index` contains 17 DDS names is modeled as a 17-child topology regression;
- same-size DDS authoring -> bundle rebuild -> reopen preserves the physical texture target;
- rename-only DDS labels do not redirect subsequent authoring;
- 16 labels against 17 physical DDS children fail closed.

## Remaining identity/name-authority research

1. `.post` remains **UNRESOLVED**. No alias to PNST is permitted without evidence.
2. Exact historical source implementation for unconfirmed `.index` grammar edge cases is not recovered; only the confirmed narrow grammar is accepted.
3. Exact historical spelling of all 17 `st001_001.index` DDS labels is not present in the retained checkout evidence used by this pass; do not fabricate it.
4. Size-changing texture packed reflow can move DDS byte spans. The stable lineage remains `TEXTURE[n]`, but exact `ResourceId::canonical()` includes offset/size and therefore is only claimed invariant for layout-preserving/same-size rebuilds.
5. A real-retail execution receipt pairing actual `st001_001.index` bytes with the actual 17 DDS physical records is still desirable before calling the nested naming boundary corpus-closed.
6. These passes do not promote Layer 1 to 100% complete; they close naming/identity gaps only.
