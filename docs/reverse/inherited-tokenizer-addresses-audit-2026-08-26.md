# Seven inherited addresses do not name a function — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

`docs/history/migrated-findings.md` records ten addresses for the DMC3 text
tokenizer, each with a label. They have sat there as recorded facts and none of
them is in code, which is why nothing has checked them until now.

**Three of the ten name a function entry. Seven land inside one.**

| claimed | label | verdict | enclosing function |
|---|---|---|---|
| `0x140322CA0` | init | entry | — |
| `0x140322CB0` | clear | entry | — |
| `0x140322CC0` | advance | entry | — |
| `0x140322D40` | peek | **inside** | `0x140322CD0`, +0x70 |
| `0x140322D90` | read token | **inside** | `0x140322CD0`, +0xC0 |
| `0x140322E20` | read int | **inside** | `0x140322CD0`, +0x150 |
| `0x140322E70` | read float | **inside** | `0x140322E40`, +0x30 |
| `0x140322F10` | read string | **inside** | `0x140322EF0`, +0x20 |
| `0x140322FB0` | skip whitespace/comments | **inside** | `0x140322F20`, +0x90 |
| `0x140323050` | parse `#SET` | **inside** | `0x140323030`, +0x20 |

Two of the seven are not even instruction boundaries: `0x140322D40` decodes as
`add byte ptr [rax], al` — the `00 00` of data — and `0x140322FB0` does not
decode at all.

## How they were found

Function entries in this image are separated by `int3` padding. Scanning
`0x140322C00`–`0x140323200` for a `0xCC` followed by a non-`0xCC` gives twelve
real entries:

```text
0x140322CA0  0x140322CB0  0x140322CC0  0x140322CD0
0x140322E40  0x140322E80  0x140322EF0  0x140322F20
0x140323030  0x140323080  0x140323110  0x140323170
```

Three claimed addresses are in that set. The other seven are 0x20 to 0x150 past
an entry, which is the signature of an address read off the middle of a
disassembly listing rather than from its top.

## What is and is not being said

The **labels** are not disputed here. `0x140322CD0` may well contain a peek, a
token read and an int read — three claims point into it, and a tokenizer core
plausibly does all three. What is wrong is the addresses: they do not name
callable functions, so none of them can be used as a call target, a byte-window
anchor, or a receipt subject.

Also unchecked: the claimed labels have no receipts and no reading behind them
in this repository. They are inherited.

## Why this matters beyond ten numbers

This is the concrete cost of the documented-but-unintegrated gap the ledger
measures. An address that never becomes code is never exercised, so nothing
ever contradicts it, and it accumulates the authority of having been written
down. Forty-four addresses are still in that state.

The cheap defence is the one already in use everywhere else: bind a recovered
address to an image hash with a byte-window receipt, and put it in a contract
where a `static_assert` can reach it. A receipt cannot be taken over a
mid-function address without noticing, because the window would not begin at a
prologue.
