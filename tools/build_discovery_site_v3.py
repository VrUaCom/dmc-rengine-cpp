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
    base.ROOT / "site" / "intent-pages-wave4.json",
)
_V2_LOAD_MANIFEST = v2.load_manifest
INDEXNOW_KEY_SOURCE = base.ROOT / "site" / "6ef2bc73288e1e2b580119a4b1bfc2fa.txt"
INDEXNOW_ALLOWED_CHARS = set(
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-"
)


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


def _append_related(page: dict, path: str, label: str) -> None:
    links = page.setdefault("related_links", [])
    if not any(isinstance(link, dict) and link.get("path") == path for link in links):
        links.append({"path": path, "label": label})


def _promote_mod_writer_gate(by_path: dict[str, dict]) -> None:
    mod_format = by_path["/formats/mod/"]
    mod_format["summary"] = (
        "The DMC3 HD MOD reverse is complete for the canonical project scope: serialized layout, "
        "runtime-facing behavior and preservation boundaries have terminal evidence states. Writer, "
        "retail NBZ and original-game acceptance remain separate capability gates."
    )

    inspect_mod = by_path["/guides/inspect-mod/"]
    inspect_mod["description"] = (
        "DMC3 MOD reverse-complete viewer and bounded preserve-layout writer guide covering the "
        "canonical model contract, retail PNST reintegration and remaining authoring gates."
    )
    inspect_mod["summary"] = (
        "Open and inspect DMC3 MOD files through the completed canonical reverse contract. Every "
        "relevant serialized/runtime domain has a terminal evidence state, including deliberate "
        "PRESERVED_UNDECODED outcomes where stronger semantic names are not proven. Preserve-Layout "
        "Writer Gate 1 and real retail PNST reintegration are separate bounded authoring capabilities; "
        "retail NBZ selection and original-game acceptance remain open."
    )
    inspect_mod["sections"] = [
        {
            "heading": "MOD reverse complete",
            "items": [
                "The canonical DMC3 HD MOD reverse is complete: document/object/mesh ABI, hierarchy and ordering, local/world transforms, skinning, topology, texture binding, runtime object projection, post-load relocation and the MOD-side animation boundary all have evidence-backed terminal states.",
                "Reverse completeness does not require speculative artistic names. PRESERVED_UNDECODED, RESERVED_OBSERVED_ZERO and REJECTED are valid terminal outcomes when the strongest evidence supports preservation, bounded zero observation or rejection rather than a stronger semantic label.",
                "Header +0x14, BLENDINDICES.x, source flag 0x00200000, mesh +0x0C/+0x38/+0x4C and transform +0x1C therefore no longer represent open MOD reverse work; their preservation/family-sensitive boundaries are the closed result."
            ],
        },
        {
            "heading": "Preserve-layout writer and retail PNST gate",
            "items": [
                "Preserve-Layout Writer Gate 1 starts from an immutable original serialized image and authorizes only fixed-size edits to already-typed spans such as object bounds and existing position, normal and UV streams.",
                "The provenance-bound no-edit corpus gate passes 38/38 retail MOD files with exact source/output equality across 882,736 source bytes, while one controlled em000_021.mod bounding-radius edit changes exactly three authorized bytes and survives disk reread and canonical reopen.",
                "PR #372 proves provenance-bound real retail PNST reintegration for the same-size authored MOD child: parent size and slot table remain unchanged, only the expected three child bytes change, and canonical reparse/re-expand returns the exact writer output."
            ],
        },
        {
            "heading": "What remains after reverse completion",
            "items": [
                "A synthetic MOD -> container -> NBZ overlay -> reopen chain is proven through the existing NBZ infrastructure, but provenance-bound retail NBZ selection and original dmc3.exe consumption remain separate acceptance gates.",
                "Typed-IR-only layout synthesis/reflow, transform and skin authoring, material/source-flag/texture-binding authoring, texture-companion rewriting and broader mutation authority for preservation-only fields remain writer work rather than unfinished reverse.",
                "The public surface may say MOD reverse complete or fully reverse-engineered within the canonical DMC3 HD scope, but it must not claim a full MOD writer, arbitrary original-game-safe editing or Capcom authoring-tool equivalence."
            ],
        },
    ]
    _append_related(
        inspect_mod,
        "/formats/mod/",
        "Read the reverse-complete canonical DMC3 MOD format map",
    )
    _append_related(
        inspect_mod,
        "/models/",
        "Return to the DMC3 model extraction and authoring hub",
    )

    extract_models = by_path["/guides/extract-models/"]
    extract_models["description"] = (
        "DMC3 model extraction guide connecting NBZ/PAC/PNST resources to the reverse-complete "
        "MOD contract, SCM inspection and bounded MOD authoring/reintegration gates."
    )
    extract_models["summary"] = (
        "Find and inspect DMC3 HD model resources while preserving archive/container provenance. "
        "The canonical MOD binary/runtime reverse is complete, while its writer remains deliberately "
        "bounded; SCM stays a separate scene-family authority rather than being flattened into MOD."
    )
    extract_models["sections"] = [
        {
            "heading": "From archive slot to reverse-complete MOD",
            "items": [
                "Model discovery begins in the NBZ materialization path and may cross PAC or PNST before a leaf payload can be classified as MOD, SCM or another family; physical archive and slot provenance stay attached to the result.",
                "For canonical DMC3 HD MOD resources the reverse contract is complete, covering structure, hierarchy, transforms, texture relationships, skinning, topology, post-load behavior and terminal preservation classifications.",
                "SCM remains scene-oriented and uses its own canonical parser/writer authority, so extraction should route each payload through its real family instead of flattening every geometry resource into a generic model representation."
            ],
        },
        {
            "heading": "Reverse complete, authoring still bounded",
            "items": [
                "Preserve-Layout Writer Gate 1 can apply fixed-size edits to object bounds and existing position, normal and UV data while preserving the original physical layout and rejecting unauthorized byte changes.",
                "The no-edit retail corpus passes 38/38 MOD files with exact byte/hash parity and canonical reopen, and PR #372 separately proves real retail PNST reintegration of one same-size authored MOD child.",
                "The synthetic NBZ reopen gate does not establish provenance-bound retail NBZ selection or original dmc3.exe consumption; topology/cardinality, transform, skin, material and texture-companion authoring remain separate writer gates."
            ],
        },
    ]

    models = by_path["/models/"]
    sections = models.setdefault("sections", [])
    if not any(
        isinstance(section, dict)
        and section.get("heading") == "MOD reverse complete; authoring bounded"
        for section in sections
    ):
        sections.append(
            {
                "heading": "MOD reverse complete; authoring bounded",
                "items": [
                    "DMC Rengine now treats the canonical DMC3 HD MOD reverse as complete: every relevant serialized/runtime domain has reached a terminal evidence state, including preservation-only outcomes where stronger semantics are not evidenced.",
                    "The promoted Preserve-Layout Writer Gate 1 remains intentionally narrower than the reverse contract; 38/38 provenance-bound retail MOD files pass no-edit exact parity and canonical reopen, and one authored child has real retail PNST reintegration proof.",
                    "Provenance-bound retail NBZ selection, original-game acceptance and broader transform/skin/material/texture-companion authoring remain separate gates and do not reopen the completed MOD reverse."
                ],
            }
        )
    _append_related(
        models,
        "/guides/inspect-mod/",
        "Inspect the reverse-complete MOD contract and bounded authoring gates",
    )


