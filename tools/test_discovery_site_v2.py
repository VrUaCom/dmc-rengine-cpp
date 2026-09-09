#!/usr/bin/env python3
"""Regression tests for the canonical format encyclopedia discovery-site layer."""

from __future__ import annotations

import json
import shutil
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import build_discovery_site_v2 as site  # noqa: E402
from discovery_format_catalog import format_route, public_format_entries  # noqa: E402


def main() -> None:
    manifest = site.load_manifest()
    pages = manifest["pages"]
    by_path = {page["path"]: page for page in pages}
    entries = public_format_entries(ROOT)

    assert len(entries) >= 50
    assert len(pages) > 70
    assert len(manifest["format_catalog"]) == len(entries)
    assert "/formats/" in by_path
    assert "/formats/mod/" in by_path
    assert "/formats/mot/" in by_path
    assert "/formats/dds/" in by_path
    assert "/formats/ptx/" in by_path
    assert "/archives/nbz/" in by_path
    assert "/formats/hits-dollar/" not in by_path
    assert "/formats/sltc/" not in by_path
    assert "/formats/sch/" not in by_path

    for entry in entries:
        route = format_route(entry)
        assert route in by_path, (entry["id"], route)
        if route != "/formats/":
            assert by_path[route].get("format_entry", {}).get("id") == entry["id"]

    mod = by_path["/formats/mod/"]
    assert mod["source_path"] == "docs/research/dmc3-mod-public-status-2026-09-09.md"
    assert len(mod["learning_links"]) == 12
    assert all(lesson.get("route", "").startswith("/formats/mod/lessons/") for lesson in mod["learning_links"])
    assert len({lesson["route"] for lesson in mod["learning_links"]}) == 12
    assert any(section["heading"] == "Current MOD evidence baseline" for section in mod["sections"])
    assert any(section["heading"] == "Important 2026-09-09 closures and corrections" for section in mod["sections"])
    assert any(section["heading"] == "What MOD still does not authorize" for section in mod["sections"])

    first_lesson_route = "/formats/mod/lessons/01-what-is-mod/"
    last_lesson_route = "/formats/mod/lessons/reference-map/"
    glossary_route = "/formats/mod/lessons/glossary/"
    assert first_lesson_route in by_path
    assert last_lesson_route in by_path
    assert glossary_route in by_path
    assert by_path[first_lesson_route]["source_path"] == "learning/mod/01-what-is-mod.md"
    assert by_path[first_lesson_route]["title"].startswith("Урок 1 — Що таке .MOD")
    assert "`.MOD`" not in by_path[first_lesson_route]["title"]
    assert by_path[first_lesson_route]["lesson_next"] == "/formats/mod/lessons/02-document-objects-meshes/"
    assert by_path[first_lesson_route]["lesson_previous"] is None

    temp_dir = Path(tempfile.mkdtemp(prefix="_site-v2-test-", dir=ROOT))
    try:
        output = temp_dir / "generated"
        base_url = "https://vruacom.github.io/dmc-rengine-cpp"
        site.build(output, base_url)

        site_index_path = output / "site-index.json"
        sitemap_path = output / "sitemap.xml"
        assert site_index_path.is_file()
        assert sitemap_path.is_file()

        site_index = json.loads(site_index_path.read_text(encoding="utf-8"))
        assert site_index["page_count"] == len(pages)
        assert site_index["format_count"] == len(entries)
        assert site_index["paths"] == [page["path"] for page in pages]
        assert len({item["route"] for item in site_index["formats"]}) == len(entries)
        assert first_lesson_route in site_index["paths"]
        assert glossary_route in site_index["paths"]
        assert last_lesson_route in site_index["paths"]

        sitemap_root = ET.fromstring(sitemap_path.read_bytes())
        locs = [node.text or "" for node in sitemap_root.findall(".//{*}loc")]
        expected_locs = {base_url.rstrip("/") + path for path in site_index["paths"]}
        assert len(locs) == site_index["page_count"]
        assert set(locs) == expected_locs
        assert base_url + first_lesson_route in locs

        formats_html = (output / "formats" / "index.html").read_text(encoding="utf-8")
        mod_html = (output / "formats" / "mod" / "index.html").read_text(encoding="utf-8")
        mot_html = (output / "formats" / "mot" / "index.html").read_text(encoding="utf-8")
        dds_html = (output / "formats" / "dds" / "index.html").read_text(encoding="utf-8")
        ptx_html = (output / "formats" / "ptx" / "index.html").read_text(encoding="utf-8")
        nbz_html = (output / "archives" / "nbz" / "index.html").read_text(encoding="utf-8")
        lesson_html = (output / "formats" / "mod" / "lessons" / "01-what-is-mod" / "index.html").read_text(encoding="utf-8")
        lesson_two_html = (output / "formats" / "mod" / "lessons" / "02-document-objects-meshes" / "index.html").read_text(encoding="utf-8")

        assert "Complete canonical format and resource-family catalog" in formats_html
        assert f"{len(entries)} canonical public families are currently represented" in formats_html
        assert 'class="format-card"' in formats_html
        assert f'{base_url}/formats/mod/' in formats_html
        assert f'{base_url}/formats/mot/' in formats_html
        assert f'{base_url}/formats/dds/' in formats_html
        assert f'{base_url}/formats/ptx/' in formats_html
        assert f'{base_url}/archives/nbz/' in formats_html

        assert "MOD at a glance" in mod_html
        assert "Current MOD evidence baseline" in mod_html
        assert "Important 2026-09-09 closures and corrections" in mod_html
        assert "38 unique MOD resources" in mod_html
        assert "20,976 vertices" in mod_html
        assert "BLENDINDICES.x" in mod_html
        assert "ReadWriteMask 0xE" in mod_html
        assert "TEST_1 AREF" in mod_html
        assert "universal decimal interpretation" in mod_html
        assert "PRESERVED_UNDECODED" in mod_html
        assert "100% reverse" in mod_html
        assert "MOD learning path" in mod_html
        assert "01 — What is MOD?" in mod_html
        assert "10 — Exercises and debugging" in mod_html
        assert "MOD glossary" in mod_html
        assert "MOD reference map" in mod_html
        assert "Read lesson on site" in mod_html
        assert f'{base_url}/formats/mod/lessons/01-what-is-mod/' in mod_html
        assert "learning/mod/03-skeleton-hierarchy-transforms.md" in mod_html
        assert "dmc3-mod-public-status-2026-09-09.md" in mod_html

        assert "Урок 1 — Що таке .MOD" in lesson_html
        assert "Урок 1 — Що таке `.MOD`" not in lesson_html
        assert "Не «просто 3D-модель»" in lesson_html
        assert "DMC3-*.nbz" in lesson_html
        assert "serialized MOD" in lesson_html
        assert '<pre><code class="language-text">' in lesson_html
        assert "Next lesson →" in lesson_html
        assert "MOD learning hub" in lesson_html
        assert f'{base_url}/formats/mod/lessons/02-document-objects-meshes/' in lesson_html
        assert "learning/mod/01-what-is-mod.md" in lesson_html
        assert "← Previous lesson" in lesson_two_html
        assert f'{base_url}/formats/mod/lessons/01-what-is-mod/' in lesson_two_html
        assert "38 unique MOD" in lesson_two_html
        assert "runtime_metadata_u32" in lesson_two_html
        assert "REJECTED" in lesson_two_html
        assert "0x00100000" in lesson_two_html

        assert "MOT at a glance" in mot_html
        assert "Practical tutorial path" in mot_html
        assert "Motion/animation resources" in mot_html
        assert "DDS at a glance" in dds_html
        assert "HD texture representation" in dds_html
        assert "PTX at a glance" in ptx_html
        assert "Texture bundle/container-like resource" in ptx_html
        assert "NBZ at a glance" in nbz_html

        assert not (output / "formats" / "hits-dollar" / "index.html").exists()
        assert not (output / "formats" / "sltc" / "index.html").exists()
        assert not (output / "formats" / "sch" / "index.html").exists()

        no_base = temp_dir / "generated-no-base"
        site.build(no_base, None)
        no_base_index = json.loads((no_base / "site-index.json").read_text(encoding="utf-8"))
        assert no_base_index["page_count"] == len(pages)
        assert first_lesson_route in no_base_index["paths"]
        assert not (no_base / "sitemap.xml").exists()
    finally:
        shutil.rmtree(temp_dir, ignore_errors=True)

    print("discovery-site v2 format encyclopedia tests: OK")


if __name__ == "__main__":
    main()
