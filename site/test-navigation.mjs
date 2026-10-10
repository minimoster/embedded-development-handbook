/* Dependency-free event/DOM simulation. Not a browser or physical-mobile test. */
import fs from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';

const app = fs.readFileSync(new URL('./assets/app.js', import.meta.url), 'utf8');
const readHash = id => `#/read/docs%2F10-c-cpp%2Fc01.md?section=${id}`;

function fixture(width = 320, initialHash = readHash('直接回答'), source = app) {
  const tasks = [], windowListeners = new Map(), elements = new Map();
  const document = {activeElement: null, title: '', documentElement: {dataset: {}, style: {setProperty() {}}}};
  const scrolls = [];
  class Element {
    constructor(id = '', tagName = 'DIV', classes = '') {
      this.id = id; this.tagName = tagName; this.className = classes;
      this.dataset = {}; this.attributes = new Map(); this.listeners = new Map();
      this.parentElement = null; this.open = false; this.inert = false; this.hidden = false; this.value = '';
      const tokens = new Set(classes.split(' ').filter(Boolean));
      this.classList = {
        contains: name => tokens.has(name), add: name => tokens.add(name), remove: name => tokens.delete(name),
        toggle(name, on = !tokens.has(name)) {if (on) tokens.add(name); else tokens.delete(name); return on;}
      };
      if (id) elements.set(id, this);
    }
    setAttribute(name, value) {this.attributes.set(name, String(value));}
    getAttribute(name) {return this.attributes.get(name) ?? null;}
    removeAttribute(name) {this.attributes.delete(name);}
    addEventListener(name, callback) {this.listeners.set(name, callback);}
    contains(target) {for (let node = target; node; node = node.parentElement) if (node === this) return true; return false;}
    matches(selector) {
      return selector.split(',').some(s => s.startsWith('.') ? this.classList.contains(s.slice(1)) :
        s === 'a[data-section]' ? this.tagName === 'A' && this.dataset.section !== undefined : this.tagName === s.toUpperCase());
    }
    closest(selector) {for (let node = this; node; node = node.parentElement) if (node.matches(selector)) return node; return null;}
    focus() {for (let node = this; node; node = node.parentElement) if (node.inert) return; document.activeElement = this;}
    scrollIntoView() {scrolls.push({id: this.id, tocOpen: toc.open});}
    querySelector(selector) {return this.querySelectorAll(selector)[0] ?? null;}
    querySelectorAll(selector) {
      if (this === sidebar) return selector === '.brand' ? [brand] : selector === 'a,button' ? [brand, navClose, lastLink] : [];
      if (this === main && this.className === 'reader-main') {
        if (selector === '.mobile-toc') return [toc];
        if (selector === '[data-section]') return links;
      }
      return [];
    }
  }
  const main = new Element('main'), sidebar = new Element('sidebar'), backdrop = new Element('sidebar-backdrop');
  const navOpen = new Element('nav-open', 'BUTTON'), navClose = new Element('nav-close', 'BUTTON');
  const workspace = new Element('', 'DIV', 'workspace'), brand = new Element('', 'A', 'brand');
  const lastLink = new Element('', 'A'), skip = new Element('', 'A', 'skip-link');
  main.parentElement = workspace; brand.parentElement = navClose.parentElement = lastLink.parentElement = sidebar;
  for (const id of ['global-search', 'theme-toggle', 'saved-count', 'done-count', 'module-nav', 'topbar-context', 'toast']) new Element(id);
  const toc = new Element('', 'DETAILS', 'mobile-toc'); toc.parentElement = main;
  const article = new Element('', 'ARTICLE'); article.parentElement = main;
  const headings = ['直接回答', '原理拆解'].map(id => {const h = new Element(id, 'H2'); h.parentElement = article; return h;});
  const links = headings.map(h => {
    const a = new Element('', 'A'); a.parentElement = toc; a.dataset.section = h.id; a.setAttribute('href', readHash(h.id)); return a;
  });
  document.getElementById = id => elements.get(id);
  document.querySelector = selector => selector === '.workspace' ? workspace : selector === '.skip-link' ? skip : null;
  document.querySelectorAll = () => [];
  document.addEventListener = (name, callback) => windowListeners.set(`document:${name}`, callback);
  const historyHashes = [initialHash]; let position = 0;
  const location = {};
  Object.defineProperty(location, 'hash', {
    get: () => historyHashes[position],
    set(hash) {if (hash === historyHashes[position]) return; historyHashes.splice(position + 1); historyHashes.push(hash); position++; tasks.push(() => windowListeners.get('hashchange')());}
  });
  const history = {
    get length() {return historyHashes.length;},
    replaceState(_state, _unused, hash) {historyHashes[position] = hash;},
    back() {if (position) {position--; tasks.push(() => windowListeners.get('hashchange')());}},
    forward() {if (position + 1 < historyHashes.length) {position++; tasks.push(() => windowListeners.get('hashchange')());}}
  };
  const doc = {path: 'docs/10-c-cpp/c01.md', title: 'Array and pointer contract', module: 'reference', moduleName: 'Reference',
    headings: headings.map(h => ({id: h.id, text: h.id, level: 2})), priorities: [], source: '', sourceUrl: '', html: '', kind: 'chapter'};
  const context = {
    document, location, history, innerWidth: width, URLSearchParams, navigator: {},
    localStorage: {getItem() {return null;}, setItem() {}},
    requestAnimationFrame: callback => tasks.push(callback), setTimeout() {return 1;}, clearTimeout() {},
    getComputedStyle: element => ({display: element === navClose && context.innerWidth > 960 ? 'none' : 'block'}),
    STUDY_DATA: {docs: [doc], modules: [{id: 'reference', number: '13', label: 'Reference', count: 1}]},
    STUDY_SEARCH: {normalize: value => value, create: () => ({query: () => ({results: [], terms: []})})},
    matchMedia: () => ({matches: false}), addEventListener: (name, callback) => windowListeners.set(name, callback),
    scrollTo: () => scrolls.push({id: 'top', tocOpen: toc.open})
  };
  context.window = context;
  vm.runInNewContext(source, context);
  const flush = () => {while (tasks.length) tasks.shift()();}; flush();
  const click = (target, overrides = {}) => {
    const event = {target, button: 0, ctrlKey: false, metaKey: false, shiftKey: false, altKey: false,
      defaultPrevented: false, preventDefault() {this.defaultPrevented = true;}, ...overrides};
    main.listeners.get('click')(event); return event;
  };
  return {main, sidebar, backdrop, workspace, navOpen, navClose, brand, lastLink, toc, headings, links, document, scrolls, history, location,
    click, flush, resize(next) {context.innerWidth = next; windowListeners.get('resize')();},
    openNav() {navOpen.listeners.get('click')();}, closeNav() {navClose.listeners.get('click')();},
    key(key, overrides = {}) {windowListeners.get('document:keydown')({key, preventDefault() {}, ...overrides});}};
}

