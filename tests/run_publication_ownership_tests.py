#!/usr/bin/env python3
"""Only the new C++17 publication lab. No racy negative runtime fixtures."""
from __future__ import annotations

import argparse
import datetime as dt
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--disable-leak-detection', action='store_true',
                        help='Explicit restricted-host LSan workaround; not an LSan pass')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    if args.disable_leak_detection and not args.sanitize:
        parser.error('--disable-leak-detection requires --sanitize')
    compiler = shutil.which(args.compiler)
    if compiler is None:
        parser.error(f'compiler not found: {args.compiler}')
    if args.report is not None:
        report_path = args.report.resolve()
        if not report_path.is_relative_to(ROOT):
            parser.error('--report must stay inside this lab root')
    else:
        report_path = None
    flags = ['-std=c++17', '-pthread', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-Wconversion', '-Wsign-conversion', '-Wshadow', '-O1', '-g']
    if args.sanitize:
        flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                  '-fno-omit-frame-pointer']
    env = os.environ.copy()
    # Record effective options rather than silently treating disabled LSan as passed.
    if args.disable_leak_detection:
        options = env.get('ASAN_OPTIONS', '')
        env['ASAN_OPTIONS'] = ':'.join(filter(None, [options, 'detect_leaks=0']))
    version = subprocess.run([compiler, '--version'], capture_output=True, text=True,
                             check=True, timeout=10).stdout.splitlines()[0]
    report = {
        'scope': 'C++17 host publication lab only',
        'started_utc': dt.datetime.now(dt.timezone.utc).isoformat(),
        'compiler': version,
        'flags': flags,
        'asan_options': env.get('ASAN_OPTIONS', ''),
        'ubsan_options': env.get('UBSAN_OPTIONS', ''),
        'leak_detection_disabled_explicitly': args.disable_leak_detection,
        'checks': [],
        'never_run': ['ThreadSanitizer', 'Clang compilation', 'formal model checker',
                      'old host/protocol suites', 'MCU cross-compilation/disassembly',
                      'ISR/RTOS/DMA/cache tests', 'WCET/fairness proof',
                      'deliberately racy runtime variants'],
    }

    def run(name: str, command: list[str], timeout: int,
            expect_failure: bool = False, diagnostic: str = '') -> bool:
        print('$', shlex.join(command), flush=True)
        try:
            result = subprocess.run(command, capture_output=True, text=True,
                                    timeout=timeout, env=env, cwd=ROOT)
        except subprocess.TimeoutExpired as error:
            report['checks'].append({'name': name, 'command': command,
                                     'status': 'timeout', 'timeout_seconds': timeout})
            print(f'FAIL: {name}: timeout {error.timeout}s', flush=True)
            return False
        passed = ((result.returncode != 0 and diagnostic in result.stderr)
                  if expect_failure else result.returncode == 0)
        report['checks'].append({
            'name': name, 'command': command,
            'status': 'passed' if passed else 'failed', 'returncode': result.returncode,
            'expected_compile_rejection': expect_failure,
            'stdout': result.stdout, 'stderr': result.stderr,
        })
        if expect_failure:
            print(('PASS' if passed else 'FAIL') + ': ' + name +
                  ' (compile rejection expected; not executed)', flush=True)
            if not passed:
                print(result.stdout + result.stderr, end='', flush=True)
        else:
            print(result.stdout + result.stderr, end='', flush=True)
        return passed

    (ROOT / 'validation').mkdir(exist_ok=True)
    passed = True
    with tempfile.TemporaryDirectory(prefix='publication-build-', dir=ROOT / 'validation') as build:
        build_path = Path(build).relative_to(ROOT)
        for name, source in [('demo', 'examples/cpp/publication_demo.cpp'),
                             ('ownership', 'tests/host_publication_ownership_test.cpp')]:
            executable = build_path / name
            if not run(name + '-strict-compile', [compiler, *flags, source,
                                                  '-o', str(executable)], 60):
                passed = False
                break
            if not run(name + '-runtime', [str(executable)], 35):
                passed = False
                break
        if passed:
            include = '#include "spsc_publication_slot.hpp"\n'
            fixtures = [
                ('reject-throwing-move', include +
                 'struct ThrowMove { ThrowMove() noexcept = default; '
                 'ThrowMove& operator=(ThrowMove&&) noexcept(false) { return *this; } };\n'
                 'publication_lab::SpscPublicationSlot<ThrowMove> slot;\n',
                 'T must have nonthrowing move assignment'),
                ('reject-throwing-visitor', include +
                 'int main() { publication_lab::SpscPublicationSlot<int> slot; '
                 'return slot.try_visit([](const int&) {}) ? 0 : 1; }\n',
                 'visitor(const T&) must be noexcept'),
                ('reject-copy-slot', include +
                 'int main() { publication_lab::SpscPublicationSlot<int> slot; '
                 'auto copy = slot; (void)copy; }\n',
                 'deleted'),
            ]
            for name, content, diagnostic in fixtures:
                source = build_path / (name + '.cpp')
                (ROOT / source).write_text(content, encoding='utf-8')
                command = [compiler, *flags, '-I', 'examples/cpp',
                           '-fsyntax-only', str(source)]
                passed = run(name, command, 60, True, diagnostic) and passed
    observed = {check['name'] for check in report['checks']}
    for name in ['demo-strict-compile', 'demo-runtime', 'ownership-strict-compile',
                 'ownership-runtime', 'reject-throwing-move',
                 'reject-throwing-visitor', 'reject-copy-slot']:
        if name not in observed:
            report['checks'].append({'name': name, 'status': 'not_run',
                                     'reason': 'an earlier required stage failed'})
    report['status'] = 'passed' if passed else 'failed'
    report['finished_utc'] = dt.datetime.now(dt.timezone.utc).isoformat()
    if report_path is not None:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n',
                               encoding='utf-8')
    print('Scope: new bounded host lab only; successful schedules are not a memory-model proof.')
    if args.disable_leak_detection:
        print('LeakSanitizer explicitly disabled: NOT a leak-check pass.')
    print('publication focused runner:', 'PASS' if passed else 'FAIL')
    return 0 if passed else 1


if __name__ == '__main__':
    sys.exit(main())
