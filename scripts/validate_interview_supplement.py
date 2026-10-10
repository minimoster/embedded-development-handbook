#!/usr/bin/env python3
"""Validate original interview scenarios and bounded public self-report metadata."""
from pathlib import Path
from collections import Counter
from urllib.parse import urlsplit, unquote
import json,re
R=Path(__file__).resolve().parents[1]
def load(name):return json.loads((R/name).read_text())
def canonical(url):
 p=urlsplit(url);return p.hostname.removeprefix('www.').removeprefix('ac.'),unquote(p.path).rstrip('/')
catalog=load('sources/interview-supplement.json');prior=load('sources/interview-sample.json');manifest=load('docs/120-interviews/manifest.json')
assert catalog['schema_version']==manifest['schema_version']==1
full=catalog['records'];partial=catalog['partial_records'];logs=catalog['additional_sources'];cards=manifest['cards']
assert len(full)==17 and len(partial)==1 and len(logs)==1 and len(cards)==7
assert {r['id'] for r in full}=={f'EN{x:02}' for x in range(1,19) if x!=15}
assert partial[0]['id']=='EN15' and logs[0]['id']=='ENG01'
assert len({r['id'] for r in full+partial+logs})==19
assert len({canonical(r['url']) for r in full+partial+logs})==19
assert not ({canonical(r['url']) for r in full+partial+logs}&{canonical(r['url']) for r in prior['records']})
assert not ({r['author_display'] for r in full+partial+logs}&{r['author'] for r in prior['records']})
assert catalog['counts']['new_full_records_vs_existing']==17 and len(prior['records'])==6
assert catalog['counts']['existing_full_records_retained']==6
assert all(r['platform']=='牛客' and urlsplit(r['url']).hostname=='www.nowcoder.com' for r in full+partial)
assert logs[0]['platform']=='GitHub' and urlsplit(logs[0]['url']).hostname=='github.com'
assert logs[0]['published_date_displayed'] is None
for r in full:
 assert r['evidence_level'].startswith('A：') and r['technical_answer_authority'] is False
 assert r['date_year_in_interview'] is False and r['company_claim'] and r['role_claim'] and r['date_type']
 assert r['checked_at']=='2026-10-10'
assert partial[0]['evidence_level'].startswith('B：')
years=Counter(re.match(r'(\d{4})-',r['published_date_displayed']).group(1) for r in full)
assert dict(years)==catalog['counts']['full_date_year_distribution']=={'2021':1,'2022':3,'2023':2,'2024':5,'2025':6}
assert catalog['counts']['full_date_year_2026']==0 and catalog['counts']['verified_xiaohongshu_original_records']==0
assert sum(r['scope_class'] in ('explicit_internship','explicit_junior') for r in full)==catalog['counts']['full_explicit_internship_or_junior']==10
assert {c['id'] for c in cards}=={f'IC{x:02}' for x in range(1,8)}
assert manifest['counts']=={'original_scenarios':7,'new_full_public_pages':17,'partial_public_pages':1,'undated_github_logs':1,'verified_xiaohongshu_original_pages':0,'new_2026_full_public_pages':0}
allowed={'id','source_card_id','title','path','module','kind','category','public_report_ids','foundation_paths','official_urls','exercise_status'}
official_hosts={'raw.githubusercontent.com','docs.kernel.org','docs.zephyrproject.org','www.st.com'}
for c in cards:
 assert set(c)==allowed and c['module']=='interview' and c['kind']=='original_scenario'
 assert c['exercise_status']=='design_only_unexecuted'
 assert c['path']==f'docs/120-interviews/{c["id"].lower()}.md'
 assert set(c['public_report_ids'])<={r['id'] for r in full}
 assert c['foundation_paths'] and all((R/p).is_file() for p in c['foundation_paths'])
 s=(R/c['path']).read_text();assert s.startswith(f'# {c["id"]} · ')
 for h in ('公开面经主题联系','故障场景','先理解机制','直接回答','面试追问','常见误区与失败边界','动手练习（尚未执行）','验收标准（尚未执行）','版本与适用边界','先修与复习入口'):
  assert '\n## '+h+'\n' in s,(c['id'],h)
 assert all(url in s and urlsplit(url).hostname in official_hosts for url in c['official_urls'])
 assert '未执行' in s and len(s)>1100
 assert not re.search(r'agent_backend|Java21|RabbitMQ|Spring|SQLite幂等|learning_cards/',s)
assert '正在处理数' in (R/cards[0]['path']).read_text()
assert 'xQueueCreateStatic' in (R/cards[1]['path']).read_text()
assert 'DMA mask' in (R/cards[3]['path']).read_text()
assert len(load('coverage/core-question-manifest.json')['questions'])==120
hot=load('docs/80-hot100-cpp/manifest.json');assert len(hot['problems'])==100 and len(hot['supplementary'])==1
print('Interview supplement PASS: 17 distinct added full self-reports, 1 preview, 1 undated log; 7 separately classified original scenarios, exact dates/counts/mapping and explicit unexecuted limits.')
