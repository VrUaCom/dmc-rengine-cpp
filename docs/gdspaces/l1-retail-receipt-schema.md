# GDSpaces L1 direct-retail receipt contract

Status: canonical acceptance contract candidate
Layer: 1 — Resource Materialization

## Purpose

Define the evidence record required to promote a direct-retail DMC3 materialization/edit/rebuild run. This is deliberately stricter than a successful parser invocation or synthetic round-trip.

## Receipt chain

A valid receipt MUST bind all of the following identities in one run:

1. Retail archive artifact
   - path used for observation;
   - byte size;
   - SHA-256 from the artifact-bound observation stream.
2. NBZ central entry
   - central index;
   - logical path/name bytes as indexed;
   - compression method and flags;
   - CRC32;
   - compressed size;
   - uncompressed size;
   - local-header/data offset.
3. Materialized member
   - byte size;
   - SHA-256;
   - transform (`zip_stored` or `zip_deflate`);
   - provenance span from the bound archive.
4. PAC/PNST representation when applicable
   - magic/family;
   - declared slot count;
   - protected prefix/table boundary;
   - ordered slot offsets;
   - empty-slot set;
   - alias partition for equal non-zero offsets.
5. Target child
   - slot index;
   - original byte range and size;
   - original SHA-256.
6. Replacement
   - replacement byte size;
   - replacement SHA-256;
   - whether size changed.
7. Rebuilt PAC/PNST
   - rebuilt size and SHA-256;
   - canonical reparse succeeds;
   - declared slot count unchanged;
   - empty-slot set unchanged;
   - alias partition unchanged;
   - target child equals replacement bytes;
   - every untouched populated child is byte-exact to the source representation.
8. Authored NBZ artifact
   - output path is distinct from retail input;
   - output size and SHA-256;
   - publication is no-replace/fail-closed;
   - canonical GDSpaces reopen succeeds.
9. Rematerialization
   - selected authored central entry identity;
   - rematerialized member SHA-256 equals rebuilt PAC/PNST SHA-256;
   - reparsed target child SHA-256 equals replacement SHA-256;
   - untouched-child verification remains byte-exact after NBZ serialization.

## Failure rules

The run is invalid and MUST NOT be promoted when any of these occurs:

- archive SHA/size differs from the artifact-bound snapshot;
- selected central entry is ambiguous, encrypted, unsupported, or outside the archive;
- stored/materialized size or CRC does not match indexed metadata;
- PAC/PNST parsing requires guessed offsets or guessed alignment;
- target slot is empty or outside the declared topology;
- writer changes an untouched populated child;
- writer changes the empty-slot set or alias partition;
- output overwrites the retail artifact;
- authored NBZ cannot be reopened by canonical GDSpaces;
- any receipt field is reconstructed from a different archive observation after the fact.

## Authority boundary

This receipt proves product-side byte correctness for the observed retail artifact and authored copy. It does not prove Capcom offline-writer equivalence. Original-game consumption is a subsequent acceptance receipt and must be recorded separately.
