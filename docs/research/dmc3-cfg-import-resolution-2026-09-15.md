# Canonical CFG import resolution — 2026-09-15

Branch: Ада-Астра. Source SHA-256:
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Input: the verified `exe-cfg-20260914` traversal, not a new full-game CFG.

## Results

| Indirect operand category | Calls | Jumps |
|---|---:|---:|
| Confirmed normal IAT slot | 524 | 108 |
| Register, target unresolved | 32 | 634 |
| Other memory, target unresolved | 8,081 | 640 |
| RIP-relative non-IAT slot | 0 | 1 |
| Total | 8,637 | 1,383 |

Of 10,020 visited indirect sites, 632 reference normal import-table slots,
leaving **9,388 unresolved indirect sites**. The 632 references cover 220 of
the 229 parsed imports. Absence from this visited subset does not prove an
import unused by the game.

Another **867 direct call/jump sites** target a visited instruction that itself
is an unconditional RIP-relative IAT jump. These are recorded separately as
one-hop import thunks. They do not subtract from the indirect-site remainder
and do not resolve arbitrary wrappers or register-mediated dispatch.

Evidence status **EXE_CONFIRMED** applies to import metadata and FF operand
address calculations. Runtime execution, semantic purpose of callers, virtual
method identity and final loaded function addresses are not established.
In particular, other-memory dispatch is not automatically a vtable call.

## Remaining RIP-relative slot

`0x140346714` is a seven-byte REX.W-prefixed indirect jump. Its signed RIP
displacement resolves to memory slot `0x14034F7F8`, outside the parsed normal
IAT. The target remains unresolved; no import or function name is assigned.

## Reproduction and verification

```sh
python scripts/reverse/resolve_cfg_imports.py /path/to/dmc3.exe \
  data/reverse/exe-cfg-20260914 /tmp/dmc3-imports
```

Python standard library only. The source hash is checked before output.
The parser handles REX/legacy prefixes and signed RIP displacements; address
override and FS/GS-relative operands are not treated as ordinary RIP slots.

All 633 computed RIP slots (632 imports plus the unresolved slot) independently
match GNU objdump annotations. Seven operand cases exercise negative
displacements, REX, registers, other memory, address/segment overrides and
unsupported instructions. Input CFG files match their committed Git blob
hashes. Machine-readable output records input SHA-256 hashes, per-site
classifications, thunk references, import frequencies and verification receipt.

Next: identify the non-IAT slot's initialization; join memory-indirect sites
with receiver/type evidence and vtable anchors. Neither zero decoding errors
nor normal-import resolution establishes full reverse or recompilation.
