#!/usr/bin/env python3
"""Regression tests for the dependency-free discovery-site builder."""

from __future__ import annotations

import copy
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

    manifest = site.load_manifest()
    assert len(manifest["pages"]) == 12
    assert len({site.normalized_primary_text(page) for page in manifest["pages"]}) == 12
    for page in manifest["pages"]:
        assert len(page["sections"]) >= 2
        assert len(page["related_links"]) >= 2
        assert sum(len(item) for section in page["sections"] for item in section["items"]) >= 400

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

    duplicate = copy.deepcopy(manifest)
    duplicate["pages"][1]["summary"] = duplicate["pages"][0]["summary"]
    duplicate["pages"][1]["sections"] = copy.deepcopy(duplicate["pages"][0]["sections"])
    expect_exit(
        lambda: site.validate_manifest(duplicate),
        "duplicate primary page content must be rejected",
    )

    temp_dir = Path(tempfile.mkdtemp(prefix="_site-test-", dir=ROOT))
    try:
        output = temp_dir / "generated"
        base = "https://vruacom.github.io/dmc-rengine-cpp"
        site.build(output, base)

        index = (output / "index.html").read_text(encoding="utf-8")
        scm = (output / "formats" / "scm" / "index.html").read_text(encoding="utf-8")
        robots = (output / "robots.txt").read_text(encoding="utf-8")
        sitemap = (output / "sitemap.xml").read_text(encoding="utf-8")

        assert 'rel="canonical" href="https://vruacom.github.io/dmc-rengine-cpp/"' in index
        assert "https://vruacom.github.io/dmc-rengine-cpp/assets/style.css" in index
        assert "https://vruacom.github.io/dmc-rengine-cpp/formats/" in index
        assert "Research surface" in index
        assert "Browse DMC3 HD file-format research" in index
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
            for link in page["related_links"]:
                assert site.public_url(base, link["path"]) in generated
                assert link["label"] in generated

        no_base = temp_dir / "generated-no-base"
        site.build(no_base, None)
        no_base_index = (no_base / "index.html").read_text(encoding="utf-8")
        assert 'rel="canonical"' not in no_base_index
        assert not (no_base / "sitemap.xml").exists()
    finally:
        shutil.rmtree(temp_dir, ignore_errors=True)

    print("discovery-site builder tests: OK")


if __name__ == "__main__":
    main()
