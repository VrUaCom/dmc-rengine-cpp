# DMC3 EXE format runtime evidence

The local reverse guide with function-level findings is maintained on the F:
drive at `F:\Reverse-Core-Corpus\dmc3-stage-reverse\guides\EXE_FORMAT_RUNTIME.md`.

The verified runtime facts are:

- `FUN_1402db1f0` dispatches loaded objects by the three-byte prefixes `MOD`,
  `EFM`, `SCM`, `MRP`, and `SHW`.
- `FUN_1402f7a90` rebases EFM relative pointers and expands its flagged index
  stream; `FUN_1403051b0` performs the analogous SCM pointer/index processing.
- `FUN_1402e01a0` recognizes `.cam` and assigns runtime type `2`.
- The corpus contains 1,718/1,718 `SCM` and 508/508 `EFM` header-prefix matches.

Generated decompilation and native-reader reports stay outside the source repo;
only the sanitized conclusions and reproducible Ghidra scripts are tracked.
