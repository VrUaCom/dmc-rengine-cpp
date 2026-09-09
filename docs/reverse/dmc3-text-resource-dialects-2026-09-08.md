# DMC3 text resources — "txt" is an encoding, not an identity

**Corpus:** a complete `em000` extraction supplied 2026-09-08.
**Source archive SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`
**Payloads examined:** 302 extracted records; 9 of them readable ASCII.

## What was wrong

Nine records in the corpus are text. All nine were reported as `txt`, which is
a statement about their encoding and about nothing else. Eight are cloth
definitions and one is a scroll table — two different resources, typed by two
different codes in the registry that owns them, collapsed into one label
because the reader stopped at "these bytes are printable".

The collapse is not cosmetic. It is why a browser walking a `.pac` shows a
column of identical rows where the payloads are not identical, and it is why an
extraction of those slots produced files no tool downstream could route.

## Why the name was not available

The runtime types these through the **second** resource registry
(`profiles::dmc3::AnimationTypeContract`, `0x1402E01A0`), and that registry has
no content probe at all: it runs `strstr` over the resource's name and refuses
a name it cannot match. A payload that arrives without a name is therefore not
typed by the game either — which is exactly the situation a container slot is
in, since a slot carries an offset and an extent and no name.

So the identity cannot come from the runtime's own path. It has to come from
the payload.

## Both dialects announce themselves

| dialect | marker | position | count |
|---------|--------|----------|-------|
| CLT | first line is `;<name>.clt` | 0 | 8 |
| TSC | a `.TSC` tag line | 2 | 1 |

A CLT opens with its own filename as a comment. That is worth more than the
type: it is an **embedded original name** — not a placeholder this project
synthesized and not a line an extraction tool wrote beside the file, but the
resource naming itself in its own bytes.

It is also the only correct name available, because the slot disagrees with it.
In `em000`, slots 6, 9, 11, 14, 16, 20 and 22 hold cloth belonging to `em001`,
`em002`, `em003` and `em005` — shared definitions packed into a neighbour's
container. A name derived from the enclosing container would have been wrong
for **seven of the eight**.

## The marker is only evidence when the bytes are text

The TSC marker is searched within the opening window rather than anchored,
because the blank first line is a property of the observed payloads and not
obviously of the format. A searched four-byte literal turns up in binary, so
`TextResourceDialects::identify` first requires the probe window to read as
text and refuses the dialect otherwise. A binary payload carrying the bytes
`.TSC` is not a scroll table, and the test suite pins that.

## What this does not establish

Neither dialect's records are parsed. The probe settles identity — which
resource this is, and for CLT what it was called — and nothing about content.
Both remain `recognized` in the format registry, with no structural parser.
