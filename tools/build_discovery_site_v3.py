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

    return site


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


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="_site")
    parser.add_argument("--base-url", default=None)
    args = parser.parse_args()
    build(base.ROOT / args.output, args.base_url)
