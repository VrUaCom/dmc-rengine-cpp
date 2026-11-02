"""Validated, release-bound download and capability presentation for the static hub."""
from __future__ import annotations

import html
import json
import re
from datetime import date
from pathlib import Path
from urllib.parse import urlsplit

OPERATIONS = {
    'recognition': 'Recognition', 'parsing': 'Parsing', 'preview': 'Preview',
    'editing': 'Editing', 'writer': 'Rebuild / write', 'acceptance': 'Original-game acceptance',
}
STATES = {
    'documented': 'Source documented', 'release-notes': 'Release documented',
    'build-only': 'Build only', 'tested-dev': 'Tested development', 'bounded': 'Bounded support',
    'unsupported': 'Not supported', 'not-established': 'Not established',
    'planned': 'Planned', 'not-applicable': 'Not applicable',
}
STAGES = {'test': 'Test build', 'preview': 'Preview', 'unsigned': 'Unsigned',
          'development': 'Development', 'research': 'Research', 'planned': 'Planned'}
NATIVE_REPO = 'https://github.com/VrUaCom/DMC-Native-Reader'


def _text(value) -> bool:
    return isinstance(value, str) and bool(value.strip())


def _github_url(value) -> bool:
    if not isinstance(value, str):
        return False
    parsed = urlsplit(value)
    return (parsed.scheme == 'https' and parsed.netloc == 'github.com'
            and parsed.path.startswith('/VrUaCom/') and not parsed.query
            and not any(c.isspace() for c in value) and '%' not in parsed.path
            and all(part not in ('.', '..') for part in parsed.path.split('/')))


def validate_catalog(catalog: dict) -> dict:
    def require(ok, message):
        if not ok:
            raise SystemExit('tool catalog: ' + message)

    require(isinstance(catalog, dict) and catalog.get('schema') == 'dmc-rengine.first-party-tool-catalog.v2', 'unsupported schema')
    try:
        date.fromisoformat(catalog['verified_on'])
    except (KeyError, TypeError, ValueError):
        raise SystemExit('tool catalog: invalid verification date')
    require(_text(catalog.get('verification_scope')), 'missing verification scope')
    products = catalog.get('products')
    require(isinstance(products, list) and bool(products), 'empty product catalog')
    product_ids = set()
    for product in products:
        require(isinstance(product, dict), 'invalid product')
        identifier = product.get('id')
        require(_text(identifier) and identifier not in product_ids, 'duplicate/missing product id')
        product_ids.add(identifier)
        require(all(_text(product.get(k)) for k in ('name', 'role', 'status', 'notes', 'action_label')), 'missing product content')
        require(_github_url(product.get('repo_url')), 'invalid product repository')
        require(isinstance(product.get('public_repository'), bool), 'missing public repository flag')
        require(product.get('action_path') in ('/downloads/', '/capabilities/', '/gdspaces/'), 'invalid product action route')
    releases = catalog.get('releases')
    require(isinstance(releases, list) and bool(releases), 'empty release list')
    release_ids = set()
    for release in releases:
        require(isinstance(release, dict), 'invalid release')
        identifier = release.get('id')
        require(_text(identifier) and re.fullmatch(r'[a-z][a-z0-9-]*', identifier) is not None and identifier not in release_ids, 'duplicate/unsafe release id')
        release_ids.add(identifier)
        require(release.get('product_id') == 'native-reader' and 'native-reader' in product_ids, 'unreleased product cannot have a binary download')
        require(release.get('stage') in ('test', 'preview', 'unsigned'), 'unverified production release')
        require(all(_text(release.get(k)) for k in ('tag','version','platform','architecture','format','file_name','description','limitations','install','selection_note','stage_label')), 'missing release content')
        require(re.fullmatch(r'[a-zA-Z0-9_.-]+', release['tag']) is not None, 'unsafe release tag')
        require(re.fullmatch(r'[a-zA-Z0-9_.-]+', release['file_name']) is not None, 'unsafe asset name')
        require(release.get('url') == NATIVE_REPO + '/releases/download/' + release['tag'] + '/' + release['file_name'], 'asset URL does not match the pinned tag/name')
        require(release.get('release_url') == NATIVE_REPO + '/releases/tag/' + release['tag'], 'release URL mismatch')
        require(_github_url(release.get('notes_url')), 'invalid release evidence link')
        require(re.fullmatch(r'[0-9a-f]{40}', str(release.get('source_commit',''))) is not None, 'missing source commit')
        require(re.fullmatch(r'[0-9a-f]{64}', str(release.get('sha256',''))) is not None, 'missing SHA-256')
        require(type(release.get('size_bytes')) is int and release['size_bytes'] > 0, 'invalid asset size')
        require(release.get('bytes_verified_on') == catalog['verified_on'], 'asset bytes were not verified for this snapshot')
        require((release['format'], release['stage']) in (('APK','test'), ('ZIP','preview'), ('IPA','unsigned')), 'package/stage mismatch')
    rows = catalog.get('capabilities')
    require(isinstance(rows, list) and bool(rows), 'empty capability matrix')
    row_ids = set()
    for row in rows:
        require(isinstance(row, dict), 'invalid capability row')
        identifier = row.get('id')
        require(_text(identifier) and identifier not in row_ids, 'duplicate/missing capability id')
        row_ids.add(identifier)
        require(row.get('product_id') in product_ids, 'unknown capability product')
        require(row.get('stage') in STAGES, 'unknown capability stage')
        require(all(_text(row.get(k)) for k in ('platform','version','game','notes')), 'missing capability scope')
        require(isinstance(row.get('formats'), list) and bool(row['formats']) and all(_text(f) for f in row['formats']), 'missing format scope')
        operations = row.get('operations', {})
        require(isinstance(operations, dict) and set(operations) == set(OPERATIONS), 'incomplete operation matrix')
        require(all(state in STATES for state in operations.values()), 'unknown operation status')
        require(operations['acceptance'] == 'not-established', 'original-game acceptance cannot be inferred from a reader/build')
        if row['product_id'] == 'native-reader':
            require(operations['editing'] == operations['writer'] == 'unsupported', 'reader must not be promoted into an editor')
        release_id = row.get('release_id')
        require(release_id is None or release_id in release_ids, 'unknown capability release')
        if release_id:
            release = next(r for r in releases if r['id'] == release_id)
            require(row['stage'] == release['stage'] and row['product_id'] == release['product_id'], 'capability/release identity mismatch')
            if release['stage'] == 'unsigned':
                require(all(operations[k] == 'build-only' for k in ('recognition','parsing','preview')), 'unsigned iOS runtime is not accepted')
        evidence = row.get('evidence')
        require(isinstance(evidence, list) and bool(evidence), 'missing capability evidence')
        require(all(isinstance(e, dict) and _text(e.get('label')) and _github_url(e.get('url')) for e in evidence), 'invalid evidence link')
    return catalog


