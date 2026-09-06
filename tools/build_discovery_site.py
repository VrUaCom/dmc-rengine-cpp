#!/usr/bin/env python3
"""Build the dependency-free DMC Rengine public discovery site.

The generated site is intentionally a navigation/readability layer. Canonical technical
truth remains in repository documentation referenced by each page's source_path.
"""

from __future__ import annotations

import argparse
import html
import json
import shutil
from pathlib import Path
from urllib.parse import urljoin

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "site" / "manifest.json"
CSS_SOURCE = ROOT / "site" / "assets" / "style.css"


def load_manifest() -> dict:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    pages = data.get("pages", [])
    if not pages:
        raise SystemExit("site manifest contains no pages")

    seen = set()
    for page in pages:
        path = page["path"]
        if not path.startswith("/") or (path != "/" and not path.endswith("/")):
            raise SystemExit(f"invalid public path: {path}")
        if path in seen:
            raise SystemExit(f"duplicate public path: {path}")
        seen.add(path)

        source = ROOT / page["source_path"]
        try:
            source.relative_to(ROOT)
        except ValueError as exc:
            raise SystemExit(f"source escapes repository: {page['source_path']}") from exc
        if not source.is_file():
            raise SystemExit(f"canonical source does not exist: {page['source_path']}")

    return data


def page_output(output: Path, public_path: str) -> Path:
    if public_path == "/":
        return output / "index.html"
    return output / public_path.lstrip("/") / "index.html"


def canonical_url(base_url: str | None, public_path: str) -> str | None:
    if not base_url:
        return None
    base = base_url.rstrip("/") + "/"
    return urljoin(base, public_path.lstrip("/"))


def nav_html(pages: list[dict]) -> str:
    primary = [
        ("/", "Home"),
        ("/formats/", "Formats"),
        ("/archives/nbz/", "NBZ"),
        ("/gdspaces/", "GDSpaces"),
        ("/reverse-engineering/", "Reverse Engineering"),
        ("/faq/", "FAQ"),
        ("/status/", "Status"),
    ]
    valid = {p["path"] for p in pages}
    return "".join(
        f'<a href="{path}">{html.escape(label)}</a>' for path, label in primary if path in valid
    )


def render_page(site: dict, page: dict, base_url: str | None) -> str:
    repo = site["repository_url"].rstrip("/")
    source_url = f"{repo}/blob/main/{page['source_path']}"
    canonical = canonical_url(base_url, page["path"])
    canonical_tag = (
        f'\n    <link rel="canonical" href="{html.escape(canonical, quote=True)}">'
        if canonical
        else ""
    )

    return f"""<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>{html.escape(page['title'])}</title>
    <meta name="description" content="{html.escape(page['description'], quote=True)}">{canonical_tag}
    <meta name="robots" content="index,follow">
    <meta property="og:type" content="website">
    <meta property="og:title" content="{html.escape(page['title'], quote=True)}">
    <meta property="og:description" content="{html.escape(page['description'], quote=True)}">
    <link rel="stylesheet" href="/assets/style.css">
</head>
<body>
<header class="site-header">
    <a class="brand" href="/">DMC Rengine</a>
    <nav>{nav_html(site['pages'])}</nav>
</header>
<main>
    <p class="eyebrow">Devil May Cry 3 HD Collection · Evidence-first C++20 research</p>
    <h1>{html.escape(page['title'])}</h1>
    <p class="lead">{html.escape(page['summary'])}</p>

    <section class="authority">
        <h2>Canonical source</h2>
        <p>This page is a public discovery and readability layer. It does not replace the repository's current implementation, status, evidence or format-specific documentation.</p>
        <p><a class="button" href="{html.escape(source_url, quote=True)}">Open canonical GitHub source</a></p>
    </section>

    <section>
        <h2>Evidence boundary</h2>
        <p>A parser is not automatically a writer. A successful writer is not automatically equivalent to Capcom's offline tools. Synthetic validation does not establish original-game behavioral equivalence.</p>
        <p>Use the <a href="/status/">current status</a> for promoted capabilities and open proof gates.</p>
    </section>
</main>
<footer>
    <p>DMC Rengine is an independent community research/modding project and is not affiliated with or endorsed by Capcom.</p>
    <p><a href="{html.escape(repo, quote=True)}">GitHub repository</a></p>
</footer>
</body>
</html>
"""


def build(output: Path, base_url: str | None) -> None:
    site = load_manifest()
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)

    assets = output / "assets"
    assets.mkdir(parents=True)
    shutil.copy2(CSS_SOURCE, assets / "style.css")

    for page in site["pages"]:
        target = page_output(output, page["path"])
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(render_page(site, page, base_url), encoding="utf-8")

    robots = ["User-agent: *", "Allow: /"]
    if base_url:
        robots.append(f"Sitemap: {base_url.rstrip('/')}/sitemap.xml")
    (output / "robots.txt").write_text("\n".join(robots) + "\n", encoding="utf-8")

    if base_url:
        urls = "\n".join(
            f"  <url><loc>{html.escape(canonical_url(base_url, p['path']) or '')}</loc></url>"
            for p in site["pages"]
        )
        sitemap = (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">\n'
            f"{urls}\n"
            "</urlset>\n"
        )
        (output / "sitemap.xml").write_text(sitemap, encoding="utf-8")

    # Fail closed if the key public surfaces were not generated.
    required = [
        output / "index.html",
        output / "formats" / "index.html",
        output / "faq" / "index.html",
        output / "status" / "index.html",
        output / "robots.txt",
    ]
    if base_url:
        required.append(output / "sitemap.xml")
    missing = [str(p) for p in required if not p.is_file()]
    if missing:
        raise SystemExit("missing generated site files: " + ", ".join(missing))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="_site")
    parser.add_argument("--base-url", default=None)
    args = parser.parse_args()
    build(ROOT / args.output, args.base_url)
