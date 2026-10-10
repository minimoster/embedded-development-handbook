/* Shared, dependency-free full-text model: used by the reader and Node tests. */
(() => {
  'use strict';
  const normalize = s => String(s || '').normalize('NFKC').toLocaleLowerCase('zh-CN').replace(/\s+/g,' ').trim();
  function create(docs) {
    const indexed=docs.map(d=>({doc:d,all:normalize(`${d.title} ${d.problemId||d.resourceProblemId||''} ${d.problemId||d.resourceProblemId?'LC'+(d.problemId||d.resourceProblemId):''} ${d.questionIds.join(' ')} ${d.category} ${d.sections.map(s=>s.heading+' '+s.text).join(' ')}`),title:normalize(d.title),sections:d.sections.map(s=>({...s,index:normalize(s.heading+' '+s.text)}))}));
    return {query(params,state={saved:[],done:[]}) {
      const terms=normalize(params.get('q')||'').split(' ').filter(Boolean);
      const module=params.get('module')||'',topic=params.get('topic')||'',difficulty=params.get('difficulty')||'',priority=params.get('priority')||'',view=params.get('view')||'';
      const results=[];
      for(const item of indexed){
        const d=item.doc;
        if(d.kind==='external-resource'&&view)continue; // saved/read state is chapter-level
        if(d.kind==='resource-bank'&&module==='supplement'&&!terms.length&&!view)continue;
        if(module&&d.module!==module||topic&&d.category!==topic||difficulty&&d.difficulty!==difficulty||priority&&!d.priorities.includes(priority))continue;
        if(view==='saved'&&!state.saved.includes(d.path)||view==='done'&&!state.done.includes(d.path))continue;
        if(!terms.every(t=>item.all.includes(t)))continue;
        let section=null,score=0;
        if(terms.length){
          score=terms.reduce((sum,t)=>sum+(item.title.includes(t)?100:0),0);
          const id=d.problemId||d.resourceProblemId;
          if(id&&terms.some(t=>t===String(id)||t==='lc'+id))score+=200;
          let bestScore=-1;
          for(const s of item.sections){
            const titleNorm=normalize(s.heading);
            const sectionScore=terms.reduce((sum,t)=>sum+(titleNorm.includes(t)?45:0)+(s.index.includes(t)?5:0),0);
            if(sectionScore>bestScore){bestScore=sectionScore;section=s;}
          }
          score+=Math.max(0,bestScore);
        }
        results.push({d,section,score});
      }
      if(terms.length)results.sort((a,b)=>b.score-a.score||a.d.path.localeCompare(b.d.path,'en'));
      else results.sort((a,b)=>a.d.module==='supplement'&&b.d.module==='supplement'?(a.d.resourceOrder||0)-(b.d.resourceOrder||0):a.d.module==='rtos'&&b.d.module==='rtos'?(a.d.readingOrder||0)-(b.d.readingOrder||0):a.d.module==='hot100cpp'&&b.d.module==='hot100cpp'?(a.d.kind==='overview'?-1:b.d.kind==='overview'?1:(a.d.problemId||0)-(b.d.problemId||0)):a.d.path.localeCompare(b.d.path,'en'));
      return {results,terms};
    }};
  }
  const api={normalize,create};
  if(typeof window!=='undefined')window.STUDY_SEARCH=api;
  if(typeof module!=='undefined'&&module.exports)module.exports=api;
})();