def _promote_post_372_model_format_truth(by_path: dict[str, dict]) -> None:
    inspect_scm = by_path["/guides/inspect-scm/"]
    inspect_scm["description"] = (
        "DMC3 SCM viewer and bounded writer/rebuild guide for scene hierarchy, geometry, "
        "transforms, canonical_rebuild and current proof boundaries."
    )
    inspect_scm["summary"] = (
        "Inspect DMC3 SCM scene resources through the canonical parser and the selected bounded "
        "authoring stack. PR #372 promotes preserve_layout and deterministic canonical_rebuild, "
        "typed geometry and node-transform edits, output reparse and mutation guards, while "
        "production reintegration and original-game acceptance remain stronger open gates."
    )
    inspect_scm["sections"] = [
        {
            "heading": "Canonical SCM inspection and authoring stack",
            "items": [
                "SCM inspection exposes scene hierarchy, object/mesh relationships, position/normal/UV streams, topology, transforms and texture-facing state without flattening the original scene organization.",
                "The selected canonical writer supports preserve_layout for source-bound same-layout edits and deterministic canonical_rebuild from typed IR, including typed geometry, UV, texture-slot, alpha/filter, GS CLAMP REGION_REPEAT and node translation/rotation edits.",
                "Writer output is canonically reparsed, source-bound mutation guards reject unauthorized state, and layout-changing rebuild fails closed when retained source bytes contain non-zero unmodeled data."
            ],
        },
        {
            "heading": "SCM corpus gate and remaining boundaries",
            "items": [
                "The consolidated receipt covers 78 paths / 68 unique SHA-256 inputs with 78/78 parse, 78/78 preserve-layout exact parity, 78/78 canonical rebuild + reparse and 78/78 canonical exact no-edit parity.",
                "ScmResourceBundleWriter provides bounded SCM/texture-companion count and index coherence through the existing texture framing/reflow path.",
                "Representative provenance-bound semantic edits, retail texture rewriting, real-retail size-changing rebuild, SCM PAC/PNST/NBZ reintegration and original dmc3.exe acceptance remain open, so no production or 100% SCM authoring claim is made."
            ],
        },
    ]

    scm_format = by_path["/formats/scm/"]
    scm_format["summary"] = (
        "Canonical SCM scene-format entry point covering structural inspection plus the bounded "
        "preserve_layout and canonical_rebuild authoring stack promoted by PR #372. The 78-path / "
        "68-unique no-edit corpus is closed at that scope; retail reintegration and original-game "
        "acceptance remain explicit open gates."
    )

    animations = by_path["/guides/animations/"]
    animations["description"] = (
        "DMC3 animation inspection guide connecting MOD hierarchy to the canonical MOT "
        "NativeReaderModuleRegistry path, parser/IR and bounded compression-3 key evaluation."
    )
    animations["summary"] = (
        "Inspect DMC3 animation relationships through MOD hierarchy and the canonical MOT module. "
        "MOT is already registered in NativeReaderModuleRegistry, while PR #372 consolidates its "
        "parser/IR and bounded key-evaluation semantics. Complete animation-player and original-game "
        "parity remain separate proof gates."
    )
    animations["sections"] = [
        {
            "heading": "Canonical MOT structural and reader path",
            "items": [
                "MOT is a canonical NativeReaderModuleRegistry member through native_reader_modules::mot(), while PR #372 consolidates structural decoding onto one modular parser/IR.",
                "The structural contract includes the MOT\\0 marker, aligned header/channel-mask extent, nine-bit channel mask, record-count/popcount relationship, bounded track extents and typed compression-2 and compression-3 key payloads.",
                "Three hash-bound real MOT payloads parse through the modular path, with unknown-compression track bytes preserved rather than normalized into invented semantics."
            ],
        },
        {
            "heading": "Bounded key evaluation versus complete player parity",
            "items": [
                "Canonical-EXE-backed recovery covers nine-channel binding traversal, signed 16-bit track start-time offsets, quantization and compression-3 linear versus cubic/Hermite segment algebra with incoming/outgoing slope orientation.",
                "The interpolation helper is an algebraic semantic recovery, not a bit-identical SSE implementation and not proof of the game's complete segment-search, cache or whole-animation player behavior.",
                "Exact segment lookup and duplicate-time handling, flag 0x2 alternate binding, other compression modes, looping, blending, full transform composition, edited MOT authoring and original-game output comparison remain open."
            ],
        },
    ]

    mot_format = by_path["/formats/mot/"]
    mot_format["summary"] = (
        "Canonical DMC3 MOT entry point: NativeReaderModuleRegistry membership, consolidated parser/IR, "
        "real-payload structural validation and bounded compression-3 key evaluation are promoted; "
        "complete player and original-game parity remain open."
    )

    status = by_path["/status/"]
    status_sections = status.setdefault("sections", [])
    status_sections.append(
        {
            "heading": "Post-PR #372 model-format reconciliation",
            "items": [
                "The canonical DMC3 HD MOD reverse is complete; MOD also has provenance-bound real retail PNST reintegration of a same-size authored child, while the synthetic MOD-to-NBZ reopen gate does not yet prove provenance-bound retail NBZ selection or original-game consumption.",
                "SCM has the selected bounded preserve_layout/canonical_rebuild stack and a 78-path / 68-unique no-edit corpus gate, while production reintegration and original-game acceptance remain open.",
                "NativeReaderModuleRegistry directly includes MOT plus SO graph, SO volume and SO link; registry membership is product integration and does not by itself grant universal semantic or writer authority."
            ],
        }
    )


