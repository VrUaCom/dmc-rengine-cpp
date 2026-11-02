# How to Extract Files from DMC3 NBZ Archives

> **Direct answer:** in Devil May Cry 3 HD Collection on PC, `dmc3-0.nbz` is a ZIP-backed archive surface that can be inspected from a **copy** with ordinary ZIP-capable tools such as 7-Zip/WinRAR for practical community workflows. DMC Rengine separately models NBZ as a provenance-aware source/materialization layer and can index/materialize supported STORE and raw-DEFLATE members in its canonical code path. The public Native Reader does **not** extract NBZ.

## Scope and checked baseline

- **Game:** Devil May Cry 3: Special Edition in Devil May Cry HD Collection.
- **Platform:** Windows PC for the practical archive example.
- **Checked:** 2026-11-02.
- **Archive example:** `<HD Collection>/data/dmc3/dmc3-0.nbz`.
- **DMC Rengine technical support:** ZIP-backed NBZ indexing/materialization, STORE + raw-DEFLATE member extraction, integrity/provenance observations, nested PAC/PNST handoff.
- **Important boundary:** original NBZ archives are treated as source/read-only evidence in the normal resource workflow; bounded overlay/rebuild research is not a license to overwrite your retail archive casually.

The exact archive names and resource paths can vary by edition/language/update. The September 28, 2026 Steam update modified `data/dmc3/dmc3-0.nbz`, so record the game build/date when reproducing an extraction.

## Prerequisites

For a simple user workflow:

- a legitimate Devil May Cry HD Collection installation;
- free disk space for a backup and extracted data;
- 7-Zip or WinRAR if you want to inspect the ZIP-backed NBZ directly;
- DMC Native Reader only **after** you have extracted a supported leaf resource.

For developers using DMC Rengine internals, build the repository and use the canonical GDSpaces/NBZ APIs and tests rather than treating the Native Reader as an archive browser.

## Safe extraction workflow

### 1. Find the HD Collection DMC3 data folder

Typical Steam installation structure:

```text
Devil May Cry HD Collection/
  data/
    dmc3/
      dmc3-0.nbz
```

Do not confuse this with the original 2006 Ubisoft PC release.

### 2. Copy the archive before opening it

Create a working copy, for example:

```text
DMC3-work/
  source/
    dmc3-0.nbz
  extracted/
```

Keep the retail file in the game directory unchanged.

### 3. Open the copy as a ZIP-compatible archive

Community workflows and public HD Collection mod instructions consistently open the NBZ with 7-Zip/WinRAR. Extract the member/folder you need into the working directory.

**Expected result:** you can enumerate/extract archive members without modifying the original game file.

If your archive tool refuses to open the file, verify that you are using the HD Collection DMC3 NBZ and that the file is not truncated. Do not simply rename an unrelated resource to `.zip`.

### 4. Preserve the member path

Do not dump everything into a flat directory. Keep the original member path because later resource identification depends on provenance.

Example:

```text
source archive: dmc3-0.nbz
member path:    GData.afs/.../example.pac
working copy:   extracted/GData.afs/.../example.pac
```

The exact inner path is resource-specific; this example is intentionally generic.

### 5. If the member is PAC or PNST, continue through the container

An NBZ member is often not the final model/texture. Continue with:

- [Extract DMC3 PAC files](extract-pac.md)
- [Browse DMC3 PNST containers](browse-pnst.md)

Keep the physical slot/container identity where your tool exposes it.

### 6. Open the leaf resource with the appropriate reader

For current Windows Native Reader v1.0.0 Preview:

- `MOD` -> [MOD inspection guide](inspect-mod.md)
- `SCM` -> [SCM inspection guide](inspect-scm.md)
- `DDS` / `PTX` -> [texture inspection guide](extract-textures.md)

Download the reader from the [Windows release](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview) or the [site downloads page](https://vruacom.github.io/dmc-rengine-cpp/downloads/).

## How to verify extraction

A successful extraction claim should be narrower than “the archive works”:

1. the working copy opens;
2. the target member is listed;
3. extraction completes without an archive error;
4. the extracted byte length is stable across a second extraction;
5. if you have a trusted hash/CRC for the member, compare it;
6. if the leaf is a supported Native Reader format, open the extracted bytes and confirm the expected parser accepts them.

DMC Rengine's canonical NBZ work additionally records CRC/size/SHA and byte provenance where the code path exposes those observations.

## DMC Rengine's NBZ boundary

The repository currently documents:

- classic ZIP-backed NBZ indexing/materialization;
- STORE and raw-DEFLATE member extraction;
- integrity/provenance observations;
- recursive handoff into PAC/PNST;
- immutable/copy rebuild research;
- a bounded next-contiguous STORE overlay authoring path.

These capabilities do **not** establish that every edited resource can be repacked into a retail NBZ and accepted by the original game. Resolver selection, nested-resource validity and original-runtime acceptance are separate gates.

## Common problems

### “I extracted the NBZ but cannot find the model/texture”

The resource may be inside a PAC or PNST, or the name you expected may not exist in that edition. Continue down the container hierarchy instead of searching only top-level filenames.

### “I found a PAC but Native Reader will not open it”

That is expected. The Windows preview is a leaf-resource viewer for supported `MOD`, `SCM`, `DDS` and `PTX`, not a PAC/NBZ explorer.

### “Steam replaced my modded NBZ”

Game updates can replace retail files. Restore the original archive and use a non-destructive workflow. Community users commonly prefer local `GData.afs`/secondary-archive approaches rather than modifying `dmc3-0.nbz` in place.

### “The instructions online say DMC3-1.nbz”

That can be a language/overlay/community-mod path. Record which edition/build the instruction targets; do not assume it is identical to `dmc3-0.nbz`.

## What was actually verified for this guide

- Repository evidence for the DMC Rengine NBZ materialization boundary was reviewed at main commit `086868779495ecd309df531f5a1d1e4a72d724ce`.
- Public Steam/Nexus/PCGamingWiki instructions independently corroborate that HD Collection users open `dmc3-0.nbz` as a ZIP-compatible archive and that updates can replace it.
- No new retail archive was extracted on a local machine during this documentation pass.
- No claim is made that 7-Zip/WinRAR is the canonical DMC Rengine implementation; those are third-party practical tools.

## Sources

- [GDSpaces contract](../gdspaces-contract.md)
- [Current status](../status/current.md)
- [NBZ ZIP reconciliation](../gdspaces/nbz-zip-reconciliation.md)
- [Steam discussion: September 2026 update and dmc3-0.nbz](https://steamcommunity.com/app/631510/discussions/0/591816596100694650/)
- [PCGamingWiki — Devil May Cry HD Collection](https://www.pcgamingwiki.com/wiki/Devil_May_Cry_HD_Collection)

**Author:** DMC Rengine project.  
**Phase-1 evidence review:** 2026-11-02.

## Next

- [Extract PAC](extract-pac.md)
- [Browse PNST](browse-pnst.md)
- [Open MOD](inspect-mod.md)
- [Open SCM](inspect-scm.md)
- [Inspect DDS/PTX](extract-textures.md)
