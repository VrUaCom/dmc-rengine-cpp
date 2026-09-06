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
from urllib.parse import urljoin, urlparse

ROOT = Path(__file__).resolve().parents[1]
ROOT_RESOLVED = ROOT.resolve()
MANIFEST = ROOT / "site" / "manifest.json"
CSS_SOURCE = ROOT / "site" / "assets" / "style.css"


def require_within_repo(path: Path, label: str) -> Path:
    resolved = path.resolve()
    try:
        resolved.relative_to(ROOT_RESOLVED)
    except ValueError as exc:
        raise SystemExit(f"{label} escapes repository: {path}") from exc
    return resolved


def require_safe_output(path: Path) -> Path:
    resolved = require_within_repo(path, "site output")
    if resolved == ROOT_RESOLVED:
        raise SystemExit("site output must not be the repository root")
    relative = resolved.relative_to(ROOT_RESOLVED)
    if not relative.parts or not relative.parts[0].startswith("_site"):
        raise SystemExit("site output must live in a reserved _site* workspace")
    return resolved


def normalize_base_url(base_url: str | None) -> str | None:
    if not base_url:
        return None
    parsed = urlparse(base_url)
    if parsed.scheme != "https" or not parsed.netloc:
        raise SystemExit("site base URL must be an absolute HTTPS URL")
    if parsed.username or parsed.password or parsed.params or parsed.query or parsed.fragment:
        raise SystemExit("site base URL must not contain credentials, params, query, or fragment")
    path = parsed.path.rstrip("/")
    return f"https://{parsed.netloc}{path}"


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
        if ".." in Path(path).parts:
            raise SystemExit(f"public path contains traversal: {path}")
        if path in seen:
            raise SystemExit(f"duplicate public path: {path}")
        seen.add(path)

        source = require_within_repo(ROOT / page["source_path"], "canonical source")
        if not source.is_file():
            raise SystemExit(f"canonical source does not exist: {page['source_path']}")

    return data


def page_output(output: Path, public_path: str) -> Path:
    if public_path == "/":
        return output / "index.html"
    return output / public_path.lstrip("/") / "index.html"


def public_url(base_url: str | None, public_path: str) -> str:
    if not base_url:
        return public_path
    base = base_url.rstrip("/") + "/"
    return urljoin(base, public_path.lstrip("/"))


def canonical_url(base_url: str | None, public_path: str) -> str | None:
    return public_url(base_url, public_path) if base_url else None


def nav_html(pages: list[dict], base_url: str | None) -> str:
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
        f'<a href="{html.escape(public_url(base_url, path), quote=True)}">{html.escape(label)}</a>'
        for path, label in primary
        if path in valid
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
    og_url = (
        f'\n    <meta property="og:url" content="{html.escape(canonical, quote=True)}">'
        if canonical
        else ""
    )
    css_url = public_url(base_url, "/assets/style.css")
    home_url = public_url(base_url, "/")
    status_url = public_url(base_url, "/status/")

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
    <meta property="og:description" content="{html.escape(page['description'], quote=True)}">{og_url}
    <link rel="stylesheet" href="{html.escape(css_url, quote=True)}">
</head>
<body>
<header class="site-header">
    <a class="brand" href="{html.escape(home_url, quote=True)}">DMC Rengine</a>
    <nav>{nav_html(site['pages'], base_url)}</nav>
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
        <p>Use the <a href="{html.escape(status_url, quote=True)}">current status</a> for promoted capabilities and open proof gates.</p>
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
    base_url = normalize_base_url(base_url)
    output = require_safe_output(output)

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

    robots = [
        "User-agent: *",
        "Allow: /",
        "",
        "User-agent: OAI-SearchBot",
        "Allow: /",
    ]
    if base_url:
        robots.extend(["", f"Sitemap: {base_url.rstrip('/')}/sitemap.xml"])
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
