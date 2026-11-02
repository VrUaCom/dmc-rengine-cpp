"""Render the guide's small Markdown subset with escaped text and safe links.

The Markdown file remains the single full-content source for GitHub and Pages.
No raw HTML, script, images, remote includes or third-party dependency is used.
"""
from __future__ import annotations

import html
import re
from urllib.parse import urlsplit

TOKEN = re.compile(r'`([^`]+)`|\[([^\]]+)\]\(([^\s)]+)\)|\*\*([^*]+)\*\*')
GUIDE_SITE = 'https://vruacom.github.io/dmc-rengine-cpp'


def inline(text: str, base_url: str | None = None) -> str:
    result, pos = [], 0
    for match in TOKEN.finditer(text):
        result.append(html.escape(text[pos:match.start()]))
        code, label, href, strong = match.groups()
        if code is not None:
            result.append('<code>' + html.escape(code) + '</code>')
        elif href is not None:
            parsed = urlsplit(href)
            if not (href.startswith('#') or (href.startswith('/') and not href.startswith('//'))
                    or (parsed.scheme == 'https' and parsed.netloc)):
                raise ValueError('unsupported guide link: ' + href)
            if href.startswith(GUIDE_SITE + '/'):
                href = (base_url.rstrip('/') if base_url else '') + href[len(GUIDE_SITE):]
            elif href.startswith('/') and base_url:
                href = base_url.rstrip('/') + href
            result.append('<a href="' + html.escape(href, quote=True) + '">' + inline(label, base_url) + '</a>')
        else:
            result.append('<strong>' + inline(strong, base_url) + '</strong>')
        pos = match.end()
    result.append(html.escape(text[pos:]))
    return ''.join(result)


def render(markdown: str, base_url: str | None = None) -> str:
    lines = markdown.splitlines()
    body, toc, ids = [], [], set()
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if not line:
            i += 1
            continue
        if line.startswith('# '):
            i += 1  # The page shell already provides the sole h1.
            continue
        if line.startswith(('## ', '### ')):
            level = 3 if line.startswith('### ') else 2
            title = line[level + 1:]
            anchor = re.sub(r'[^a-z0-9]+', '-', title.lower()).strip('-')
            if not anchor or anchor in ids:
                raise ValueError('empty or duplicate guide heading: ' + title)
            ids.add(anchor)
            body.append(f'<h{level} id="{anchor}">' + inline(title, base_url) + f'</h{level}>')
            if level == 2:
                toc.append('<li><a href="#' + anchor + '">' + inline(title, base_url) + '</a></li>')
            i += 1
            continue
        if line.startswith('|'):
            rows = []
            while i < len(lines) and lines[i].strip().startswith('|'):
                rows.append([cell.strip() for cell in lines[i].strip().strip('|').split('|')])
                i += 1
            if len(rows) < 3 or not all(re.fullmatch(r':?-{3,}:?', c) for c in rows[1]):
                raise ValueError('guide table requires a header and separator')
            width = len(rows[0])
            if any(len(row) != width for row in rows):
                raise ValueError('guide table column count differs')
            head = ''.join('<th scope="col">' + inline(c, base_url) + '</th>' for c in rows[0])
            data = ''.join('<tr>' + ''.join('<td>' + inline(c, base_url) + '</td>' for c in row) + '</tr>' for row in rows[2:])
            body.append('<div class="guide-table" tabindex="0" role="region" aria-label="Scrollable reference table">'
                        '<table><thead><tr>' + head + '</tr></thead><tbody>' + data + '</tbody></table></div>')
            continue
        if line.startswith('- '):
            items = []
            while i < len(lines) and lines[i].strip().startswith('- '):
                items.append('<li>' + inline(lines[i].strip()[2:], base_url) + '</li>')
                i += 1
            body.append('<ul>' + ''.join(items) + '</ul>')
            continue
        paragraph = [line]
        i += 1
        while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith(('#', '|', '- ')):
            paragraph.append(lines[i].strip())
            i += 1
        body.append('<p>' + inline(' '.join(paragraph), base_url) + '</p>')
    return ('<article class="reader-guide" aria-label="Complete Android reader guide">'
            '<nav class="guide-toc" aria-label="Guide contents"><h2>In this guide</h2><ol>'
            + ''.join(toc) + '</ol></nav>' + '\n'.join(body) + '</article>')
