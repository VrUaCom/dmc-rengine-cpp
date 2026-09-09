#!/usr/bin/env python3
"""Build the DMC Rengine discovery site with format and user-intent catalogs."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import build_discovery_site as base
import build_discovery_site_v2 as v2

INTENT_WAVES = (
    base.ROOT / "site" / "intent-pages-wave2.json",
    base.ROOT / "site" / "intent-pages-wave3.json",
    base.ROOT / "site" / "intent-pages-wave4.json",
)
_V2_LOAD_MANIFEST = v2.load_manifest
INDEXNOW_KEY_SOURCE = base.ROOT / "site" / "6ef2bc73288e1e2b580119a4b1bfc2fa.txt"
INDEXNOW_ALLOWED_CHARS = set(
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-"
)


def _merge_intent_wave(site: dict, manifest_path: Path) -> dict:
    raw = json.loads(manifest_path.read_text(encoding="utf-8"))
    extra = raw.get("pages", [])
    if not isinstance(extra, list) or not extra:
        raise SystemExit(f"intent manifest contains no pages: {manifest_path}")

    pages = site.get("pages")
    if not isinstance(pages, list):
        raise SystemExit("expanded discovery manifest has no page list")

    seen = {page.get("path") for page in pages if isinstance(page, dict)}
    for page in extra:
        if not isinstance(page, dict):
            raise SystemExit(f"intent page must be an object: {manifest_path}")
        path = page.get("path")
        if path in seen:
            raise SystemExit(f"duplicate intent route in {manifest_path.name}: {path}")
        pages.append(page)
        seen.add(path)
    return site


def _append_related(page: dict, path: str, label: str) -> None:
    links = page.setdefault("related_links", [])
    if not any(isinstance(link, dict) and link.get("path") == path for link in links):
        links.append({"path": path, "label": label})


def _promote_mod_writer_gate(by_path: dict[str, dict]) -> None:
    inspect_mod = by_path["/guides/inspect-mod/"]
    inspect_mod["description"] = (
        "DMC3 MOD viewer and bounded preserve-layout writer guide for hierarchy, meshes, "
        "skinning, fixed-layout edits and evidence boundaries."
    )
    inspect_mod["summary"] = (
        "Open and inspect DMC3 MOD model files through the canonical reader, then use the "
        "promoted Preserve-Layout Writer Gate 1 only for its proven fixed-size edit surface. "
        "The writer preserves the original physical layout, protects unauthorized bytes and "
        "passes a provenance-bound 38/38 no-edit retail corpus gate, while unrestricted model "
        "authoring and original-game edited-MOD acceptance remain separate proof gates."
    )
    inspect_mod["sections"] = [
        {
            "heading": "Reader plus preserve-layout writer Gate 1",
            "items": [
                "The canonical MOD reader exposes document/object/mesh structure, hierarchy and transforms, texture-facing relationships, runtime topology evidence, inverse-rest ownership and packed skinning data for structural and pose-aware inspection.",
                "Preserve-Layout Writer Gate 1 starts from an immutable original serialized image and authorizes only fixed-size edits to already-typed spans: object bounding center/radius plus existing mesh positions, normals and UV values.",
                "The writer does not synthesize offsets, tables or stream layout; it independently checks every changed byte against the authorized spans and reparses the output through the canonical MOD parser before reporting success."
            ],
        },
        {
            "heading": "What the writer gate proves and what it does not",
            "items": [
                "The provenance-bound no-edit corpus gate passes 38/38 retail MOD files with exact source/output SHA equality, zero modified bytes across 882,736 source bytes and 38/38 canonical reopen, which is real writer evidence rather than a read-only claim.",
                "Gate 1 still blocks transform authoring, skin/blend-index edits, texture-slot or material-state authoring, stream/cardinality changes, layout synthesis and unresolved preservation-only fields.",
                "DMC Rengine therefore does not claim a complete MOD editor, texture-companion rewriting, PAC/PNST/NBZ reintegration of MOD writer output, original dmc3.exe acceptance of rebuilt or edited MOD files, Capcom authoring-tool equivalence or a 100% MOD writer."
            ],
        },
    ]
    _append_related(
        inspect_mod,
        "/formats/mod/",
        "Read the canonical DMC3 MOD format and writer evidence surface",
    )
    _append_related(
        inspect_mod,
        "/models/",
        "Return to the DMC3 model extraction and authoring hub",
    )

    extract_models = by_path["/guides/extract-models/"]
    extract_models["description"] = (
        "DMC3 model extraction guide connecting NBZ/PAC/PNST resources to MOD and SCM "
        "inspection plus the bounded MOD preserve-layout writer gate."
    )
    extract_models["summary"] = (
        "Find and inspect DMC3 HD model-related resources while preserving the archive and "
        "container path, then keep MOD and SCM as separate binary authorities. MOD now has a "
        "bounded preserve-layout writer for proven fixed-size fields; this is stronger than "
        "read-only inspection but deliberately narrower than unrestricted model editing."
    )
    extract_models["sections"] = [
        {
            "heading": "From archive slot to typed model resource",
            "items": [
                "Model discovery begins in the NBZ materialization path and may cross PAC or PNST before a leaf payload can be classified as MOD, SCM or another family; physical archive and slot provenance stay attached to the result.",
                "MOD remains the primary character/enemy-style model-family research surface while SCM is scene-oriented, so extraction should route each payload through its real format authority instead of flattening every resource into a generic model representation.",
                "The canonical MOD reader supports meaningful hierarchy, transform, texture-slot and skinning inspection, including skeleton/weight visualization and pose-aware analysis under the promoted evidence set."
            ],
        },
        {
            "heading": "Bounded MOD editing is now a separate proved capability",
            "items": [
                "Preserve-Layout Writer Gate 1 can apply fixed-size edits to object bounds, existing positions, existing normals and existing UVs while preserving the original physical layout and rejecting unauthorized byte changes.",
                "Its no-edit retail corpus receipt passes 38/38 MOD files with exact byte/hash parity and canonical reopen, but that does not establish topology/cardinality changes, transform or skin authoring, material and texture-binding authoring, or texture-companion rewriting.",
                "Archive reintegration and original dmc3.exe acceptance remain stronger gates, so search terms such as DMC3 MOD editor should lead to this bounded capability rather than to a false claim of a finished general-purpose model editor."
            ],
        },
    ]

    models = by_path["/models/"]
    sections = models.setdefault("sections", [])
    if not any(
        isinstance(section, dict)
        and section.get("heading") == "Bounded MOD preserve-layout authoring"
        for section in sections
    ):
        sections.append(
            {
                "heading": "Bounded MOD preserve-layout authoring",
                "items": [
                    "DMC Rengine now has a promoted MOD Preserve-Layout Writer Gate 1 in addition to the canonical reader, so the model surface is no longer accurately described as purely read-only.",
                    "The writer authorizes fixed-size changes only for currently proved spans and preserves all other bytes against the immutable source image; 38/38 provenance-bound retail MOD files pass the no-edit exact-parity and canonical-reopen gate.",
                    "This is not a promise of arbitrary model editing, topology changes, transform/skin/material authoring, archive reintegration or original-game edited-file acceptance; each stronger operation keeps its own evidence gate."
                ],
            }
        )
    _append_related(
        models,
        "/guides/inspect-mod/",
        "Inspect MOD files and review the bounded preserve-layout writer gate",
    )


def _augment_intent_graph(site: dict) -> dict:
    pages = site.get("pages")
    if not isinstance(pages, list):
        raise SystemExit("intent graph requires page list")
    by_path = {
        page.get("path"): page
        for page in pages
        if isinstance(page, dict) and isinstance(page.get("path"), str)
    }

    required = (
        "/guides/",
        "/models/",
        "/textures/",
        "/formats/mod/",
        "/formats/scm/",
        "/formats/mot/",
        "/guides/extract-models/",
        "/guides/inspect-mod/",
        "/guides/character-models/",
        "/guides/enemy-models/",
        "/native-reader/android/",
        "/guides/vergil-model-textures/",
        "/guides/weapon-models/",
        "/guides/blender-import/",
    )
    missing = [path for path in required if path not in by_path]
    if missing:
        raise SystemExit(f"intent graph missing required routes: {missing}")

    guides = by_path["/guides/"]
    sections = guides.setdefault("sections", [])
    if not any(
        isinstance(section, dict)
        and section.get("heading") == "Character and Android viewer entry points"
        for section in sections
    ):
        sections.append(
            {
                "heading": "Character and Android viewer entry points",
                "items": [
                    "Find DMC3 character models through provenance-aware archive navigation and canonical MOD verification instead of relying on one guessed universal filename.",
                    "Use the enemy-model guide for the same typed resource pipeline across enemy assets and variants while keeping inspection distinct from authoring proof.",
                    "Use the DMC Native Reader Android page for the current app-facing MOD, SCM, DDS and PTX boundary; Windows, Web and iOS are not advertised as shipped platforms without implementation evidence.",
                ],
            }
        )
    if not any(
        isinstance(section, dict)
        and section.get("heading") == "Vergil, weapon and Blender workflows"
        for section in sections
    ):
        sections.append(
            {
                "heading": "Vergil, weapon and Blender workflows",
                "items": [
                    "Use the Vergil guide when the search starts from a character name, then verify model candidates through the same archive-to-MOD evidence path and follow texture relationships into PTX or DDS resources.",
                    "Use the weapon-model guide for Yamato, Rebellion, Beowulf, Force Edge and similar named searches without assuming that every weapon has one universal standalone MOD path.",
                    "Use the Blender-import guide to bridge extracted and identified DMC3 resources into independent community DCC tooling while keeping DMC Rengine parser/extraction claims separate from Blender importer or exporter claims.",
                ],
            }
        )
    _append_related(guides, "/guides/character-models/", "Find DMC3 character models")
    _append_related(guides, "/guides/enemy-models/", "Find and inspect DMC3 enemy models")
    _append_related(guides, "/native-reader/android/", "Open the DMC Native Reader Android capability page")
    _append_related(guides, "/guides/vergil-model-textures/", "Find Vergil models and textures through canonical resource evidence")
    _append_related(guides, "/guides/weapon-models/", "Find Yamato, Rebellion, Beowulf and other DMC3 weapon resources")
    _append_related(guides, "/guides/blender-import/", "Connect DMC3 extraction and inspection to community Blender workflows")

    models = by_path["/models/"]
    _append_related(models, "/guides/character-models/", "Find character models through the archive-to-MOD pipeline")
    _append_related(models, "/guides/enemy-models/", "Find and inspect enemy models")
    _append_related(models, "/native-reader/android/", "See current Android MOD and SCM viewing support")
    _append_related(models, "/guides/vergil-model-textures/", "Find Vergil model resources and associated textures")
    _append_related(models, "/guides/weapon-models/", "Find named DMC3 weapon model resources")
    _append_related(models, "/guides/blender-import/", "Continue from model extraction into community Blender import workflows")

    textures = by_path["/textures/"]
    _append_related(textures, "/native-reader/android/", "See current Android DDS and PTX viewing support")
    _append_related(textures, "/guides/dante-model-textures/", "Find Dante-related model and texture resources")
    _append_related(textures, "/guides/vergil-model-textures/", "Find Vergil-related model and texture resources")

    _append_related(by_path["/formats/mod/"], "/guides/blender-import/", "Relate canonical MOD research to community Blender import workflows")
    _append_related(by_path["/formats/scm/"], "/guides/blender-import/", "Relate canonical SCM research to community Blender import workflows")
    _append_related(by_path["/formats/mot/"], "/guides/blender-import/", "Relate MOT research to evidence-bounded Blender import workflows")

    _promote_mod_writer_gate(by_path)
    return site


def _publish_indexnow_key(output: Path) -> None:
    try:
        key = INDEXNOW_KEY_SOURCE.read_text(encoding="utf-8").strip()
    except OSError as exc:
        raise SystemExit(f"IndexNow key source cannot be read: {INDEXNOW_KEY_SOURCE}") from exc

    if not 8 <= len(key) <= 128:
        raise SystemExit("IndexNow key length must be between 8 and 128 characters")
    if any(char not in INDEXNOW_ALLOWED_CHARS for char in key):
        raise SystemExit("IndexNow key contains characters outside the protocol allow-list")
    if INDEXNOW_KEY_SOURCE.stem != key:
        raise SystemExit("IndexNow key filename must match the key value")

    target = output / INDEXNOW_KEY_SOURCE.name
    target.write_text(key + "\n", encoding="utf-8")
    if target.read_text(encoding="utf-8").strip() != key:
        raise SystemExit("generated IndexNow key file failed round-trip validation")


def load_manifest() -> dict:
    site = _V2_LOAD_MANIFEST()
    for manifest_path in INTENT_WAVES:
        site = _merge_intent_wave(site, manifest_path)
    site = _augment_intent_graph(site)
    return base.validate_manifest(site)


def build(output: Path, base_url: str | None) -> None:
    original_load_manifest = v2.load_manifest
    v2.load_manifest = load_manifest
    try:
        v2.build(output, base_url)
    finally:
        v2.load_manifest = original_load_manifest
    _publish_indexnow_key(output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="_site")
    parser.add_argument("--base-url", default=None)
    args = parser.parse_args()
    build(base.ROOT / args.output, args.base_url)
