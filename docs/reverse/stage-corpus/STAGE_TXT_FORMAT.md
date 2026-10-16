# DMC3 stage TXT reverse guide (corpus phase 3)

Status: **confirmed lexical pass**

## Corpus result

- TXT resources in `st*` scope: **524**
- Decode and lexical validation: **524/524**
- Encoding: UTF-8 or CP932; trailing NUL padding is stripped before lexing.
- Lexical errors: **0**
- `DOOR` tokens: **377**
- `NEXTROOM` tokens: **377**

## Reader facts

The canonical lexer is in `the repository\src\formats\stage_txt.cpp`.
It recognizes comments, quoted strings, identifiers, numbers, symbols, `#SET`, `DOOR`, `BOXIN`, `NEXTROOM`, and stage-set values `DUMMY`, `STAY`, `BREAK`, `ORBREAK`, `SEAL`, `SWITCH`, `YURE`.

The extracted corpus contains Japanese comments in CP932, so a UTF-8-only reader will report false failures. Some files have NUL padding after the logical text end; this is padding, not a binary stage format.

Evidence: `evidence/txt_validation.json`.

