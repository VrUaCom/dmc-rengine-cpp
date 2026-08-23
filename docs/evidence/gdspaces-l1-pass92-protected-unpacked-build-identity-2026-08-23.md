# GDSpaces L1 Pass 92 — protected/unpacked build identity — 2026-08-23

Scope: **Layer 1 — Resource Materialization / provenance boundary**  
Status: **same linked DMC3 image VERY HIGH; protection-state distinction confirmed; direct NBZ member receipt still open**

## Purpose

Earlier Layer-1 notes treated the connected vanilla distribution executable and the canonical reverse executable as a possible build-pair mismatch because their file SHA-256 values differ:

- protected distribution `dmc3.exe`: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes;
- canonical analysis `dmc3.exe`: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.

Pass 92 compares both complete PE images directly. The result materially narrows that boundary: the evidence supports **one linked DMC3 image represented in two protection states**, not two independently linked game builds.

This does not reconstruct the protector or prove the historical derivation command that produced `e454...`; it proves the bounded PE/image identity stated below.

## Source provenance

Connected vanilla Drive ancestry:

```text
DMC HD Vanilla/
  Devil May Cry HD Collection/
    dmc3.exe                         protected distribution image
    data/
      dmc3/
        dmc3-0.nbz                  retail resource volume
```

Drive identities:

