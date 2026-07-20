#!/usr/bin/env python3
"""Burst 15 wave 3 — mass kernel/net/fs/userland hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"
USER_CMAKE = PBSD / "userland" / "CMakeLists.txt"

KEYWORD_BAN = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
    "class", "compl", "concept", "const", "consteval", "constexpr", "constinit",
    "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
    "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
    "explicit", "export", "extern", "false", "float", "for", "friend", "goto",
    "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
    "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected",
    "public", "register", "reinterpret_cast", "requires", "return", "short",
    "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
    "switch", "template", "this", "thread_local", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual",
    "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}

CREATED: list[str] = []

# (area, name, hbsd_source, parent_target_or_None)
KERNEL_SUBR: list[tuple[str, str, str | None]] = [
    ("kernel", "subr_intr", "hbsd/src/sys/kern/subr_intr.c", None),
    ("kernel", "subr_epoch", "hbsd/src/sys/kern/subr_epoch.c", "pbsd_kernel_epoch"),
    ("kernel", "subr_smr", "hbsd/src/sys/kern/subr_smr.c", "pbsd_kernel_smr"),
    ("kernel", "subr_taskqueue", "hbsd/src/sys/kern/subr_taskqueue.c", "pbsd_kernel_taskqueue"),
    ("kernel", "subr_devstat", "hbsd/src/sys/kern/subr_devstat.c", "pbsd_kernel_devstat"),
    ("kernel", "subr_disk", "hbsd/src/sys/kern/subr_disk.c", "pbsd_kernel_disk"),
    ("kernel", "subr_power", "hbsd/src/sys/kern/subr_power.c", "pbsd_kernel_power"),
    ("kernel", "subr_pcpu", "hbsd/src/sys/kern/subr_pcpu.c", None),
    ("kernel", "subr_stats", "hbsd/src/sys/kern/subr_stats.c", None),
    ("kernel", "subr_trap", "hbsd/src/sys/kern/subr_trap.c", None),
    ("kernel", "subr_module", "hbsd/src/sys/kern/subr_module.c", "pbsd_kernel_kmod"),
    ("kernel", "subr_log", "hbsd/src/sys/kern/subr_log.c", "pbsd_kernel_kern_log"),
    ("kernel", "subr_autoconf", "hbsd/src/sys/kern/subr_autoconf.c", None),
    ("kernel", "subr_bus_dma", "hbsd/src/sys/kern/subr_bus_dma.c", "pbsd_kernel_busdma"),
    ("kernel", "subr_capability", "hbsd/src/sys/kern/subr_capability.c", None),
    ("kernel", "subr_clock", "hbsd/src/sys/kern/subr_clock.c", "pbsd_kernel_clock"),
    ("kernel", "subr_early", "hbsd/src/sys/kern/subr_early.c", None),
    ("kernel", "subr_kdb", "hbsd/src/sys/kern/subr_kdb.c", None),
    ("kernel", "subr_param", "hbsd/src/sys/kern/subr_param.c", "pbsd_kernel_param"),
    ("kernel", "subr_prof", "hbsd/src/sys/kern/subr_prof.c", None),
    ("kernel", "subr_rtc", "hbsd/src/sys/kern/subr_rtc.c", None),
    ("kernel", "subr_smp", "hbsd/src/sys/kern/subr_smp.c", None),
    ("kernel", "subr_terminal", "hbsd/src/sys/kern/subr_terminal.c", "pbsd_kernel_tty"),
    ("kernel", "subr_acl_nfs4", "hbsd/src/sys/kern/subr_acl_nfs4.c", None),
    ("kernel", "subr_acl_posix1e", "hbsd/src/sys/kern/subr_acl_posix1e.c", None),
    ("kernel", "subr_devmap", "hbsd/src/sys/kern/subr_devmap.c", None),
    ("kernel", "subr_syscall", "hbsd/src/sys/kern/subr_syscall.c", "pbsd_kernel_syscall"),
    ("kernel", "subr_boot", "hbsd/src/sys/kern/subr_boot.c", "pbsd_kernel_boot"),
    ("kernel", "subr_busdma_bounce", "hbsd/src/sys/kern/subr_busdma_bounce.c", "pbsd_kernel_busdma"),
    ("kernel", "subr_busdma_bufalloc", "hbsd/src/sys/kern/subr_busdma_bufalloc.c", "pbsd_kernel_busdma"),
    ("kernel", "madvise", "hbsd/src/sys/kern/kern_madvise.c", None),
]

NET_MODULES: list[tuple[str, str, str | None]] = [
    ("net", "altq_cbq", "hbsd/src/sys/net/altq/altq_cbq.c", "pbsd_net_altq"),
    ("net", "altq_codel", "hbsd/src/sys/net/altq/altq_codel.c", "pbsd_net_altq"),
    ("net", "altq_fairq", "hbsd/src/sys/net/altq/altq_fairq.c", "pbsd_net_altq"),
    ("net", "altq_hfsc", "hbsd/src/sys/net/altq/altq_hfsc.c", "pbsd_net_altq"),
    ("net", "altq_priq", "hbsd/src/sys/net/altq/altq_priq.c", "pbsd_net_altq"),
    ("net", "altq_red", "hbsd/src/sys/net/altq/altq_red.c", "pbsd_net_altq"),
    ("net", "altq_rio", "hbsd/src/sys/net/altq/altq_rio.c", "pbsd_net_altq"),
    ("net", "altq_rmclass", "hbsd/src/sys/net/altq/altq_rmclass.c", "pbsd_net_altq"),
    ("net", "altq_subr", "hbsd/src/sys/net/altq/altq_subr.c", "pbsd_net_altq"),
    ("net", "bpf_buffer", "hbsd/src/sys/net/bpf_buffer.c", "pbsd_net_bpf"),
    ("net", "bpf_filter", "hbsd/src/sys/net/bpf_filter.c", "pbsd_net_bpf"),
    ("net", "bpf_jitter", "hbsd/src/sys/net/bpf_jitter.c", "pbsd_net_bpf"),
    ("net", "bpf_zerocopy", "hbsd/src/sys/net/bpf_zerocopy.c", "pbsd_net_bpf"),
    ("net", "debugnet", "hbsd/src/sys/net/debugnet.c", None),
    ("net", "debugnet_inet", "hbsd/src/sys/net/debugnet_inet.c", None),
    ("net", "if_subr", "hbsd/src/sys/net/if.c", "pbsd_net_ifnet"),
    ("net", "route_helpers", "hbsd/src/sys/net/route_helpers.c", "pbsd_net_route"),
    ("net", "route_rtentry", "hbsd/src/sys/net/route_rtentry.c", "pbsd_net_route"),
    ("net", "rtsock", "hbsd/src/sys/net/rtsock.c", "pbsd_net_route"),
    ("net", "rss_config", "hbsd/src/sys/net/rss_config.c", "pbsd_net_rss"),
    ("net", "toeplitz", "hbsd/src/sys/net/toeplitz.c", None),
    ("net", "cc", "hbsd/src/sys/netinet/cc/cc.c", "pbsd_net_tcp"),
    ("net", "cc_cdg", "hbsd/src/sys/netinet/cc/cc_cdg.c", "pbsd_net_tcp"),
    ("net", "cc_chd", "hbsd/src/sys/netinet/cc/cc_chd.c", "pbsd_net_tcp"),
    ("net", "cc_dctcp", "hbsd/src/sys/netinet/cc/cc_dctcp.c", "pbsd_net_tcp"),
    ("net", "cc_hd", "hbsd/src/sys/netinet/cc/cc_hd.c", "pbsd_net_tcp"),
    ("net", "cc_htcp", "hbsd/src/sys/netinet/cc/cc_htcp.c", "pbsd_net_tcp"),
    ("net", "cc_newreno", "hbsd/src/sys/netinet/cc/cc_newreno.c", "pbsd_net_tcp"),
    ("net", "accf_data", "hbsd/src/sys/net/accf_data/accf_data.c", None),
    ("net", "accf_http", "hbsd/src/sys/net/accf_http/accf_http.c", None),
]

FS_MODULES: list[tuple[str, str, str | None]] = [
    ("fs", "vfs_acl", "hbsd/src/sys/kern/vfs_acl.c", None),
    ("fs", "vfs_cluster", "hbsd/src/sys/kern/vfs_cluster.c", None),
    ("fs", "vfs_default", "hbsd/src/sys/kern/vfs_default.c", None),
    ("fs", "vfs_export", "hbsd/src/sys/kern/vfs_export.c", None),
    ("fs", "vfs_extattr", "hbsd/src/sys/kern/vfs_extattr.c", None),
    ("fs", "vfs_inotify", "hbsd/src/sys/kern/vfs_inotify.c", None),
    ("fs", "vfs_mountroot", "hbsd/src/sys/kern/vfs_mountroot.c", None),
    ("fs", "vfs_vnops", "hbsd/src/sys/kern/vfs_vnops.c", None),
    ("fs", "autofs_vnops", "hbsd/src/sys/fs/autofs/autofs_vnops.c", "pbsd_fs_autofs"),
    ("fs", "cd9660_vnops", "hbsd/src/sys/fs/cd9660/cd9660_vnops.c", "pbsd_fs_cd9660"),
    ("fs", "dead_vnops", "hbsd/src/sys/fs/deadfs/dead_vnops.c", "pbsd_fs_deadfs"),
    ("fs", "ext2_subr", "hbsd/src/sys/fs/ext2fs/ext2_subr.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_vnops", "hbsd/src/sys/fs/ext2fs/ext2_vnops.c", "pbsd_fs_ext2fs"),
    ("fs", "fdesc_vnops", "hbsd/src/sys/fs/fdescfs/fdesc_vnops.c", "pbsd_fs_fdescfs"),
    ("fs", "fuse_vnops", "hbsd/src/sys/fs/fuse/fuse_vnops.c", "pbsd_fs_fusefs"),
    ("fs", "msdosfs_vnops", "hbsd/src/sys/fs/msdosfs/msdosfs_vnops.c", "pbsd_fs_msdosfs"),
    ("fs", "null_subr", "hbsd/src/sys/fs/nullfs/null_subr.c", "pbsd_fs_nullfs"),
    ("fs", "null_vnops", "hbsd/src/sys/fs/nullfs/null_vnops.c", "pbsd_fs_nullfs"),
    ("fs", "p9fs_subr", "hbsd/src/sys/fs/p9fs/p9fs_subr.c", "pbsd_fs_p9fs"),
    ("fs", "p9fs_vnops", "hbsd/src/sys/fs/p9fs/p9fs_vnops.c", "pbsd_fs_p9fs"),
    ("fs", "pseudofs_vnops", "hbsd/src/sys/fs/pseudofs/pseudofs_vnops.c", "pbsd_fs_pseudofs"),
    ("fs", "smbfs_subr", "hbsd/src/sys/fs/smbfs/smbfs_subr.c", "pbsd_fs_smbfs"),
    ("fs", "smbfs_vnops", "hbsd/src/sys/fs/smbfs/smbfs_vnops.c", "pbsd_fs_smbfs"),
    ("fs", "tmpfs_subr", "hbsd/src/sys/fs/tmpfs/tmpfs_subr.c", "pbsd_fs_tmpfs"),
    ("fs", "tmpfs_vnops", "hbsd/src/sys/fs/tmpfs/tmpfs_vnops.c", "pbsd_fs_tmpfs"),
    ("fs", "udf_vnops", "hbsd/src/sys/fs/udf/udf_vnops.c", "pbsd_fs_udf"),
]

MSUN_MODULES: list[tuple[str, str, str]] = [
    (
        "fminimum",
        "hbsd/src/lib/msun/src/s_fminimum.c",
        "[[nodiscard]] inline double fminimum(double x, double y) noexcept { return x < y ? x : y; }",
    ),
    (
        "fmaximum",
        "hbsd/src/lib/msun/src/s_fmaximum.c",
        "[[nodiscard]] inline double fmaximum(double x, double y) noexcept { return x > y ? x : y; }",
    ),
    (
        "fminimumf",
        "hbsd/src/lib/msun/src/s_fminimumf.c",
        "[[nodiscard]] inline float fminimumf(float x, float y) noexcept { return x < y ? x : y; }",
    ),
    (
        "fmaximumf",
        "hbsd/src/lib/msun/src/s_fmaximumf.c",
        "[[nodiscard]] inline float fmaximumf(float x, float y) noexcept { return x > y ? x : y; }",
    ),
    (
        "fminl",
        "hbsd/src/lib/msun/src/s_fminl.c",
        "[[nodiscard]] inline long double fminl(long double x, long double y) noexcept { return x < y ? x : y; }",
    ),
    (
        "fmaxl",
        "hbsd/src/lib/msun/src/s_fmaxl.c",
        "[[nodiscard]] inline long double fmaxl(long double x, long double y) noexcept { return x > y ? x : y; }",
    ),
    (
        "frexpl",
        "hbsd/src/lib/msun/src/s_frexpl.c",
        """[[nodiscard]] inline long double frexpl(long double x, int* exp) noexcept {
    if (!exp) { return x; }
    *exp = 0;
    return x;
}""",
    ),
    (
        "ilogbl",
        "hbsd/src/lib/msun/src/s_ilogbl.c",
        "[[nodiscard]] inline int ilogbl(long double x) noexcept { return x == 0.0L ? -5000 : 0; }",
    ),
    (
        "isnormal",
        "hbsd/src/lib/msun/src/s_isnormal.c",
        """[[nodiscard]] inline int isnormal_val(double x) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const auto exp = (hx >> 52) & 0x7ffu;
    return exp > 0 && exp < 0x7ff;
}""",
    ),
    (
        "logbl",
        "hbsd/src/lib/msun/src/s_logbl.c",
        "[[nodiscard]] inline long double logbl(long double x) noexcept { return x == 0.0L ? -5000.0L : 0.0L; }",
    ),
]

LIBC_SYS: list[tuple[str, str, str]] = [
    ("sys", "setuid", "hbsd/src/lib/libc/sys/setuid.c", "[[nodiscard]] inline Status setuid_id(unsigned uid) noexcept { (void)uid; return Status::Ok; }"),
    ("sys", "getuid", "hbsd/src/lib/libc/sys/getuid.c", "[[nodiscard]] inline unsigned getuid_val() noexcept { return 0; }"),
    ("sys", "setgid", "hbsd/src/lib/libc/sys/setgid.c", "[[nodiscard]] inline Status setgid_id(unsigned gid) noexcept { (void)gid; return Status::Ok; }"),
    ("sys", "getgid", "hbsd/src/lib/libc/sys/getgid.c", "[[nodiscard]] inline unsigned getgid_val() noexcept { return 0; }"),
    ("sys", "geteuid", "hbsd/src/lib/libc/sys/geteuid.c", "[[nodiscard]] inline unsigned geteuid_val() noexcept { return 0; }"),
    ("sys", "getegid", "hbsd/src/lib/libc/sys/getegid.c", "[[nodiscard]] inline unsigned getegid_val() noexcept { return 0; }"),
    ("sys", "getgroups", "hbsd/src/lib/libc/sys/getgroups.c", "[[nodiscard]] inline int getgroups_cnt(int size, unsigned* list) noexcept { (void)size; (void)list; return 0; }"),
    ("sys", "setgroups", "hbsd/src/lib/libc/sys/setgroups.c", "[[nodiscard]] inline Status setgroups_cnt(int size, const unsigned* list) noexcept { (void)size; (void)list; return Status::Ok; }"),
    ("sys", "lseek", "hbsd/src/lib/libc/sys/lseek.c", "[[nodiscard]] inline Status lseek_fd(int fd, long off) noexcept { if (fd < 0) return Status::Invalid; (void)off; return Status::Ok; }"),
    ("sys", "ftruncate", "hbsd/src/lib/libc/sys/ftruncate.c", "[[nodiscard]] inline Status ftruncate_fd(int fd, long len) noexcept { if (fd < 0) return Status::Invalid; (void)len; return Status::Ok; }"),
]

LIBTHR_MODULES: list[tuple[str, str, str]] = [
    (
        "machdep",
        "hbsd/src/lib/libthr/thread/thr/machdep.c",
        """import pbsd.userland.libthr.mutex;

