#!/usr/bin/env python3
"""Canonical format-catalog expansion and rendering for the public discovery site."""

from __future__ import annotations

import copy
import html
import json
import re
from collections import defaultdict
from pathlib import Path
from urllib.parse import urljoin

REGISTRY_REL = "docs/formats/dmc3-hd-format-purpose-registry.json"
GUIDES_REL = "site/format-guides.json"

SPECIAL_ROUTES = {
    "nbz": "/archives/nbz/",
    "pac": "/formats/pac/",
    "pnst": "/formats/pnst/",
    "scm": "/formats/scm/",
    "mod": "/formats/mod/",
    "shw": "/formats/shw/",
    "hits": "/formats/hits/",
}

EXCLUDED_IDENTITY = {"REJECTED", "SYNTHETIC_ONLY"}
EXCLUDED_PRODUCT = {"none", "synthetic-only"}


def public_url(base_url: str | None, public_path: str) -> str:
    if not base_url:
        return public_path
    return urljoin(base_url.rstrip("/") + "/", public_path.lstrip("/"))


def _load_json(path: Path, label: str) -> dict:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SystemExit(f"cannot load {label}: {path}") from exc
    if not isinstance(data, dict):
        raise SystemExit(f"{label} root must be an object: {path}")
    return data


def load_registry(root: Path) -> list[dict]:
    data = _load_json(root / REGISTRY_REL, "format purpose registry")
    columns = data.get("columns")
    families = data.get("families")
    if not isinstance(columns, list) or not columns or not isinstance(families, list):
        raise SystemExit("format purpose registry must contain columns and families")

    entries: list[dict] = []
    seen_ids: set[str] = set()
    for row in families:
        if not isinstance(row, list) or len(row) != len(columns):
            raise SystemExit(f"malformed format registry row: {row!r}")
        entry = dict(zip(columns, row, strict=True))
        entry_id = entry.get("id")
        if not isinstance(entry_id, str) or not entry_id:
            raise SystemExit(f"format registry row has invalid id: {row!r}")
        if entry_id in seen_ids:
            raise SystemExit(f"duplicate format registry id: {entry_id}")
        seen_ids.add(entry_id)
        entries.append(entry)
    return entries


def is_public_format(entry: dict) -> bool:
    return (
        entry.get("identityStatus") not in EXCLUDED_IDENTITY
        and entry.get("productStatus") not in EXCLUDED_PRODUCT
    )


def public_format_entries(root: Path) -> list[dict]:
    return [entry for entry in load_registry(root) if is_public_format(entry)]


def format_route(entry: dict) -> str:
    entry_id = entry["id"]
    return SPECIAL_ROUTES.get(entry_id, f"/formats/{entry_id}/")


def primary_name(entry: dict) -> str:
    names = str(entry.get("names", entry["id"]))
    return names.split(",", 1)[0].strip()


def domain_label(domain: str) -> str:
    return domain.replace("-", " ").title()


