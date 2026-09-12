# The dialect that was declared and could never be returned

**Corpus:** the complete `em000` extraction supplied 2026-09-08.
**Source archive SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`
**Pack:** `em000_041.pnst` — slot 0 is the manifest, slot 1 the records it names.

## What was wrong

`TextResourceDialect::effect_manifest` has existed since the text dialects were
recovered on 2026-09-08. It is in the enum and it is in `to_string`, which maps
it to `effect-manifest`. `TextResourceDialects::identify()` had no branch that
produced it.

So the enumerator was unreachable, and slot 0 of every effect pack in the
corpus classified as `txt` — the one resource whose entire content is the names
of the records beside it, listed as untyped text.

This is the project's second recurring defect, the one that is not a false
reading but an absent route: something implemented, correct, and unreachable.
The SO family and both texture framings were the same shape in
`dmc3-so-family-and-texture-reach-2026-09-09.md`, where nine of the corpus's
302 payloads were readable by parsers already in the tree and never offered to
them.

A test had made it worse rather than caught it. `text_resource_dialect_tests`
used `G 13\r\nG 75\r\nG 152\r\n` as its example of *text this project cannot
place*, and asserted that the classifier answers `bin` for it. That payload is
an effect manifest. The assertion pinned the gap as though it were the correct
answer, in both this repository and the Android tree that vendors it.

## The grammar, and where it now lives

A manifest is CRLF ASCII: one `<kind> <decimal identifier>` line per record,
`#` comments, and a `# End` terminator. `EffectPackContract` has carried those
constants since pass #254; the line reader that used them was a static function
inside `formats/effect_pack.cpp`.

Identification needs the same rule. Writing it a second time in
`text_resource_dialects.cpp` is how a slot comes to be a manifest to one reader
and not to the other, so the rule moved onto the contract instead:

```cpp
EffectPackContract::read_manifest_line(std::string_view)
    -> ManifestLine{line_kind, kind, identifier}
```

`formats/effect_pack.cpp` walks a manifest with it and the dialect probe judges
a slot with it. The identifier is parsed by hand rather than with `from_chars`
so the whole grammar is constant-evaluable and can be pinned by `static_assert`;
an identifier too wide for the field is `invalid` rather than a wrap, because a
line naming a record number the file does not contain is not a line.

## The asymmetry between reading and identifying

The pack reader has already been handed a manifest and only has to walk it, so
it admits any single-character kind. The dialect probe has to decide whether an
unnamed slot *is* a manifest, and admits only the seven kinds the corpus holds
(`V E P T A G M`).

It also requires either two record lines, or one closed by the terminator. One
line alone is a coincidence three bytes of anything can produce — `A 1` followed
by a NUL is not evidence of a format.

Only whole lines count. The probe window is 64 bytes and a manifest is longer,
so its last line is usually cut; judging a fragment would make the verdict
depend on where the window happens to land.

## What this does not establish

Nothing here changes what pass #254 said and
`dmc3-effect-pack-companions-2026-09-10.md` repeated: **no original executable
read site for this text has been found.** What is recovered is the pack's own
arithmetic and the shape of its manifest. That the game reads these names is
not claimed, and `EffectPackContract::manifest_read_site_found` stays `false`.

The manifest names the records. It says nothing about its own filename, and
none is synthesized for it.
