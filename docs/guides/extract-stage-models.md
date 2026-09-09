# How to Extract DMC3 Stage Models

Stage resources in Devil May Cry 3 HD are distributed through the same archive/container hierarchy as other game data, but stage geometry and scene data belong to different resource families and should not be flattened into one generic "model" type.

## Practical route

Start from the numbered NBZ archives, materialize the relevant member, preserve PAC/PNST slot provenance and then classify the resulting resource before opening it with a typed reader.

```text
NBZ
  -> stage-related member
  -> PAC / PNST
  -> materialized child
  -> SCM / MOD / config / other typed resource
```

SCM is the primary scene-oriented geometry research surface in DMC Rengine. It contains scene-node, object, mesh, stream, transform and texture-facing relationships. MOD is a separate model family and should not be treated as physically identical to SCM merely because both can contain geometry.

## Inspection workflow

Use the canonical SCM reader for scene hierarchy, object/mesh relationships, streams and transforms. Follow texture references into the texture resource layer rather than assuming the image bytes are embedded directly in the same semantic object.

When a materialized stage child belongs to another family such as HITS, Stage TXT, LIG/LIG2 or DCA, route it to that parser instead of interpreting it as geometry by filename alone.

## Evidence boundary

Structural reading and resource extraction are stronger than unrestricted scene authoring or original-game acceptance. Any edited/rebuilt stage workflow must follow the writer and runtime proof gates documented by the canonical project status.