def _augment_intent_graph(site: dict) -> dict:
    pages = site.get("pages")
    if not isinstance(pages, list):
        raise SystemExit("intent graph requires page list")
    by_path = {
        page.get("path"): page
        for page in pages
        if isinstance(page, dict) and isinstance(page.get("path"), str)
    }

    required = (
        "/guides/",
        "/models/",
        "/textures/",
        "/status/",
        "/formats/mod/",
        "/formats/scm/",
        "/formats/mot/",
        "/guides/extract-models/",
        "/guides/inspect-mod/",
        "/guides/inspect-scm/",
        "/guides/animations/",
        "/guides/character-models/",
        "/guides/enemy-models/",
        "/native-reader/android/",
        "/guides/vergil-model-textures/",
        "/guides/weapon-models/",
        "/guides/blender-import/",
    )
    missing = [path for path in required if path not in by_path]
    if missing:
        raise SystemExit(f"intent graph missing required routes: {missing}")

    guides = by_path["/guides/"]
    sections = guides.setdefault("sections", [])
    if not any(
        isinstance(section, dict)
        and section.get("heading") == "Character and Android viewer entry points"
        for section in sections
    ):
        sections.append(
            {
                "heading": "Character and Android viewer entry points",
                "items": [
                    "Find DMC3 character models through provenance-aware archive navigation and canonical MOD verification instead of relying on one guessed universal filename.",
                    "Use the enemy-model guide for the same typed resource pipeline across enemy assets and variants while keeping inspection distinct from authoring proof.",
                    "Use the DMC Native Reader Android page for the current app-facing MOD, SCM, DDS and PTX boundary; Windows, Web and iOS are not advertised as shipped platforms without implementation evidence.",
                ],
            }
        )
    if not any(
        isinstance(section, dict)
        and section.get("heading") == "Vergil, weapon and Blender workflows"
        for section in sections
    ):
        sections.append(
            {
                "heading": "Vergil, weapon and Blender workflows",
                "items": [
                    "Use the Vergil guide when the search starts from a character name, then verify model candidates through the same archive-to-MOD evidence path and follow texture relationships into PTX or DDS resources.",
                    "Use the weapon-model guide for Yamato, Rebellion, Beowulf, Force Edge and similar named searches without assuming that every weapon has one universal standalone MOD path.",
                    "Use the Blender-import guide to bridge extracted and identified DMC3 resources into independent community DCC tooling while keeping DMC Rengine parser/extraction claims separate from Blender importer or exporter claims.",
                ],
            }
        )
    _append_related(guides, "/guides/character-models/", "Find DMC3 character models")
    _append_related(guides, "/guides/enemy-models/", "Find and inspect DMC3 enemy models")
    _append_related(guides, "/native-reader/android/", "Open the DMC Native Reader Android capability page")
    _append_related(guides, "/guides/vergil-model-textures/", "Find Vergil models and textures through canonical resource evidence")
    _append_related(guides, "/guides/weapon-models/", "Find Yamato, Rebellion, Beowulf and other DMC3 weapon resources")
    _append_related(guides, "/guides/blender-import/", "Connect DMC3 extraction and inspection to community Blender workflows")

    models = by_path["/models/"]
    _append_related(models, "/guides/character-models/", "Find character models through the archive-to-MOD pipeline")
    _append_related(models, "/guides/enemy-models/", "Find and inspect enemy models")
    _append_related(models, "/native-reader/android/", "See current Android MOD and SCM viewing support")
    _append_related(models, "/guides/vergil-model-textures/", "Find Vergil model resources and associated textures")
    _append_related(models, "/guides/weapon-models/", "Find named DMC3 weapon model resources")
    _append_related(models, "/guides/blender-import/", "Continue from model extraction into community Blender import workflows")

    textures = by_path["/textures/"]
    _append_related(textures, "/native-reader/android/", "See current Android DDS and PTX viewing support")
    _append_related(textures, "/guides/dante-model-textures/", "Find Dante-related model and texture resources")
    _append_related(textures, "/guides/vergil-model-textures/", "Find Vergil-related model and texture resources")

    _append_related(by_path["/formats/mod/"], "/guides/blender-import/", "Relate canonical MOD research to community Blender import workflows")
    _append_related(by_path["/formats/scm/"], "/guides/blender-import/", "Relate canonical SCM research to community Blender import workflows")
    _append_related(by_path["/formats/mot/"], "/guides/blender-import/", "Relate MOT research to evidence-bounded Blender import workflows")

    _promote_mod_writer_gate(by_path)
    _promote_post_372_model_format_truth(by_path)
    return site


