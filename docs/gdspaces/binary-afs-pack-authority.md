# Binary AFS/PACK authority boundary

Snapshot: 2026-08-29
Scope: GDSpaces Layer 1 / DMC3 HD

## Corrected model

`GData.afs/` and `GDataX360.afs/` are logical resource namespaces on the
canonical DMC3-HD path. The runtime builds those candidate strings and resolves
them through the mounted NBZ/ZIP provider or the physical provider. The name
does not introduce a second binary AFS decode stage.

The file `DMC 3 RENGINE (6).zip` is the Web DMC Rengine product source. Its
`parsePackArchive()` implementation is useful product-implementation input, but it is
not an original game archive, a raw PACK corpus receipt, or executable parser
authority. Its unnamed fields must remain hypotheses.

## Reproduced evidence receipt

Artifacts inspected in this correction pass:

- canonical analysis `dmc3.exe`: 6,356,432 bytes, SHA-256
  `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- Web DMC Rengine source package `DMC 3 RENGINE (6).zip`: 237,658,858
  bytes, SHA-256
  `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`.

The Web source package contains `parsePackArchive()`. That locates the product
hypothesis exactly, but does not change its evidence class.

A first-four-byte census over all 2,239 packaged
`analysis_inputs/stage_drops` files produced:

- `PAC\0`: 32;
- `PNST`: 21;
- `PACK`: 0;
- `AFS\0`: 0.

These packaged analysis inputs are not a direct-retail provenance receipt. The
zero counts are bounded negative evidence for this exact packaged corpus only.

The canonical executable whole-image literal scan produced:

- `PACK`: 0;
- `AFS\0`: 0;
- `GData.afs/`: 1;
- `GDataX360.afs/`: 1.

Literal absence alone cannot reject a binary backend because the executable may
dispatch without an inline magic comparison. Combined with the recovered
two-provider resolver, however, it does not supply the missing AFS/PACK parser
or writer authority.

## Product identities

| Identity | Meaning | Expandable | Writable |
|---|---|---:|---:|
| `afs-namespace` | DMC3-HD logical resolver namespace | no | through the selected NBZ member path only |
| `afs-binary-candidate` | possible profile-specific raw AFS artifact | no | no |
| `pack-binary-candidate` | possible raw PACK artifact | no | no |

The byte classifier may retain `AFS\0` or `PACK` prefix candidates. Prefix
recognition alone must not register a parser, recursively expose entries, allow
a WorkingCopy, or advertise an authoring mode.

## Promotion gate for a binary parser

A binary AFS or PACK parser may be promoted only when all applicable items are
bound together:

1. supported game/profile and hash-bound raw artifact;
2. byte-range-backed header/table/entry layout;
3. bounded malformed-input behavior;
4. exact entry offset/size/name identity and transform rules;
5. corpus receipt produced by the same parser revision;
6. original consumer/backend evidence when original-runtime authority is
   claimed.

## Additional gate for authoring

A writer additionally requires:

1. immutable source identity and explicit supported mode;
2. preservation rules for every unknown header/entry field;
3. byte-exact no-edit round trip;
4. changed-entry rebuild with untouched-region proof;
5. reopen through the canonical parser;
6. profile-specific original-game consumption and rollback receipt.

Until those gates are satisfied, binary AFS/PACK authoring is not an incomplete
DMC3-HD writer feature. It is a separate evidence-gated compatibility track.
The supported DMC3-HD namespace authoring path remains:

```text
logical request
 -> selected GData*.afs/<member> identity
 -> PAC/PNST bounded authoring
 -> next-contiguous NBZ overlay
 -> canonical resolver/reopen/rematerialize
```
