# How to Unpack Devil May Cry 3 HD Collection Files

If you are searching for a **Devil May Cry 3 unpacker**, **DMC3 HD extractor**, or a way to inspect files inside the HD Collection, the important first step is to identify which layer of the resource stack you are actually opening.

DMC Rengine does not treat the game as one universal archive format. Its evidence-backed resource work separates numbered archives, container framing, slots and final resources so tools do not lose provenance or invent filenames.

## Resource path

The current research model is broadly:

```text
DMC3 HD game data
  -> numbered NBZ archive
  -> materialized archive/container data
  -> PAC / PNST and nested container layers where present
  -> individual resources
  -> model / texture / stage / configuration / other format reader
```

The exact path depends on the resource. Do not assume every file passes through every container family.

## What "unpack" can mean

People often use *unpack* for several different operations:

1. opening a numbered DMC3 HD archive;
2. extracting/materializing a PAC or PNST container;
3. selecting a slot or nested resource;
4. opening the final MOD, SCM, DDS, PTX or another resource;
5. rebuilding or repacking modified data.

DMC Rengine keeps those operations separate. Successful extraction does not automatically mean that arbitrary modified data can already be repacked and accepted by the original game.

## Where to go next

- **NBZ / numbered archives:** see [`../gdspaces/`](../gdspaces/) and the canonical GDSpaces/resource-materialization documentation.
- **PAC / PNST:** start from [`../formats/public-index.md`](../formats/public-index.md) and the current status.
- **Models:** MOD and SCM research is indexed from the format documentation.
- **Textures:** DDS/PTX recognition and texture-slot framing are documented separately; PTX and DDS are not treated as interchangeable identities.
- **Current capabilities:** check [`../status/current.md`](../status/current.md) before relying on a reader, writer, conversion or reintegration path.

## DMC Native Reader and the wider ecosystem

The canonical C++20 Native Reader modules in DMC Rengine expose typed format readers that downstream products can use. The wider project also includes user-facing reader/browser work such as **DMC Native Reader** and GDSpaces/PocketGDS.

Product and platform claims belong to each product's current evidence. A planned Android, Windows, Web or iOS target is not described as released support until the corresponding build/product path is actually validated.

## Why DMC Rengine uses an evidence-first path

Legacy DMC3 tooling is useful, but old workflows frequently collapse archive extraction, guessed naming and format semantics into one operation. DMC Rengine instead records physical/materialized provenance and promotes semantics only when supported by corpus, executable/runtime or structural evidence.

That makes the resource path more verbose, but it is important for the project's longer-term goal: safe editing and progressively stronger reconstruction/recompilation without silently depending on guessed binary meanings.

## Technical research

For developers and reverse engineers, continue with:

- [`../formats/public-index.md`](../formats/public-index.md) — format research index;
- [`../gdspaces-contract.md`](../gdspaces-contract.md) — GDSpaces contract;
- [`../discovery/search-intent-v2.md`](../discovery/search-intent-v2.md) — public discovery/search-intent policy;
- [`../status/current.md`](../status/current.md) — current canonical implementation maturity.
