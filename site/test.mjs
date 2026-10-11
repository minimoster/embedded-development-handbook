import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import assert from 'node:assert/strict';
import {createRequire} from 'node:module';
import {fileURLToPath} from 'node:url';
const site=path.dirname(fileURLToPath(import.meta.url)),root=path.dirname(site);
const html=fs.readFileSync(path.join(site,'index.html'),'utf8');
const scripts=[...html.matchAll(/<script src="([^"\n]+)" defer><\/script>/g)].map(m=>m[1]);
const shards=scripts.filter(f=>/^assets\/data\.[^.]+\.\d{3}\.js$/.test(f));
assert(shards.length>1,'Reader must use multiple bounded data shards');
assert.equal(new Set(scripts).size,scripts.length,'Script paths must be unique');
assert.deepEqual(scripts.slice(0,shards.length),shards,'All shards load before the data assembler');
assert.deepEqual(scripts.slice(shards.length),['assets/data.js','assets/search.js','assets/app.js?v=named-source-downloads-20261011'],'Data assembly precedes search and UI');
const context={window:{}};
for(const f of shards){
 const file=path.join(site,f);assert(fs.statSync(file).size<=262144,'Reader shard byte bound');
 vm.runInNewContext(fs.readFileSync(file,'utf8'),context);
}
const payload=context.window.STUDY_DATA_TEXT;
vm.runInNewContext(fs.readFileSync(path.join(site,'assets/data.js'),'utf8'),context);
assert.equal(context.window.STUDY_DATA_TEXT,undefined,'Transient reader text is released');
assert.equal(JSON.stringify(context.window.STUDY_DATA),payload,'Ordered shards reconstruct exact JSON');
const digest=context.window.STUDY_DATA.sourceHash.slice(0,16);
assert(shards.every((f,i)=>f===`assets/data.${digest}.${String(i+1).padStart(3,'0')}.js`),'Digest and sequence guard shard identity');
assert.deepEqual(fs.readdirSync(path.join(site,'assets')).filter(f=>/^data\..+\.js$/.test(f)).sort(),shards.map(f=>path.basename(f)).sort(),'No unreferenced or stale shards');
const data=context.window.STUDY_DATA;assert.equal(data.coreCount,JSON.parse(fs.readFileSync(path.join(root,'coverage/core-question-manifest.json'),'utf8')).questions.length);
const docs=data.docs;assert.equal(new Set(docs.map(d=>d.path)).size,docs.length);
const byPath=new Map(docs.map(d=>[d.path,d]));
assert(byPath.get('README.md').html.includes('href="https://minimoster.github.io/embedded-development-handbook/"'),'README must expose the verified online reader URL');
for(const d of docs){
 assert.equal(d.source,fs.readFileSync(path.join(root,d.path),'utf8'),`Source differs ${d.path}`);
 assert.equal(new Set(d.headings.map(h=>h.id)).size,d.headings.length);
 assert(!/<script\b|<iframe\b|<img\b|\son(?:error|load|click)=|href="javascript:/i.test(d.html),d.path);
 for(const m of d.html.matchAll(/href="#\/read\/([^"?]+)(?:\?section=([^"]+))?"/g)){
  const target=byPath.get(decodeURIComponent(m[1]));assert(target,`${d.path} target ${m[1]}`);
  if(m[2])assert(target.headings.some(h=>h.id===decodeURIComponent(m[2])),`${d.path} section ${m[2]}`);
 }
 for(const m of d.html.matchAll(/href="(assets\/repo\/[^"#]+)(?:#[^"]*)?"/g))assert(fs.existsSync(path.join(site,decodeURIComponent(m[1]))),m[1]);
}
const require=createRequire(import.meta.url);const engine=require('./assets/search.js').create(docs);
for(const term of ['C01','C17','AR09','RCU','volatile','DMA','EAGAIN','优先级反转','LC1'])assert(engine.query(new URLSearchParams({q:term})).results.length,term);
assert(engine.query(new URLSearchParams({q:'AR04'})).results.some(r=>r.d.path.endsWith('ar04.md')));
assert.equal(engine.query(new URLSearchParams({q:'不可能存在的连续关键词z9x8c7'})).results.length,0);
const core=engine.query(new URLSearchParams({module:'c',priority:'P0'})).results;assert(core.length>8);assert(core.every(r=>r.d.module==='c'&&r.d.priorities.includes('P0')));
const alg=engine.query(new URLSearchParams({module:'hot100cpp',difficulty:'HARD'})).results;assert(alg.length>0);assert(alg.every(r=>r.d.difficulty==='HARD'));
assert.equal(docs.filter(d=>d.module==='hot100cpp'&&d.problemId&&!d.supplemental).length,data.hot100Count);
assert.equal(docs.filter(d=>d.supplemental).length,data.supplementaryCount);
const interview=JSON.parse(fs.readFileSync(path.join(root,'docs/120-interviews/manifest.json'),'utf8'));
assert.equal(data.coreCount,120);assert.equal(data.hot100Count,100);assert.equal(data.supplementaryCount,1);
assert.equal(data.originalScenarioCount,7);assert.equal(data.newPublicReportCount,17);assert.equal(data.interviewCounts.new_2026_full_public_pages,0);assert.equal(data.interviewCounts.verified_xiaohongshu_original_pages,0);
const cases=docs.filter(d=>d.kind==='scenario');assert.equal(cases.length,7);assert(cases.every(d=>d.module==='interview'&&!d.problemId&&!d.supplemental&&!d.priorities.length));
for(const c of interview.cards){const d=byPath.get(c.path);assert(d&&d.questionIds.includes(c.id)&&d.category===c.category);assert(engine.query(new URLSearchParams({q:c.id,module:'interview'})).results.some(r=>r.d.path===c.path),c.id);}
for(const term of ['EN01','EN18','STM32','remove','试运行确认'])assert(engine.query(new URLSearchParams({q:term,module:'interview'})).results.length,term);
assert.equal(engine.query(new URLSearchParams({module:'interview'})).results.length,9);
assert(engine.query(new URLSearchParams({q:'IC01'})).results.some(r=>r.d.path.endsWith('ic01.md')));
const study=JSON.parse(fs.readFileSync(path.join(root,'docs/50-rtos/source-study-manifest.json'),'utf8'));
assert.equal(data.sourceStudyCount,7);assert.equal(data.optionalDepthCount,16);assert.equal(docs.length,273);
const studyDocs=docs.filter(d=>d.kind==='source-study');assert.equal(studyDocs.length,7);
for(const c of study.chapters){const d=byPath.get(c.path);assert(d&&d.module==='rtos'&&d.category===c.category&&!d.questionIds.length&&!d.problemId&&!d.supplemental);assert(d.priorities.includes(c.priority));assert(engine.query(new URLSearchParams({module:'rtos',topic:c.category})).results.some(r=>r.d.path===c.path),c.path);}
assert.equal(docs.filter(d=>d.module==='rtos'&&d.kind==='question').length,10);
const rtosRoute=engine.query(new URLSearchParams({module:'rtos'})).results.map(r=>r.d.path);
assert.deepEqual(Array.from(rtosRoute.slice(10,17)),study.chapters.map(c=>c.path));
for(const [term,filename] of [['pxContainer','kernel-lists.md'],['TCB','kernel-task-lifecycle.md'],['超时复查','kernel-queue-waits.md'],['事件组','kernel-event-groups.md'],['daemon','kernel-timer-daemon.md'],['co-routine','historical-co-routines.md']])assert(engine.query(new URLSearchParams({q:term,module:'rtos'})).results.some(r=>r.d.path.endsWith(filename)),term);
const optional=JSON.parse(fs.readFileSync(path.join(root,'docs/130-reference/optional-depth-manifest.json'),'utf8'));
assert.equal(docs.filter(d=>d.kind==='optional-depth').length,16);
for(const c of optional.chapters){const d=byPath.get(c.path);assert(d&&d.kind===c.kind&&d.module===c.module&&d.category===c.category&&d.priorities.includes(c.priority)&&!d.questionIds.length&&!d.problemId&&!d.supplemental);assert(engine.query(new URLSearchParams({module:c.module,topic:c.category,priority:c.priority})).results.some(r=>r.d.path===c.path),c.path);}
assert(byPath.get('docs/130-reference/optional-depth.md').html.includes('task-diagnostics.md'));
for(const [term,filename] of [['bus-clear','i2c-timing-recovery.md'],['spi_ioc_transfer','spi-edges-spidev.md'],['ssthresh','tcp-reno-rto.md']])assert(engine.query(new URLSearchParams({q:term,module:'protocol'})).results.some(r=>r.d.path.endsWith(filename)),term);
for(const [term,filename] of [['双向所有权','publication-ownership-contracts.md'],['PRIGROUP','nvic-priority-audit.md'],['terminate_async','dma-lifecycle-contracts.md'],['LSPACT','cortex-m4-fault-frame-contracts.md']])assert(engine.query(new URLSearchParams({q:term})).results.some(r=>r.d.path.endsWith(filename)),term);
assert(byPath.get('docs/130-reference/interview-depth-refresh.md').html.includes('xiaohongshu.com'));
const app=fs.readFileSync(path.join(site,'assets/app.js'),'utf8');assert(app.includes('zhixing.embedded.reader.v1'));assert(!app.includes('zhixing.reader.v1'));assert(!/\bfetch\s*\(|XMLHttpRequest|https?:\/\//.test(app));
// Exercise the production helper independently of MIME-specific browser behavior.
const escLine=app.split('\n').find(line=>line.trim().startsWith('const esc ='));
const downloadStart=app.indexOf('  function sourceDownloads(html) {');
const downloadEnd=app.indexOf('  function libraryHash(params={})',downloadStart);
assert(downloadStart>0&&downloadEnd>downloadStart);
assert(app.includes('${sourceDownloads(d.html)}'),'Helper applies only to rendered reader article HTML');
const sourceDownloads=vm.runInNewContext(`${escLine}\n${app.slice(downloadStart,downloadEnd)}\nsourceDownloads`,{}, {timeout:1000});
for(const extension of ['c','h','cpp','hpp','py','json']){
 const original=`<a href="assets/repo/examples/demo.${extension}">查看源码</a>`;
 const converted=sourceDownloads(original);
 assert(converted.startsWith(original+' '),'Plain source browsing remains intact');
 assert(converted.includes(`download="demo.${extension}"`));
 assert(converted.includes(`aria-label="下载 demo.${extension}"`));
 const explicit=`<a href="assets/repo/examples/demo.${extension}" class="button">下载源码</a>`;
 assert.equal((sourceDownloads(explicit).match(/<a /g)||[]).length,1,'Existing download control is not duplicated');
 assert(sourceDownloads(explicit).includes(`download="demo.${extension}"`));
}
for(const unchanged of [
 '<a href="https://github.com/owner/repo/blob/main/demo.cpp">查看源码</a>',
 '<a href="https://example.com/assets/repo/demo.cpp">下载源码</a>',
 '<a href="//example.com/assets/repo/demo.cpp">下载源码</a>',
 '<a href="#/read/docs%2Fdemo.md">讲义</a>',
 '<a href="assets/repo/diagram.svg">图示</a>',
 '<a href="assets/repo/demo.cpp?token=x">源码</a>',
 '<a href="assets/repo/../demo.cpp">源码</a>',
 '<a href="assets/repo/%2e%2e/demo.cpp">源码</a>',
 '<a href="assets/repo/%2fetc/demo.cpp">源码</a>',
 '<a href="assets/repo/%5cetc/demo.cpp">源码</a>',
 '<a href="assets/repo/%00demo.cpp">源码</a>',
 '<a href="assets/repo/%zzdemo.cpp">源码</a>',
 '<a href="assets/repo/demo.cpp" download="custom.cpp">已命名下载</a>'
])assert.equal(sourceDownloads(unchanged),unchanged,'Unrelated or invalid resource anchors stay unchanged');
const quoted=sourceDownloads('<a href="assets/repo/%22demo.cpp#L5">源码</a>');
assert(quoted.includes('download="&quot;demo.cpp"'),'Decoded filenames are escaped');
assert(quoted.includes('class="resource-download" href="assets/repo/%22demo.cpp"'),'New download controls omit view-only fragments');
let resourceAnchors=0;
for(const d of docs){
 const rendered=sourceDownloads(d.html);
 for(const match of d.html.matchAll(/<a href="assets\/repo\/[^"#?]+\.(?:c|h|cpp|hpp|py|json)(?:#[^"]*)?"[^>]*>[\s\S]*?<\/a>/gi)){
  resourceAnchors++;
  if(!/下载/.test(match[0]))assert(rendered.includes(match[0]),`Original source anchor lost: ${d.path}`);
 }
 assert(!/download="[^"<>]*[<>]/.test(rendered));
}
assert(resourceAnchors>=252,'Check download behavior across the complete resource-linked reader corpus');
console.log(`Named source download helper PASS: six teaching extensions, unchanged browsing/external/Markdown/image links, unsafe-path rejection, escaped filenames, ${resourceAnchors} full-corpus resource anchors. Browser clicks remain a separate gate.`);

console.log(`Static reader PASS: ${docs.length} source-consistent docs, exact local routes/anchors, escaped output, resources, search/filter/counts.`);

await import('./test-navigation.mjs');
