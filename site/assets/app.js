/* Browser-only reader. No analytics, accounts, fetch calls, or remote storage. */
(() => {
  'use strict';
  const data = window.STUDY_DATA;
  const main = document.getElementById('main');
  if (!data) { main.textContent = '阅读数据未加载。请从仓库根目录运行 node site/build.mjs。'; return; }
  const esc = s => String(s ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const normalize = window.STUDY_SEARCH.normalize;
  const byPath = new Map(data.docs.map(d => [d.path, d]));
  const catalogueDocs = [...data.docs,...(data.supplementaryResources||[])];
  const difficultyName = { EASY:'简单', MEDIUM:'中等', HARD:'困难', Easy:'简单', Medium:'中等', Hard:'困难' };
  let state = { saved:[], done:[], lastPath:'', fontSize:17, lineHeight:1.95, theme:'' };
  let storageOK = true;
  try { const old = JSON.parse(localStorage.getItem('zhixing.embedded.reader.v1') || '{}'); state = {...state,...old}; } catch { storageOK = false; }
  state.saved = Array.isArray(state.saved) ? state.saved.filter(p => byPath.has(p)) : [];
  state.done = Array.isArray(state.done) ? state.done.filter(p => byPath.has(p)) : [];
  state.fontSize = Math.min(22,Math.max(15,Number(state.fontSize)||17));
  state.lineHeight = [1.8,1.95,2.15].includes(state.lineHeight) ? state.lineHeight : 1.95;
  if (!['light','dark'].includes(state.theme)) state.theme = window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  let route;
  let currentPath = '';
  let observer;
  let searchTimer;
  let toastTimer;
  const search = document.getElementById('global-search');
  const sidebar = document.getElementById('sidebar');
  const backdrop = document.getElementById('sidebar-backdrop');
  const navOpen = document.getElementById('nav-open');
  const theme = document.getElementById('theme-toggle');
  const searchEngine = window.STUDY_SEARCH.create(catalogueDocs);
  // The app's route also lives in the hash. A plain #main navigation would
  // replace the current reading route, so the skip link moves focus directly.
  document.querySelector('.skip-link').addEventListener('click',e=>{
    e.preventDefault();
    main.focus({preventScroll:true});
    main.scrollIntoView({block:'start'});
  });

  function persist() {
    try { localStorage.setItem('zhixing.embedded.reader.v1',JSON.stringify(state)); } catch { storageOK = false; }
    document.getElementById('saved-count').textContent = state.saved.length;
    document.getElementById('done-count').textContent = state.done.length;
  }
  function applyPreferences() {
    document.documentElement.dataset.theme = state.theme;
    document.documentElement.style.setProperty('--reader-size',state.fontSize+'px');
    document.documentElement.style.setProperty('--reader-line',state.lineHeight);
    theme.setAttribute('aria-label',`切换到${state.theme==='light'?'深':'浅'}色主题`);
    theme.title = theme.getAttribute('aria-label');
  }
  function toast(message) {
    const element=document.getElementById('toast');
    clearTimeout(toastTimer);element.textContent=message;element.classList.add('visible');
    toastTimer=setTimeout(()=>element.classList.remove('visible'),2800);
  }
  function hashForRead(path, section='') { return `#/read/${encodeURIComponent(path)}${section?'?section='+encodeURIComponent(section):''}`; }
  function libraryHash(params={}) { const q=new URLSearchParams(Object.entries(params).filter(([,v])=>v!==''&&v!=null));return '#/library'+(q.size?'?'+q.toString():''); }
  function navigate(hash, replace=false) {
    if (hash===location.hash) { render(); return; }
    if(replace){history.replaceState(null,'',hash);render();}else location.hash=hash;
  }
  function parseRoute() {
    const raw=location.hash.slice(1)||'/';
    const index=raw.indexOf('?');
    const pathname=index<0?raw:raw.slice(0,index);
    const query=new URLSearchParams(index<0?'':raw.slice(index+1));
    let path='';
    try { path=decodeURIComponent(pathname.replace(/^\/read\//,'')); } catch { path=''; }
    return {type:pathname.startsWith('/read/')?'read':pathname==='/library'?'library':'home',path,query};
  }
  function buildNav() {
    document.getElementById('module-nav').innerHTML=`<a href="#/" class="nav-item" data-nav="home"><span aria-hidden="true">⌂</span><span>学习总览</span></a>`+data.modules.map(m=>`<a href="${libraryHash({module:m.id})}" class="nav-item" data-nav="${m.id}"><span aria-hidden="true">${m.number}</span><span>${esc(m.label)}</span><span class="nav-count">${m.count}</span></a>`).join('');
  }
  function highlight(value,terms) {
    const text=String(value||'');
    if(!terms.length)return esc(text);
    const escaped=terms.map(t=>t.replace(/[.*+?^${}()|[\]\\]/g,'\\$&')).sort((a,b)=>b.length-a.length);
    const expression=new RegExp('('+escaped.join('|')+')','giu');
    return text.split(expression).map(part=>terms.some(t=>normalize(part)===t)?`<mark>${esc(part)}</mark>`:esc(part)).join('');
  }
  function snippet(text,terms) {
    const clean=String(text||'').replace(/\s+/g,' ').trim();
    const found=terms.map(t=>normalize(clean).indexOf(t)).filter(n=>n>=0);
    const at=found.length?Math.min(...found):0;
    const start=Math.max(0,at-35),end=Math.min(clean.length,start+145);
    return (start?'…':'')+clean.slice(start,end)+(end<clean.length?'…':'');
  }
  function badges(d) {
    return (d.kind==='external-resource'||d.kind==='resource-bank'?'<span class="badge">外部资源题库</span>':'')+(d.difficulty?`<span class="badge ${d.difficulty.toLowerCase()}">${difficultyName[d.difficulty]}</span>`:'') + d.priorities.map(p=>`<span class="badge priority">${p} 优先级</span>`).join('');
  }
  function art() {
    return `<svg class="hero-art" aria-hidden="true" viewBox="0 0 245 215"><g fill="none" stroke="currentColor" stroke-width="1.15"><circle cx="121" cy="103" r="84" opacity=".1"/><circle cx="121" cy="103" r="63" stroke-dasharray="2 5" opacity=".15"/><path d="M46 76 120 54l75 22v101l-75-22-74 22z" fill="var(--panel)"/><path d="M120 54v101M46 76l74 23 75-23M120 99v74M46 177l74 22 75-22v-101"/><path d="m57 88 49 15m-49-1 49 15m-49-1 35 11m-35 3 44 14m-44 0 24 8m111-64-49 15m49-1-49 15m49-1-35 11m35 3-44 14m44 0-24 8" opacity=".3"/><path d="M23 48h41m139-2h24m-7 0v27M15 143h19m173 21h26" opacity=".3"/><path d="m145 32 11 4 11-4-11-4zM156 36v14" opacity=".7"/></g><g fill="currentColor"><circle cx="44" cy="42" r="3" opacity=".5"/><circle cx="213" cy="128" r="2" opacity=".4"/><circle cx="31" cy="183" r="2" opacity=".4"/></g><text x="157" y="198" fill="currentColor" font-size="7" letter-spacing="1.7" opacity=".5">LEARN · CONNECT</text></svg>`;
  }
  function renderHome() {
    const resume=byPath.get(state.lastPath);
    main.className='home-main';
    main.innerHTML=`<section class="home-hero"><div><div class="eyebrow">EMBEDDED SYSTEMS / 学习与面试</div><h1 class="home-title">从一段代码，<br>走到一条可靠的<span class="hero-final-clause">数据链。</span></h1><p class="hero-description">先看懂语言与内存，再连接硬件、并发与系统。<br>用例子讲清机制，用测试确认边界，用证据回答追问。</p><div class="hero-tags"><span class="quiet-tag">原创讲解</span><span class="quiet-tag">C11 / C++17</span><span class="quiet-tag">来源可追溯</span><span class="quiet-tag">离线阅读</span></div><div class="hero-stats"><div class="stat"><strong>${data.coreCount}</strong>核心主题</div><div class="stat"><strong>${data.hot100Count}</strong>Hot100 题解</div><div class="stat"><strong>${data.supplementaryCount}</strong>独立补充题</div></div></div>${art()}</section><section class="reading-note" aria-label="开始阅读建议"><span class="note-icon" aria-hidden="true">↗</span><div class="note-content"><strong>${resume?'接着上次的章节，逐步走完一条链路':'第一次来？先选一个主方向。'}</strong><p>${resume?esc(resume.title)+' · '+esc(resume.moduleName):'MCU / RTOS、Linux 应用或 BSP / 驱动。共享基础先修，算法独立练习。'}</p></div><a class="text-link" href="${resume?hashForRead(resume.path):hashForRead('docs/00-guide/learning-path.md')}">${resume?'继续阅读':'查看学习路线'} →</a></section><section aria-labelledby="modules-heading"><div class="section-heading"><h2 id="modules-heading">把知识连成主线</h2><p>直接回答 · 原理 · 例子 · 误区 · 追问 · 验收</p></div><div class="module-grid">${data.modules.filter(m=>m.id!=='reference').map(m=>`<a class="module-card ${m.id==='hot100cpp'?'hot100':''}" href="${libraryHash({module:m.id})}"><div class="card-topline"><span class="card-number">${m.number} /</span><span class="card-count">${m.count} 篇文档</span></div><h3>${esc(m.label)}</h3><p>${esc(m.description)}</p><span class="card-arrow" aria-hidden="true">↗</span></a>`).join('')}</div></section><div class="home-bottom"><section class="small-section"><h3>先解释，再动手</h3><p>试着独立回答，展开解答对照，再改变一个边界输入。题号、概念与完整正文都可搜索；P0/P1/P2 是学习建议，算法难度单独筛选。</p><a href="${hashForRead('examples/README.md')}">运行主机练习 →</a></section><section class="small-section"><h3>知道证据的边界</h3><p>主机测试不是板测。公开技术文档与目标版本分别记录；未验证的硬件保持明确。阅读记录只存在本机。</p><a href="${hashForRead('docs/130-reference/coverage-boundaries.md')}">查看来源与覆盖 →</a></section></div>`;
  }
  function resultRows(params) { return searchEngine.query(params,state); }
  function renderLibrary() {
    const p=route.query;
    const module=p.get('module')||'',q=p.get('q')||'',view=p.get('view')||'';
    const moduleInfo=data.modules.find(m=>m.id===module);
    const title=q?'搜索结果':view==='saved'?'我的收藏':view==='done'?'已读章节':moduleInfo?.label||'全部学习资料';
    const desc=q?`搜索“${q}”，匹配标题、题号和完整正文。多个关键词用空格分隔。`:view==='saved'?'值得回来再读的章节，留在你的本机浏览器里。':view==='done'?'阅读过的章节，随时返回复习。已读标记可以取消。':moduleInfo?.description||'按主题找一条主线，也可以用关键词直接定位问题。';
    const topics=[...new Set(catalogueDocs.filter(d=>(!module||d.module===module)&&!(module==='supplement'&&d.kind==='resource-bank')).map(d=>d.category))];
    const {results,terms}=resultRows(p);
    const pageSize=24,totalPages=Math.max(1,Math.ceil(results.length/pageSize));
    let page=Math.min(totalPages,Math.max(1,Number(p.get('page'))||1));
    const visible=results.slice((page-1)*pageSize,page*pageSize);
    main.className='library-main';
    main.innerHTML=`<div class="breadcrumb"><a href="#/">学习总览</a><span>/</span><span>${esc(title)}</span></div><div class="page-heading"><div class="eyebrow">${q?'FIND YOUR THREAD':view?'YOUR READING':'EXPLORE & UNDERSTAND'}</div><h1>${esc(title)}</h1><p>${esc(desc)}</p></div>${!storageOK&&view?'<p class="storage-warning">此浏览器未允许本地存储，当前记录只在本次打开期间保留。</p>':''}<section class="filters" aria-label="筛选学习资料"><div class="filter-field"><label for="filter-module">专题</label><select id="filter-module" data-filter="module"><option value="">全部专题</option>${data.modules.map(m=>`<option value="${m.id}"${m.id===module?' selected':''}>${esc(m.label)}</option>`).join('')}</select></div><div class="filter-field"><label for="filter-topic">主题</label><select id="filter-topic" data-filter="topic"><option value="">全部主题</option>${topics.map(t=>`<option value="${esc(t)}"${t===p.get('topic')?' selected':''}>${esc(t)}</option>`).join('')}</select></div><div class="filter-field"><label for="filter-difficulty">算法难度</label><select id="filter-difficulty" data-filter="difficulty"><option value="">全部难度</option>${Object.entries({EASY:'简单',MEDIUM:'中等',HARD:'困难'}).map(([v,label])=>`<option value="${v}"${v===p.get('difficulty')?' selected':''}>${label}</option>`).join('')}</select></div><div class="filter-field"><label for="filter-priority">章节含优先级</label><select id="filter-priority" data-filter="priority"><option value="">全部优先级</option>${['P0','P1','P2'].map(v=>`<option value="${v}"${v===p.get('priority')?' selected':''}>${v}</option>`).join('')}</select></div><span class="filter-note">难度仅适用于算法；P0/P1/P2 是课程学习建议。</span></section><div class="results-meta" role="status" aria-live="polite"><span>${results.length} 个匹配条目${results.length>pageSize?` · 第 ${page} / ${totalPages} 页`:''}</span><button class="reset-button" id="reset-filters">清空搜索与筛选</button></div>${results.length?`<div class="result-list">${visible.map(({d,section})=>{const heading=section&&/^[A-Z]+\d+\b/.test(section.heading)?section.heading:d.title;const marker=d.problemId||d.resourceProblemId||(d.module==='reference'?'§':data.modules.find(m=>m.id===d.module)?.number||'·');return `<a class="result-item" href="${hashForRead(d.path,section?.id||d.targetAnchor||'')}"><span class="result-marker" aria-hidden="true">${d.problemId||marker}</span><div><h2 class="result-title">${highlight(heading,terms)}</h2><div class="result-meta"><span>${esc(d.moduleName)}</span>${d.category!==d.moduleName?`<span>· ${esc(d.category)}</span>`:''}${badges(d)}${d.kind==='external-resource'?'<span>外部链接与复习提示</span>':`<span>约 ${d.minutes} 分钟</span>`}${state.saved.includes(d.path)?'<span aria-label="已收藏">☆ 已收藏</span>':''}${state.done.includes(d.path)?'<span>✓ 已读</span>':''}</div><p class="result-snippet">${highlight(snippet(section?.text||d.summary,terms),terms)}</p></div><span class="result-arrow" aria-hidden="true">↗</span></a>`;}).join('')}</div>`:`<section class="empty-state"><div class="empty-symbol" aria-hidden="true">⌕</div><h2>${view&&!q&&!p.get('difficulty')&&!p.get('module')&&!p.get('topic')?'这里还没有章节':'暂时没有匹配的章节'}</h2><p>${view?'在阅读页点击收藏或标记已读，记录就会留在这里。':'试试更短的关键词、明确的题号，或放宽筛选条件。'}</p><button class="button" id="empty-reset">${view?'浏览学习资料':'清空搜索与筛选'}</button></section>`}${totalPages>1?`<nav class="pagination" aria-label="搜索结果分页"><button class="button" data-page="${page-1}"${page===1?' disabled':''}>← 上一页</button><span>${page} / ${totalPages}</span><button class="button" data-page="${page+1}"${page===totalPages?' disabled':''}>下一页 →</button></nav>`:''}`;
  }
  function tocLinks(d) {
    return d.headings.filter(h=>h.level>1&&h.level<=3).map(h=>`<a href="${hashForRead(d.path,h.id)}" data-section="${esc(h.id)}" class="${h.level===3?'toc-sub':''}">${esc(h.text)}</a>`).join('');
  }
  function renderReader() {
    const d=byPath.get(route.path);
    if(!d){main.className='';main.innerHTML='<section class="empty-state"><h1>这一页没有找到</h1><p>路径可能已更新，试试搜索章节标题或题号。</p><a class="button" href="#/library">返回学习资料</a></section>';currentPath='';return;}
    currentPath=d.path;
    state.lastPath=d.path;persist();
    const toc=tocLinks(d);
    const peers=data.docs.filter(x=>x.module===d.module&&x.kind!=='overview');
    const i=peers.findIndex(x=>x.path===d.path),prev=peers[i-1],next=peers[i+1];
    main.className='reader-main';
    main.innerHTML=`<div class="breadcrumb"><a href="#/">学习总览</a><span>/</span><a href="${libraryHash({module:d.module})}">${esc(d.moduleName)}</a><span>/</span><span>${d.problemId?d.problemId+' · '+esc(d.category):'章节阅读'}</span></div><div class="reader-layout"><div class="reader-column"><header><div class="eyebrow">${d.problemId?(d.supplemental?'独立补充 / ':'HOT100 · C++ / ')+esc(d.category):esc(d.moduleName)+' / READ & UNDERSTAND'}</div><h1 class="reader-title">${esc(d.title)}</h1><div class="reader-meta">${badges(d)}<span>约 ${d.minutes} 分钟</span><span>${d.supplemental?'· 额外公开练习':'· 原创讲义'}</span><span>· 来源与版本见正文</span></div><div class="reader-tools"><button class="button" id="save-doc" aria-pressed="${state.saved.includes(d.path)}">${state.saved.includes(d.path)?'★ 已收藏':'☆ 收藏章节'}</button><button class="button" id="done-doc" aria-pressed="${state.done.includes(d.path)}">${state.done.includes(d.path)?'✓ 已读':'标记已读'}</button><a class="button" href="${esc(d.sourceUrl)}" target="_blank" rel="noopener noreferrer">GitHub 原文 ↗</a><button class="button" id="toggle-answers" aria-pressed="false">收起知识 / 解答</button><div class="text-size-controls"><span>字号</span><button id="font-minus" aria-label="缩小正文字号"${state.fontSize<=15?' disabled':''}>A−</button><button id="font-plus" aria-label="放大正文字号"${state.fontSize>=22?' disabled':''}>A+</button><button id="line-height" aria-label="切换正文行距" title="切换正文行距">↕</button></div></div></header>${toc?`<details class="mobile-toc"><summary>本章目录 · ${d.headings.filter(h=>h.level>1&&h.level<=3).length} 个小节</summary><nav aria-label="本章目录">${toc}</nav></details>`:''}<article class="reader-content" id="reader-content" aria-label="${esc(d.title)}正文">${d.html}</article><details class="source-view"><summary>查看本章 Markdown 源文 · 构建自 ${esc(d.path)}</summary><pre>${esc(d.source)}</pre></details><nav class="reader-next" aria-label="同专题前后章节">${prev?`<a href="${hashForRead(prev.path)}"><span>← 上一篇</span>${esc(prev.title)}</a>`:''}${next?`<a href="${hashForRead(next.path)}"><span>下一篇 →</span>${esc(next.title)}</a>`:''}</nav></div>${toc?`<aside class="toc" aria-label="本章目录"><h2>这一章的主线</h2><nav>${toc}</nav></aside>`:''}</div>`;
    observeHeadings();
  }
  function observeHeadings() {
    observer?.disconnect();
    if(!('IntersectionObserver' in window))return;
    observer=new IntersectionObserver(entries=>{const visible=entries.filter(e=>e.isIntersecting).sort((a,b)=>a.boundingClientRect.top-b.boundingClientRect.top);if(visible[0])markToc(visible[0].target.id);},{rootMargin:'-90px 0px -65% 0px'});
    main.querySelectorAll('.reader-content h2[id],.reader-content h3[id],.reader-content details[id]').forEach(h=>observer.observe(h));
  }
  function markToc(id) { main.querySelectorAll('[data-section]').forEach(a=>a.classList.toggle('active',a.dataset.section===id)); }
  function scrollToSection(id) {
    // Collapse the mobile TOC before measuring the article's final position.
    const mobileToc=main.querySelector('.mobile-toc');if(mobileToc)mobileToc.open=false;
    if(!id){window.scrollTo(0,0);return;}
    const target=document.getElementById(id);
    if(!target||!main.contains(target)){toast('本节锚点未找到，可以在章内目录选择小节。');return;}
    let parent=target.parentElement;
    while(parent&&parent!==main){if(parent.tagName==='DETAILS')parent.open=true;parent=parent.parentElement;}
    if(target.tagName==='DETAILS')target.open=true;
    target.scrollIntoView({block:'start'});markToc(id);
    if(target.matches('h2,h3,h4'))target.focus({preventScroll:true});
    else if(target.tagName==='DETAILS')target.querySelector('summary')?.focus({preventScroll:true});
  }
  function render() {
    const previous=route;
    route=parseRoute();
    const q=route.type==='library'?route.query.get('q')||'':'';
    if(search.value!==q)search.value=q;
    const active=route.type==='home'?'home':route.type==='read'?byPath.get(route.path)?.module:route.query.get('module');
    document.querySelectorAll('[data-nav]').forEach(a=>{const on=a.dataset.nav===active;a.classList.toggle('active',on);if(on)a.setAttribute('aria-current','page');else a.removeAttribute('aria-current');});
    document.getElementById('topbar-context').textContent=route.type==='read'?'专注阅读':route.type==='library'?'资料检索':'学习空间';
    if(route.type==='read'&&currentPath===route.path&&previous?.type==='read') { closeNav();scrollToSection(route.query.get('section')||'');return; }
    currentPath='';observer?.disconnect();
    if(route.type==='home')renderHome();else if(route.type==='library')renderLibrary();else renderReader();
    document.title=(route.type==='read'?(byPath.get(route.path)?.title||'页面未找到'):route.type==='library'?(route.query.get('q')?'搜索结果':data.modules.find(m=>m.id===route.query.get('module'))?.label||'学习资料'):'嵌入式开发学习手册')+' · 知行';
    closeNav();
    if(route.type==='read')requestAnimationFrame(()=>scrollToSection(route.query.get('section')||''));
    else if(!(document.activeElement===search))window.scrollTo(0,0);
  }
  function openNav() { sidebar.inert=false;sidebar.removeAttribute('aria-hidden');sidebar.classList.add('open');backdrop.hidden=false;navOpen.setAttribute('aria-expanded','true');document.querySelector('.workspace').inert=true;document.getElementById('nav-close').focus(); }
  function closeNav() {
    const wasOpen=sidebar.classList.contains('open'),mobile=innerWidth<=960;
    const hadSidebarFocus=sidebar.contains(document.activeElement),focusedClose=document.activeElement===document.getElementById('nav-close');
    sidebar.classList.remove('open');backdrop.hidden=true;navOpen.setAttribute('aria-expanded','false');
    sidebar.inert=mobile;if(mobile)sidebar.setAttribute('aria-hidden','true');else sidebar.removeAttribute('aria-hidden');
    document.querySelector('.workspace').inert=false;
    if(mobile&&(wasOpen||hadSidebarFocus))navOpen.focus();
    else if(wasOpen&&focusedClose)sidebar.querySelector('.brand')?.focus();
  }
  async function copyCode(button) {
    const code=button.closest('.code-block').querySelector('code').textContent;
    try {
      if(navigator.clipboard?.writeText)await navigator.clipboard.writeText(code);
      else {const textarea=document.createElement('textarea');textarea.value=code;textarea.className='sr-only';document.body.append(textarea);textarea.select();const copied=document.execCommand('copy');textarea.remove();if(!copied)throw Error('copy unavailable');}
      button.textContent='已复制';toast('代码已复制');setTimeout(()=>{if(button.isConnected)button.textContent='复制代码';},2000);
    }catch{toast('当前浏览器不允许自动复制，请选中代码后手动复制。');}
  }
  search.addEventListener('input',()=>{
    clearTimeout(searchTimer);
    searchTimer=setTimeout(()=>{const old=route.type==='library'?Object.fromEntries(route.query):{};delete old.page;navigate(libraryHash({...old,q:search.value.trim()}),true);},170);
  });
  search.addEventListener('keydown',e=>{if(e.key==='Enter'){clearTimeout(searchTimer);const old=route.type==='library'?Object.fromEntries(route.query):{};delete old.page;navigate(libraryHash({...old,q:search.value.trim()}),true);main.focus();}});
  main.addEventListener('change',e=>{
    const key=e.target.dataset.filter;if(!key)return;
    const p=Object.fromEntries(route.query);p[key]=e.target.value;delete p.page;
    if(key==='module')delete p.topic;
    navigate(libraryHash(p));
  });
  main.addEventListener('click',e=>{
    const section=e.target.closest('a[data-section]');
    // Same-hash clicks do not fire hashchange. Keep modified clicks native.
    if(section&&e.button===0&&!e.ctrlKey&&!e.metaKey&&!e.shiftKey&&!e.altKey){e.preventDefault();navigate(section.getAttribute('href'));return;}
    const button=e.target.closest('button');if(!button)return;
    if(button.classList.contains('copy-code')){copyCode(button);return;}
    if(button.dataset.page){navigate(libraryHash({...Object.fromEntries(route.query),page:button.dataset.page}));return;}
    if(['reset-filters','empty-reset'].includes(button.id)){navigate(libraryHash(route.query.get('view')&&button.id==='reset-filters'?{view:route.query.get('view')}:{}));return;}
    if(button.id==='save-doc'||button.id==='done-doc'){
      const key=button.id==='save-doc'?'saved':'done';const included=state[key].includes(currentPath);
      state[key]=included?state[key].filter(p=>p!==currentPath):[...state[key],currentPath];persist();
      button.setAttribute('aria-pressed',String(!included));button.textContent=key==='saved'?included?'☆ 收藏章节':'★ 已收藏':included?'标记已读':'✓ 已读';
      toast(key==='saved'?(included?'已移除收藏':'已收藏，本机保存'):(included?'已取消已读标记':'已标记已读，本机保存'));
      if(!storageOK)toast('浏览器未允许本地存储，当前记录只在本次打开期间保留。');return;
    }
    if(button.id==='toggle-answers'){
      const collapsed=button.getAttribute('aria-pressed')!=='true';
      main.querySelectorAll('.reader-content details').forEach(d=>d.open=!collapsed);
      button.setAttribute('aria-pressed',String(collapsed));button.textContent=collapsed?'展开知识 / 解答':'收起知识 / 解答';return;
    }
    if(button.id==='font-plus'||button.id==='font-minus'){
      state.fontSize=Math.min(22,Math.max(15,state.fontSize+(button.id==='font-plus'?1:-1)));applyPreferences();persist();
      document.getElementById('font-minus').disabled=state.fontSize<=15;document.getElementById('font-plus').disabled=state.fontSize>=22;
      toast(`正文字号 ${state.fontSize}px`);return;
    }
    if(button.id==='line-height'){const choices=[1.8,1.95,2.15];state.lineHeight=choices[(choices.indexOf(state.lineHeight)+1)%3];applyPreferences();persist();toast(`正文行距 ${state.lineHeight}`);}
  });
  theme.addEventListener('click',()=>{state.theme=state.theme==='light'?'dark':'light';applyPreferences();persist();});
  navOpen.addEventListener('click',openNav);document.getElementById('nav-close').addEventListener('click',closeNav);backdrop.addEventListener('click',closeNav);
  document.addEventListener('keydown',e=>{
    if(e.key==='Escape'&&sidebar.classList.contains('open')){closeNav();return;}
    const editing=/INPUT|TEXTAREA|SELECT/.test(document.activeElement?.tagName||'')||document.activeElement?.isContentEditable;
    if((e.key==='/'&&!editing)||((e.ctrlKey||e.metaKey)&&e.key.toLowerCase()==='k')){e.preventDefault();search.focus();search.select();}
    if(e.key==='Tab'&&sidebar.classList.contains('open')){
      const focusable=[...sidebar.querySelectorAll('a,button')].filter(el=>getComputedStyle(el).display!=='none');const first=focusable[0],last=focusable.at(-1);
      if(e.shiftKey&&document.activeElement===first){e.preventDefault();last.focus();}else if(!e.shiftKey&&document.activeElement===last){e.preventDefault();first.focus();}
    }
  });
  window.addEventListener('hashchange',()=>{clearTimeout(searchTimer);render();});
  window.addEventListener('resize',()=>{if(innerWidth>960||!sidebar.classList.contains('open'))closeNav();});
  buildNav();persist();applyPreferences();render();
})();
