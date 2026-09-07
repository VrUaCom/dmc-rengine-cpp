#!/usr/bin/env python3
"""Regression tests for the dependency-free discovery-site builder."""

from __future__ import annotations

import copy
import hashlib
import html
import shutil
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import build_discovery_site as site  # noqa: E402


def expect_exit(fn, message: str) -> None:
    try:
        fn()
    except SystemExit:
        return
    raise AssertionError(message)


def main() -> None:
    assert site.normalize_base_url("https://vruacom.github.io/dmc-rengine-cpp/") == (
        "https://vruacom.github.io/dmc-rengine-cpp"
    )
    assert site.public_url(
        "https://vruacom.github.io/dmc-rengine-cpp",
        "/formats/",
    ) == "https://vruacom.github.io/dmc-rengine-cpp/formats/"

    expect_exit(
        lambda: site.normalize_base_url("http://vruacom.github.io/dmc-rengine-cpp"),
        "HTTP base URL must be rejected",
    )
    expect_exit(
        lambda: site.normalize_base_url("https://vruacom.github.io/dmc-rengine-cpp?q=1"),
        "query-bearing base URL must be rejected",
    )
    expect_exit(
        lambda: site.normalize_base_url("https://vruacom.github.io/dmc-rengine-cpp#frag"),
        "fragment-bearing base URL must be rejected",
    )
    expect_exit(
        lambda: site.require_within_repo(ROOT.parent / "outside-site", "test path"),
        "outside-repository path must be rejected",
    )
    expect_exit(
        lambda: site.require_safe_output(ROOT / "docs"),
        "non-_site repository directory must be rejected as destructive output",
    )
    expect_exit(
        lambda: site.require_safe_output(ROOT),
        "repository root must be rejected as destructive output",
    )

    preview_bytes = site.validate_social_preview()
    assert hashlib.sha256(preview_bytes).hexdigest() == site.SOCIAL_PREVIEW_SHA256
    assert site.png_dimensions(preview_bytes) == site.SOCIAL_PREVIEW_DIMENSIONS
    expect_exit(
        lambda: site.png_dimensions(b"not-a-png"),
        "malformed PNG metadata must fail closed",
    )

    manifest = site.load_manifest()
    assert len(manifest["pages"]) == 12
    assert len({site.normalized_primary_text(page) for page in manifest["pages"]}) == 12
    assert len({site.normalized_section_text(page) for page in manifest["pages"]}) == 12
    assert len({site.normalized_text(page["summary"]) for page in manifest["pages"]}) == 12
    for page in manifest["pages"]:
        assert len(page["sections"]) >= 2
        assert len(page["related_links"]) >= 2
        assert sum(len(item) for section in page["sections"] for item in section["items"]) >= 400

    pages_by_path = {page["path"]: page for page in manifest["pages"]}
    assert [page["path"] for page in site.breadcrumb_chain(
        manifest["pages"], pages_by_path["/formats/scm/"]
    )] == ["/", "/formats/", "/formats/scm/"]
    assert [page["path"] for page in site.breadcrumb_chain(
        manifest["pages"], pages_by_path["/archives/nbz/"]
    )] == ["/", "/archives/nbz/"]
    assert site.breadcrumb_chain(manifest["pages"], pages_by_path["/"]) == []
    for page in manifest["pages"]:
        chain = site.breadcrumb_chain(manifest["pages"], page)
        if page["path"] == "/":
            assert chain == []
        else:
            assert len(chain) >= 2
            assert chain[0]["path"] == "/"
            assert chain[-1]["path"] == page["path"]
            assert all(crumb["path"] in pages_by_path for crumb in chain)

    missing_root = copy.deepcopy(manifest)
    missing_root["pages"] = [page for page in missing_root["pages"] if page["path"] != "/"]
    expect_exit(
        lambda: site.validate_manifest(missing_root),
        "manifest without a root page must be rejected",
    )

    unresolved = copy.deepcopy(manifest)
    unresolved["pages"][0]["related_links"][0]["path"] = "/missing-route/"
    expect_exit(
        lambda: site.validate_manifest(unresolved),
        "unresolved related links must be rejected",
    )

    self_link = copy.deepcopy(manifest)
    self_link["pages"][0]["related_links"][0]["path"] = "/"
    expect_exit(
        lambda: site.validate_manifest(self_link),
        "self-related links must be rejected",
    )

    thin = copy.deepcopy(manifest)
    thin["pages"][0]["sections"] = [
        {"heading": "Thin", "items": ["This sentence is deliberately long enough for the item length guard."]}
    ]
    expect_exit(
        lambda: site.validate_manifest(thin),
        "thin page content must be rejected",
    )

    duplicate_summary = copy.deepcopy(manifest)
    duplicate_summary["pages"][1]["summary"] = duplicate_summary["pages"][0]["summary"]
    expect_exit(
        lambda: site.validate_manifest(duplicate_summary),
        "duplicate page summaries must be rejected",
    )

    duplicate_sections = copy.deepcopy(manifest)
    duplicate_sections["pages"][1]["sections"] = copy.deepcopy(
        duplicate_sections["pages"][0]["sections"]
    )
    expect_exit(
        lambda: site.validate_manifest(duplicate_sections),
        "duplicate substantive section bodies must be rejected",
    )

    malformed_section = copy.deepcopy(manifest)
    malformed_section["pages"][0]["sections"][0] = "not-an-object"
    expect_exit(
        lambda: site.validate_manifest(malformed_section),
        "malformed section definitions must fail cleanly",
    )

    malformed_link = copy.deepcopy(manifest)
    malformed_link["pages"][0]["related_links"][0] = "not-an-object"
    expect_exit(
        lambda: site.validate_manifest(malformed_link),
        "malformed related-link definitions must fail cleanly",
    )

    temp_dir = Path(tempfile.mkdtemp(prefix="_site-test-", dir=ROOT))
    try:
        mutated_preview = temp_dir / "mutated-preview.png"
        mutated = bytearray(preview_bytes)
        mutated[-1] ^= 0x01
        mutated_preview.write_bytes(mutated)
        expect_exit(
            lambda: site.validate_social_preview(mutated_preview),
            "mutated social preview must fail the approved SHA-256 gate",
        )

        output = temp_dir / "generated"
        base = "https://vruacom.github.io/dmc-rengine-cpp"
        site.build(output, base)

        index = (output / "index.html").read_text(encoding="utf-8")
        scm = (output / "formats" / "scm" / "index.html").read_text(encoding="utf-8")
        nbz = (output / "archives" / "nbz" / "index.html").read_text(encoding="utf-8")
        robots = (output / "robots.txt").read_text(encoding="utf-8")
        sitemap = (output / "sitemap.xml").read_text(encoding="utf-8")
        published_preview = output / "assets" / "social-preview.png"

        assert published_preview.is_file()
        assert published_preview.read_bytes() == preview_bytes
        assert hashlib.sha256(published_preview.read_bytes()).hexdigest() == site.SOCIAL_PREVIEW_SHA256

        assert 'rel="canonical" href="https://vruacom.github.io/dmc-rengine-cpp/"' in index
        assert '<meta property="og:site_name" content="DMC Rengine">' in index
        assert '<meta name="twitter:card" content="summary">' in index
        assert "og:image" not in index
        assert "twitter:image" not in index
        assert 'aria-label="Breadcrumb"' not in index
        assert '"@type":"BreadcrumbList"' not in index
        assert "https://vruacom.github.io/dmc-rengine-cpp/assets/style.css" in index
        assert "https://vruacom.github.io/dmc-rengine-cpp/formats/" in index
        assert "Research surface" in index
        assert "Browse DMC3 HD file-format research" in index

        assert 'aria-label="Breadcrumb"' in scm
        assert '<a href="https://vruacom.github.io/dmc-rengine-cpp/">DMC Rengine</a>' in scm
        assert '<a href="https://vruacom.github.io/dmc-rengine-cpp/formats/">DMC3 HD File Formats</a>' in scm
        assert '<span aria-current="page">DMC3 SCM Format</span>' in scm
        assert '"@type":"BreadcrumbList"' in scm
        assert '"name":"DMC Rengine","item":"https://vruacom.github.io/dmc-rengine-cpp/"' in scm
        assert '"name":"DMC3 HD File Formats","item":"https://vruacom.github.io/dmc-rengine-cpp/formats/"' in scm
        assert '"name":"DMC3 SCM Format","item":"https://vruacom.github.io/dmc-rengine-cpp/formats/scm/"' in scm
        assert '"item":"https://vruacom.github.io/dmc-rengine-cpp/archives/"' not in nbz

        assert "What SCM research covers" in scm
        assert "Questions this entry point helps answer" in scm
        assert "Compare SCM with other DMC3 HD formats" in scm
        assert index != scm
        assert "User-agent: OAI-SearchBot" in robots
        assert "https://vruacom.github.io/dmc-rengine-cpp/sitemap.xml" in robots
        assert "https://vruacom.github.io/dmc-rengine-cpp/formats/scm/" in sitemap
        assert "https://github.com/VrUaCom/dmc-rengine-cpp/blob/main/README.md" in index
        assert "https://github.com/VrUaCom/dmc-rengine-cpp/blob/main/docs/formats/scm.md" in scm

        for page in manifest["pages"]:
            generated = site.page_output(output, page["path"]).read_text(encoding="utf-8")
            escaped_title = html.escape(page["title"], quote=True)
            escaped_description = html.escape(page["description"], quote=True)
            assert f'<meta property="og:title" content="{escaped_title}">' in generated
            assert f'<meta property="og:description" content="{escaped_description}">' in generated
            assert '<meta property="og:site_name" content="DMC Rengine">' in generated
            assert '<meta name="twitter:card" content="summary">' in generated
            assert f'<meta name="twitter:title" content="{escaped_title}">' in generated
            assert f'<meta name="twitter:description" content="{escaped_description}">' in generated
            assert "og:image" not in generated
            assert "twitter:image" not in generated
            if page["path"] == "/":
                assert 'aria-label="Breadcrumb"' not in generated
                assert '"@type":"BreadcrumbList"' not in generated
            else:
                assert 'aria-label="Breadcrumb"' in generated
                assert '"@type":"BreadcrumbList"' in generated
                assert site.breadcrumb_label(page) in generated
            for link in page["related_links"]:
                assert site.public_url(base, link["path"]) in generated
                assert link["label"] in generated

        no_base = temp_dir / "generated-no-base"
        site.build(no_base, None)
        no_base_index = (no_base / "index.html").read_text(encoding="utf-8")
        no_base_scm = (no_base / "formats" / "scm" / "index.html").read_text(encoding="utf-8")
        assert 'rel="canonical"' not in no_base_index
        assert '<meta name="twitter:card" content="summary">' in no_base_index
        assert 'aria-label="Breadcrumb"' in no_base_scm
        assert '<a href="/formats/">DMC3 HD File Formats</a>' in no_base_scm
        assert '"@type":"BreadcrumbList"' not in no_base_scm
        assert (no_base / "assets" / "social-preview.png").read_bytes() == preview_bytes
        assert not (no_base / "sitemap.xml").exists()
    finally:
        shutil.rmtree(temp_dir, ignore_errors=True)

    print("discovery-site builder tests: OK")


if __name__ == "__main__":
    main()
