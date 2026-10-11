#!/usr/bin/env python3
"""Check optional chapter identities, separate counts, and navigation metadata.

This validates authored teaching inputs and does not execute target software.
"""
from pathlib import Path
import ast
import hashlib
import json
import re

R = Path(__file__).resolve().parents[1]
manifest = json.loads((R / 'docs/130-reference/optional-depth-manifest.json').read_text())
assert set(manifest) == {'schema_version', 'checked_date', 'kind', 'chapter_count', 'preserved_counts', 'chapters'}
assert manifest['schema_version'] == 1 and manifest['kind'] == 'original_optional_depth'
assert manifest['chapter_count'] == 9
assert manifest['preserved_counts'] == {'core': 120, 'official_hot100': 100, 'supplementary_algorithm': 1, 'interview_scenarios': 7, 'freertos_source_study': 7}
chapters = manifest['chapters']
assert len(chapters) == 9 and len({c['path'] for c in chapters}) == 9
core = json.loads((R / 'coverage/core-question-manifest.json').read_text())['questions']
core_ids = {q['id'] for q in core}
study = json.loads((R / 'docs/50-rtos/source-study-manifest.json').read_text())['chapters']
assert len(core) == 120 and len(study) == 7
assert not ({c['path'] for c in chapters} & ({q['path'] for q in core} | {c['path'] for c in study}))
modules = json.loads((R / 'site/config.json').read_text())['modules']
folder_modules = {m['folder']: m['id'] for m in modules}
catalog = (R / 'docs/130-reference/optional-depth.md').read_text()
python_blocks = 0
for order, c in enumerate(chapters, 1):
    assert set(c) == {'path', 'title', 'module', 'kind', 'category', 'priority', 'order', 'prerequisite_ids', 'sha256'}
    assert c['order'] == order and c['kind'] == 'optional-depth'
    assert c['priority'] in {'P1', 'P2'} and c['prerequisite_ids']
    assert set(c['prerequisite_ids']) <= core_ids
    p = R / c['path']
    assert p.is_file() and folder_modules[p.relative_to(R).parts[1]] == c['module']
    text = p.read_text()
    assert text.startswith('# ' + c['title'] + '\n')
    assert hashlib.sha256(p.read_bytes()).hexdigest() == c['sha256'], c['path']
    assert not re.search(r'^# [A-Z]+\d+\b', text, re.M)
    assert c['priority'] in text
    assert c['path'].removeprefix('docs/') in catalog
    fence = None
    for line in text.splitlines():
        match = re.match(r'^\s{0,3}(`{3,}|~{3,})', line)
        if match:
            token = match[1]
            if fence is None:
                fence = token
            elif token[0] == fence[0] and len(token) >= len(fence):
                fence = None
    assert fence is None, (c['path'], 'unclosed code fence')
    for code in re.findall(r'^```python\s*\n(.*?)^```', text, re.M | re.S):
        ast.parse(code, filename=c['path'])
        python_blocks += 1
summary = json.loads((R / 'coverage/topic-summary.json').read_text())
assert summary['optional_depth_chapter_count'] == 9
assert summary['core_count'] == 120 and summary['official_hot100_count'] == 100
assert summary['supplementary_count'] == 1 and summary['original_interview_scenario_count'] == 7
assert summary['freertos_source_study_chapter_count'] == 7
event = (R / 'docs/50-rtos/kernel-event-groups.md').read_text()
assert '(task-diagnostics.md)' in event
assert event.count('可选追踪编号不是对象寿命凭证') == 1
assert 'kernel-event-groups.md' in catalog and 'kernel-timer-daemon.md' in catalog
print(f'Optional-depth metadata PASS: 9 hash-fixed chapters; separate counts, prerequisites, paired trace/diagnostics links, {python_blocks} parsed Python examples. No target execution claim.')
