#!/usr/bin/env python3
"""Build the DMC Rengine discovery site with the canonical format encyclopedia."""

from __future__ import annotations

import argparse
import html
import json
import posixpath
import re
from pathlib import Path
from urllib.parse import urlsplit

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


def _lesson_slug(source_path: str) -> str:
    stem = Path(source_path).stem.lower()
    return re.sub(r"[^a-z0-9]+", "-", stem).strip("-")


def _markdown_heading(markdown: str, fallback: str) -> str:
    for line in markdown.splitlines():
        if line.startswith("# "):
            return line[2:].strip()
    return fallback


def _expand_native_lessons(site: dict) -> dict:
    pages = site.get("pages", [])
    by_path = {page.get("path"): page for page in pages if isinstance(page, dict)}
    mod = by_path.get("/formats/mod/")
    if not isinstance(mod, dict):
        raise SystemExit("native MOD lessons require /formats/mod/")
    lessons = mod.get("learning_links")
    if not isinstance(lessons, list) or not lessons:
        raise SystemExit("native MOD lessons require canonical learning_links")

    for lesson in lessons:
        lesson["route"] = f"/formats/mod/lessons/{_lesson_slug(lesson['path'])}/"

    total = len(lessons)
    for index, lesson in enumerate(lessons):
        source_path = lesson["path"]
        markdown = (base.ROOT / source_path).read_text(encoding="utf-8")
        heading = _markdown_heading(markdown, lesson["title"])
        route = lesson["route"]
        previous_route = lessons[index - 1]["route"] if index > 0 else None
        next_route = lessons[index + 1]["route"] if index + 1 < total else None
        related = [
            {"path": "/formats/mod/", "label": "Return to the DMC3 MOD format and learning hub"}
        ]
        if previous_route:
            related.append({"path": previous_route, "label": "Read the previous MOD lesson in the canonical course"})
        if next_route:
            related.append({"path": next_route, "label": "Continue to the next MOD lesson in the canonical course"})

        page = {
            "path": route,
            "title": f"{heading} — DMC Rengine",
            "description": f"{lesson['description']} Canonical DMC Rengine MOD learning material rendered on the project site.",
            "summary": f"Lesson {index + 1} of {total} in the canonical DMC3 MOD learning path. {lesson['description']} The repository Markdown remains the source authority for this educational page.",
            "source_path": source_path,
            "sections": [
                {
                    "heading": f"Lesson {index + 1} scope",
                    "items": [
                        f"This page renders the canonical repository lesson {source_path} as first-class DMC Rengine Pages content; the source Markdown remains authoritative and is not replaced by a separate website-only copy.",
                        f"Within the {total}-part MOD learning sequence, this lesson focuses on the following bounded topic: {lesson['description']}"
                    ],
                },
                {
                    "heading": f"How to use lesson {index + 1}",
                    "items": [
                        f"Read this material together with the MOD format page and the neighboring lessons in the declared course order; educational wording does not promote a technical claim beyond the implementation, research and evidence recorded in main.",
                        f"Navigation for this lesson is generated from site/format-guides.json, while all detailed content below is rendered directly from {source_path} so future canonical learning updates flow into the public site automatically."
                    ],
                },
            ],
            "related_links": related,
            "lesson_document": markdown,
            "lesson_index": index,
            "lesson_total": total,
            "lesson_previous": previous_route,
            "lesson_next": next_route,
            "lesson_source_routes": {item["path"]: item["route"] for item in lessons},
        }
        if route in by_path:
            raise SystemExit(f"duplicate native lesson route: {route}")
        pages.append(page)
        by_path[route] = page

    return site


def _resolve_markdown_target(target: str, source_path: str, source_routes: dict[str, str], repository_url: str, base_url: str | None) -> str:
    if target.startswith("#"):
        return target
    parsed = urlsplit(target)
    if parsed.scheme in ("http", "https"):
        return target
    if parsed.scheme or target.startswith("//"):
        return "#"

    raw_path, separator, fragment = target.partition("#")
    resolved = posixpath.normpath(posixpath.join(posixpath.dirname(source_path), raw_path))
    if resolved.startswith("../") or resolved == "..":
        return "#"
    if resolved in source_routes:
        href = base.public_url(base_url, source_routes[resolved])
    else:
        href = f"{repository_url.rstrip('/')}/blob/main/{resolved}"
    if separator and fragment:
        href += "#" + fragment
    return href


def _inline_markdown(text: str, page: dict, site: dict, base_url: str | None) -> str:
    escaped = html.escape(text, quote=False)
    source_path = page["source_path"]
    routes = page.get("lesson_source_routes", {})
    repository_url = site["repository_url"]

    def link_repl(match: re.Match[str]) -> str:
        label = match.group(1)
        target = html.unescape(match.group(2))
        href = _resolve_markdown_target(target, source_path, routes, repository_url, base_url)
        return f'<a href="{html.escape(href, quote=True)}">{label}</a>'

    escaped = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", link_repl, escaped)
    escaped = re.sub(r"`([^`]+)`", r"<code>\1</code>", escaped)
    escaped = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", escaped)
    return escaped


