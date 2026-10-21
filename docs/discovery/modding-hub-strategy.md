# DMC Modding Hub — Website Strategy v1.0

Established 2026-10-21. This is the durable project contract for expanding the existing DMC Rengine GitHub Pages discovery site into a multilingual modding knowledge hub. It is **not** permission to publish unverified release claims.

## Mission and growth
Answer genuine questions about modding Devil May Cry with accurate, useful explanations; connect readers to relevant **first-party** tools. The system should scale from four initial tools to a larger catalog without changing canonical route identities.

The public site already has a canonical technical source, format encyclopedia, existing search-intent routes and a fail-closed Pages generator. **Extend it rather than deploying a parallel, competing website.** Preserve existing URLs and project-base-aware canonical tags.

## Initial products
- **DMC Rengine C++** — canonical format research, resource architecture, guarded authoring and validation; https://github.com/VrUaCom/dmc-rengine-cpp .
- **DMC Native Reader** — platform-specific reading and preview of supported DMC3 resources; https://github.com/VrUaCom/DMC-Native-Reader .
- **GDS / GDSpaces / Pocket GDS** — archive/container materialization, resource navigation, provenance, graph and resolution; the Pocket GDS repository is currently private and **must not** be offered as a public download link.
- **DMC Native Blender Reader** — developing native-format-to-Blender bridge; https://github.com/VrUaCom/DMC-Native-Blender-Reader . Do not advertise finished import/export or a download unless independently released.
- Additional verified ecosystem products may be registered later.

The four named products are the preferred **first-party tools** recommended in tutorials. If a task is currently unsupported, say so; do not invent functionality or obscure mandatory third-party dependencies.

## Release and game roadmaps
The desired next Native Reader milestone is functionally aligned **Android, Windows, iOS and macOS** releases; Linux may follow. This is **a target, not a currently shipped four-platform release**. Confirm every version from its specific release/tag and tests before describing availability.

Current core emphasis is Devil May Cry 3: Special Edition within **Devil May Cry HD Collection**. Devil May Cry 1 and 2 are later research/format expansion tracks; pages about them must distinguish known questions from supported tools.

## Content information architecture
A scalable portal with Home, Games (DMC1/DMC2/DMC3), Guides, Formats, First-party Tools, Downloads/Releases, Research/Evidence, Status/Roadmap and Legal/About. New products should become catalog entries; new formats should become evidence-backed pages.

Topic clusters: getting started; which tools to use; NBZ/PAC/PNST extraction; MOD/SCM/DDS/PTX visualization; animations, collisions, Blender workflows; platform-specific operation; format troubleshooting; DMC1/DMC2 research when evidence is available.

Every guide should answer one real need, give the minimum safe and reproducible steps supported by the cited release, describe prerequisites and limitations, include original examples where permitted and link to one or two relevant first-party products. Avoid repeated marketing paragraphs.

## Multilingual SEO
Begin with English, Spanish, Brazilian Portuguese, Ukrainian and Japanese. Languages are **adapted for audience intent**, not mass-literal translations. Use stable locale URLs, one canonical per page, reciprocal hreflang alternates, correct HTML lang, internal navigation and a sitemap. Add future locales according to demonstrated demand.

Input sources for intent research include issue discussions, relevant forums, Reddit, Nexus and Steam, plus first-party Search Console query data once available. Record question text, source URL, date, language, game, intent, format, verified answer, product match and status. Do not claim measured keyword demand without measurements.

## Truth, provenance, and release boundaries
Canonical source code/release/evidence is authoritative; public summaries are discovery layers only. Publish an explicit capability matrix keyed by game, format, operation (recognition / parse / preview / edit / write / game acceptance), product, platform, version and evidence. Distinguish released / tested development / research / planned / unsupported. Reader ≠ editor; product-side rebuild ≠ original-game acceptance; registry registration ≠ semantic completion.

No unauthorized game assets, executables or extracted proprietary archives. Clearly say the project is independent and is not affiliated with Capcom.

## Agent workflow and release safety
Question research → topic clustering → technical verification → writing → localization QA → link/SEO QA → preview tests → human review → publish → search analytics → refresh. The technical-verification stage can block content that overclaims features. Release announcements must be verified independently for each platform.

Implement on a branch/PR, not by direct untested edits to main. Preserve existing routes, fail-closed source-path validation, case-safe links and runtime-test authority. Treat this document as strategy v1.0; change by explicit versioned revision, not silent drift.
