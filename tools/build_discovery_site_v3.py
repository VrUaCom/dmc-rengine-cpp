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


def load_manifest() -> dict:
    site = _V2_LOAD_MANIFEST()
    for manifest_path in INTENT_WAVES:
        site = _merge_intent_wave(site, manifest_path)
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