def status_class(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")


def generic_format_page(entry: dict) -> dict:
    name = primary_name(entry)
    names = str(entry["names"])
    domain = str(entry["domain"])
    purpose = str(entry["purpose"])
    identity = str(entry["identityStatus"])
    purpose_status = str(entry["purposeStatus"])
    schema = str(entry["schemaStatus"])
    product = str(entry["productStatus"])
    route = format_route(entry)

    return {
        "path": route,
        "title": f"DMC3 {name} Resource Family — DMC Rengine",
        "description": f"DMC3 {name} resource-family guide: {purpose} Canonical evidence and product status are shown explicitly.",
        "summary": f"Evidence-aware entry point for {name} in Devil May Cry 3 HD. The page explains the known purpose, identifiers, current schema/product maturity and a practical route for inspecting the family without promoting unresolved semantics.",
        "source_path": REGISTRY_REL,
        "sections": [
            {
                "heading": f"What {name} is",
                "items": [
                    f"The canonical registry places {name} in the {domain_label(domain)} domain and records its purpose as: {purpose}",
                    f"Known identifiers for this family are recorded as {names}. The public page keeps those physical or logical identifiers separate instead of assuming that similarly named extensions, runtime objects or converted representations are interchangeable.",
                    f"This page is generated from the canonical purpose registry rather than from filename intuition, so its description follows the currently promoted DMC Rengine evidence boundary for the {entry['id']} family."
                ],
            },
            {
                "heading": "Evidence and implementation status",
                "items": [
                    f"Identity status is {identity}, purpose status is {purpose_status}, schema status is {schema}, and current product status is {product}; these fields describe different proof dimensions and must not be collapsed into one generic supported/unsupported flag.",
                    f"A {schema} schema status does not automatically authorize writing, conversion or original-game reintegration, while a {product} product status does not upgrade unresolved field semantics beyond the evidence recorded for this exact family.",
                    "When the registry says OPEN, RESEARCH_REQUIRED, runtime-only or capability-only, the site preserves that limitation visibly instead of filling the missing knowledge with presentation-layer guesses."
                ],
            },
            {
                "heading": "Practical tutorial path",
                "items": [
                    f"First locate a real {name} instance through the canonical GDSpaces/container path and preserve its provenance: numbered NBZ volume, container/member identity, slot position and original byte range should remain attached to the sample you inspect.",
                    f"Next confirm identity using the evidence appropriate to {name}: extension, magic, dispatcher, registry mapping or structural checks should be used only where the canonical research actually establishes them, and ambiguous .bin/.ukn-style naming must not be treated as semantic proof.",
                    f"Then compare the sample with the canonical parser, tests and research notes for the {entry['id']} family. Promote a field from raw bytes to typed meaning only when executable, corpus or structural evidence reaches the project's required evidence level."
                ],
            },
            {
                "heading": "Current limitations and safe next steps",
                "items": [
                    f"The current public status for {name} is {product} with schema state {schema}; operations beyond that boundary require a separate implementation and validation receipt rather than an assumption that recognition implies editing support.",
                    "If a writer exists later, structural round-trip tests, preservation of unknown bytes, reintegration into the correct container lineage and original-game consumption are separate gates; none is implied merely by this encyclopedia page.",
                    f"For deeper work, use the canonical source and Current Status links below, then follow the reverse-engineering workflow to turn remaining {name} unknowns into bounded research targets instead of broad speculative format claims."
                ],
            },
        ],
        "related_links": [
            {"path": "/formats/", "label": "Return to the complete DMC3 HD format catalog"},
            {"path": "/status/", "label": f"Check current {name} implementation and evidence maturity"},
            {"path": "/gdspaces/", "label": "Understand resource provenance and materialization through GDSpaces"},
        ],
        "format_entry": {**entry, "route": route},
    }


def load_guides(root: Path) -> dict[str, dict]:
    path = root / GUIDES_REL
    data = _load_json(path, "format guides")
    guides = data.get("guides", {})
    if not isinstance(guides, dict):
        raise SystemExit("format guides must contain an object named guides")

    for entry_id, guide in guides.items():
        if not isinstance(entry_id, str) or not isinstance(guide, dict):
            raise SystemExit("format guide entries must be id -> object mappings")
        source_path = guide.get("source_path")
        if not isinstance(source_path, str) or not (root / source_path).is_file():
            raise SystemExit(f"format guide canonical source does not exist: {entry_id}")
        sections = guide.get("sections", [])
        lessons = guide.get("lessons", [])
        if not isinstance(sections, list) or not isinstance(lessons, list):
            raise SystemExit(f"format guide sections/lessons must be lists: {entry_id}")
        for lesson in lessons:
            if not isinstance(lesson, dict):
                raise SystemExit(f"malformed lesson in format guide: {entry_id}")
            for field in ("title", "path", "description"):
                if not isinstance(lesson.get(field), str) or not lesson[field].strip():
                    raise SystemExit(f"lesson is missing {field}: {entry_id}")
            if not (root / lesson["path"]).is_file():
                raise SystemExit(f"lesson source does not exist: {lesson['path']}")
    return guides


def expand_manifest(base_manifest: dict, root: Path) -> dict:
    site = copy.deepcopy(base_manifest)
    pages = site.get("pages")
    if not isinstance(pages, list):
        raise SystemExit("site manifest pages must be a list before catalog expansion")

    entries = public_format_entries(root)
    by_path = {page.get("path"): page for page in pages if isinstance(page, dict)}
    catalog: list[dict] = []

    for entry in entries:
        route = format_route(entry)
        page = by_path.get(route)
        if page is None:
            page = generic_format_page(entry)
            pages.append(page)
            by_path[route] = page
        else:
            page["format_entry"] = {**entry, "route": route}

        catalog.append({**entry, "route": route})

    guides = load_guides(root)
    entries_by_id = {entry["id"]: entry for entry in entries}
    for entry_id, guide in guides.items():
        entry = entries_by_id.get(entry_id)
        if entry is None:
            raise SystemExit(f"format guide targets a non-public registry id: {entry_id}")
        route = format_route(entry)
        page = by_path[route]
        page["source_path"] = guide["source_path"]
        page.setdefault("sections", []).extend(copy.deepcopy(guide.get("sections", [])))
        page["learning_links"] = copy.deepcopy(guide.get("lessons", []))

    formats_page = by_path.get("/formats/")
    if formats_page is None:
        raise SystemExit("expanded site must contain /formats/")
    formats_page["format_catalog"] = catalog

    site["format_catalog"] = catalog
    return site


def render_format_status(page: dict) -> str:
    entry = page.get("format_entry")
    if not isinstance(entry, dict):
        return ""

    name = html.escape(primary_name(entry))
    names = html.escape(str(entry["names"]))
    domain = html.escape(domain_label(str(entry["domain"])))
    statuses = [
        ("Identity", str(entry["identityStatus"])),
        ("Purpose", str(entry["purposeStatus"])),
        ("Schema", str(entry["schemaStatus"])),
        ("Product", str(entry["productStatus"])),
    ]
    badges = "".join(
        f'<span class="badge badge-{status_class(value)}"><b>{html.escape(label)}:</b> {html.escape(value)}</span>'
        for label, value in statuses
    )
    return (
        '<section class="format-status">'
        f'<h2>{name} at a glance</h2>'
        '<dl class="format-facts">'
        f'<div><dt>Identifiers</dt><dd>{names}</dd></div>'
        f'<div><dt>Domain</dt><dd>{domain}</dd></div>'
        '</dl>'
        f'<div class="format-badges">{badges}</div>'
        '</section>'
    )


def render_format_catalog(page: dict, base_url: str | None) -> str:
    catalog = page.get("format_catalog")
    if page.get("path") != "/formats/" or not isinstance(catalog, list):
        return ""

    grouped: dict[str, list[dict]] = defaultdict(list)
    for entry in catalog:
        grouped[str(entry["domain"])].append(entry)

    groups: list[str] = []
    for domain in sorted(grouped, key=lambda value: domain_label(value).lower()):
        cards: list[str] = []
        for entry in sorted(grouped[domain], key=lambda value: primary_name(value).lower()):
            name = primary_name(entry)
            href = html.escape(public_url(base_url, entry["route"]), quote=True)
            purpose = html.escape(str(entry["purpose"]))
            badges = "".join(
                f'<span class="badge badge-{status_class(str(entry[key]))}">{html.escape(str(entry[key]))}</span>'
                for key in ("identityStatus", "schemaStatus", "productStatus")
            )
            cards.append(
                '<a class="format-card" href="' + href + '">'
                f'<span class="format-card-name">{html.escape(name)}</span>'
                f'<span class="format-card-purpose">{purpose}</span>'
                f'<span class="format-badges">{badges}</span>'
                '</a>'
            )
        groups.append(
            '<section class="format-domain">'
            f'<h2>{html.escape(domain_label(domain))}</h2>'
            f'<div class="format-grid">{"".join(cards)}</div>'
            '</section>'
        )

    return (
        '<section class="format-catalog-intro">'
        '<h2>Complete canonical format and resource-family catalog</h2>'
        '<p>Select a family to open its dedicated evidence-aware page. Rejected historical identifiers and synthetic test-only containers are intentionally excluded from the shipped-format list.</p>'
        f'<p class="catalog-count">{len(catalog)} canonical public families are currently represented.</p>'
        '</section>'
        + "".join(groups)
    )


def render_learning(page: dict, repository_url: str) -> str:
    lessons = page.get("learning_links")
    if not isinstance(lessons, list) or not lessons:
        return ""

    repo = repository_url.rstrip("/")
    cards: list[str] = []
    for lesson in lessons:
        source_url = f"{repo}/blob/main/{lesson['path']}"
        cards.append(
            '<article class="lesson-card">'
            f'<h3>{html.escape(lesson["title"])}</h3>'
            f'<p>{html.escape(lesson["description"])}</p>'
            f'<p><a href="{html.escape(source_url, quote=True)}">Open lesson</a></p>'
            '</article>'
        )
    return (
        '<section class="learning-path">'
        '<h2>MOD learning path</h2>'
        '<p>These lessons are the repository’s canonical learning material for this format. They follow the current reverse evidence rather than a simplified third-party schema.</p>'
        f'<div class="lesson-grid">{"".join(cards)}</div>'
        '</section>'
    )


def inject_page_extras(document: str, site: dict, page: dict, base_url: str | None) -> str:
    marker = '<section class="related">'
    if marker not in document:
        raise SystemExit(f"cannot inject format encyclopedia content into {page.get('path')}")

    extras = "\n".join(
        part
        for part in (
            render_format_status(page),
            render_format_catalog(page, base_url),
            render_learning(page, site["repository_url"]),
        )
        if part
    )
    if not extras:
        return document
    return document.replace(marker, extras + "\n" + marker, 1)


def site_index_payload(site: dict) -> dict:
    pages = site["pages"]
    catalog = site.get("format_catalog", [])
    return {
        "schema": "dmc-rengine.discovery-site-index.v1",
        "page_count": len(pages),
        "format_count": len(catalog),
        "paths": [page["path"] for page in pages],
        "formats": [
            {
                "id": entry["id"],
                "names": entry["names"],
                "domain": entry["domain"],
                "route": entry["route"],
                "identityStatus": entry["identityStatus"],
                "purposeStatus": entry["purposeStatus"],
                "schemaStatus": entry["schemaStatus"],
                "productStatus": entry["productStatus"],
            }
            for entry in catalog
        ],
    }


def write_site_index(output: Path, site: dict) -> None:
    payload = site_index_payload(site)
    (output / "site-index.json").write_text(
        json.dumps(payload, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
