# GDSpaces blocked EXE-window acquisition packet

**Scope:** reverse-evidence acquisition support for GDSpaces roadmap gaps.  
**Artifact authority:** canonical DMC3 HD analysis executable only.  
**Canonical SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

This runbook exists because several high-value GDSpaces reverse gaps currently share the same blocker: the connected automation environment has summaries and historical reports, but not the raw canonical executable body windows required to prove the remaining exact semantics.

The packet does **not** promote any semantic claim. It only reacquires exact byte windows through the already canonical `dmc-rengine extract-exe-window` command.

## Architecture boundary

```text
canonical analysis dmc3.exe
 -> dmc-rengine extract-exe-window
 -> SHA gate
 -> PE mapping gate
 -> exact VA window receipt
 -> local packet orchestrator
 -> reverse/disassembly work
 -> evidence review
 -> only then semantic promotion
```

The Python packet script is orchestration only. It does not open/map the PE itself and therefore does not create another executable-byte authority.

The protected distribution executable whose known SHA is `81c7...` is not an instruction-level reverse authority and must not be substituted for the analysis build.

## Validate the plan without proprietary bytes

```text
python scripts/reverse/extract_exe_window_packet.py \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --validate-plan-only
```

This checks schema, artifact identity shape, duplicate IDs, VA/size ranges, mode rules and known-body hash requirements. The same validation runs in Ubuntu and Windows CI.

## Acquire the packet locally

Build `dmc-rengine`, then run:

```text
python scripts/reverse/extract_exe_window_packet.py \
  --dmc-rengine <path-to-dmc-rengine> \
  --exe <canonical-analysis-dmc3.exe> \
  --expected-sha256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --output <private-output-directory>
```

Add `--hex` only for local reverse work that actually needs raw bytes. Raw executable-byte receipts are proprietary local evidence and must not be committed to the public repository.

The output directory is no-replace: if it already exists the run fails rather than overwriting prior evidence. Every successful child receipt is hashed into `packet.receipt.json`.

If any requested window fails SHA/PE/range acquisition, the packet is not published.

## Current packet targets

The profile plan groups current blocked/supporting targets across the three-layer roadmap:

### L2 / physical-provider support

- `0x140326D20` — physical mount anchor;
- `0x140327430` — resource mount resolution;
- `0x140327720` — path-existence/final-open caller context.

Primary gate: #204 exact type-0 physical provider after recovered `0x0C` normalization.

### L1 / ZIP and loose-container support

- `0x140328540` — ZIP/inflater stream initializer;
- `0x140328820` — known InflateRead neighbor/context anchor;
- `0x140328F50` — known ZipEntryRead neighbor/context anchor;
- `0x140328FE0` — compressed seek/reset/reinflate path;
- `0x14002DA40` — `.lst` synthesis/builder follow-up;
- `0x14002EF4D0` — shared staging/materialization helper.

Primary ledger: #100 / #55.

### L3 / Level-E readiness support

- `0x1401B84E0` — successful LoadedResource release path;
- `0x1401B8CA0` — state-aware record lookup/reuse context;
- `0x1401B8DC0` — state-2 finalizer / typed-ready transition lead;
- `0x1403051B0` — SCM post-load layout contradiction follow-up.

Primary ledger: #88 / #55. The state-2 finalizer is also directly relevant to future #209 instrumentation because Level-E needs consumer-ready evidence rather than a crash-free launch.

## Probe-window rule

Every current GDS packet entry is `mode = probe`.

The configured `0x400` size is **coverage for reacquisition**, not an asserted function length. A probe that starts at a known VA must never be relabeled as an exact function body solely because the bytes were acquired successfully.

Exact body promotion requires an independently evidenced boundary and body hash. When those become known, the plan may promote a target to `known-body` with `body_sha256`; the orchestrator then fails closed if the reacquired hash differs.

## After acquisition

Do not merge raw probe bytes or jump directly to C++ behavior.

For each target:

1. verify artifact and window receipt identity;
2. disassemble the acquired range locally;
3. establish actual function/callee boundaries and xrefs;
4. distinguish direct observation from inference;
5. update the owning issue with exact range/evidence status;
6. promote only semantics supported by the reacquired bytes;
7. keep unresolved tails explicitly unresolved.

Priority after acquisition remains roadmap-driven:

```text
#204 physical final-open chain
 -> #100 ZIP initializer / compressed seek where needed
 -> .lst dynamic semantics if activated by acceptance
 -> #88 state-writer / typed-ready ownership
 -> Level-E instrumentation/receipt support
```

This packet reduces evidence-acquisition friction. It does not change the rule that L1 reaches 100% only after the real-retail and original-game acceptance receipts are valid.