def load_catalog(path: Path) -> dict:
    return validate_catalog(json.loads(path.read_text(encoding='utf-8')))


def _e(value) -> str:
    return html.escape(str(value), quote=True)


def _link(url: str, label: str, css: str = '') -> str:
    return f'<a class="{_e(css)}" href="{_e(url)}">{_e(label)}</a>'


def downloads_html(catalog: dict, capability_url: str) -> str:
    cards = []
    advanced = []
    for release in catalog['releases']:
        size = f"{release['size_bytes'] / (1024 * 1024):.2f} MiB"
        card = (
            f'<article class="download-card" id="{_e(release["id"])}">'
            f'<p class="release-stage stage-{_e(release["stage"])}">{_e(release["stage_label"])}</p>'
            f'<h3>{_e(release["platform"])} <span>{_e(release["architecture"])}</span></h3>'
            f'<p class="download-version">{_e(release["version"])}</p>'
            f'<p>{_e(release["description"])}</p><p class="release-boundary">{_e(release["limitations"])}</p>'
            f'<p class="install-note">{_e(release["install"])}</p>'
            + _link(release['url'], f'Download {"unsigned " if release["stage"] == "unsigned" else ""}{release["format"]} · {size}', 'download-action')
            + '<div class="release-links">' + _link(release['release_url'], 'Release details ↗')
            + _link(release['notes_url'], 'Evidence & limitations ↗') + '</div>'
            + '<details class="asset-details"><summary>File identity & SHA-256</summary>'
            f'<dl><dt>File</dt><dd>{_e(release["file_name"])}</dd><dt>Release tag</dt><dd>{_e(release["tag"])}</dd>'
            f'<dt>SHA-256</dt><dd><code>{_e(release["sha256"])}</code></dd><dt>Bytes verified</dt><dd>{_e(release["bytes_verified_on"])}</dd>'
            f'<dt>Source revision</dt><dd>{_e(release["source_commit"])}</dd></dl><p>{_e(release["selection_note"])}</p></details></article>'
        )
        (advanced if release['stage'] == 'unsigned' else cards).append(card)
    return (
        '<section class="release-picker" aria-labelledby="choose-build"><p class="eyebrow">NATIVE READER / PUBLIC FILES</p>'
        '<h2 id="choose-build">Choose your platform.</h2><p>Each package has its own release stage. Check its scope before installing.</p>'
        '<nav class="platform-links" aria-label="Choose a platform"><a href="#android">Android ↘</a><a href="#windows">Windows ↘</a><a href="#ios-developer">iOS developer artifact ↘</a></nav>'
        '<div class="download-grid">' + ''.join(cards) + '</div>'
        '<details class="advanced-download" id="ios-developer"><summary>iOS — unsigned developer artifact</summary>'
        + ''.join(advanced) + '</details>'
        '<p class="catalog-verified">Files downloaded and SHA-256 checked on <time datetime="' + _e(catalog['verified_on']) + '">' + _e(catalog['verified_on']) + '</time>. This verifies file identity, not device behavior.</p>'
        + _link(capability_url, 'Compare capabilities by version and platform →', 'button') + '</section>'
    )


