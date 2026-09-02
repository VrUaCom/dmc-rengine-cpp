# What the unpacker actually reaches, measured — 2026-08-28

Asked whether the index work is a system or something fitted to the corpus, and
told that the foundation is cracking under the naming work. Both deserve a
measurement rather than an assurance.

## The measurement

Running the **product** path — the registry parser plus `ContainerExpander`,
recursing as deep as it goes — over the six-container corpus:

```
containers expanded: 15
leaves reached:      91
absent slots:        27
max depth:            2
```

The 91 leaves type as:

| format | n | | format | n |
|---|---|---|---|---|
| dds | 34 | | pos | 2 |
| txt | 8 | | sef | 2 |
| scm | 7 | | ste | 2 |
| hits | 4 | | cam | 2 |
| lig2 | 2 | | itm | 2 |
| dca | 2 | | eve | 2 |
| mot | 1 | | est | 1 |
| **unknown** | **20** | | | |

## The 20 are not twenty unknown formats

They are **one family**: every untyped leaf is an effect record, and the corpus
has exactly two effect containers.

```
st001_effect.pac/slot_0001.pnst/  9 records
st114_effect.pac/slot_0001.pnst/ 11 records
```

Their kinds are known — `V`, `E`, `P`, `T`, `A` — and their extents are known
and corroborated across both files. What is unrecovered is the **inside** of a
record. So the honest statement is not "we do not know all the file types". It
is "one format family, five kinds, layout unrecovered", which is a much smaller
and more tractable thing.

## The crack was real, and it was a layering crack

The measurement found those 20 leaves displayed as `slot_0000.bin`, while the
Android app showed `V 922` for the same slot.

Both were running the same core. The difference was that the rule which applies
an effect container's stored names lived in **one application's session layer**.
A phone had it; a CLI, a test harness and every other consumer of this library
did not.

That is the shape of the problem, and it is worth naming precisely: **a naming
rule that only one caller has is not a naming rule.** The foundation was not
cracking because the recoveries are wrong. It was cracking because the
recoveries were being wired in at the top.

### The fix

`ContainerExpander::expand` now takes a `ContainerNamingContext`:

```cpp
struct ContainerNamingContext {
    std::span<const std::byte> enclosing_container;
    std::uint32_t slot_index_within_enclosing;
};
```

Some containers are named by their *enclosing* one — an effect pack writes its
record names in the outer container's slot 0 and they name the slots of the
inner container in slot 1. The expander sees one container at a time and cannot
reach across, so the caller, which holds the enclosing bytes, brings them. The
**rule** stays in the core; only the bytes travel.

The application's copy of the rule is deleted. There is one authority.

## Is it a system, or fitted to these files?

The probe and the expander decide by structure, never by filename:

- a container is a container because its magic compares and its offset
  arithmetic closes;
- an index is an index because its lines parse and its count matches the slots
  it claims to name;
- a name becomes a display name only when the payload's independently read type
  agrees with it.

Over 15 containers it found four indexes and correctly refused the other
eleven, including every `cfg` container, whose slot 0 is text that names
nothing.

**But the honest limit:** max depth in this corpus is 2. Nothing here is
evidence about deeper nesting, and a claim that it works at depth 5 would be a
claim about files nobody has. The mechanism has no depth limit; the *evidence*
stops at two.

## What this leaves open

| gap | what would move it |
|---|---|
| effect record internals, 5 kinds | reading the handlers, or a corpus with variation |
| depth beyond 2 | an archive that nests deeper |
| external `.index` shape | one real file from an unpacked folder |
| `.lst` against real data | one real `.lst` |

None of these is answered by more reasoning about the files we have.
