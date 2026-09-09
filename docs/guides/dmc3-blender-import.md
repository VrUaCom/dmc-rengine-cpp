# DMC3 Blender Import and Community Tools

People searching for `DMC3 Blender import`, `DMC3 MOD Blender`, `DMC3 SCM Blender`, `DMC3 MOT Blender` or `DMC3HDC Import Tools` are usually trying to move extracted Devil May Cry 3 HD resources into a DCC workflow. That search intent overlaps with DMC Rengine, but it is not the same product capability.

## Separate extraction from Blender import

A safe workflow is:

```text
DMC3 HD resource hierarchy
  -> DMC Rengine / GDSpaces extraction and provenance
  -> canonical MOD / SCM / MOT / texture inspection
  -> compatible community Blender tooling when appropriate
```

DMC Rengine remains the C++20 parser, reverse-evidence and resource-architecture authority for its promoted formats. It must not claim a native Blender importer or exporter merely because community Blender tools can consume related DMC3 resource formats.

## Existing community tooling

Two public community projects are relevant to this search intent:

- [DMC3HDC Import Tools](https://github.com/deshayu/DMC3HDC-Import-Tools) by deshayu;
- [DMC3 Blender Import Addon](https://github.com/HansLichtner/DMC3-Blender-Import-Addon) by HansLichtner.

These are independent projects. Their supported Blender versions, import behavior, format coverage and authoring guarantees are governed by their own repositories, not by DMC Rengine.

## Where DMC Rengine helps

DMC Rengine can provide the evidence-aware path to the resource: preserve NBZ/PAC/PNST provenance, identify the actual leaf format, inspect MOD or SCM structure, inspect promoted texture resources and keep unresolved semantics explicit. That makes it easier to know what asset is being handed to another tool and where it came from.

For MOT and animation work, distinguish parser/research maturity from application or DCC-tool support. A community importer accepting MOT does not by itself prove DMC Rengine animation authoring or runtime reintegration.

## Capability boundary

This page is a discovery bridge, not a claim that DMC Rengine owns or bundles the community Blender projects. Extraction, inspection, Blender import, Blender export, model editing, repacking and original-game acceptance are separate capabilities with separate evidence.

Use the canonical model, scene, animation and texture pages for DMC Rengine technical truth, and the linked community repositories for their own Blender-specific instructions.