def _render_markdown(markdown: str, page: dict, site: dict, base_url: str | None) -> str:
    lines = markdown.splitlines()
    rendered: list[str] = []
    index = 0
    skipped_title = False

    while index < len(lines):
        line = lines[index]
        stripped = line.strip()
        if not stripped:
            index += 1
            continue

        if stripped.startswith("```"):
            language = stripped[3:].strip()
            index += 1
            code: list[str] = []
            while index < len(lines) and not lines[index].strip().startswith("```"):
                code.append(lines[index])
                index += 1
            if index < len(lines):
                index += 1
            class_attr = f' class="language-{html.escape(language, quote=True)}"' if language else ""
            rendered.append(f"<pre><code{class_attr}>{html.escape(chr(10).join(code))}</code></pre>")
            continue

        heading_match = re.match(r"^(#{1,6})\s+(.+)$", stripped)
        if heading_match:
            level = len(heading_match.group(1))
            text = heading_match.group(2)
            if level == 1 and not skipped_title:
                skipped_title = True
                index += 1
                continue
            html_level = min(4, max(2, level))
            rendered.append(f"<h{html_level}>{_inline_markdown(text, page, site, base_url)}</h{html_level}>")
            index += 1
            continue

        if re.match(r"^[-*]\s+", stripped):
            items: list[str] = []
            while index < len(lines):
                item_match = re.match(r"^\s*[-*]\s+(.+)$", lines[index])
                if not item_match:
                    break
                items.append(f"<li>{_inline_markdown(item_match.group(1), page, site, base_url)}</li>")
                index += 1
            rendered.append("<ul>" + "".join(items) + "</ul>")
            continue

        if re.match(r"^\d+\.\s+", stripped):
            items = []
            while index < len(lines):
                item_match = re.match(r"^\s*\d+\.\s+(.+)$", lines[index])
                if not item_match:
                    break
                items.append(f"<li>{_inline_markdown(item_match.group(1), page, site, base_url)}</li>")
                index += 1
            rendered.append("<ol>" + "".join(items) + "</ol>")
            continue

        if stripped.startswith(">"):
            quote_lines: list[str] = []
            while index < len(lines) and lines[index].lstrip().startswith(">"):
                quote_lines.append(lines[index].lstrip()[1:].strip())
                index += 1
            rendered.append(f"<blockquote>{_inline_markdown(' '.join(quote_lines), page, site, base_url)}</blockquote>")
            continue

        if stripped in ("---", "***"):
            rendered.append("<hr>")
            index += 1
            continue

        paragraph = [stripped]
        index += 1
        while index < len(lines):
            candidate = lines[index].strip()
            if not candidate:
                break
            if candidate.startswith("```") or re.match(r"^(#{1,6})\s+", candidate):
                break
            if re.match(r"^[-*]\s+", candidate) or re.match(r"^\d+\.\s+", candidate) or candidate.startswith(">"):
                break
            paragraph.append(candidate)
            index += 1
        rendered.append(f"<p>{_inline_markdown(' '.join(paragraph), page, site, base_url)}</p>")

    return "\n".join(rendered)


def _lesson_navigation(page: dict, base_url: str | None) -> str:
    parts = [f'<a href="{html.escape(base.public_url(base_url, "/formats/mod/"), quote=True)}">MOD learning hub</a>']
    if page.get("lesson_previous"):
        parts.insert(0, f'<a href="{html.escape(base.public_url(base_url, page["lesson_previous"]), quote=True)}">← Previous lesson</a>')
    if page.get("lesson_next"):
        parts.append(f'<a href="{html.escape(base.public_url(base_url, page["lesson_next"]), quote=True)}">Next lesson →</a>')
    return '<nav class="lesson-navigation" aria-label="Lesson navigation">' + "".join(parts) + "</nav>"


def _inject_native_lesson(document: str, site: dict, page: dict, base_url: str | None) -> str:
    markdown = page.get("lesson_document")
    if not isinstance(markdown, str):
        return document
    marker = '<section class="related">'
    if marker not in document:
        raise SystemExit(f"cannot inject native lesson into {page.get('path')}")
    lesson_html = (
        _lesson_navigation(page, base_url)
        + '<article class="lesson-body">'
        + _render_markdown(markdown, page, site, base_url)
        + '</article>'
        + _lesson_navigation(page, base_url)
    )
    return document.replace(marker, lesson_html + "\n" + marker, 1)


def _rewrite_learning_cards(document: str, site: dict, page: dict, base_url: str | None) -> str:
    lessons = page.get("learning_links")
    if not isinstance(lessons, list):
        return document
    repo = site["repository_url"].rstrip("/")
    for lesson in lessons:
        route = lesson.get("route")
        if not isinstance(route, str):
            continue
        source_url = f"{repo}/blob/main/{lesson['path']}"
        escaped_source = html.escape(source_url, quote=True)
        needle = f'<a href="{escaped_source}">Open lesson</a>'
        native_url = html.escape(base.public_url(base_url, route), quote=True)
        replacement = (
            f'<a href="{native_url}">Read lesson on site</a>'
            f' <span aria-hidden="true">·</span> <a href="{escaped_source}">Source</a>'
        )
        document = document.replace(needle, replacement)
    return document


def load_manifest() -> dict:
    raw = json.loads(base.MANIFEST.read_text(encoding="utf-8"))
    expanded = expand_manifest(raw, base.ROOT)
    expanded = _refine_presence_boundaries(expanded)
    expanded = _expand_native_lessons(expanded)
    return base.validate_manifest(expanded)


def render_page(site: dict, page: dict, base_url: str | None) -> str:
    document = _ORIGINAL_RENDER_PAGE(site, page, base_url)
    document = inject_page_extras(document, site, page, base_url)
    document = _inject_native_lesson(document, site, page, base_url)
    return _rewrite_learning_cards(document, site, page, base_url)


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
