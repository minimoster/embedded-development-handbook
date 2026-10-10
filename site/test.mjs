import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import assert from 'node:assert/strict';
import {createRequire} from 'node:module';
import {fileURLToPath} from 'node:url';
const site=path.dirname(fileURLToPath(import.meta.url)),root=path.dirname(site);
const context={window:{}};vm.runInNewContext(fs.readFileSync(path.join(site,'assets/data.js'),'utf8'),context);
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
const app=fs.readFileSync(path.join(site,'assets/app.js'),'utf8');assert(app.includes('zhixing.embedded.reader.v1'));assert(!app.includes('zhixing.reader.v1'));assert(!/\bfetch\s*\(|XMLHttpRequest|https?:\/\//.test(app));
console.log(`Static reader PASS: ${docs.length} source-consistent docs, exact local routes/anchors, escaped output, resources, search/filter/counts.`);

await import('./test-navigation.mjs');
