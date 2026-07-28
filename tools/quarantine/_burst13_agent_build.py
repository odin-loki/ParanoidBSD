#!/usr/bin/env python3
import os
import shutil
import subprocess

SRC = "/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd"
DEST = "/tmp/pbsd-b13-agent"
BUILD = "/tmp/pbsd-b13-agent-build"
os.environ["PATH"] = "/usr/lib/llvm-18/bin:" + os.environ.get("PATH", "")

os.makedirs(BUILD, exist_ok=True)
if os.path.isdir(DEST):
    shutil.rmtree(DEST)
os.makedirs(DEST, exist_ok=True)

subprocess.check_call([
    "rsync", "-a",
    "--exclude", "ports/",
    "--exclude", "_build*/",
    "--exclude", "build-*/",
    "--exclude", "build/",
    "--exclude", ".git/",
    "--exclude", "*.o",
    "--exclude", "CMakeFiles/",
    f"{SRC}/", f"{DEST}/",
])

os.chdir(BUILD)
subprocess.check_call([
    "cmake", DEST, "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-18",
    "-DCMAKE_C_COMPILER=/usr/bin/clang-18",
])
subprocess.check_call(["ninja", "-j4", "pbsd_si_harness"])
print("BUILD_OK")
