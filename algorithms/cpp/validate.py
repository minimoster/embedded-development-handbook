#!/usr/bin/env python3
"""Reproducible local C++17 curriculum inventory, compilation, runtime checks."""
import argparse, concurrent.futures, datetime, hashlib, json, os, pathlib, re, subprocess, tempfile, time
ROOT = pathlib.Path(__file__).resolve().parents[2]
DOC = ROOT / 'docs/80-hot100-cpp'

def shell(command, **kwargs):
    result = subprocess.run(command, capture_output=True, text=True, **kwargs)
    return {'returncode': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def inventory(manifest):
    errors = []
    official = manifest['problems']; extra = manifest.get('supplementary', [])
    ids = [p['id'] for p in official]
    if len(ids) != 100 or len(set(ids)) != 100: errors.append('Official scope must be 100 unique ids')
    if set(ids) & {p['id'] for p in extra}: errors.append('Supplementary ids overlap official ids')
    if sum(c['count'] for c in manifest['categories']) != 100: errors.append('Category counts do not sum to 100')
    hashes = {}; missing = []; links = 0
    for p in official + extra:
        for key in ['doc_path', 'cpp_path', 'test_path']:
            target = ROOT / p[key]
            if not target.is_file(): missing.append(p[key]); continue
            hashes[p[key]] = digest(target)
        docpath = ROOT / p['doc_path']; sourcepath = ROOT / p['cpp_path']; testpath = ROOT / p['test_path']
        if not all(x.exists() for x in [docpath, sourcepath, testpath]): continue
        text = docpath.read_text()
        blocks = re.findall(r'```cpp\s*\n(.*?)```', text, re.S)
        if sourcepath.read_text().strip() not in [b.strip() for b in blocks]:
            errors.append(f"{p['id']}: embedded complete C++ differs from source")
        if len(text) < 1600: errors.append(f"{p['id']}: lesson unexpectedly short")
        prose = re.sub(r'```[^\n]*\n.*?```', '', text, flags=re.S)
        for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)', prose):
            if '://' in target or target.startswith('#'): continue
            local = target.split('#')[0]
            links += 1
            if not (docpath.parent / local).resolve().exists(): errors.append(f"{p['id']}: broken link {target}")
        if f"p{p['id']:04d}_" not in testpath.read_text(): errors.append(f"{p['id']}: test does not include problem source")
    for path in [DOC/'README.md', DOC/'SOURCES.md', DOC/'PATTERNS.md', DOC/'ERRATA.md', DOC/'TESTING.md']:
        if not path.exists(): missing.append(str(path.relative_to(ROOT))); continue
        for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)',path.read_text()):
            if '://' in target or target.startswith('#'):continue
            links += 1
            if not (path.parent/target.split('#')[0]).resolve().exists():errors.append(f'{path.name}: broken link {target}')
    hashes['algorithms/cpp/support.hpp'] = digest(ROOT/'algorithms/cpp/support.hpp')
    return {'official_count': len(official), 'unique_official_count': len(set(ids)), 'supplementary_count': len(extra),
            'category_count': len(manifest['categories']), 'official_cpp_present': sum((ROOT/p['cpp_path']).exists() for p in official),
            'official_docs_present': sum((ROOT/p['doc_path']).exists() for p in official),
            'official_tests_present': sum((ROOT/p['test_path']).exists() for p in official),
            'local_links_checked': links, 'errors': errors, 'missing': missing, 'source_sha256': hashes}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitizers', action='store_true', help='Address/undefined checks; leak scanner disabled due executor ptrace restriction')
    parser.add_argument('--debug-iterators', action='store_true', help='Add libstdc++ checked iterator/container runtime')
    parser.add_argument('--exhaustive', action='store_true', help='Add independent bounded exhaustive oracles for 17 high-risk problems')
    parser.add_argument('--ids', help='Optional comma-separated subset of problem ids')
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--output', type=pathlib.Path, default=DOC/'validation-result.json')
    args = parser.parse_args(); start=time.monotonic()
    manifest=json.loads((DOC/'manifest.json').read_text())
    catalog=manifest['problems']+manifest.get('supplementary', [])
    selected={int(x) for x in args.ids.split(',')} if args.ids else {p['id'] for p in catalog}
    unknown=selected-{p['id'] for p in catalog}
    if unknown: raise SystemExit(f'Unknown ids: {sorted(unknown)}')
    selected_problems=[p for p in catalog if p['id'] in selected]
    report={'checked_at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'compiler':shell(['g++','--version'])['stdout'].splitlines()[0],
            'scope':'local compilation and tests; no online judge submissions',
            'inventory':inventory(manifest),'selected_ids':sorted(selected), 'modes':{},
            'leak_sanitizer':{'status':'not_run','reason':'Execution environment ptrace blocks LeakSanitizer; ASan uses detect_leaks=0. Do not interpret as leak-free proof.'}}
    mode_flags={'normal':['-O2']}
    if args.sanitizers:mode_flags['address_undefined']=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']
    if args.debug_iterators:mode_flags['checked_iterators']=['-O1','-g','-D_GLIBCXX_DEBUG','-D_GLIBCXX_ASSERTIONS']
    with tempfile.TemporaryDirectory(prefix='hot100-cpp-') as directory:
        build=pathlib.Path(directory)
        def test_one(mode,p,flags):
            output=build/f"{mode}-p{p['id']:04d}"
            command=['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic']+flags+[str(ROOT/p['test_path']),'-o',str(output)]
            try:
                compile_result=shell(command,timeout=180)
                if compile_result['returncode'] != 0:return {'id':p['id'],'compile':compile_result,'passed':False}
                env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0:halt_on_error=1';env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
                runtime=shell([str(output)],timeout=120,env=env)
                return {'id':p['id'],'compile':compile_result,'run':runtime,'passed':runtime['returncode']==0}
            except subprocess.TimeoutExpired as error:return {'id':p['id'],'passed':False,'error':str(error)}
        for mode,flags in mode_flags.items():
            results=[]
            with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,args.jobs)) as executor:
                futures={executor.submit(test_one,mode,p,flags):p for p in selected_problems}
                for future in concurrent.futures.as_completed(futures):
                    result=future.result();results.append(result)
                    print(f"{mode} p{result['id']:04d}: {'PASS' if result['passed'] else 'FAIL'}",flush=True)
                    if not result['passed']:print(json.dumps(result,ensure_ascii=False),flush=True)
            results.sort(key=lambda r:r['id'])
            report['modes'][mode]={'tested_entrypoints':len(results),'passed':all(r['passed'] for r in results),'results':results}
        report['additional_exhaustive'] = {'status': 'not_requested', 'problem_ids': [3,4,15,31,32,39,41,42,45,51,75,76,84,131,239,287,438]}
        if args.exhaustive:
            extra_source = ROOT/'algorithms/cpp/tests/independent_exhaustive.cpp'
            extra_binary = build/'independent-exhaustive'
            compiled = shell(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pedantic','-D_GLIBCXX_DEBUG','-D_GLIBCXX_ASSERTIONS',str(extra_source),'-o',str(extra_binary)], timeout=180)
            runtime = shell([str(extra_binary)], timeout=180) if compiled['returncode']==0 else None
            report['additional_exhaustive'].update(status='run', compile=compiled, run=runtime,
                source_sha256=digest(extra_source), passed=compiled['returncode']==0 and runtime is not None and runtime['returncode']==0)
            print('Additional independent exhaustive: '+str(report['additional_exhaustive']['passed']), flush=True)
        combined=build/'include-all.cpp'
        combined.write_text('\n'.join(f'#include "{(ROOT/p["cpp_path"]).resolve()}"' for p in selected_problems)+'\nint main() {}\n')
        report['all_headers_together']=shell(['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fsyntax-only',str(combined)],timeout=180)
    before = report['inventory']['source_sha256']
    final_inventory = inventory(manifest)
    report['compiled_sources_changed_during_run'] = [name for name, value in before.items()
        if name.endswith(('.hpp', '.cpp')) and final_inventory['source_sha256'].get(name) != value]
    report['inventory'] = final_inventory
    report['post_run_inventory_checked_at_utc'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    full_inventory=not report['inventory']['errors'] and not report['inventory']['missing'] and not report['compiled_sources_changed_during_run']
    report['passed']=full_inventory and all(m['passed'] for m in report['modes'].values()) and report['all_headers_together']['returncode']==0 and report['additional_exhaustive'].get('passed', True)
    report['elapsed_seconds']=round(time.monotonic()-start,3)
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(f'Report: {args.output}; overall passed={report["passed"]}',flush=True)
    return 0 if report['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