def _publish_indexnow_key(output: Path) -> None:
    try:
        key = INDEXNOW_KEY_SOURCE.read_text(encoding="utf-8").strip()
    except OSError as exc:
        raise SystemExit(f"IndexNow key source cannot be read: {INDEXNOW_KEY_SOURCE}") from exc

    if not 8 <= len(key) <= 128:
        raise SystemExit("IndexNow key length must be between 8 and 128 characters")
    if any(char not in INDEXNOW_ALLOWED_CHARS for char in key):
        raise SystemExit("IndexNow key contains characters outside the protocol allow-list")
    if INDEXNOW_KEY_SOURCE.stem != key:
        raise SystemExit("IndexNow key filename must match the key value")

    target = output / INDEXNOW_KEY_SOURCE.name
    target.write_text(key + "\n", encoding="utf-8")
    if target.read_text(encoding="utf-8").strip() != key:
        raise SystemExit("generated IndexNow key file failed round-trip validation")


def load_manifest() -> dict:
    site = _V2_LOAD_MANIFEST()
    for manifest_path in INTENT_WAVES:
        site = _merge_intent_wave(site, manifest_path)
    site = _augment_intent_graph(site)
    return base.validate_manifest(site)


def build(output: Path, base_url: str | None) -> None:
    original_load_manifest = v2.load_manifest
    v2.load_manifest = load_manifest
    try:
        v2.build(output, base_url)
    finally:
        v2.load_manifest = original_load_manifest
    _publish_indexnow_key(output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="_site")
    parser.add_argument("--base-url", default=None)
    args = parser.parse_args()
    build(base.ROOT / args.output, args.base_url)