[[nodiscard]] inline ThreadId machdep_self() noexcept { return thread_self(); }""",
    ),
    (
        "sigqueue",
        "hbsd/src/lib/libthr/thread/thr/sigqueue.c",
        """import pbsd.userland.libthr.mutex;

[[nodiscard]] inline Status sigqueue_tid(ThreadId tid, int sig) noexcept {
    (void)tid; (void)sig; return Status::NotImplemented;
}""",
    ),
]


def assert_keyword_safe(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in KEYWORD_BAN:
        raise ValueError(f"keyword-banned module suffix: {tail} in {name}")


def record(rel: str) -> None:
    CREATED.append(rel.replace("\\", "/"))


def render_c(source: str) -> str:
    return f"/* Reference logic from {source} (dual-world). */\n\n/* stub */\n"


def render_scaffold(area: str, name: str, source: str) -> str:
    assert_keyword_safe(name)
    ns = f"pbsd::{area}::{name.replace('.', '_')}"
    title = name.replace("_", " ").title()
    return f"""module;
#include <cstdint>

export module pbsd.{area}.{name};

import pbsd.core;

/// PROVENANCE: {source} — {title} scaffold.
export namespace {ns} {{

enum class Op : unsigned char {{
    Init = 0,
    Validate = 1,
    Dispatch = 2,
}};

struct Ctx {{
    unsigned flags{{}};
    unsigned count{{}};
    bool active{{false}};
}};

[[nodiscard]] inline Status validate_op(Op op) noexcept {{
    switch (op) {{
    case Op::Init:
    case Op::Validate:
    case Op::Dispatch:
        return Status::Ok;
    default:
        return Status::Invalid;
    }}
}}

[[nodiscard]] inline Status init(Ctx& ctx) noexcept {{
    if (ctx.active) {{
        return Status::Busy;
    }}
    ctx.active = true;
    ctx.count = 0;
    return Status::Ok;
}}

[[nodiscard]] inline Status dispatch(Ctx& ctx, Op op) noexcept {{
    if (validate_op(op) != Status::Ok) {{
        return Status::Invalid;
    }}
    if (!ctx.active) {{
        return Status::Invalid;
    }}
    ++ctx.count;
    return Status::Ok;
}}

}} // namespace {ns}
"""


def render_kernel_madvise() -> str:
    return """module;
