#!/usr/bin/env python3
"""Validate the bounded public teaching metadata and offline resource inventory."""
from pathlib import Path
import json,re,hashlib
R=Path(__file__).resolve().parents[1]
def read(name): return json.loads((R/name).read_text())
expected={'README.md','core-catalog.json','official-catalog.json','primary-verification.json','interview-sample.json'}
assert {p.name for p in (R/'sources').iterdir() if p.is_file()} == expected,'Unexpected source inventory'
assert {p.name for p in (R/'coverage').iterdir() if p.is_file()} == {'README.md','core-question-manifest.json','topic-summary.json','validation-summary.json'},'Unexpected coverage inventory'
core=read('coverage/core-question-manifest.json')
allowed={'id','title','path','module','priority','source_ids','anchor','kind'}
for q in core['questions']:assert set(q)<=allowed,(q['id'],'unexpected origin metadata')
hot=read('docs/80-hot100-cpp/manifest.json')
allowed_problem={'id','slug','title_zh','title_en','category','category_order','order_in_category','difficulty','official_url','official_en_url','order','doc_path','cpp_path','test_path','title','solution_path','tests','supplemental'}
for q in hot['problems']+hot.get('supplementary',[]):
 assert set(q)<=allowed_problem,(q['id'],'unexpected origin metadata')
 assert q['official_url'].startswith('https://leetcode.cn/problems/'),q['id']
assert len(hot['problems'])==100 and len(hot.get('supplementary',[]))==1
assert hot['supplementary'][0]['id']==695
for name in ('core-catalog.json','official-catalog.json','primary-verification.json'):
 j=read('sources/'+name)
 for item in j:assert item['url'].startswith('https://'),(name,item)
for p in R.rglob('*'):
 if not p.is_file():continue
 try:s=p.read_text(encoding='utf-8')
 except UnicodeDecodeError:raise AssertionError(f'Unexpected binary public asset: {p.relative_to(R)}')
 if p.suffix in ('.md','.json') or p.name=='data.js':assert not re.search(r'(?:ghp_[A-Za-z0-9]{30,}|sediment://file_|["\'](?:sentry-trace|scrapeId|indexId|download_url|library_file_id)["\']\s*:)',s),str(p.relative_to(R))
freeze=read('docs/80-hot100-cpp/freeze-manifest.json')
assert len(freeze['files_sha256'])==freeze['file_count']
for name,digest in freeze['files_sha256'].items():assert (R/name).is_file() and hashlib.sha256((R/name).read_bytes()).hexdigest()==digest,name
for p in (R/'site/assets/repo').rglob('*'):
 if p.is_file():
  source=R/p.relative_to(R/'site/assets/repo')
  assert source.is_file() and source.read_bytes()==p.read_bytes(),p
print(f'Public metadata PASS: {len(core["questions"])} core topics, 100 official problems, 1 extra public exercise; bounded schemas and exact offline source copies.')
