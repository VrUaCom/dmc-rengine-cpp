# Known Executable Targets

Known-target metadata is separated from the generic PE parser.

## Generic model

`exe::KnownExecutableTarget` contains:

- stable target ID;
- display name;
- SHA-256;
- expected PE kind;
- expected machine;
- expected image base;
- expected entry-point RVA.

It supports two independent checks:

1. `matches_hash()` — artifact identity;
2. `matches_metadata()` — parsed structural consistency.

A hash match does not bypass PE parsing or structural validation. If the hash matches but parsed metadata differs, the CLI reports a warning.

## DMC3 canonical Phase 12 target

Profile registry:

```text
dmc::rengine::profiles::dmc3::phase12_canonical_target()
```

Recorded metadata:

- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- PE kind: PE32+;
- machine: AMD64;
- image base: `0x140000000`;
- entry-point RVA: `0x34615C`;
- entry-point VA: `0x14034615C`.

The public Evidence Packet is stored at:

- `evidence/known-targets/dmc3-hdc-phase12.evidence.json`.

## CLI behavior

```bash
dmc-rengine inspect-exe <path>
```

The command:

1. reads the file through GDSpaces;
2. calculates SHA-256;
3. parses PE metadata generically;
4. compares the hash to known targets;
5. checks expected metadata;
6. prints recognition status and sections.

## Extension policy

Future target versions must be added through profile-specific registries and Evidence Packets. The generic PE reader must not accumulate DMC3-specific offsets, patches, or stage knowledge.
