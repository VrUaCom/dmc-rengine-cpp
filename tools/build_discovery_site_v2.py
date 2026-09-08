#!/usr/bin/env python3
"""Build the DMC Rengine discovery site with the canonical format encyclopedia."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import build_discovery_site as base
from discovery_format_catalog import expand_manifest, inject_page_extras, write_site_index

_ORIGINAL_RENDER_PAGE = base.render_page


def _refine_presence_boundaries(site: dict) -> dict:
    for page in site.get("pages", []):
        entry = page.get("format_entry")
        if not isinstance(entry, dict) or entry.get("productStatus") != "capability-only":
            continue
        name = str(entry.get("names", entry.get("id", "resource"))).split(",", 1)[0].strip()
        for section in page.get("sections", []):
            if section.get("heading") != "Practical tutorial path":
                continue
            section["items"] = [
                f"Start by treating {name} as an executable/media capability label, not as proof that a standalone shipped DMC3 asset of that family exists. Establish corpus presence or absence as a separate evidence question before looking for file structure.",
                f"If a real {name} payload is found, preserve its exact source provenance and then determine whether the executable capability corresponds to that physical representation; matching a supported media name alone is not enough to promote a DMC3-specific schema.",
                f"If no shipped {name} instance is confirmed, keep the page capability-only and use executable call sites, media-library boundaries and translation paths as the research surface instead of inventing a parser for an unobserved file format."
            ]
            break
    return site


def load_manifest() -> dict:
    raw = json.loads(base.MANIFEST.read_text(encoding="utf-8"))
    expanded = expand_manifest(raw, base.ROOT)
    expanded = _refine_presence_boundaries(expanded)
    return base.validate_manifest(expanded)


def render_page(site: dict, page: dict, base_url: str | None) -> str:
    document = _ORIGINAL_RENDER_PAGE(site, page, base_url)
    return inject_page_extras(document, site, page, base_url)


def build(output: Path, base_url: str | None) -> None:
    original_load_manifest = base.load_manifest
    original_render_page = base.render_page
    base.load_manifest = load_manifest
    base.render_page = render_page
    try:
        base.build(output, base_url)
        write_site_index(output, load_manifest())
    finally:
        base.load_manifest = original_load_manifest
        base.render_page = original_render_page


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="_site")
    parser.add_argument("--base-url", default=None)
    args = parser.parse_args()
    build(base.ROOT / args.output, args.base_url)
