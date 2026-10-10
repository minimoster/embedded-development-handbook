#!/usr/bin/env python3
"""Repository content checks; not hardware or browser testing."""
from pathlib import Path
import json,re,urllib.parse,sys
R=Path(__file__).resolve().parents[1]
core=json.loads((R/'coverage/core-question-manifest.json').read_text())['questions']
sources={x['id']:x for x in json.loads((R/'sources/core-catalog.json').read_text())}
assert len({q['id'] for q in core})==len(core),'Duplicate question id'
assert len({q['path'] for q in core})==len(core),'Duplicate core document'
sections=['直接回答','原理拆解','例子与推演','常见误区与失败边界','面试追问','动手练习与验收','来源与适用范围']
for q in core:
    p=R/q['path'];assert p.is_file(),q
    s=p.read_text();assert s.startswith('# '+q['id']+' · '),q['id']
    for title in sections:assert '## '+title+'\n' in s,(q['id'],title)
    assert len(s)>=550,(q['id'],'shallow document')
    assert q['priority'] in ['P0','P1','P2']
    for id in q['source_ids']:assert id in sources,(q['id'],id)
    assert q['source_ids'],q['id']
manifest=R/'docs/80-hot100-cpp/manifest.json'
if manifest.exists():
    hot=json.loads(manifest.read_text());assert len(hot['problems'])==100
    assert len({p['id'] for p in hot['problems']})==100
    assert not ({p['id'] for p in hot['problems']} & {p['id'] for p in hot.get('supplementary',[])})
    if '--strict' in sys.argv:
        for p in hot['problems']+hot.get('supplementary',[]):
            for key in ['doc_path','cpp_path','test_path']:assert (R/p[key]).is_file(),(p['id'],key)
# Markdown target existence. Exact generated anchor checking is performed by site/test.mjs.
for p in R.rglob('*.md'):
    if 'site/assets' in str(p):continue
    s=p.read_text()
    assert not re.search(r'(?:sentry-trace|scrapeId|indexId|ghp_[A-Za-z0-9]+)',s),p
    for href in re.findall(r'\[[^\]]*\]\(([^\s)]+)(?:\s+"[^"]*")?\)',s):
        if href.startswith(('http://','https://','#','mailto:')):continue
        target=urllib.parse.unquote(href.split('#')[0].split('?')[0])
        assert (p.parent/target).resolve().is_file(),(str(p.relative_to(R)),href)
# No unapproved document assets, credentials or raw operational IDs in public metadata.
assert not list(R.rglob('*.pdf')),'Do not rehost unapproved documents'
for p in (R/'sources').glob('*.json'):
    s=p.read_text();assert not re.search(r'sentry-trace|scrapeId|indexId|download_url',s),p
print(f'Content PASS: {len(core)} complete original core cards; manifest paths, source mapping, local links, public metadata.')
