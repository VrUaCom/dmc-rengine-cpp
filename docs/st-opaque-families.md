# `st*` opaque-family inventory

The exhaustive F-drive inventory is maintained at
`F:\Reverse-Core-Corpus\dmc3-stage-reverse\guides\ST_OPAQUE_FAMILIES.md`.

Current confirmed classifications:

- 281 `.ukn` files carry the `HITS` collision header.
- 189 `.ukn` files carry `DCA\0` and pass the Native Reader structural DCA
  scanner.
- 189 `ITM\0` and 189 `STE\0` files are identified by their headers; their
  semantic parsers are still open.
- 112 `EST\0` files are identified by header; semantic parsing is still open.
- All 359 stage `.mod` files carry `MOD ` and use the structural MOD route.
- `.bd` and `.so` have stable 16-byte record strides but no proven field names.

The source binaries and generated per-file reports stay on F:. Only this
sanitized inventory and the reproducible scanner are tracked in the repository.
