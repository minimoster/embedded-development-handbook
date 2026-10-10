#!/usr/bin/env python3
"""Optional actual-browser QA. Requires Python playwright and installed Chromium.
Tests the cloud host's isolated browser, not the user's phone or computer.
"""
from pathlib import Path
import json,argparse
from playwright.sync_api import sync_playwright
R=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--output',default='/tmp/embedded-reader-qa');args=a.parse_args();out=Path(args.output);out.mkdir(parents=True,exist_ok=True)
errors=[];external=[]
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True)
    context=browser.new_context(viewport={'width':1440,'height':1000})
    page=context.new_page();page.on('pageerror',lambda e:errors.append(str(e)))
    page.on('request',lambda r:external.append(r.url) if r.url.startswith(('http://','https://')) else None)
    page.goto((R/'site/index.html').as_uri());page.wait_for_selector('.home-title')
    assert page.locator('.hero-stats').inner_text().startswith(str(page.evaluate('STUDY_DATA.coreCount')))
    assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
    page.screenshot(path=str(out/'desktop-home.png'),full_page=True)
    # Full-text, exact IDs, filters and empty state.
    search=page.locator('#global-search');search.fill('DMA cache');page.wait_for_timeout(350)
    assert page.locator('.result-item').count()>0
    search.fill('C04');page.wait_for_timeout(350);page.locator('.result-item').first.click();page.wait_for_selector('.reader-title')
    assert 'volatile' in page.locator('.reader-title').inner_text()
    assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
    page.locator('#save-doc').click();page.locator('#done-doc').click()
    assert page.locator('#save-doc').get_attribute('aria-pressed')=='true'
    page.locator('#toggle-answers').click();assert not page.locator('.answer-section').first.evaluate('(d)=>d.open')
    page.locator('#toggle-answers').click();assert page.locator('.answer-section').first.evaluate('(d)=>d.open')
    page.locator('#font-plus').click();page.locator('#line-height').click()
    page.screenshot(path=str(out/'desktop-reader.png'),full_page=True)
    page.goto((R/'site/index.html').as_uri()+'#/library?module=hot100cpp&difficulty=HARD');page.wait_for_selector('.result-item')
    assert page.locator('.badge.hard').count()>0
    page.goto((R/'site/index.html').as_uri()+'#/library?view=saved');page.wait_for_selector('.result-item');assert page.locator('.result-item').count()==1
    page.goto((R/'site/index.html').as_uri()+'#/library?q=z9x8c7nomatch');page.wait_for_selector('.empty-state')
    # Reload persistence is isolated to the embedded storage namespace.
    page.reload();assert page.evaluate("JSON.parse(localStorage.getItem('zhixing.embedded.reader.v1')).saved.length")==1
    assert page.evaluate("localStorage.getItem('zhixing.reader.v1')===null")
    # Every document route renders without runtime errors or page-wide overflow.
    paths=page.evaluate('STUDY_DATA.docs.map(d=>d.path)')
    from urllib.parse import quote
    for path in paths:
        page.goto((R/'site/index.html').as_uri()+'#/read/'+quote(path,safe=''))
        page.wait_for_selector('.reader-title');assert page.locator('.reader-content').count()==1
        assert page.evaluate('document.documentElement.scrollWidth<=innerWidth'),path
    mobile=browser.new_context(viewport={'width':390,'height':844},is_mobile=True,has_touch=True)
    m=mobile.new_page();m.on('pageerror',lambda e:errors.append(str(e)))
    m.goto((R/'site/index.html').as_uri());m.wait_for_selector('.home-title')
    assert m.locator('#sidebar').evaluate('(x)=>x.inert')
    assert m.evaluate('document.documentElement.scrollWidth<=innerWidth')
    m.screenshot(path=str(out/'mobile-home.png'),full_page=True)
    m.locator('#nav-open').click();assert m.locator('#nav-open').get_attribute('aria-expanded')=='true'
    m.keyboard.press('Escape');assert m.locator('#nav-open').get_attribute('aria-expanded')=='false'
    m.locator('#nav-open').click();m.locator('[data-nav="rtos"]').click();m.wait_for_selector('.result-item')
    assert m.locator('#nav-open').get_attribute('aria-expanded')=='false'
    m.goto((R/'site/index.html').as_uri()+'#/read/docs%2F10-c-cpp%2Fc12.md');m.wait_for_selector('.code-block')
    assert m.evaluate('document.documentElement.scrollWidth<=innerWidth')
    m.locator('.mobile-toc summary').click();assert m.locator('.mobile-toc').evaluate('(x)=>x.open')
    m.locator('.mobile-toc a').filter(has_text='原理拆解').click();m.wait_for_timeout(100)
    assert m.locator('.reader-content h2').filter(has_text='原理拆解').count()==1
    m.screenshot(path=str(out/'mobile-reader.png'),full_page=True)
    assert not errors,errors;assert not external,external
    browser.close()
report={'actual_browser':'cloud host Chromium headless, isolated profiles','desktop':'1440x1000','mobile_emulation':'390x844 touch','document_routes':len(paths),'tests':['offline file://','no external requests','search ids/full-text/filter/empty state','all document routes and overflow','answer collapse','local read state/reload/namespace','font/line spacing','mobile nav Esc/inert/link close','mobile TOC'],'result':'passed','page_errors':errors,'external_requests':external,'limitations':'Mobile emulation is not a physical phone/accessibility audit; no hardware, RTOS or kernel integration.'}
(out/'browser-report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(json.dumps(report,ensure_ascii=False))
