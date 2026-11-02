#!/usr/bin/env python3
"""Guard public artifact identities and operation-specific support boundaries."""
from __future__ import annotations
import copy
import json
import shutil
import sys
import tempfile
from html.parser import HTMLParser
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import site_product_catalog as catalog
import build_discovery_site_v3 as site


def rejected(data):
    try:
        catalog.validate_catalog(data)
    except SystemExit:
        return
    raise AssertionError('unsafe release/capability catalog accepted')


class Links(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []
    def handle_starttag(self, tag, attrs):
        if tag == 'a': self.links.append(dict(attrs).get('href', ''))


def main():
    data = catalog.load_catalog(ROOT / 'site/tool-catalog.json')
    for mutation in (
        lambda d: d['releases'][0].update(url='javascript:alert(1)'),
        lambda d: d['releases'][0].update(url='https://github.com/VrUaCom/pocket-gdspace/releases/download/test/private.apk'),
        lambda d: d['releases'][0].update(sha256=''),
        lambda d: d['releases'][0].update(source_commit='main'),
        lambda d: d['releases'][0].update(stage='stable'),
        lambda d: d['releases'][0].update(bytes_verified_on=''),
        lambda d: d['releases'][2].update(stage='preview'),
        lambda d: d['capabilities'][0]['operations'].update(editing='bounded'),
        lambda d: d['capabilities'][2]['operations'].update(preview='release-notes'),
        lambda d: d['capabilities'][3]['operations'].update(acceptance='tested-dev'),
        lambda d: d['capabilities'][0].update(release_id='missing'),
        lambda d: d['capabilities'][0].update(evidence=[]),
    ):
        altered = copy.deepcopy(data)
        mutation(altered)
        rejected(altered)
    parent = Path(tempfile.mkdtemp(prefix='_site-products-', dir=ROOT))
    try:
        base = 'https://vruacom.github.io/dmc-rengine-cpp'
        site.build(parent / 'generated', base)
        downloads = (parent / 'generated/downloads/index.html').read_text()
        matrix = (parent / 'generated/capabilities/index.html').read_text()
        products = (parent / 'generated/tools/index.html').read_text()
        parser = Links(); parser.feed(downloads)
        asset_links = [url for url in parser.links if '/releases/download/' in url]
        assert set(asset_links) == {r['url'] for r in data['releases']}
        assert len(asset_links) == len(data['releases']) == 3
        assert '55f874d.apk' not in downloads
        assert 'Download unsigned IPA' in downloads
        assert 'id="ios-developer"' in downloads
        assert 'This IPA cannot be installed by tapping it' in downloads
        for release in data['releases']:
            assert release['sha256'] in downloads
            assert release['source_commit'] in downloads
        for row in data['capabilities']:
            assert f'data-capability="{row["id"]}"' in matrix
        assert matrix.count('class="capability-operations"') == len(data['capabilities'])
        assert all(label in matrix for label in catalog.OPERATIONS.values())
        assert f'{base}/downloads/' in products and f'{base}/capabilities/' in products
        for document in (downloads, matrix, products):
            parser = Links(); parser.feed(document)
            assert not any(url.startswith(('/downloads/', '/capabilities/')) for url in parser.links)
            assert '/pocket-gdspace/' not in document
        site_index = json.loads((parent / 'generated/site-index.json').read_text())
        assert {'/downloads/', '/capabilities/'}.issubset(site_index['paths'])
        assert (parent / 'generated/sitemap.xml').read_text().count(base + '/downloads/') == 1
        # Buildless root preview must also work without a canonical origin.
        page = next(p for p in site.load_manifest()['pages'] if p['path'] == '/downloads/')
        preview = site.render_page(site.load_manifest(), page, None)
        assert 'href="/capabilities/"' in preview
    finally:
        shutil.rmtree(parent, ignore_errors=True)
    print('Release catalog, exact asset links and operation-boundary regression tests: OK')


if __name__ == '__main__': main()
