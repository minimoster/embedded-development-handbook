#!/usr/bin/env python3
"""Only four new original host labs; no target/kernel/hardware execution."""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--disable-leak-detection', action='store_true')
    args = parser.parse_args()
    if args.disable_leak_detection and not args.sanitize:
        parser.error('--disable-leak-detection requires --sanitize')
    for pattern in ('test_nvic_priority_contract.py', 'test_cortex_m4_fault_snapshot.py',
                    'test_dma_lifecycle_contract.py'):
        command = [sys.executable, '-m', 'unittest', 'discover', '-s', 'tests',
                   '-p', pattern, '-v']
        print('$', ' '.join(command), flush=True)
        # Avoid an empty discovery being mistaken for a pass.
        if not (ROOT / 'tests' / pattern).is_file():
            print(f'FAIL: missing test input {pattern}', flush=True)
            return 1
        result = subprocess.run(command, cwd=ROOT, timeout=60, check=False)
        if result.returncode:
            return result.returncode
    command = [sys.executable, 'tests/run_publication_ownership_tests.py']
    if args.sanitize:
        print('Sanitizers apply to the C++ lab only; Python models are not C sanitizer runs.', flush=True)
        command.append('--sanitize')
    if args.disable_leak_detection:
        print('LeakSanitizer explicitly disabled; this is not a leak-check pass.', flush=True)
        command.append('--disable-leak-detection')
    print('$', ' '.join(command), flush=True)
    result = subprocess.run(command, cwd=ROOT, timeout=300, check=False)
    if result.returncode:
        return result.returncode
    print('Interview-depth host labs PASS: finite models and C++ contracts only.', flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
