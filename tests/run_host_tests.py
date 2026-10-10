#!/usr/bin/env python3
"""Finite host tests only; does not test RTOS, devices, Linux kernel modules or WCET."""
from pathlib import Path
import json,subprocess,tempfile,os,argparse
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');p.add_argument('--disable-leak-detection',action='store_true',help='Use only when LeakSanitizer cannot run under ptrace');args=p.parse_args()
flags=['-O1','-g','-Wall','-Wextra','-Wpedantic','-Werror']
if args.sanitize:flags+=['-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']
results=[]
env=os.environ.copy()
if args.disable_leak_detection: env['ASAN_OPTIONS']=env.get('ASAN_OPTIONS','')+':detect_leaks=0'
with tempfile.TemporaryDirectory(prefix='embedded-host-') as build:
    cases=[('numeric-parse','gcc',['-std=c11',*flags,str(ROOT/'examples/c/numeric_parse.c'),str(ROOT/'tests/host_numeric_test.c')]),
           ('numeric-differential','gcc',['-std=c11',*flags,'-Wconversion','-Wsign-conversion',str(ROOT/'examples/c/numeric_parse.c'),str(ROOT/'tests/host_numeric_differential_test.c')]),
           ('c-core','gcc',['-std=c11',*flags,str(ROOT/'examples/c/embedded_core.c'),str(ROOT/'tests/host_core_test.c')]),
           ('linux-lifecycle','gcc',['-std=c11',*flags,str(ROOT/'tests/host_linux_lifecycle_test.c')]),
           ('cpp-language','g++',['-std=c++17',*flags,str(ROOT/'tests/host_cpp_language_test.cpp')]),
           ('cpp-core','g++',['-std=c++17','-pthread',*flags,str(ROOT/'tests/host_cpp_test.cpp')])]
    for name,compiler,inputs in cases:
        exe=str(Path(build)/name)
        subprocess.run([compiler,*inputs,'-o',exe],check=True)
        r=subprocess.run([exe],capture_output=True,text=True,timeout=30,env=env)
        if r.stderr:
            print(r.stderr,end='')
        if r.returncode:
            print(r.stdout);r.check_returncode()
        print(r.stdout,end='');results.append({'suite':name,'result':'passed','sanitizers':args.sanitize,'output':r.stdout.strip()})
    # The caller must really be compiled as C, not accepted incidentally by g++.
    caller=str(Path(build)/'c-abi-caller.o');implementation=str(Path(build)/'c-abi-implementation.o')
    subprocess.run(['gcc','-std=c11',*flags,'-c',str(ROOT/'tests/host_c_abi_test.c'),'-o',caller],check=True)
    subprocess.run(['g++','-std=c++17','-fno-exceptions',*flags,'-c',str(ROOT/'examples/cpp/device_boundary.cpp'),'-o',implementation],check=True)
    exe=str(Path(build)/'c-abi')
    subprocess.run(['g++',*flags,caller,implementation,'-o',exe],check=True)
    r=subprocess.run([exe],capture_output=True,text=True,timeout=30,env=env)
    if r.stderr:print(r.stderr,end='')
    if r.returncode:print(r.stdout);r.check_returncode()
    print(r.stdout,end='');results.append({'suite':'c-abi','result':'passed','sanitizers':args.sanitize,'output':r.stdout.strip()})
print('Scope: host only; no hardware/RTOS/kernel integration/real-time guarantee.')
if args.disable_leak_detection: print('LeakSanitizer disabled explicitly: not a leak-check pass.')
