# DMC3 MOT — a second track kind, and a stamp that is not signed

**Corpus:** a complete `em000` extraction supplied 2026-09-08.
**Archive SHA-256:** `0a56b0e3cf8e7dabd6f75e409db3ebf28d1fa482292c2d109d1cb6b4a6a1e67d`
**Payloads examined:** 82 motions across three nested containers, 5,118 tracks.

## What was wrong

The MOT layout was first recovered from a single payload — `st001.pac` slot 7 —
and two of its properties were written down as the format's when they were that
payload's.

Against the full corpus this reader accepted **10 of 82** real motions. The ten
were not a well-behaved subset; they were simply the ten that happen to contain
no track of the second kind.

## Track geometry is the kind's, not the format's

The track header's third field is a kind. Two values occur:

| kind | header | key | tracks | violations of `size == header + keys x key` |
|------|--------|-----|--------|-----|
| 3    | 32     | 8   | 4,962  | 0 |
| 2    | 16     | 4   | 156    | 0 |

Every one of the 5,118 tracks satisfies its own kind's arithmetic exactly. The
reader applied kind 3's to all of them, so a single compact track anywhere in a
payload refused the whole motion.

Kind 2's six observed shapes: `(20,1) (24,2) (28,3) (32,4) (36,5) (56,10)`.

## The key stamp is fifteen bits under a flag

The leading 16-bit value of a key was read as a signed stamp, which is why the
first stamp was recorded as `-32768`. That is `0x8000`: a flag bit over a stamp
of zero.

Read as `raw & 0x7FFF`:

- all 5,118 tracks are strictly increasing, against three payloads that looked
  corrupt before;
- 4,915 of the 4,997 multi-key tracks span exactly the duration the header
  declares twice — the same closure the original single-payload recovery
  found, now with an explanation rather than a coincidence;
- the top bit is set on 94,410 of 94,875 keys and clear on 465, so it carries
  something. What it carries is not recovered here and is not claimed.

## The four-byte terminator is padding

The chain was required to end exactly four bytes short of the payload. Across
82 payloads the tail after the last track is 0, 4, 8 or 12 zero bytes, and
every payload's size is an exact multiple of 16. It is alignment padding; the
one payload the rule came from happened to need four bytes of it.

The reader now requires the chain to reach within one alignment unit of the
end, every tail byte to be zero, and the payload size to be a multiple of 16 —
which still refuses a stray trailing byte.

## Result

82 of 82 real motions parse. The arithmetic is pinned in
`tests/mot_structural_tests.cpp` against literal (size, key count) pairs
counted off this corpus, not against the contract's own formula.
