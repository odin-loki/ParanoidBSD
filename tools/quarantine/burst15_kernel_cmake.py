#!/usr/bin/env python3
"""Burst 15 — wire kernel helpers + UDA virtio descriptors into CMake."""
from pathlib import Path

path = Path(__file__).resolve().parent.parent / "pbsd" / "CMakeLists.txt"
cmake = path.read_text(encoding="utf-8")

kernel_modules = [
    ("pbsd_kernel_subr_sbuf", "kernel/kern/pbsd.kernel.subr_sbuf.cppm", "pbsd_core pbsd_kernel_sbuf"),
    ("pbsd_kernel_subr_panic", "kernel/kern/pbsd.kernel.subr_panic.cppm", "pbsd_core pbsd_kernel_panic"),
    ("pbsd_kernel_subr_uuid", "kernel/kern/pbsd.kernel.subr_uuid.cppm", "pbsd_core pbsd_kernel_uuid"),
    ("pbsd_kernel_subr_linker", "kernel/kern/pbsd.kernel.subr_linker.cppm", "pbsd_core pbsd_kernel_linker"),
    ("pbsd_kernel_subr_vmem", "kernel/kern/pbsd.kernel.subr_vmem.cppm", "pbsd_core pbsd_kernel_vmem"),
    ("pbsd_kernel_subr_bus", "kernel/kern/pbsd.kernel.subr_bus.cppm", "pbsd_core"),
    ("pbsd_kernel_device", "kernel/kern/pbsd.kernel.device.cppm", "pbsd_core"),
    ("pbsd_kernel_vm_meter", "kernel/vm/pbsd.kernel.vm_meter.cppm", "pbsd_core"),
    ("pbsd_kernel_vm_kern", "kernel/vm/pbsd.kernel.vm_kern.cppm", "pbsd_core pbsd_kernel_vm"),
    ("pbsd_kernel_vm_reserv", "kernel/vm/pbsd.kernel.vm_reserv.cppm", "pbsd_core"),
    ("pbsd_kernel_subr_ether", "kernel/net/pbsd.kernel.subr_ether.cppm", "pbsd_core pbsd_kernel_ether"),
    ("pbsd_kernel_mbuf_helpers", "kernel/mbuf/pbsd.kernel.mbuf_helpers.cppm", "pbsd_core pbsd_kernel_mbuf"),
    ("pbsd_kernel_ifnet_helpers", "kernel/ifnet/pbsd.kernel.ifnet_helpers.cppm", "pbsd_core pbsd_kernel_ifnet"),
    ("pbsd_kernel_rtentry_helpers", "kernel/net/pbsd.kernel.rtentry_helpers.cppm", "pbsd_core pbsd_kernel_rtentry"),
]

uda_modules = [
    ("pbsd_uda_virtio_vsock", "uda/descriptors/virtio_vsock.cppm", "pbsd_uda_virtio_common"),
    ("pbsd_uda_virtio_fs", "uda/descriptors/virtio_fs.cppm", "pbsd_uda_virtio_common"),
    ("pbsd_uda_virtio_crypto", "uda/descriptors/virtio_crypto.cppm", "pbsd_uda_virtio_common"),
    ("pbsd_uda_pci_hostbridge", "uda/descriptors/pci_hostbridge.cppm", "pbsd_uda_schema pbsd_uda_interp"),
]

blocks = []
for target, src, deps in kernel_modules + uda_modules:
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

if blocks:
    k_marker = "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)"
    u_marker = "if(NOT TARGET pbsd_uda)\nadd_library(pbsd_uda INTERFACE)"
    insert = "\n".join(blocks[: len(kernel_modules)]) + "\n\n"
    if k_marker not in cmake:
        raise SystemExit("kernel marker not found")
    cmake = cmake.replace(k_marker, insert + k_marker, 1)
    u_blocks = "\n".join(blocks[len(kernel_modules) :]) + "\n\n"
    if u_marker not in cmake:
        raise SystemExit("uda marker not found")
    cmake = cmake.replace(u_marker, u_blocks + u_marker, 1)

new_kernel = [t for t, _, _ in kernel_modules if t not in cmake]
new_uda = [t for t, _, _ in uda_modules if t not in cmake]

k_part = cmake.split("target_link_libraries(pbsd_kernel INTERFACE")[1].split("target_compile_options(pbsd_kernel INTERFACE")[0]
to_add_k = [t for t, _, _ in kernel_modules if t not in k_part]
if to_add_k:
    old = "    pbsd_kernel_ether pbsd_kernel_altq pbsd_kernel_subr_counter)"
    extra = "\n    ".join(to_add_k)
    cmake = cmake.replace(old, f"    pbsd_kernel_ether pbsd_kernel_altq pbsd_kernel_subr_counter\n    {extra})", 1)

u_part = cmake.split("target_link_libraries(pbsd_uda INTERFACE")[1].split(")\nendif()")[0]
to_add_u = [t for t, _, _ in uda_modules if t not in u_part]
if to_add_u:
    old = "    pbsd_uda_mem pbsd_uda_zero pbsd_uda_tun pbsd_uda_speaker\n    pbsd_handles)"
    extra = "\n    ".join(to_add_u)
    cmake = cmake.replace(old, f"    pbsd_uda_mem pbsd_uda_zero pbsd_uda_tun pbsd_uda_speaker\n    {extra}\n    pbsd_handles)", 1)

path.write_text(cmake, encoding="utf-8")
print(f"Added {len(blocks)} target blocks")
print("Kernel:", ", ".join(to_add_k))
print("UDA:", ", ".join(to_add_u))