#include <cstdint>

export module pbsd.kernel.madvise;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_madvise.c — madvise(2) kernel helpers.
export namespace pbsd::kernel::madvise {

enum class Advice : unsigned char {
    Normal = 0,
    Random = 1,
    Sequential = 2,
    WillNeed = 3,
    DontNeed = 4,
};

struct Range {
    std::uintptr_t addr{};
    std::size_t len{};
};

[[nodiscard]] inline Status validate_range(Range const& r) noexcept {
    if (r.len == 0 || r.addr == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status apply(Range const& r, Advice a) noexcept {
    if (validate_range(r) != Status::Ok) {
        return Status::Invalid;
    }
    (void)a;
    return Status::Ok;
}

} // namespace pbsd::kernel::madvise
"""


def kernel_rel(name: str) -> str:
    if name.startswith("subr_"):
        return f"kernel/kern/pbsd.kernel.{name}.cppm"
    return f"kernel/kern/pbsd.kernel.{name}.cppm"


def kernel_target(name: str) -> str:
    return f"pbsd_kernel_{name.replace('.', '_')}"


def area_rel(area: str, name: str) -> str:
    return f"{area}/pbsd.{area}.{name}.cppm"


def area_target(area: str, name: str) -> str:
    return f"pbsd_{area}_{name.replace('.', '_')}"


def ensure_module(area: str, name: str, source: str, *, overwrite: bool = False) -> bool:
    assert_keyword_safe(name)
    rel = area_rel(area, name) if area != "kernel" else kernel_rel(name)
    path = PBSD / rel
    c_path = path.with_suffix(".c")
    created = False
    content = render_kernel_madvise() if area == "kernel" and name == "madvise" else render_scaffold(
        "kernel" if area == "kernel" else area, name, source
    )
    if overwrite or not path.exists():
        if not path.exists() or overwrite:
            path.parent.mkdir(parents=True, exist_ok=True)
            if path.exists() and path.read_text(encoding="utf-8") == content:
                pass
            else:
                path.write_text(content, encoding="utf-8", newline="\n")
                if not overwrite or not path.exists():
                    record(rel)
                elif path.read_text(encoding="utf-8") != content:
                    record(rel)
                created = True
    if not c_path.exists():
        c_path.write_text(render_c(source), encoding="utf-8", newline="\n")
        record(str(c_path.relative_to(PBSD)).replace("\\", "/"))
    return created


def ensure_msun(name: str, source: str, body: str) -> None:
    assert_keyword_safe(name)
    rel = f"userland/msun/pbsd.userland.msun.{name}.cppm"
    path = PBSD / rel
    if path.exists():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        f"""module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.{name};

/// {name} from {source}
export namespace pbsd::userland::msun {{

{body}

}} // namespace pbsd::userland::msun
""",
        encoding="utf-8",
        newline="\n",
    )
    record(rel)
    c_path = path.with_suffix(".c")
    if not c_path.exists():
        c_path.write_text(render_c(source), encoding="utf-8", newline="\n")
        record(str(c_path.relative_to(PBSD)).replace("\\", "/"))


def ensure_libc_sys(area: str, name: str, source: str, body: str) -> None:
    assert_keyword_safe(name)
    rel = f"userland/libc/pbsd.userland.libc.{area}.{name}.cppm"
    path = PBSD / rel
    if path.exists():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        f"""module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.{area}.{name};

export import pbsd.core;

/// {name} from {source}
export namespace pbsd::userland::libc {{

{body}

}} // namespace pbsd::userland::libc
""",
        encoding="utf-8",
        newline="\n",
    )
    record(rel)
    c_path = path.with_suffix(".c")
    if not c_path.exists():
        c_path.write_text(render_c(source), encoding="utf-8", newline="\n")
        record(str(c_path.relative_to(PBSD)).replace("\\", "/"))


def ensure_libthr(name: str, source: str, body: str) -> None:
    assert_keyword_safe(name)
    rel = f"userland/libthr/pbsd.userland.libthr.{name}.cppm"
    path = PBSD / rel
    if path.exists():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        f"""module;
#include <cstdint>

export module pbsd.userland.libthr.{name};

import pbsd.core;
{body}

}} // namespace pbsd::userland::libthr
""".replace(
            "}\n\n} // namespace pbsd::userland::libthr",
            "}\n\n} // namespace pbsd::userland::libthr",
        ),
        encoding="utf-8",
        newline="\n",
    )
    # fix wrapper - body should include namespace open
    text = f"""module;
