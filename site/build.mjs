#!/usr/bin/env node
/* Deterministic static build: canonical Markdown plus manifests, no network. */
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import { fileURLToPath } from 'node:url';
import { Marked,Renderer } from './vendor/marked.mjs';
const site=path.dirname(fileURLToPath(import.meta.url)),root=path.dirname(site);
const check=process.argv.includes('--check');
const esc=s=>String(s??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const plain=s=>String(s).replace(/!?\[([^\]]+)\]\([^)]*\)/g,'$1').replace(/[`*]/g,'');
const slug=s=>plain(s).trim().toLowerCase().replace(/[^\p{L}\p{N}_\-\s]/gu,'').replace(/\s+/g,'-')||'section';
const walk=d=>fs.existsSync(path.join(root,d))?fs.readdirSync(path.join(root,d),{withFileTypes:true}).sort((a,b)=>a.name.localeCompare(b.name,'en')).flatMap(e=>e.isDirectory()?walk(`${d}/${e.name}`):[`${d}/${e.name}`]):[];
const config=JSON.parse(fs.readFileSync(path.join(site,'config.json'),'utf8'));
const core=JSON.parse(fs.readFileSync(path.join(root,'coverage/core-question-manifest.json'),'utf8'));
const hotpath='docs/80-hot100-cpp/manifest.json';
const hot=fs.existsSync(path.join(root,hotpath))?JSON.parse(fs.readFileSync(path.join(root,hotpath),'utf8')):{problems:[],supplementary:[],categories:[]};
if(!/^https:\/\/github\.com\/[\w.-]+\/[\w.-]+$/.test(config.repository_url))throw Error('Invalid source repository');
const sourceUrl=(file,anchor='')=>`${config.repository_url}/blob/${encodeURIComponent(config.branch)}/${file.split('/').map(encodeURIComponent).join('/')}${anchor?'#'+encodeURIComponent(anchor):''}`;
const scenarioPath='docs/120-interviews/manifest.json';
const interviews=fs.existsSync(path.join(root,scenarioPath))?JSON.parse(fs.readFileSync(path.join(root,scenarioPath),'utf8')):{cards:[],counts:{}};
const scenarios=new Map(interviews.cards.map(c=>[c.path,c]));
const studyPath='docs/50-rtos/source-study-manifest.json';
const study=JSON.parse(fs.readFileSync(path.join(root,studyPath),'utf8'));
const studyChapters=new Map(study.chapters.map(c=>[c.path,c]));
const indexed=new Map(core.questions.map(q=>[q.path,q]));
const problems=new Map([...hot.problems,...(hot.supplementary||[])].map(p=>[p.doc_path,p]));
const files=[...new Set([...['docs','sources','coverage','examples','algorithms'].flatMap(d=>walk(d).filter(f=>f.endsWith('.md'))),'README.md','QUESTION_INDEX.md','VERSIONS.md','CHANGELOG.md','CONTRIBUTING.md','LICENSE.md','site/README.md'])].filter(f=>fs.existsSync(path.join(root,f))).sort();
const fileSet=new Set(files),resources=new Set(),broken=[];
const inputs=['site/config.json','coverage/core-question-manifest.json',studyPath,...(fs.existsSync(path.join(root,scenarioPath))?[scenarioPath]:[]),...(fs.existsSync(path.join(root,hotpath))?[hotpath]:[])].map(f=>[f,fs.readFileSync(path.join(root,f))]);
const docs=[];
for(const file of files){
 const source=fs.readFileSync(path.join(root,file),'utf8');inputs.push([file,source]);
 const q=indexed.get(file),p=problems.get(file),c=scenarios.get(file),s=studyChapters.get(file);
 const module=config.modules.find(m=>file.startsWith(`docs/${m.folder}/`))||config.modules.find(m=>m.id==='reference');
 const title=plain(source.match(/^# (.+)$/m)?.[1]||file),headings=[],sections=[],ids=new Set();
 const renderer=new Renderer();let nextAnchor='';
 const link=href=>{
  if(/^https?:/i.test(href))return href;
  if(/^[a-z][\w+.-]*:/i.test(href))return '';
  const [local,frag='']=href.split('#');let decoded;
  try{decoded=decodeURIComponent(local.split('?')[0]);}catch{decoded=local;}
  const target=decoded?path.posix.normalize(path.posix.join(path.posix.dirname(file),decoded)):file;
  if(target.startsWith('../')||path.posix.isAbsolute(target))return '';
  if(target==='site/index.html')return '#/';
  if(fileSet.has(target))return `#/read/${encodeURIComponent(target)}${frag?'?section='+encodeURIComponent(decodeURIComponent(frag)):''}`;
  if(fs.existsSync(path.join(root,target))&&fs.statSync(path.join(root,target)).isFile()){
   resources.add(target);return `assets/repo/${target.split('/').map(encodeURIComponent).join('/')}${frag?'#'+encodeURIComponent(frag):''}`;
  }
  broken.push({from:file,href,target});return `../${href}`;
 };
 renderer.link=function(t){const h=link(t.href||'');return `<a href="${esc(h)}"${/^https?:/.test(h)?' target="_blank" rel="noopener noreferrer"':''}>${this.parser.parseInline(t.tokens)}</a>`;};
 renderer.html=t=>{const a=t.text.match(/^\s*<a\s+id=["']([\w-]+)["']\s*>\s*<\/a>\s*$/);if(a){nextAnchor=a[1];return '';}return esc(t.text);};
 renderer.image=t=>`<span>[外部图示：${esc(t.text)}] <a href="${esc(link(t.href))}">原来源</a></span>`;
 renderer.heading=function(t){let id=nextAnchor||slug(t.text);nextAnchor='';const base=id;let n=1;while(ids.has(id))id=`${base}-${n++}`;ids.add(id);t.readingId=id;headings.push({id,text:plain(t.text),level:t.depth});return `<h${t.depth} id="${esc(id)}" tabindex="-1">${this.parser.parseInline(t.tokens)}</h${t.depth}>\n`;};
 renderer.code=t=>{const lang=(t.lang||'text').split(/\s/)[0].replace(/[^\w+-]/g,'');return `<div class="code-block"><div class="code-toolbar"><span>${esc(lang)}</span><button type="button" class="copy-code" aria-label="复制 ${esc(lang)} 代码">复制代码</button></div><pre tabindex="0" aria-label="代码，可横向滚动"><code>${esc(t.text)}</code></pre></div>`;};
 renderer.table=function(t){return `<div class="table-wrap" role="region" aria-label="可横向滚动的数据表" tabindex="0">${Renderer.prototype.table.call(this,t)}</div>`;};
 const marked=new Marked({gfm:true,renderer});const tokens=marked.lexer(source);let html='',section={id:'',heading:title,text:''},answer=false;
 for(const t of tokens){
  const a=(t.raw||'').trim().match(/^<a\s+id=["']([\w-]+)["']\s*>\s*<\/a>$/);if(a){nextAnchor=a[1];continue;}
  if(t.type==='heading'){
   if(answer){html+='</div></details>';answer=false;}
   if(section.text.trim())sections.push(section);
   const rendered=marked.parser([t]);section={id:t.readingId,heading:plain(t.text),text:''};
   if(t.depth===2&&/^(直接回答|核心思路|解答|参考答案|参考解法|为什么正确|C\+\+ 实现)$/.test(plain(t.text))){html+=`<details class="answer-section" id="${esc(t.readingId)}" open><summary>${esc(plain(t.text))}<span class="summary-hint">展开 / 收起</span></summary><div class="answer-content">`;answer=true;}else html+=rendered;
  }else{html+=marked.parser([t]);if(!['html','space'].includes(t.type))section.text+=plain(t.type==='code'?t.text:t.raw||t.text||'')+'\n';}
 }
 if(answer)html+='</div></details>';if(section.text.trim())sections.push(section);
 const summary=(sections.find(s=>s.heading==='直接回答')||sections.find(s=>s.text.length>50))?.text.replace(/\s+/g,' ').trim().slice(0,180)||module.description;
 docs.push({path:file,sourceUrl:sourceUrl(file),title,module:module.id,moduleName:module.label,category:p?.category||c?.category||s?.category||module.label,difficulty:p?.difficulty||null,problemId:p?.id||null,supplemental:Boolean(p?.supplemental),questionIds:q?[q.id]:p?[`LC${p.id}`]:c?[c.id]:[],priorities:q?[q.priority]:s?[s.priority]:[],readingOrder:module.id==='rtos'?(q?Number(q.id.slice(2)):s?100+s.order:0):null,optionalHistorical:Boolean(s?.optional_historical),kind:p?'problem':q?'question':c?'scenario':s?'source-study':path.basename(file)==='README.md'?'overview':'chapter',summary,minutes:Math.max(1,Math.ceil(source.replace(/\s/g,'').length/450)),headings,sections,html,source});
}
if(broken.length)throw Error('Broken local links:\n'+JSON.stringify(broken,null,2));
const hashFiles=new Set(inputs.map(([f])=>f));
for(const f of [...resources].sort()){if(!hashFiles.has(f))inputs.push([f,fs.readFileSync(path.join(root,f))]);}
const sourceHash=crypto.createHash('sha256').update(inputs.sort(([a],[b])=>a.localeCompare(b,'en')).map(([f,b])=>`${f}\0${b}`).join('\0')).digest('hex');
const algDocs=docs.filter(d=>d.module==='hot100cpp'&&d.problemId&&!d.supplemental);
const data={schemaVersion:'1.0',repositoryUrl:config.repository_url,checkedDate:config.checked_date,sourceHash,coreCount:core.questions.length,sourceStudyCount:study.chapters.length,originalScenarioCount:interviews.cards.length,newPublicReportCount:interviews.counts.new_full_public_pages||0,interviewCounts:interviews.counts,questionCount:core.questions.length+algDocs.length,hot100Count:algDocs.length,hot100Expected:hot.problems.length,supplementaryCount:docs.filter(d=>d.supplemental).length,hot100Categories:hot.categories.map(c=>c.name),modules:config.modules.map(m=>({...m,count:docs.filter(d=>d.module===m.id).length,countUnit:'篇文档'})),docs};
const output='window.STUDY_DATA = '+JSON.stringify(data)+';\n';
const generated=path.join(site,'assets/data.js');
const index={checked_date:config.checked_date,documents:docs.map(d=>({path:d.path,title:d.title,module:d.module,question_ids:d.questionIds})),items:[...core.questions.map(q=>({...q,relative_path:q.path})),...interviews.cards.map(c=>({id:c.id,title:c.title,path:c.path,module:c.module,kind:c.kind})),...study.chapters.map(c=>({title:c.title,path:c.path,module:c.module,kind:c.kind,category:c.category,priority:c.priority,order:c.order,optional_historical:c.optional_historical})),...docs.filter(d=>d.problemId).map(d=>({id:'LC'+d.problemId,title:d.title,path:d.path,module:d.module,supplemental:d.supplemental}))]};
if(check){if(!fs.existsSync(path.join(root,'content-index.json'))||fs.readFileSync(path.join(root,'content-index.json'),'utf8')!==JSON.stringify(index,null,2)+'\n')throw Error('Generated content index is stale');if(!fs.existsSync(generated)||fs.readFileSync(generated,'utf8')!==output)throw Error('Generated reader is stale');const actualResources=walk('site/assets/repo').map(f=>f.slice('site/assets/repo/'.length)).sort();if(JSON.stringify(actualResources)!==JSON.stringify([...resources].sort()))throw Error('Offline resource inventory is stale');for(const f of resources){if(!fs.existsSync(path.join(site,'assets/repo',f))||!fs.readFileSync(path.join(root,f)).equals(fs.readFileSync(path.join(site,'assets/repo',f))))throw Error(`Resource stale: ${f}`);}}else{
 fs.writeFileSync(generated,output);fs.writeFileSync(path.join(root,'content-index.json'),JSON.stringify(index,null,2)+'\n');fs.rmSync(path.join(site,'assets/repo'),{recursive:true,force:true});for(const f of resources){const dest=path.join(site,'assets/repo',f);fs.mkdirSync(path.dirname(dest),{recursive:true});fs.copyFileSync(path.join(root,f),dest);}
}
console.log(`${check?'Checked':'Built'} ${docs.length} documents, ${core.questions.length} core topics, ${algDocs.length}/${hot.problems.length} official Hot100 docs, ${data.supplementaryCount} supplementary, ${interviews.cards.length} original interview scenarios, ${study.chapters.length} FreeRTOS source-study chapters; ${resources.size} offline resources.`);
