#!/usr/bin/env python3
"""Finite protocol teaching tests. No device nodes, kernel, or live network.

Python models use virtual events; the C transaction test injects a mock ioctl.
ASan/UBSan apply only to the compiled C test, not to Python or real devices.
"""
from pathlib import Path
import argparse
import os
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--disable-leak-detection", action="store_true",
                        help="Explicitly skip LeakSanitizer in a ptrace-restricted executor")
    args = parser.parse_args()
    if args.disable_leak_detection and not args.sanitize:
        parser.error("--disable-leak-detection requires --sanitize")
    env = os.environ.copy()
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    if args.disable_leak_detection:
        env["ASAN_OPTIONS"] = env.get("ASAN_OPTIONS", "") + ":detect_leaks=0"
    env["UBSAN_OPTIONS"] = env.get("UBSAN_OPTIONS", "") + ":halt_on_error=1"
    for script in ("host_i2c_timing_recovery_test.py", "host_tcp_reno_rto_test.py"):
        subprocess.run([sys.executable, str(ROOT / "tests" / script)],
                       check=True, timeout=30, env=env, cwd=ROOT)
    flags = ["-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Wconversion", "-Wsign-conversion"]
    if args.sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                  "-fno-omit-frame-pointer"]
    with tempfile.TemporaryDirectory(prefix="protocol-depth-") as directory:
        executable = str(Path(directory) / "spidev-mock-test")
        subprocess.run(["gcc", *flags, str(ROOT / "examples/c/spidev_transaction.c"),
                        str(ROOT / "tests/host_spidev_transaction_test.c"),
                        "-o", executable], check=True, timeout=60, env=env, cwd=ROOT)
        subprocess.run([executable], check=True, timeout=30, env=env, cwd=ROOT)
    print("Protocol-depth PASS: finite I2C/TCP models and mock SPI syscall only.")
    print("Not run: board electrical/timing, Linux SPI kernel path, real TCP network.")
    if args.sanitize:
        print("C mock test ASan/UBSan passed; Python models are outside sanitizer scope.")
    if args.disable_leak_detection:
        print("LeakSanitizer disabled explicitly: this is not a leak-check pass.")


if __name__ == "__main__":
    main()
