#!/usr/bin/env python3
"""Guide renderer safety, navigation and full-manual content checks."""
from html.parser import HTMLParser
from pathlib import Path
import re
import unittest

from site_guide_markdown import inline, render

ROOT = Path(__file__).resolve().parents[1]


class Elements(HTMLParser):
    def __init__(self):
        super().__init__()
        self.nodes = []

    def handle_starttag(self, tag, attrs):
        self.nodes.append((tag, dict(attrs)))


class GuideTests(unittest.TestCase):
    def test_markup_is_escaped_and_only_safe_links_are_allowed(self):
        self.assertNotIn('<script>', render('## Safety\n\n<script>alert(1)</script>'))
        self.assertIn('&lt;script&gt;', render('## Safety\n\n<script>alert(1)</script>'))
        self.assertEqual(inline('**Source `abc`**'), '<strong>Source <code>abc</code></strong>')
        for url in ('javascript:alert', 'data:text/html,bad', '//evil.example/path'):
            with self.assertRaises(ValueError):
                inline('[unsafe](' + url + ')')
        self.assertIn('href="https://example.test/project/downloads/"', inline('[Download](/downloads/)', 'https://example.test/project'))

    def test_tables_and_heading_errors_are_not_silently_dropped(self):
        with self.assertRaises(ValueError):
            render('## Same\n\n## Same')
        with self.assertRaises(ValueError):
            render('| A | B |\n| --- | --- |\n| only one |')
        document = render('## Table\n\n| A | B |\n| --- | --- |\n| <x> | `value` |')
        self.assertIn('scope="col"', document)
        self.assertIn('tabindex="0"', document)
        self.assertIn('&lt;x&gt;', document)
        self.assertIn('<code>value</code>', document)

    def test_complete_manual_navigation_and_original_audit_coverage(self):
        source = (ROOT / 'docs/guides/native-reader-android.md').read_text()
        document = render(source, 'https://vruacom.github.io/dmc-rengine-cpp')
        parsed = Elements()
        parsed.feed(document)
        anchors = [attrs['id'] for _, attrs in parsed.nodes if 'id' in attrs]
        self.assertEqual(len(anchors), len(set(anchors)))
        chapters = re.findall(r'^## \d+\.', source, re.M)
        self.assertEqual(len(chapters), 18)
        for tag, attrs in parsed.nodes:
            href = attrs.get('href', '')
            if tag == 'a' and href.startswith('#'):
                self.assertIn(href[1:], anchors)
        # Keep the distinctive requested ranges and evidence boundaries present.
        for token in ('7f8caeacac9b168de3954ee15fa42beb01d61f15', '875', '151',
                      '109 real P', '74 real G', 'G → G', '212 ms → 69 ms',
                      '124 MP', '200 MP', '4000000', '1/2000', 'ARCore',
                      'V135', 'V142', 'PL000', 'EM028', 'st001cfg.pac',
                      'debug/device-test artifact', 'unsigned developer artifact',
                      'Full V142 acceptance was not established'):
            self.assertIn(token, source)
        self.assertNotIn('<h1', document)
        self.assertGreaterEqual(document.count('<table>'), 14)


if __name__ == '__main__':
    unittest.main()
