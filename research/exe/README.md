# Executable extractors

Small, single-purpose readers of the canonical research target that recover a
specific structural fact and print it as JSON. They exist so that a table in a
note is regenerated rather than transcribed.

The executable is not part of this repository and is never committed. Every
script here takes a path to a local copy and refuses one whose SHA-256 is not

```text
e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
```

unless `--allow-any-image` is passed. Their output is not committed either;
what is committed is the script, its tests and the note that cites it.

## `extract_record_decoders.py`

Recovers the field map of each byte-at-a-time record decoder in the archive
reader: for every field, the offset it reads in the record, its width, and the
offset it is stored at in the output struct.

```sh
python3 research/exe/extract_record_decoders.py /path/to/dmc3.exe
```

It decodes only the four instruction forms those functions use and is
fail-closed on everything else: an unrecognised byte inside a requested range
aborts naming the offset rather than being skipped, because a skipped
instruction would silently move every field after it. Guarding against being
*approximately* right is the whole point, so the guards are mutation-tested.

Tests need no executable:

```sh
python3 research/exe/test_extract_record_decoders.py
```

Cited by `docs/reverse/dmc3-zip-directory-2026-09-18.md` and evidence packet
`dmc3-hdc-zip-directory`.