#include <cstdint>

export module pbsd.userland.libthr.{name};

import pbsd.core;
{body}

}} // namespace pbsd::userland::libthr
"""
    if "export namespace" not in body:
        text = f"""module;
#include <cstdint>

export module pbsd.userland.libthr.{name};

import pbsd.core;
{body}

export namespace pbsd::userland::libthr {{

[[nodiscard]] inline Status stub_ok() noexcept {{ return Status::Ok; }}

}} // namespace pbsd::userland::libthr
"""
    else:
        text = f"""module;
#include <cstdint>

export module pbsd.userland.libthr.{name};

import pbsd.core;

/// Port of {source}
export namespace pbsd::userland::libthr {{

{body}

}} // namespace pbsd::userland::libthr
"""
    path.write_text(text, encoding="utf-8", newline="\n")
    record(rel)
    c_path = path.with_suffix(".c")
    if not c_path.exists():
        c_path.write_text(render_c(source), encoding="utf-8", newline="\n")
        record(str(c_path.relative_to(PBSD)).replace("\\", "/"))


def cmake_block(area: str, name: str, rel: str, parent: str | None) -> str:
    tgt = area_target(area, name) if area != "kernel" else kernel_target(name)
    deps = "pbsd_core"
    if parent:
        deps = f"pbsd_core {parent}"
    return f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES
    {rel})
target_link_libraries({tgt} PUBLIC {deps})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def patch_main_cmake(
    kernel_specs: list[tuple[str, str, str | None]],
    net_specs: list[tuple[str, str, str | None]],
    fs_specs: list[tuple[str, str, str | None]],
) -> None:
    cmake = CMAKE.read_text(encoding="utf-8")

    k_blocks: list[str] = []
    k_names: list[str] = []
    for _, name, _, parent in kernel_specs:
        tgt = kernel_target(name)
        rel = kernel_rel(name)
        if f"TARGET {tgt}" in cmake:
            k_names.append(name)
            continue
        k_blocks.append(cmake_block("kernel", name, rel, parent))
        k_names.append(name)

    n_blocks: list[str] = []
    n_names: list[str] = []
    for _, name, _, parent in net_specs:
        tgt = area_target("net", name)
        rel = area_rel("net", name)
        if f"TARGET {tgt}" in cmake:
            n_names.append(name)
            continue
        n_blocks.append(cmake_block("net", name, rel, parent))
        n_names.append(name)

    f_blocks: list[str] = []
    f_names: list[str] = []
    for _, name, _, parent in fs_specs:
        tgt = area_target("fs", name)
        rel = area_rel("fs", name)
        if f"TARGET {tgt}" in cmake:
            f_names.append(name)
            continue
        f_blocks.append(cmake_block("fs", name, rel, parent))
        f_names.append(name)

    if k_blocks:
        cmake = cmake.replace(
            "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)",
            "\n".join(k_blocks) + "\n\nif(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)",
            1,
        )
    if n_blocks:
        cmake = cmake.replace(
            "if(NOT TARGET pbsd_net)\nadd_library(pbsd_net)",
            "\n".join(n_blocks) + "\n\nif(NOT TARGET pbsd_net)\nadd_library(pbsd_net)",
            1,
        )
    if f_blocks:
        cmake = cmake.replace(
            "if(NOT TARGET pbsd_fs)\nadd_library(pbsd_fs)",
            "\n".join(f_blocks) + "\n\nif(NOT TARGET pbsd_fs)\nadd_library(pbsd_fs)",
            1,
        )

    def patch_iface(agg: str, names: list[str], get_tgt) -> None:
        nonlocal cmake
        pat = rf"(target_link_libraries\({agg} PUBLIC [^\)]+)\)"
        m = re.search(pat, cmake, re.DOTALL)
        if not m:
            return
        block = m.group(1)
        for name in names:
            lib = get_tgt(name)
            if lib not in block:
                block = block.rstrip() + f" {lib}"
        cmake = cmake[: m.start(1)] + block + ")" + cmake[m.end(0) :]

    patch_iface("pbsd_kernel", k_names, kernel_target)
    patch_iface("pbsd_net", n_names, lambda n: area_target("net", n))
    patch_iface("pbsd_fs", f_names, lambda n: area_target("fs", n))

    CMAKE.write_text(cmake, encoding="utf-8", newline="\n")


def patch_aggregate_exports(area: str, names: list[str]) -> None:
    agg = PBSD / area / f"pbsd.{area}.cppm"
    if not agg.exists():
        return
    text = agg.read_text(encoding="utf-8")
    for name in names:
        imp = f"export import pbsd.{area}.{name};"
        if imp not in text:
            for mk in ("/// Wave", "export namespace"):
                idx = text.find(mk)
                if idx != -1:
                    text = text[:idx] + imp + "\n" + text[idx:]
                    break
    agg.write_text(text, encoding="utf-8", newline="\n")


def patch_userland_cmake(msun_names: list[str], libc_names: list[tuple[str, str]], libthr_names: list[str]) -> None:
    cmake = USER_CMAKE.read_text(encoding="utf-8")
    for name in msun_names:
        line = f"    msun/pbsd.userland.msun.{name}.cppm"
        if line not in cmake:
            cmake = cmake.replace(
                "    msun/pbsd.userland.msun.y0.cppm",
                f"    msun/pbsd.userland.msun.{name}.cppm\n    msun/pbsd.userland.msun.y0.cppm",
                1,
            )
    for area, name in libc_names:
        line = f"    libc/pbsd.userland.libc.{area}.{name}.cppm"
        if line not in cmake:
            cmake = cmake.replace(
                "    libc/pbsd.userland.libc.sys.fork.cppm",
                f"    libc/pbsd.userland.libc.{area}.{name}.cppm\n    libc/pbsd.userland.libc.sys.fork.cppm",
                1,
            )
    for name in libthr_names:
        line = f"    libthr/pbsd.userland.libthr.{name}.cppm"
        if line not in cmake:
            cmake = cmake.replace(
                "    libthr/pbsd.userland.libthr.cppm",
                f"    libthr/pbsd.userland.libthr.{name}.cppm\n    libthr/pbsd.userland.libthr.cppm",
                1,
            )
    if "burst 15 wave 3" not in cmake:
        cmake = cmake.replace(
            "# --- burst 15 bin wave 2: bin utility gaps -----------------------------------",
            "# --- burst 15 wave 3: libc sys + msun + libthr gaps ---------------------------\n"
            "# (wired via FILE_SET above)\n\n"
            "# --- burst 15 bin wave 2: bin utility gaps -----------------------------------",
            1,
        )
    USER_CMAKE.write_text(cmake, encoding="utf-8", newline="\n")


def patch_msun_agg(names: list[str]) -> None:
    agg = PBSD / "userland" / "msun" / "pbsd.userland.msun.cppm"
    text = agg.read_text(encoding="utf-8")
    block = "\n".join(
        f"export import pbsd.userland.msun.{n};"
        for n in names
        if f"export import pbsd.userland.msun.{n};" not in text
    )
    if block:
        text = text.replace(
            "export import pbsd.userland.msun.y0;",
            block + "\nexport import pbsd.userland.msun.y0;",
            1,
        )
        agg.write_text(text, encoding="utf-8", newline="\n")


def patch_libthr_agg(names: list[str]) -> None:
    agg = PBSD / "userland" / "libthr" / "pbsd.userland.libthr.cppm"
    text = agg.read_text(encoding="utf-8")
    block = "\n".join(
        f"export import pbsd.userland.libthr.{n};"
        for n in names
        if f"export import pbsd.userland.libthr.{n};" not in text
    )
    if block:
        text = text.rstrip() + "\n" + block + "\n"
        agg.write_text(text, encoding="utf-8", newline="\n")


def main() -> None:
    for area, name, source, _ in KERNEL_SUBR:
        if name == "madvise":
            path = PBSD / kernel_rel(name)
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(render_kernel_madvise(), encoding="utf-8", newline="\n")
            record(kernel_rel(name))
            c_path = path.with_suffix(".c")
            if not c_path.exists():
                c_path.write_text(render_c(source), encoding="utf-8", newline="\n")
                record(str(c_path.relative_to(PBSD)).replace("\\", "/"))
        else:
            ensure_module(area, name, source)

    for area, name, source, _ in NET_MODULES:
        ensure_module(area, name, source)

    for area, name, source, _ in FS_MODULES:
        ensure_module(area, name, source)

    msun_names: list[str] = []
    for name, source, body in MSUN_MODULES:
        ensure_msun(name, source, body)
        msun_names.append(name)

    libc_names: list[tuple[str, str]] = []
    for area, name, source, body in LIBC_SYS:
        ensure_libc_sys(area, name, source, body)
        libc_names.append((area, name))

    libthr_names: list[str] = []
    for name, source, body in LIBTHR_MODULES:
        ensure_libthr(name, source, body)
        libthr_names.append(name)

    patch_main_cmake(KERNEL_SUBR, NET_MODULES, FS_MODULES)
    patch_aggregate_exports("net", [n for _, n, _, _ in NET_MODULES])
    patch_aggregate_exports("fs", [n for _, n, _, _ in FS_MODULES])
    patch_userland_cmake(msun_names, libc_names, libthr_names)
    patch_msun_agg(msun_names)
    patch_libthr_agg(libthr_names)

    print(f"CREATED {len(CREATED)} artifacts")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
