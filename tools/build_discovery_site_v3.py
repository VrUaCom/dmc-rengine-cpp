#!/usr/bin/env python3
"""Build the DMC Rengine discovery site with format and user-intent catalogs."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import build_discovery_site as base
import build_discovery_site_v2 as v2

WAVE2 = base.ROOT / "site" / "intent-pages-wave2.json"
_V2_LOAD_MANIFEST = v2.load_manifest


def _merge_wave2(site: dict) -> dict:
    raw = json.loads(WAVE2.read_text(encoding="utf-8"))
    extra = raw.get("pages", [])
    if not isinstance(extra, list) or not extra:
        raise SystemExit("second-wave intent manifest contains no pages")

    pages = site.get("pages")
    if not isinstance(pages, list):
        raise SystemExit("expanded discovery manifest has no page list")

    seen = {page.get("path") for page in pages if isinstance(page, dict)}
    for page in extra:
        if not isinstance(page, dict):
            raise SystemExit("second-wave intent page must be an object")
        path = page.get("path")
        if path in seen:
            raise SystemExit(f"duplicate second-wave intent route: {path}")
        pages.append(page)
        seen.add(path)
    return site


def load_manifest() -> dict:
    site = _V2_LOAD_MANIFEST()
    site = _merge_wave2(site)
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