def capabilities_html(catalog: dict, downloads_url: str) -> str:
    names = {p['id']: p['name'] for p in catalog['products']}
    rows = []
    for row in catalog['capabilities']:
        cells = []
        for operation, label in OPERATIONS.items():
            state = row['operations'][operation]
            cells.append(f'<div><dt>{_e(label)}</dt><dd><span class="cap-state state-{_e(state)}">{_e(STATES[state])}</span></dd></div>')
        evidence = ' '.join(_link(e['url'], e['label'] + ' ↗') for e in row['evidence'])
        download = _link(downloads_url + ('#ios-developer' if row['release_id'] == 'ios' else '#' + row['release_id']), 'View this package →', 'cap-download') if row.get('release_id') else ''
        rows.append(
            f'<article class="capability-row" data-capability="{_e(row["id"])}">'
            f'<div class="capability-heading"><div><p class="release-stage stage-{_e(row["stage"])}">{_e(STAGES[row["stage"]])}</p>'
            f'<h3>{_e(names[row["product_id"]])}</h3><p class="cap-platform">{_e(row["platform"])} · {_e(row["version"])}</p></div>{download}</div>'
            f'<p class="cap-formats">{_e(" / ".join(row["formats"]))}</p><dl class="capability-operations">' + ''.join(cells) + '</dl>'
            f'<p class="capability-note">{_e(row["notes"])}</p><div class="evidence-links">{evidence}</div></article>'
        )
    return (
        '<section class="capability-matrix" aria-labelledby="capability-matrix-title"><p class="eyebrow">PRODUCT × PLATFORM × VERSION</p>'
        '<h2 id="capability-matrix-title">What can this build actually do?</h2>'
        '<p>Game scope: Devil May Cry 3: Special Edition from Devil May Cry HD Collection.</p>'
        '<p class="catalog-verified">Reviewed ' + _e(catalog['verified_on']) + '. ' + _e(catalog['verification_scope']) + '</p>'
        '<details class="matrix-legend"><summary>How to read the evidence labels</summary><dl>'
        + ''.join(f'<div><dt>{_e(STATES[k])}</dt><dd>{_e(v)}</dd></div>' for k,v in (
            ('release-notes','Described by the specific published release; not a fresh device test.'),
            ('documented','Described in pinned source documentation; runtime acceptance is separate.'),
            ('build-only','Compile/build evidence only; the app is not established as running on device.'),
            ('tested-dev','Bounded canonical development tests and receipts are cited.'),
            ('bounded','Only the edit/rebuild cases documented in the cited evidence.'),
            ('unsupported','Outside this product or platform build’s supported operation.'),
            ('not-established','No adequate verification is promoted in this snapshot.'),
            ('not-applicable','This source layer is not itself a consumer preview/editing application.'),
            ('planned','Future work; no current released capability is implied.'),
        )) + '</dl></details><div class="capability-list">' + ''.join(rows) + '</div></section>'
    )
