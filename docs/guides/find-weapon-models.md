# How to Find DMC3 Weapon Models

Searches such as `DMC3 Yamato model`, `DMC3 Rebellion model`, `DMC3 Beowulf model`, `DMC3 Force Edge model` or simply `DMC3 weapon models` describe a user problem, not a guaranteed binary-format identity. DMC Rengine therefore starts from resource provenance and typed verification instead of assuming that every named weapon always maps to one standalone MOD file.

## Resource path

```text
NBZ
  -> archive member
  -> PAC / PNST slot
  -> candidate resource
  -> typed format verification
  -> model / texture inspection
```

Preserve the original archive, member and slot identity while navigating. If the candidate matches MOD, use the canonical MOD reader to inspect object, mesh, hierarchy, transform, texture-facing and skinning-related state. If the payload belongs to another confirmed resource family, keep that family distinct rather than forcing it into a generic weapon-model interpretation.

## Named weapon searches

Yamato, Rebellion, Beowulf and Force Edge are useful search entry points because users often know the weapon name before they know DMC3 resource formats. The guide layer should answer that intent and then route into canonical technical pages, while avoiding hard-coded universal offsets, filenames or slot numbers without corpus proof.

Texture resources related to a weapon should likewise be followed into the PTX/DDS layer instead of being described as if image data were necessarily embedded directly inside the model resource.

## Capability boundary

A verified weapon resource can support extraction and structural/visual inspection where the relevant canonical reader is promoted. That does not imply universal Blender export, arbitrary model replacement, complete writer authority or original-game acceptance of rebuilt weapon resources.

For model structure see `inspect-mod.md`; for archive navigation see `unpack-dmc3-hd.md`; for textures see `extract-textures.md`.