const reader = fixture();
for (let repeat = 0; repeat < 3; repeat++) {
  reader.toc.open = true;
  const before = reader.scrolls.length;
  assert(reader.click(reader.links[0]).defaultPrevented, 'Same-section click must go through navigate');
  assert.equal(reader.toc.open, false, 'Mobile TOC closes even when hash is already current');
  assert.equal(reader.scrolls.length, before + 1);
  assert.deepEqual(reader.scrolls.at(-1), {id: '直接回答', tocOpen: false}, 'TOC collapses before scrolling');
  assert.equal(reader.document.activeElement, reader.headings[0]);
  assert.equal(reader.history.length, 1, 'Repeated current-anchor clicks do not add history entries');
}
reader.toc.open = true;
assert(reader.click(reader.links[1]).defaultPrevented);
reader.flush();
assert.equal(reader.location.hash, readHash('原理拆解'));
assert.equal(reader.history.length, 2, 'Changed anchors retain native hash history');
assert.equal(reader.document.activeElement, reader.headings[1]);
reader.toc.open = true; reader.history.back(); reader.flush();
assert.equal(reader.location.hash, readHash('直接回答')); assert.equal(reader.document.activeElement, reader.headings[0]); assert(!reader.toc.open);
reader.history.forward(); reader.flush();
assert.equal(reader.location.hash, readHash('原理拆解')); assert.equal(reader.document.activeElement, reader.headings[1]);
for (const overrides of [{ctrlKey: true}, {metaKey: true}, {shiftKey: true}, {altKey: true}, {button: 1}, {button: 2}]) {
  reader.toc.open = true; const before = reader.scrolls.length;
  assert(!reader.click(reader.links[0], overrides).defaultPrevented, 'Modified/non-primary click stays native');
  assert(reader.toc.open); assert.equal(reader.scrolls.length, before); assert.equal(reader.history.length, 2);
}
reader.openNav(); reader.click(reader.links[1]);
assert.equal(reader.document.activeElement, reader.headings[1], 'Drawer closes before heading focus is restored');

function assertClosed(f, mobile) {
  assert(!f.sidebar.classList.contains('open')); assert.equal(f.sidebar.inert, mobile);
  assert.equal(f.sidebar.getAttribute('aria-hidden'), mobile ? 'true' : null);
  assert(f.backdrop.hidden); assert(!f.workspace.inert); assert.equal(f.navOpen.getAttribute('aria-expanded'), 'false');
}
const nav = fixture(1440, '#/'); assertClosed(nav, false);
nav.brand.focus(); nav.resize(390); assertClosed(nav, true);
assert.equal(nav.document.activeElement, nav.navOpen, 'Desktop sidebar focus moves to the visible mobile menu control');
nav.brand.focus(); assert.equal(nav.document.activeElement, nav.navOpen, 'Closed inert sidebar cannot receive focus');
nav.resize(960); assertClosed(nav, true); nav.resize(961); assertClosed(nav, false);
nav.resize(390); nav.openNav();
for (const width of [320, 960]) {
  nav.resize(width); assert(nav.sidebar.classList.contains('open')); assert(!nav.sidebar.inert);
  assert.equal(nav.sidebar.getAttribute('aria-hidden'), null); assert(!nav.backdrop.hidden); assert(nav.workspace.inert);
  assert.equal(nav.navOpen.getAttribute('aria-expanded'), 'true'); assert.equal(nav.document.activeElement, nav.navClose);
}
nav.brand.focus(); nav.key('Tab', {shiftKey: true}); assert.equal(nav.document.activeElement, nav.lastLink);
nav.key('Tab'); assert.equal(nav.document.activeElement, nav.brand);
nav.key('Escape'); assertClosed(nav, true); assert.equal(nav.document.activeElement, nav.navOpen);
nav.openNav(); nav.resize(961); assertClosed(nav, false);
assert.equal(nav.document.activeElement, nav.brand, 'Entering desktop leaves focus on a visible sidebar link');
nav.resize(960); assertClosed(nav, true); assert.equal(nav.document.activeElement, nav.navOpen);
nav.openNav(); nav.closeNav(); assertClosed(nav, true); assert.equal(nav.document.activeElement, nav.navOpen);
console.log('Reader navigation simulation PASS: repeated/current and changed anchors, TOC collapse-before-scroll, heading focus, history, modified clicks, responsive 960px boundary, drawer open/close/Esc/focus trap. Not actual browser/mobile QA.');
