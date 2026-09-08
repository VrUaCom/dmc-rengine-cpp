#!/usr/bin/env python3
"""Build the DMC Rengine discovery site with the canonical format encyclopedia."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import build_discovery_site as base
from discovery_format_catalog import expand_manifest, inject_page_extras, write_site_index

_ORIGINAL_RENDER_PAGE = base.render_page


def load_manifest() -> dict:
    raw = json.loads(base.MANIFEST.read_text(encoding="utf-8"))
    expanded = expand_manifest(raw, base.ROOT)
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
