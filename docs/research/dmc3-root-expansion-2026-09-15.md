# Metadata root expansion, 2026-09-15

Status: structurally verified bounded CFG expansion; full reverse remains open.
Canonical SHA256: e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082.

The previous completeness audit identified 7,450 unique confirmed metadata targets absent from the CFG. The tracer now seeds CRT tables, RTTI-validated first vtable entries, and direct unwind handlers, recording each source slot. It validates canonical CRT bound instructions, COL signature/self/type links, first-entry bytes, executable membership and unwind alignment. It does not promote later pointer-run candidates to confirmed vtable slots.

| Measure | Before | After |
|---|---:|---:|
| Instruction starts | 611,752 | 633,647 |
| Instruction byte sum | 2,723,341 | 2,837,178 |
| Missing CRT targets | 6,930 | 0 |
| Missing first-vtable targets | 519 | 0 |
| Missing direct unwind handler targets | 1 | 0 |
| Missing later pointer-run candidate targets | 1,418 | 899 |

All 10,112 metadata records (7,885 unique targets) land on visited instruction starts. Seed count is 19,683; seed-count growth is not equal to the old missing-root count because some metadata targets were already reached by traversal without being seeds.

Independent verification passes: 99,572 direct edges decoded from displacement bytes, 49 switch edges, 66 switch slots, 633,647 instruction boundaries. There are zero decode failures, overlaps or out-of-code targets. GNU objdump length comparisons agree at 633,561 common starts and all 86 reanchored starts. This verifies boundaries, not full operand semantics or runtime reachability.

## CRT prioritization

A reproducible bounded-prefix classifier groups 6,987 CRT targets into 35 mnemonic/operand-type shapes. 6,904 prefixes reach return; 46 stop at call, 35 at unconditional jump, two at conditional jump. No prefix hits the 32-instruction/256-byte cap. The three largest shapes contain 3,135 movaps/load-store-return, 3,125 movdqa/load-store-return and 628 xorps/store-return entries. Shapes deliberately omit constants, register identities and widths: these counts are triage, not proof of identical behavior. Explicit RIP-relative write destinations are recorded for follow-up global mapping.

## Autonomous reverse strategy

User authorized Ada to lead reverse engineering autonomously on branch Ада-Астра. Prioritize evidence gained per pass: metadata-root closure first; batch repeated initializer shapes; investigate the 83 transfer-bearing CRT prefixes; establish vtable extents before promoting the remaining 899 targets; then map constructors, global ownership and initialize/update/draw/shutdown paths. Reassess the order when byte evidence exposes a higher-value dependency. Keep scripts, address provenance, confidence and remaining unknowns in commits. Do not report a full-game completion percentage from instruction counts.

Still open: three candidate vtable runs capped at 256 slots, additional callbacks and indirect dispatches, non-RTTI classes, method signatures, ownership and runtime ordering. Existing six reviewed switches do not resolve all indirect jumps. Calls still conservatively assume return.

## Reproduce from repository root

```sh
python scripts/reverse/trace_canonical_exe.py /path/to/dmc3.exe data/reverse/root-expansion-20260915 --vtable-anchors /path/to/runtime-extractor-output/vtable_anchors.tsv
python scripts/reverse/verify_canonical_cfg.py /path/to/dmc3.exe data/reverse/root-expansion-20260915
python scripts/reverse/audit_runtime_tree_coverage.py /path/to/dmc3.exe /path/to/runtime-extractor-output data/reverse/root-expansion-20260915 data/reverse/exe-structure-20260915 data/reverse/root-expansion-audit-20260915
python scripts/reverse/classify_crt_prefixes.py /path/to/dmc3.exe data/reverse/root-expansion-20260915/metadata-roots.tsv.gz data/reverse/root-expansion-20260915
```

Requires Capstone 5 (run: 5.0.7), GNU objdump and the existing runtime extractor output matching the recorded hashes. No executable or raw disassembly is included.
