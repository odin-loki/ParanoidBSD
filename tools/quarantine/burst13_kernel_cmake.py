#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Patch pbsd/CMakeLists.txt for burst 13 kernel modules."""
from pathlib import Path

path = Path(__file__).resolve().parent.parent / "pbsd" / "CMakeLists.txt"
cmake = path.read_text(encoding="utf-8")

new_modules = [
    ("pbsd_kernel_rangeset", "kernel/kern/pbsd.kernel.rangeset.cppm", "pbsd_core"),
    ("pbsd_kernel_filter", "kernel/kern/pbsd.kernel.filter.cppm", "pbsd_core"),
    ("pbsd_kernel_stack", "kernel/kern/pbsd.kernel.stack.cppm", "pbsd_core"),
    ("pbsd_kernel_pidctrl", "kernel/kern/pbsd.kernel.pidctrl.cppm", "pbsd_core"),
    ("pbsd_kernel_prng", "kernel/kern/pbsd.kernel.prng.cppm", "pbsd_core"),
    ("pbsd_kernel_subr_lock", "kernel/kern/pbsd.kernel.subr_lock.cppm", "pbsd_core"),
    ("pbsd_kernel_scanf", "kernel/kern/pbsd.kernel.scanf.cppm", "pbsd_core"),
    ("pbsd_kernel_gtaskqueue", "kernel/kern/pbsd.kernel.gtaskqueue.cppm", "pbsd_core pbsd_kernel_taskqueue"),
    ("pbsd_kernel_physmem", "kernel/kern/pbsd.kernel.physmem.cppm", "pbsd_core"),
    ("pbsd_kernel_vmem", "kernel/kern/pbsd.kernel.vmem.cppm", "pbsd_core"),
    ("pbsd_kernel_stats", "kernel/kern/pbsd.kernel.stats.cppm", "pbsd_core"),
    ("pbsd_kernel_clock", "kernel/kern/pbsd.kernel.clock.cppm", "pbsd_core pbsd_kernel_timekeeping"),
    ("pbsd_kernel_boot", "kernel/kern/pbsd.kernel.boot.cppm", "pbsd_core"),
    ("pbsd_kernel_subr_rangeset", "kernel/kern/pbsd.kernel.subr_rangeset.cppm", "pbsd_core pbsd_kernel_pctrie"),
    ("pbsd_kernel_subr_filter", "kernel/kern/pbsd.kernel.subr_filter.cppm", "pbsd_core"),
    ("pbsd_kernel_subr_stack", "kernel/kern/pbsd.kernel.subr_stack.cppm", "pbsd_core"),
    ("pbsd_kernel_subr_pidctrl", "kernel/kern/pbsd.kernel.subr_pidctrl.cppm", "pbsd_core"),
    ("pbsd_kernel_subr_prng", "kernel/kern/pbsd.kernel.subr_prng.cppm", "pbsd_core pbsd_kernel_random"),
]

blocks = []
for target, src, deps in new_modules:
    if f"if(NOT TARGET {target})" in cmake:
        continue
    blocks.append(
        f"if(NOT TARGET {target})\n"
        f"add_library({target})\n"
        f"target_sources({target} PUBLIC FILE_SET CXX_MODULES FILES\n"
        f"    {src})\n"
        f"target_link_libraries({target} PUBLIC {deps})\n"
        f"target_compile_options({target} PUBLIC ${{PBSD_FS_CXX}})\n"
        f"endif()\n"
    )

marker = "add_library(pbsd_kernel INTERFACE)"
if blocks:
    insert = "\n".join(blocks) + "\n\n"
    if marker not in cmake:
        raise SystemExit(f"marker not found: {marker}")
    cmake = cmake.replace(marker, insert + marker, 1)

# Backfill burst 12 modules missing from INTERFACE aggregate.
backfill = [
    "pbsd_kernel_hash", "pbsd_kernel_prf", "pbsd_kernel_unit", "pbsd_kernel_fattime",
    "pbsd_kernel_msgbuf", "pbsd_kernel_firmware", "pbsd_kernel_pctrie", "pbsd_kernel_sglist",
    "pbsd_kernel_sfbuf", "pbsd_kernel_rman", "pbsd_kernel_blist", "pbsd_kernel_hints",
    "pbsd_kernel_kern_log", "pbsd_kernel_vfs_mount", "pbsd_kernel_vfs_subr",
    "pbsd_kernel_vfs_lookup", "pbsd_kernel_vfs_default", "pbsd_kernel_vfs_syscalls",
    "pbsd_kernel_vfs_vnops", "pbsd_kernel_vfs_hash", "pbsd_kernel_vfs_cache",
    "pbsd_kernel_synch", "pbsd_kernel_kern_mib", "pbsd_kernel_kern_malloc",
    "pbsd_kernel_kern_proc", "pbsd_kernel_kern_sig", "pbsd_kernel_kern_fork",
    "pbsd_kernel_kern_exit",
]
new_targets = [t for t, _, _ in new_modules]

old_tail = "    pbsd_kernel_subr_unit pbsd_kernel_subr_hash\n    pbsd_kernel_subr_msgbuf pbsd_kernel_subr_prf)"
additions = [t for t in backfill + new_targets if t not in cmake.split("target_link_libraries(pbsd_kernel INTERFACE")[1].split("target_compile_options(pbsd_kernel INTERFACE")[0]]
if additions:
    extra = "\n    ".join(additions)
    new_tail = f"    pbsd_kernel_subr_unit pbsd_kernel_subr_hash\n    pbsd_kernel_subr_msgbuf pbsd_kernel_subr_prf\n    {extra})"
    if old_tail not in cmake:
        raise SystemExit("INTERFACE tail marker not found")
    cmake = cmake.replace(old_tail, new_tail, 1)

path.write_text(cmake, encoding="utf-8")
print(f"Added {len(blocks)} target blocks")
print(f"Extended INTERFACE with {len(additions)} targets")
