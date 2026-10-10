#!/usr/bin/env python3
"""Generate navigable question index/counts from canonical manifests, no network."""
from pathlib import Path
import json
R=Path(__file__).resolve().parents[1]
core=json.loads((R/'coverage/core-question-manifest.json').read_text())['questions']
modules=json.loads((R/'site/config.json').read_text())['modules']
hotfile=R/'docs/80-hot100-cpp/manifest.json'
hot=json.loads(hotfile.read_text()) if hotfile.exists() else {'problems':[],'supplementary':[],'categories':[]}
lines=['# 题目索引','',f"原创核心：{len(core)}主题。官方Hot100：{len(hot['problems'])}题。额外公开练习：{len(hot.get('supplementary',[]))}题。",'','优先级是课程依赖建议，不是面试考频。题目按manifest生成；实现是否实际测试以验证报告为准。','']
summary=[]
for m in modules:
    qs=[q for q in core if q['module']==m['id']]
    if not qs:continue
    summary.append({'module':m['id'],'label':m['label'],'original_core_count':len(qs)})
    lines += [f"## {m['label']} · {len(qs)}主题",'']+[f"- [{q['id']} · {q['title']}]({q['path']}) · {q['priority']}" for q in qs]+['']
interviewfile=R/'docs/120-interviews/manifest.json'
interview=json.loads(interviewfile.read_text()) if interviewfile.exists() else {'cards':[],'counts':{}}
if interview['cards']:
    lines += [f"## 公开面经与原创故障场景 · {len(interview['cards'])}卡",'',f"{interview['counts']['new_full_public_pages']}份新增完整自报页面，试读和无日期GitHub日志另列；场景不计入120核心主题或Hot100。",'', '- [来源日期、岗位与学习导航](docs/120-interviews/README.md)', '- [公开自报目录](docs/120-interviews/source-catalog.md)']+[f"- [{c['id']} · {c['title']}]({c['path']})" for c in interview['cards']]+['']
lines+=['## Hot100 · C++17','','- [学习路线、完整题单与测试](docs/80-hot100-cpp/README.md)','- [源码与编译](algorithms/cpp/README.md)','']
for c in hot.get('categories',[]):
    ps=[p for p in hot['problems'] if p['category']==c['name']]
    lines += [f"### {c['name']} · {len(ps)}题",'']+[f"- [LC{p['id']} · {p['title_zh']}]({p['doc_path']}) · {p['difficulty']}" for p in ps]+['']
if hot.get('supplementary'):
    lines += ['### 额外公开练习：不计入官方100','']+[f"- [LC{p['id']} · {p['title_zh']}]({p['doc_path']})" for p in hot['supplementary']]+['']
(R/'QUESTION_INDEX.md').write_text('\n'.join(lines))
(R/'coverage/topic-summary.json').write_text(json.dumps({'checked_date':'2026-10-10','core_count':len(core),'original_interview_scenario_count':len(interview['cards']),'new_full_public_interview_pages':interview['counts'].get('new_full_public_pages',0),'core_modules':summary,'official_hot100_count':len(hot['problems']),'supplementary_count':len(hot.get('supplementary',[]))},ensure_ascii=False,indent=2)+'\n')
print(f"Index regenerated: {len(core)} core + {len(hot['problems'])} official + {len(hot.get('supplementary',[]))} supplementary")
