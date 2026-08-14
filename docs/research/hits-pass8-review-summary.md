# HITS Pass 8 — Review Summary

Pass 8 confirms that the HITS file-format/writer work from Pass 5/6 remains valid while the newer DMC3 Wave-2 resource lifecycle introduces an additional higher runtime layer above stage-local HITS construction.

Key project consequences:

- keep general resource lifecycle, HITS runtime and dynamic collision as separate ownership layers;
- retain source 0/member 3 and source 1/member 6 mappings;
- use the full 189-descriptor Stage resource universe for future HITS ecosystem coverage;
- preserve the exact Pass-5 spatial writer as canonical;
- keep original query ABI/arbitration reconstruction in recovered-game code, not GDSpaces;
- keep speculative gameplay naming and universal `CollisionResult` modeling frozen until evidence closes the remaining ABI nodes.

See the full Pass-8 review and evidence JSON for addresses, statuses and remaining targets.
