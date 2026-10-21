#!/usr/bin/env python3
"""Smoke/regression tests for first-party multilingual Modding Hub routes."""
from __future__ import annotations

import json
import shutil
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import build_discovery_site_v3 as site  # noqa: E402

BASE = "https://vruacom.github.io/dmc-rengine-cpp"
LOCALES = {"en": "/en/", "es": "/es/", "pt-BR": "/pt-br/", "uk": "/uk/", "ja": "/ja/"}


def main() -> None:
    manifest = site.load_manifest()
    pages = {page["path"]: page for page in manifest["pages"]}
    for required in ("/tools/", "/guides/start-modding-dmc3/", *LOCALES.values(), "/formats/mod/", "/"):
        assert required in pages, required

    assert len(LOCALES) == len(set(LOCALES.values()))
    for lang, route in LOCALES.items():
        assert pages[route]["locale"] == lang
        assert pages[route]["language_group"] == "modding-hub-home"
    assert pages["/tools/"]["source_path"] == "docs/discovery/our-tools.md"

    catalog = json.loads((ROOT / "site" / "tool-catalog.json").read_text(encoding="utf-8"))
    products = catalog["products"]
    assert len(products) == 4
    assert len({product["id"] for product in products}) == len(products)
    assert all(product["repo_url"].startswith("https://github.com/VrUaCom/") for product in products)
    assert any(product["id"] == "native-reader" for product in products)

    # Output must live in a _site-prefixed working area, as required by the canonical builder.
    parent = Path(tempfile.mkdtemp(prefix="_site-modding-hub-", dir=ROOT))
    try:
        generated = parent / "generated"
        site.build(generated, BASE)
        assert (generated / "index.html").is_file()
        assert (generated / "formats" / "mod" / "index.html").is_file()
        assert (generated / "guides" / "start-modding-dmc3" / "index.html").is_file()

        links = [f'<link rel="alternate" hreflang="{lang}" href="{BASE}{route}">'
                 for lang, route in LOCALES.items()]
        for lang, route in LOCALES.items():
            document = site.base.page_output(generated, route).read_text(encoding="utf-8")
            assert f'<html lang="{lang}">' in document, route
            assert f'<link rel="canonical" href="{BASE}{route}">' in document, route
            assert all(tag in document for tag in links), route
            assert f'<link rel="alternate" hreflang="x-default" href="{BASE}/en/">' in document
            assert 'class="language-switcher"' in document
            assert "DMC Native Reader" in document, route
            assert 'noindex,nofollow' not in document, route

        es = (generated / "es" / "index.html").read_text(encoding="utf-8")
        uk = (generated / "uk" / "index.html").read_text(encoding="utf-8")
        assert "Lecturas relacionadas" in es
        assert "Канонічне джерело" in uk
        assert "Esta página facilita la consulta" in es
        assert "Основним джерелом технічних фактів" in uk

        tools_html = (generated / "tools" / "index.html").read_text(encoding="utf-8")
        assert "Our ecosystem tools" in tools_html
        assert "DMC Native Blender Reader" in tools_html
        assert "Pocket GDS repository private" in tools_html
        assert tools_html.count("Source and project information") == len(products)
        assert f'{BASE}/tools/' in tools_html

        home = (generated / "index.html").read_text(encoding="utf-8")
        assert f'{BASE}/en/' in home
        assert f'{BASE}/tools/' in home

        index = json.loads((generated / "site-index.json").read_text(encoding="utf-8"))
        assert set(LOCALES.values()).issubset(set(index["paths"]))
        assert index["page_count"] == len(manifest["pages"])
        xml = ET.parse(generated / "sitemap.xml")
        locations = {n.text for n in xml.findall(".//{*}loc")}
        assert len(locations) == index["page_count"]
        assert all(BASE + route in locations for route in LOCALES.values())
    finally:
        shutil.rmtree(parent, ignore_errors=True)
    print("DMC Modding Hub v1 site, tool-directory and locale SEO tests: OK")


if __name__ == "__main__":
    main()
