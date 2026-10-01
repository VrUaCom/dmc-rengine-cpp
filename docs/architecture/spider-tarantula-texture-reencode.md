# Spider Tarantula: texture format change

Family: **Tarantula** (see `spider-family.md`). Date: 2026-10-01.

## What it replaces

The first BC7 re-encode of Dante's `pl000.pac` was a scratch Python
prototype: `ptx_to_bc7.py`, using numpy, the `etcpak` (bc7e) wheel and a
helper executable that dumped mip levels. It was never in the repository.
The Tarantula workflow does the same job natively. It also covers every
format, PAC slot selection, single gfxTexture files and safe publication.

## Workflow

```text
acquire [io]            read the file once (or take bytes in memory)
 -> inspect [cpu]       container kind; PAC texture slots (texture_payload)
 -> transform[i] [cpu]  reencode_payload for PAC slot i (operand = slot)
 -> assemble [cpu]      replace_pac_slots / single payload
 -> validate [cpu]      list_textures: every touched texture in the target
                        format; every untouched PAC slot byte-identical
 -> publish [io]        core::publish_bytes_no_replace (staged and
                        validated); --replace to overwrite
```

The steps run as two Crusader plans on the native executor, because the
transform fan-out depends on what `inspect` finds.

- **Plan 1:** acquire → inspect.
- **Plan 2:**
  - transform × N;
  - assemble, which depends on all N transforms;
  - validate → publish.

**Algorithms stay in their modules:**

- `codecs::dds_bcn` and `codecs::dds_bcn_encode`: decoding and encoding;
- `profiles::dmc3::texture_reencode`: the gfxTexture, PTX and PAC rules;
- `core::no_replace_publication`: writing the output.

The workflow code is coordination and per-step receipts only.

CLI: `dmc-rengine texture-reencode <in> <out> --format <name> [--dx10]
[--slot N] [--replace]` prints each step as `[OK]` or `[FAIL]`.

## Migration evidence (pl000.pac, 1 335 280 bytes, slot 0 to BC7)

| | Python prototype | Tarantula |
|---|---|---|
| Wall time (3 runs) | 1.70–1.75 s | 0.64–0.70 s |
| Dependencies | Python 3, numpy, etcpak, helper executable | none (C++20) |
| Output size | 1 335 280 | 1 335 280 |
| gfxTexture headers | — | identical to Python |
| Bytes outside slot 0 | unchanged | unchanged, identical to Python |
| Level-0 PSNR vs DXT5 source (texture 0) | 53.6 dB | 56.3 dB |
| Formats | BC7 only | BC1, BC2, BC3, BC4/BC5 (UNORM, SNORM), BC6H, BC7 |

**DDS header difference.** The only DDS header field that differs is
`pitchOrLinearSize`:

- Python copied the retail value `0x20000`;
- Tarantula writes the spec value for level 0, `0x40000`.

DirectXTK (the game's loader) does not read it.

Compressed blocks differ by design, since the encoders differ.

## Viewer counterpart

Native Reader runs the same re-encode as a Spider action. It uses the
Reader's own Crusader kernel, with a plan of four steps:

1. select source;
2. re-encode;
3. open the result;
4. verify.

Black Widow flags `CanReencodeTextures` and `CanSaveSource` drive its menu.
The Reader's output is byte-identical to this CLI.
