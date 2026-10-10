#!/usr/bin/env python3
"""Check original source-study metadata and its explicitly unexecuted boundary."""
from pathlib import Path
from urllib.parse import urlsplit
import hashlib,json,re
R=Path(__file__).resolve().parents[1]
manifest=json.loads((R/'docs/50-rtos/source-study-manifest.json').read_text())
assert set(manifest)=={'schema_version','checked_date','kind','chapter_count','preserved_core_count','preserved_rtos_core_ids','modern_basis','historical_basis','execution_status','chapters'}
assert manifest['schema_version']==1 and manifest['kind']=='original_source_study_extension'
assert manifest['chapter_count']==7 and manifest['preserved_core_count']==120
assert manifest['preserved_rtos_core_ids']==[f'RT{n:02}' for n in range(1,11)]
modern=manifest['modern_basis'];historical=manifest['historical_basis']
assert set(modern)=={'tag','commit','port','scope','license','url'}
assert set(historical)=={'tag','commit','license','optional','url'}
assert modern['tag']=='V11.1.0' and modern['commit']=='dbf70559b27d39c1fdb68dfb9a32140b6a6777a0'
assert modern['port']=='portable/GCC/ARM_CM4F' and modern['license']=='MIT'
assert historical['tag']=='V9.0.0' and historical['commit']=='165c24cba94e3d3f386c3b77d3bb0f4a524d8511'
assert historical['license']=='GPLv2 plus FreeRTOS exception' and historical['optional'] is True
assert manifest['execution_status']=='source_reading_and_hand_traces_only; no host model, kernel, simulator or board experiments'
expected=['source-reading.md','kernel-lists.md','kernel-task-lifecycle.md','kernel-queue-waits.md','kernel-event-groups.md','kernel-timer-daemon.md','historical-co-routines.md']
chapters=manifest['chapters'];assert len(chapters)==7
assert [Path(c['path']).name for c in chapters]==expected
assert len({c['category'] for c in chapters})==7
for order,c in enumerate(chapters,1):
 assert set(c)=={'path','title','module','kind','category','priority','order','optional_historical','sha256'}
 assert c['path']=='docs/50-rtos/'+expected[order-1] and c['order']==order
 assert c['module']=='rtos' and c['kind']=='source-study'
 assert c['priority']==('P2' if order==7 else 'P1') and c['optional_historical']==(order==7)
 p=R/c['path'];s=p.read_text();assert s.startswith('# '+c['title']+'\n')
 assert hashlib.sha256(p.read_bytes()).hexdigest()==c['sha256'],c['path']
 assert not re.search(r'^# RT\d+\b',s,re.M),'Extensions must not acquire core IDs'
 assert '未执行' in s or '未编译或运行' in s or '尚未运行' in s,c['path']
 assert ('V9.0.0' if order==7 else 'V11.1.0') in s,c['path']
 if order==7:assert 'GPLv2' in s and 'exception' in s
 else:assert modern['commit'] in s,c['path']
 fence=None
 for line in s.splitlines():
  m=re.match(r'^\s{0,3}(`{3,}|~{3,})',line)
  if m:
   token=m[1]
   if fence is None:fence=token
   elif token[0]==fence[0] and len(token)>=len(fence):fence=None
 assert fence is None,(c['path'],'unclosed code fence')
 for url in re.findall(r'\[[^\]]*\]\((https?://[^\s)]+)',s):
  u=urlsplit(url);assert u.scheme=='https' and u.hostname in {'github.com','www.freertos.org','freertos.org'},url
  if u.hostname=='github.com':assert u.path.startswith(('/FreeRTOS/FreeRTOS/','/FreeRTOS/FreeRTOS-Kernel/','/FreeRTOS/FreeRTOS-Kernel-Book/')),url
core=json.loads((R/'coverage/core-question-manifest.json').read_text())['questions']
assert len(core)==120
assert [q['id'] for q in core if q['module']=='rtos']==manifest['preserved_rtos_core_ids']
assert not ({q['path'] for q in core}&{c['path'] for c in chapters})
summary=json.loads((R/'coverage/topic-summary.json').read_text())
assert summary['freertos_source_study_chapter_count']==7 and summary['core_count']==120
assert summary['official_hot100_count']==100 and summary['supplementary_count']==1
assert summary['original_interview_scenario_count']==7
print('FreeRTOS source-study PASS: 7 hash-fixed original extensions; fixed modern/historical versions, separate license/configuration/runtime boundaries, ordered route and unchanged core/algorithm counts.')