- distribution EXE ID: `1-GouJYQyltnQqXMd0BvxEmOh9a1RQg1q`;
- distribution EXE size: 6,567,320 bytes;
- distribution EXE SHA-256 after fresh streamed download: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`;
- `dmc3-0.nbz` ID: `1dnYSWy1bqrMX_VEST9CRlno_38u7LlhQ`;
- `dmc3-0.nbz` size: 960,358,951 bytes.

The NBZ whole-file connector download remains blocked by the 268,435,456-byte raw-download ceiling.

## PE identity comparison

Both executables are PE32+ x86-64 and have identical:

- linker timestamp: `0x5AB4CBC2` / 2018-03-23 09:41:22 UTC;
- ImageBase: `0x140000000`;
- BaseOfCode: `0x1000`;
- SizeOfCode: `0x34DE00`;
- SizeOfInitializedData: `0xA59600`;
- section alignment: `0x1000`;
- file alignment: `0x200`;
- import-directory RVA/size: `0x5505D8 / 0x21C`;
- export-directory RVA/size: `0x550570 / 0x68`;
- resource-directory RVA/size: `0xD99000 / 0xF30`;
- exception-directory RVA/size: `0xD73000 / 0x23D84`;
- relocation-directory RVA/size: `0xD9A000 / 0x11DB0`;
- debug-directory RVA/size: `0x50CDF0 / 0x54`;
- TLS-directory RVA/size: `0x50CEE8 / 0x28`;
- load-config RVA/size: `0x50CE50 / 0x94`;
- IAT RVA/size: `0x34F000 / 0x7F8`.

The common section layout is identical for every base section.

| Section | RVA | Virtual size | Raw offset | Raw size | canonical SHA-256 | protected SHA-256 | Result |
|---|---:|---:|---:|---:|---|---|---|
| `.text` | `0x1000` | `0x34DD4E` | `0x400` | `0x34DE00` | `6b16d64ea8da9e3a…` | `3cea9ebbac6dadf0…` | same geometry, transformed bytes |
| `.rdata` | `0x34F000` | `0x203368` | `0x34E200` | `0x203400` | `aedd3e459caf750b…` | same | byte-identical |
| `.data` | `0x553000` | `0x81F1B8` | `0x551600` | `0x86000` | `d7142c9acb49ed48…` | same | byte-identical |
| `.pdata` | `0xD73000` | `0x23D84` | `0x5D7600` | `0x23E00` | `ee741ad2cb757d12…` | same | byte-identical |
| `.gfids` | `0xD97000` | `0x50` | `0x5FB400` | `0x200` | `5879746c15b886a3…` | same | byte-identical |
| `.tls` | `0xD98000` | `0x09` | `0x5FB600` | `0x200` | `4c6474903705cb45…` | same | byte-identical |
| `.rsrc` | `0xD99000` | `0xF30` | `0x5FB800` | `0x1000` | `33293edd26e64747…` | same | byte-identical |
| `.reloc` | `0xD9A000` | `0x11DB0` | `0x5FC800` | `0x11E00` | `6be381c6475fd8ea…` | same | byte-identical |

Exact full section SHA-256 values:

```text
.rdata aedd3e459caf750b8d3f98a56fd5cfae12e5081569d9108d20865b38c1061848
.data  d7142c9acb49ed48012c19b0ec5b873286598c6a9b99a1b67a5cfcb140018bce
.pdata ee741ad2cb757d12a4da94476064eb4e2852841044711f7172189c59e077d270
.gfids 5879746c15b886a3ac27971ad99e2ca3b4abd9b0b11984f59ece7d12b27fa6f4
.tls   4c6474903705cb45a4652ed0f2effcc52fd47c6719ecc59644fb323a96b54b6a
.rsrc  33293edd26e64747cca5f5979b9ca73ea915f8738780ec71226703e72e6bbd8b
.reloc 6be381c6475fd8ea99fa51ecf33659273ce6d15f3c35495449ce787616bedc0d
```

The only common base section whose bytes differ is `.text`.

## Protection-layer evidence

Canonical `e454...`:

- 8 sections;
- entry point `RVA 0x34615C`, inside the original `.text` image;
- `.text` entropy approximately `6.3612`.

Protected distribution `81c7...`:

- the same 8 base sections plus a ninth `.bind` section at `RVA 0xDAC000`;
- entry point moved to `RVA 0xDAC310`, inside `.bind`;
- `.text` has the same RVA, virtual size, raw offset and raw size as canonical but entropy approximately `7.99994` and only ~0.38% equal bytes at corresponding offsets;
- `.bind` entropy approximately `7.95544`;
- SizeOfImage extends from canonical `0xDAC000` to protected `0xDDF000` to accommodate `.bind`;
- the protected image has a different Authenticode/security-table envelope, as expected for a protected distribution file.

Crucially, protection did **not** shift the original section RVAs or mutate `.rdata`, `.data`, `.pdata`, `.gfids`, `.tls`, `.rsrc` or `.reloc`.

## Corrected build identity

The prior broad phrase **build-pair mismatch** is too strong.

Evidence-backed replacement:

> `81c7...` and `e454...` are VERY HIGH-confidence representations of the **same linked DMC3 image in different protection states**. `81c7...` is the protected distribution representation; `e454...` is the deprotected/unpacked analysis representation used for instruction-level reverse.

This conclusion is based on complete common-section geometry equality, byte identity of every non-`.text` common section, identical linker timestamp and identical PE data-directory topology, while the protected image encrypts/transforms the unchanged-geometry `.text` and appends `.bind`.

The exact historical derivation chain from this specific Drive `81c7...` file to the preserved `e454...` file is **not recorded**, so do not claim a cryptographic transform receipt or byte-for-byte reconstruction of the protector.

## Layer-1 consequence

The connected `dmc3-0.nbz` is no longer reasonably classified as “possibly from another DMC3 build” merely because the co-located protected EXE has a different SHA from `e454...`.

Correct provenance split:

```text
same linked DMC3 product image
  protected distribution state: 81c7... + co-located dmc3-0.nbz
  deprotected analysis state:    e454... instruction/runtime reverse authority
```

Therefore the remaining retail texture gate is narrowed to **member-byte acquisition**, not game-version ambiguity.

Still required before promoting a concrete texture resource to direct-retail byte authority:

1. extract or otherwise obtain the exact `GData.afs/<resource>.pac` bytes from the 960 MB connected NBZ;
2. hash that member and preserve its NBZ/file-ID/member-path lineage;
3. compare it against the preserved transformed DDS-bearing corpus;
4. if it matches, promote the resource provenance accordingly;
5. if it differs, identify the transformation boundary without collapsing the two representations.

## Hard freeze

- Do not use protected `.text` bytes for canonical VA instruction analysis.
- Do not claim the exact protector/decryptor algorithm or exact `81c7 -> e454` derivation receipt.
- Do not claim a concrete retail PAC SHA until that member's bytes are actually acquired.
- Do not treat the 256 MiB connector limit as an archive-format limitation; it is an acquisition-tool limitation.
- Do not reopen generic NBZ runtime compatibility merely because the on-disk protected EXE has a different SHA.
- Layer 1 remains NOT COMPLETE until concrete retail-member and original-game modified-writeback gates are closed.
