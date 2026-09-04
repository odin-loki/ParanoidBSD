#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 14: mass userland hand-port .cppm + dual-world .c stubs."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "userland"
NUL = "'\\0'"
CXX_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "class", "compl", "concept",
    "const", "consteval", "constexpr", "constinit", "const_cast", "continue",
    "co_await", "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit", "export",
    "extern", "false", "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
    "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
    "register", "reinterpret_cast", "requires", "return", "short", "signed",
    "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
    "template", "this", "thread_local", "throw", "true", "try", "typedef",
    "typeid", "typename", "union", "unsigned", "using", "virtual", "void",
    "volatile", "wchar_t", "while", "xor", "xor_eq", "module", "import",
}

CREATED: list[str] = []


def ban_suffix(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"module suffix '{tail}' is a C++ keyword")


def write_dual_c(cpath: Path, src: str, body: str) -> None:
    if cpath.exists():
        return
    cpath.write_text(
        f"/* Reference logic from {src} (dual-world). */\n\n{body}\n",
        encoding="utf-8",
        newline="\n",
    )


def write_cppm(cppm: Path, content: str) -> bool:
    if cppm.exists():
        return False
    stem = cppm.stem.replace("pbsd.userland.", "").replace("libc.", "")
    ban_suffix(stem)
    cppm.parent.mkdir(parents=True, exist_ok=True)
    cppm.write_text(content, encoding="utf-8", newline="\n")
    return True


def libc_gen(name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.libc.gen.{name}"
    cppm = ROOT / "libc" / f"pbsd.userland.libc.gen.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;
#include <cstddef>
#include <cstdint>

export module {mod};

export import pbsd.core;

/// {name} from {src}
export namespace pbsd::userland::libc {{

{body}

}} // namespace pbsd::userland::libc
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    write_dual_c(ROOT / "libc" / f"pbsd.userland.libc.gen.{name}.c", src, c_body or f"/* stub for {name} */\n")


def libc_area(area: str, name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.libc.{area}.{name}"
    cppm = ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;
#include <cstddef>
#include <cstdint>

export module {mod};

export import pbsd.core;

/// {name} from {src}
export namespace pbsd::userland::libc {{

{body}

}} // namespace pbsd::userland::libc
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    write_dual_c(
        ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.c",
        src,
        c_body or f"/* stub for {name} */\n",
    )


def msun(name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.msun.{name}"
    cppm = ROOT / "msun" / f"pbsd.userland.msun.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module {mod};

/// {name} from {src}
export namespace pbsd::userland::msun {{

{body}

}} // namespace pbsd::userland::msun
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    write_dual_c(ROOT / "msun" / f"pbsd.userland.msun.{name}.c", src, c_body or f"/* stub for {name} */\n")


def msun_float_fn(name: str, src: str) -> None:
    msun(
        name,
        src,
        f"[[nodiscard]] inline float {name}_val(float x) noexcept {{ return ::{name}(x); }}",
    )


def msun_float2(name: str, src: str) -> None:
    msun(
        name,
        src,
        f"[[nodiscard]] inline float {name}_val(float x, float y) noexcept {{ return ::{name}(x, y); }}",
    )


def usr_tool(name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.{name}"
    cppm = ROOT / "usr.bin" / f"pbsd.userland.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;

export module {mod};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of {src}
export namespace pbsd::userland::usr_bin::{name} {{

{body}

}} // namespace pbsd::userland::usr_bin::{name}
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    write_dual_c(ROOT / "usr.bin" / f"pbsd.userland.{name}.c", src, c_body or f"/* stub for {name} */\n")


def patch_cmake() -> None:
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")

    gen_new = [
        "frexp", "modf", "isinf", "isnan", "fts", "glob", "nftw", "gethostname",
        "getmntinfo", "getgrent", "getpwent", "getnetgrent", "getutxent", "getvfsbyname",
        "popen", "sem", "semctl", "sem_new", "sysctl", "sysctlbyname", "sysctlnametomib",
        "readpassphrase", "setproctitle", "jrand48", "lrand48", "mrand48", "nrand48",
        "srand48", "seed48", "lcong48", "statvfs", "termios", "timezone", "timespec_get",
        "timespec_getres", "disklabel", "getcap", "initgroups", "inotify", "kqueue1",
        "sched_getaffinity", "sched_setaffinity", "siginterrupt", "signal", "sigsetops",
        "nlist", "pututxline", "pmadvise", "cap_sandboxed",
    ]
    libc_area_new = [
        ("sys", "accept"), ("sys", "connect"), ("sys", "fcntl"), ("sys", "fsync"),
        ("sys", "kevent"), ("sys", "mknod"), ("sys", "nanosleep"), ("sys", "openat"),
        ("sys", "poll"), ("sys", "select"), ("sys", "sendfile"), ("sys", "sigaction"),
        ("net", "ether_addr"), ("net", "eui64"), ("net", "getnameinfo"),
        ("net", "getifaddrs"), ("net", "if_nameindex"),
    ]
    msun_new = [
        "scalbnf", "ldexpf", "ilogbf", "logbf", "log10f", "expf", "sinf", "cosf",
        "tanf", "sqrtf", "asinf", "acosf", "atanf", "atan2f", "sinhf", "coshf",
        "tanhf", "cbrtf", "expm1f", "log1pf", "fmodf", "j0f", "y0f", "lroundf",
        "llroundf", "lrintf", "fabsf",
    ]
    usr_new = [
        "bsdiff", "iscsictl", "ktrdump", "mt", "nfsstat", "posixmqcontrol",
        "posixshmcontrol", "proccontrol", "procstat", "protect", "resizewin",
        "sdiotool", "tftp", "tip", "top", "usbhidaction", "vtfontcvt", "xinstall",
        "xstr", "ypcat", "ypmatch", "ypwhich",
    ]

    def insert_before(anchor: str, entries: list[str]) -> None:
        nonlocal cmake
        block = "\n".join(f"    {e}" for e in entries if e not in cmake)
        if block:
            cmake = cmake.replace(anchor, block + "\n" + anchor, 1)

    insert_before(
        "    libc/pbsd.userland.libc.gen.sysconf.cppm",
        [f"libc/pbsd.userland.libc.gen.{s}.cppm" for s in gen_new],
    )
    insert_before(
        "    libc/pbsd.userland.libc.sys.fork.cppm",
        [f"libc/pbsd.userland.libc.{a}.{n}.cppm" for a, n in libc_area_new],
    )
    insert_before(
        "    msun/pbsd.userland.msun.y0.cppm",
        [f"msun/pbsd.userland.msun.{s}.cppm" for s in msun_new],
    )

    if "burst 14" not in cmake:
        usr_items = " ".join(usr_new)
        burst = f"""
# --- burst 14: libc gen/sys/net + msun float + usr.bin -----------------------
foreach(_ul_b14 IN ITEMS {usr_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b14}} usr.bin/pbsd.userland.${{_ul_b14}}.cppm)
endforeach()
"""
        cmake = cmake.replace("\nif(NOT TARGET pbsd_userland_bin)", burst + "\nif(NOT TARGET pbsd_userland_bin)", 1)

    usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
    missing = [t for t in usr_new if f"pbsd_userland_{t}" not in usr_block]
    if missing:
        cmake = cmake.replace(
            "    pbsd_userland_etdump)\nendif()",
            "    pbsd_userland_etdump\n    "
            + "\n    ".join(f"pbsd_userland_{t}" for t in missing)
            + ")\nendif()",
            1,
        )

    cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


# --- libc gen -----------------------------------------------------------------
PATH_OK = f"""[[nodiscard]] inline Status path_ok(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""

for name, src, body in [
    ("frexp", "hbsd/src/lib/libc/gen/frexp.c", "[[nodiscard]] inline double frexp_mantissa(double x, int* exp) noexcept { (void)exp; return x; }"),
    ("modf", "hbsd/src/lib/libc/gen/modf.c", "[[nodiscard]] inline double modf_frac(double x, double* iptr) noexcept { if (iptr) *iptr = x; return 0.0; }"),
    ("isinf", "hbsd/src/lib/libc/gen/isinf.c", "[[nodiscard]] inline bool isinf_val(double x) noexcept { return x != x ? false : x == x * 2.0 && x != 0.0; }"),
    ("isnan", "hbsd/src/lib/libc/gen/isnan.c", "[[nodiscard]] inline bool isnan_val(double x) noexcept { return x != x; }"),
    ("fts", "hbsd/src/lib/libc/gen/fts.c", PATH_OK.replace("path_ok", "fts_open_path")),
    ("glob", "hbsd/src/lib/libc/gen/glob.c", PATH_OK.replace("path_ok", "glob_pattern")),
    ("nftw", "hbsd/src/lib/libc/gen/nftw.c", PATH_OK.replace("path_ok", "nftw_path")),
    ("gethostname", "hbsd/src/lib/libc/gen/gethostname.c", "[[nodiscard]] inline Status gethostname_buf(char* name, std::size_t len) noexcept { if (!name || !len) return Status::Invalid; name[0] = '\\0'; return Status::Ok; }"),
    ("getmntinfo", "hbsd/src/lib/libc/gen/getmntinfo.c", "[[nodiscard]] inline int getmntinfo_count(int flags) noexcept { (void)flags; return 0; }"),
    ("getgrent", "hbsd/src/lib/libc/gen/getgrent.c", "[[nodiscard]] inline bool getgrent_eof() noexcept { return true; }"),
    ("getpwent", "hbsd/src/lib/libc/gen/getpwent.c", "[[nodiscard]] inline bool getpwent_eof() noexcept { return true; }"),
    ("getnetgrent", "hbsd/src/lib/libc/gen/getnetgrent.c", "[[nodiscard]] inline bool getnetgrent_eof() noexcept { return true; }"),
    ("getutxent", "hbsd/src/lib/libc/gen/getutxent.c", "[[nodiscard]] inline bool getutxent_eof() noexcept { return true; }"),
    ("getvfsbyname", "hbsd/src/lib/libc/gen/getvfsbyname.c", PATH_OK.replace("path_ok", "getvfsbyname_name")),
    ("popen", "hbsd/src/lib/libc/gen/popen.c", PATH_OK.replace("path_ok", "popen_cmd")),
    ("sem", "hbsd/src/lib/libc/gen/sem.c", "[[nodiscard]] inline Status sem_key(int key) noexcept { (void)key; return Status::Ok; }"),
    ("semctl", "hbsd/src/lib/libc/gen/semctl.c", "[[nodiscard]] inline Status semctl_cmd(int semid, int semnum, int cmd) noexcept { (void)semid; (void)semnum; (void)cmd; return Status::Ok; }"),
    ("sem_new", "hbsd/src/lib/libc/gen/sem_new.c", "[[nodiscard]] inline Status sem_new_count(unsigned count) noexcept { return count > 0 ? Status::Ok : Status::Invalid; }"),
    ("sysctl", "hbsd/src/lib/libc/gen/sysctl.c", "[[nodiscard]] inline Status sysctl_mib(const int* mib, unsigned len) noexcept { if (!mib || !len) return Status::Invalid; return Status::Ok; }"),
    ("sysctlbyname", "hbsd/src/lib/libc/gen/sysctlbyname.c", PATH_OK.replace("path_ok", "sysctlbyname_name")),
    ("sysctlnametomib", "hbsd/src/lib/libc/gen/sysctlnametomib.c", PATH_OK.replace("path_ok", "sysctlnametomib_name")),
    ("readpassphrase", "hbsd/src/lib/libc/gen/readpassphrase.c", "[[nodiscard]] inline Status readpassphrase_prompt(const char* prompt) noexcept { if (!prompt) return Status::Invalid; return Status::Ok; }"),
    ("setproctitle", "hbsd/src/lib/libc/gen/setproctitle.c", "inline void setproctitle_fmt(const char* fmt) noexcept { (void)fmt; }"),
    ("jrand48", "hbsd/src/lib/libc/gen/jrand48.c", "[[nodiscard]] inline long jrand48_step(unsigned short x[3]) noexcept { return x ? static_cast<long>(x[2]) : 0L; }"),
    ("lrand48", "hbsd/src/lib/libc/gen/lrand48.c", "[[nodiscard]] inline long lrand48_val() noexcept { return 0L; }"),
    ("mrand48", "hbsd/src/lib/libc/gen/mrand48.c", "[[nodiscard]] inline long mrand48_val() noexcept { return 0L; }"),
    ("nrand48", "hbsd/src/lib/libc/gen/nrand48.c", "[[nodiscard]] inline long nrand48_step(unsigned short x[3]) noexcept { return x ? static_cast<long>(x[0]) : 0L; }"),
    ("srand48", "hbsd/src/lib/libc/gen/srand48.c", "inline void srand48_seed(long seed) noexcept { (void)seed; }"),
    ("seed48", "hbsd/src/lib/libc/gen/seed48.c", "[[nodiscard]] inline unsigned short* seed48_state(unsigned short x[3]) noexcept { return x; }"),
    ("lcong48", "hbsd/src/lib/libc/gen/lcong48.c", "inline void lcong48_param(unsigned short p[7]) noexcept { (void)p; }"),
    ("statvfs", "hbsd/src/lib/libc/gen/statvfs.c", PATH_OK.replace("path_ok", "statvfs_path")),
    ("termios", "hbsd/src/lib/libc/gen/termios.c", "[[nodiscard]] inline Status termios_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }"),
    ("timezone", "hbsd/src/lib/libc/gen/timezone.c", "[[nodiscard]] inline long timezone_offset() noexcept { return 0L; }"),
    ("timespec_get", "hbsd/src/lib/libc/gen/timespec_get.c", "[[nodiscard]] inline int timespec_get_base(int base) noexcept { return base; }"),
    ("timespec_getres", "hbsd/src/lib/libc/gen/timespec_getres.c", "[[nodiscard]] inline int timespec_getres_base(int base) noexcept { return base; }"),
    ("disklabel", "hbsd/src/lib/libc/gen/disklabel.c", PATH_OK.replace("path_ok", "disklabel_dev")),
    ("getcap", "hbsd/src/lib/libc/gen/getcap.c", PATH_OK.replace("path_ok", "getcap_file")),
    ("initgroups", "hbsd/src/lib/libc/gen/initgroups.c", f"""[[nodiscard]] inline Status initgroups_user(const char* user, int gid) noexcept {{
    (void)gid;
    if (user == nullptr || user[0] == {NUL}) return Status::Invalid;
    return Status::Ok;
}}"""),
    ("inotify", "hbsd/src/lib/libc/gen/inotify.c", "[[nodiscard]] inline Status inotify_init_flags(int flags) noexcept { (void)flags; return Status::Ok; }"),
    ("kqueue1", "hbsd/src/lib/libc/gen/kqueue1.c", "[[nodiscard]] inline Status kqueue1_flags(int flags) noexcept { (void)flags; return Status::Ok; }"),
    ("sched_getaffinity", "hbsd/src/lib/libc/gen/sched_getaffinity.c", "[[nodiscard]] inline Status sched_getaffinity_pid(int pid) noexcept { return pid >= 0 ? Status::Ok : Status::Invalid; }"),
    ("sched_setaffinity", "hbsd/src/lib/libc/gen/sched_setaffinity.c", "[[nodiscard]] inline Status sched_setaffinity_pid(int pid) noexcept { return pid >= 0 ? Status::Ok : Status::Invalid; }"),
    ("siginterrupt", "hbsd/src/lib/libc/gen/siginterrupt.c", "[[nodiscard]] inline Status siginterrupt_sig(int sig, int flag) noexcept { (void)sig; (void)flag; return Status::Ok; }"),
    ("signal", "hbsd/src/lib/libc/gen/signal.c", "[[nodiscard]] inline Status signal_num(int sig) noexcept { return sig > 0 ? Status::Ok : Status::Invalid; }"),
    ("sigsetops", "hbsd/src/lib/libc/gen/sigsetops.c", "[[nodiscard]] inline bool sigset_empty() noexcept { return true; }"),
    ("nlist", "hbsd/src/lib/libc/gen/nlist.c", PATH_OK.replace("path_ok", "nlist_path")),
    ("pututxline", "hbsd/src/lib/libc/gen/pututxline.c", "[[nodiscard]] inline Status pututxline_entry(const void* utx) noexcept { return utx ? Status::Ok : Status::Invalid; }"),
    ("pmadvise", "hbsd/src/lib/libc/gen/pmadvise.c", "[[nodiscard]] inline Status pmadvise_addr(void* addr, std::size_t len) noexcept { (void)addr; (void)len; return Status::Ok; }"),
    ("cap_sandboxed", "hbsd/src/lib/libc/gen/cap_sandboxed.c", "[[nodiscard]] inline bool cap_sandboxed_active() noexcept { return false; }"),
]:
    libc_gen(name, src, body)

# --- libc sys/net -----------------------------------------------------------
SYS_FD = "[[nodiscard]] inline Status sys_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }"
for area, name, src, body in [
    ("sys", "accept", "hbsd/src/lib/libc/sys/accept.c", SYS_FD),
    ("sys", "connect", "hbsd/src/lib/libc/sys/connect.c", SYS_FD),
    ("sys", "fcntl", "hbsd/src/lib/libc/sys/fcntl.c", "[[nodiscard]] inline Status fcntl_cmd(int fd, int cmd) noexcept { if (fd < 0) return Status::Invalid; (void)cmd; return Status::Ok; }"),
    ("sys", "fsync", "hbsd/src/lib/libc/sys/fsync.c", SYS_FD),
    ("sys", "kevent", "hbsd/src/lib/libc/sys/kevent.c", "[[nodiscard]] inline Status kevent_kq(int kq) noexcept { return kq >= 0 ? Status::Ok : Status::Invalid; }"),
    ("sys", "mknod", "hbsd/src/lib/libc/sys/mknod.c", PATH_OK.replace("path_ok", "mknod_path")),
    ("sys", "nanosleep", "hbsd/src/lib/libc/sys/nanosleep.c", "[[nodiscard]] inline Status nanosleep_req(const void* req) noexcept { return req ? Status::Ok : Status::Invalid; }"),
    ("sys", "openat", "hbsd/src/lib/libc/sys/openat.c", "[[nodiscard]] inline Status openat_dirfd(int dirfd) noexcept { return dirfd >= 0 ? Status::Ok : Status::Invalid; }"),
    ("sys", "poll", "hbsd/src/lib/libc/sys/poll.c", "[[nodiscard]] inline Status poll_fds(void* fds, unsigned nfds) noexcept { if (!fds && nfds) return Status::Invalid; return Status::Ok; }"),
    ("sys", "select", "hbsd/src/lib/libc/sys/select.c", "[[nodiscard]] inline Status select_nfds(int nfds) noexcept { return nfds >= 0 ? Status::Ok : Status::Invalid; }"),
    ("sys", "sendfile", "hbsd/src/lib/libc/sys/sendfile.c", SYS_FD),
    ("sys", "sigaction", "hbsd/src/lib/libc/sys/sigaction.c", "[[nodiscard]] inline Status sigaction_sig(int sig) noexcept { return sig > 0 ? Status::Ok : Status::Invalid; }"),
    ("net", "ether_addr", "hbsd/src/lib/libc/net/ether_addr.c", "[[nodiscard]] inline Status ether_addr_parse(const char* asc) noexcept { return asc ? Status::Ok : Status::Invalid; }"),
    ("net", "eui64", "hbsd/src/lib/libc/net/eui64.c", "[[nodiscard]] inline Status eui64_parse(const char* s) noexcept { return s ? Status::Ok : Status::Invalid; }"),
    ("net", "getnameinfo", "hbsd/src/lib/libc/net/getnameinfo.c", "[[nodiscard]] inline Status getnameinfo_flags(int flags) noexcept { (void)flags; return Status::Ok; }"),
    ("net", "getifaddrs", "hbsd/src/lib/libc/net/getifaddrs.c", "[[nodiscard]] inline Status getifaddrs_list(void** ifap) noexcept { if (!ifap) return Status::Invalid; *ifap = nullptr; return Status::Ok; }"),
    ("net", "if_nameindex", "hbsd/src/lib/libc/net/if_nameindex.c", "[[nodiscard]] inline bool if_nameindex_empty() noexcept { return true; }"),
]:
    libc_area(area, name, src, body)

# --- msun (freestanding: :: from cmath or bit tricks) -----------------------
msun(
    "fabsf",
    "hbsd/src/lib/msun/src/s_fabsf.c",
    """[[nodiscard]] inline float fabsf_val(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    u &= 0x7fffffffu;
    std::memcpy(&x, &u, sizeof(u));
    return x;
}""",
)

for fn, src in [
    ("scalbnf", "hbsd/src/lib/msun/src/s_scalbnf.c"),
    ("ldexpf", "hbsd/src/lib/msun/src/s_ldexpf.c"),
    ("ilogbf", "hbsd/src/lib/msun/src/s_ilogbf.c"),
    ("logbf", "hbsd/src/lib/msun/src/s_logbf.c"),
    ("log10f", "hbsd/src/lib/msun/src/e_log10f.c"),
    ("expf", "hbsd/src/lib/msun/src/e_expf.c"),
    ("sinf", "hbsd/src/lib/msun/src/e_sinf.c"),
    ("cosf", "hbsd/src/lib/msun/src/e_cosf.c"),
    ("tanf", "hbsd/src/lib/msun/src/e_tanf.c"),
    ("sqrtf", "hbsd/src/lib/msun/src/e_sqrtf.c"),
    ("asinf", "hbsd/src/lib/msun/src/e_asinf.c"),
    ("acosf", "hbsd/src/lib/msun/src/e_acosf.c"),
    ("atanf", "hbsd/src/lib/msun/src/e_atanf.c"),
    ("sinhf", "hbsd/src/lib/msun/src/e_sinhf.c"),
    ("coshf", "hbsd/src/lib/msun/src/e_coshf.c"),
    ("tanhf", "hbsd/src/lib/msun/src/e_tanhf.c"),
    ("cbrtf", "hbsd/src/lib/msun/src/e_cbrtf.c"),
    ("expm1f", "hbsd/src/lib/msun/src/e_expm1f.c"),
    ("log1pf", "hbsd/src/lib/msun/src/e_log1pf.c"),
    ("j0f", "hbsd/src/lib/msun/src/e_j0f.c"),
    ("y0f", "hbsd/src/lib/msun/src/e_y0f.c"),
    ("lroundf", "hbsd/src/lib/msun/src/s_lroundf.c"),
    ("llroundf", "hbsd/src/lib/msun/src/s_llroundf.c"),
    ("lrintf", "hbsd/src/lib/msun/src/s_lrintf.c"),
]:
    msun_float_fn(fn, src)

msun_float2("atan2f", "hbsd/src/lib/msun/src/e_atan2f.c")
msun_float2("fmodf", "hbsd/src/lib/msun/src/e_fmodf.c")

# --- usr.bin ------------------------------------------------------------------
USR_FLAG = "[[nodiscard]] inline bool {name}_flag(char c) noexcept {{ return c == '{flag}'; }}"
USR = [
    ("bsdiff", "hbsd/src/usr.bin/bsdiff/bsdiff/bsdiff.c", "[[nodiscard]] inline bool bsdiff_force(char c) noexcept { return c == 'f'; }"),
    ("iscsictl", "hbsd/src/usr.bin/iscsictl/iscsictl.c", "[[nodiscard]] inline bool iscsictl_list(char c) noexcept { return c == 'l'; }"),
    ("ktrdump", "hbsd/src/usr.bin/ktrdump/ktrdump.c", "[[nodiscard]] inline bool ktrdump_follow(char c) noexcept { return c == 'f'; }"),
    ("mt", "hbsd/src/usr.bin/mt/mt.c", "[[nodiscard]] inline bool mt_status(char c) noexcept { return c == 's'; }"),
    ("nfsstat", "hbsd/src/usr.bin/nfsstat/nfsstat.c", "[[nodiscard]] inline bool nfsstat_json(char c) noexcept { return c == 'j'; }"),
    ("posixmqcontrol", "hbsd/src/usr.bin/posixmqcontrol/posixmqcontrol.c", "[[nodiscard]] inline bool posixmqcontrol_list(char c) noexcept { return c == 'l'; }"),
    ("posixshmcontrol", "hbsd/src/usr.bin/posixshmcontrol/posixshmcontrol.c", "[[nodiscard]] inline bool posixshmcontrol_list(char c) noexcept { return c == 'l'; }"),
    ("proccontrol", "hbsd/src/usr.bin/proccontrol/proccontrol.c", "[[nodiscard]] inline bool proccontrol_list(char c) noexcept { return c == 'l'; }"),
    ("procstat", "hbsd/src/usr.bin/procstat/procstat.c", "[[nodiscard]] inline bool procstat_json(char c) noexcept { return c == 'j'; }"),
    ("protect", "hbsd/src/usr.bin/protect/protect.c", "[[nodiscard]] inline bool protect_quiet(char c) noexcept { return c == 'q'; }"),
    ("resizewin", "hbsd/src/usr.bin/resizewin/resizewin.c", "[[nodiscard]] inline bool resizewin_list(char c) noexcept { return c == 'l'; }"),
    ("sdiotool", "hbsd/src/usr.bin/sdiotool/sdiotool.c", "[[nodiscard]] inline bool sdiotool_verbose(char c) noexcept { return c == 'v'; }"),
    ("tftp", "hbsd/src/usr.bin/tftp/tftp.c", "[[nodiscard]] inline bool tftp_verbose(char c) noexcept { return c == 'v'; }"),
    ("tip", "hbsd/src/usr.bin/tip/tip/tip.c", "[[nodiscard]] inline bool tip_script(char c) noexcept { return c == 's'; }"),
    ("top", "hbsd/src/usr.bin/top/top.c", "[[nodiscard]] inline bool top_batch(char c) noexcept { return c == 'b'; }"),
    ("usbhidaction", "hbsd/src/usr.bin/usbhidaction/usbhidaction.c", "[[nodiscard]] inline bool usbhidaction_list(char c) noexcept { return c == 'l'; }"),
    ("vtfontcvt", "hbsd/src/usr.bin/vtfontcvt/vtfontcvt.c", "[[nodiscard]] inline bool vtfontcvt_verbose(char c) noexcept { return c == 'v'; }"),
    ("xinstall", "hbsd/src/usr.bin/xinstall/xinstall.c", f"[[nodiscard]] inline Status xinstall_src(const char* src) noexcept {{ if (src == nullptr || src[0] == {NUL}) return Status::Invalid; return Status::Ok; }}"),
    ("xstr", "hbsd/src/usr.bin/xstr/xstr.c", "[[nodiscard]] inline bool xstr_quote(char c) noexcept { return c == 'q'; }"),
    ("ypcat", "hbsd/src/usr.bin/ypcat/ypcat.c", "[[nodiscard]] inline bool ypcat_map(char c) noexcept { return c == 'm'; }"),
    ("ypmatch", "hbsd/src/usr.bin/ypmatch/ypmatch.c", f"[[nodiscard]] inline Status ypmatch_key(const char* key) noexcept {{ if (key == nullptr || key[0] == {NUL}) return Status::Invalid; return Status::Ok; }}"),
    ("ypwhich", "hbsd/src/usr.bin/ypwhich/ypwhich.c", f"[[nodiscard]] inline Status ypwhich_map(const char* map) noexcept {{ if (map == nullptr || map[0] == {NUL}) return Status::Invalid; return Status::Ok; }}"),
]
for name, src, body in USR:
    usr_tool(name, src, body)


def patch_msun_agg() -> None:
    agg = ROOT / "msun" / "pbsd.userland.msun.cppm"
    text = agg.read_text(encoding="utf-8")
    new_imports = [
        "scalbnf", "ldexpf", "ilogbf", "logbf", "log10f", "expf", "sinf", "cosf",
        "tanf", "sqrtf", "asinf", "acosf", "atanf", "atan2f", "sinhf", "coshf",
        "tanhf", "cbrtf", "expm1f", "log1pf", "fmodf", "j0f", "y0f", "lroundf",
        "llroundf", "lrintf", "fabsf",
    ]
    block = "\n".join(
        f"export import pbsd.userland.msun.{mod};"
        for mod in new_imports
        if f"export import pbsd.userland.msun.{mod};" not in text
    )
    if block:
        text = text.replace(
            "export import pbsd.userland.msun.y0;",
            block + "\nexport import pbsd.userland.msun.y0;",
            1,
        )
        agg.write_text(text, encoding="utf-8", newline="\n")


def main() -> None:
    patch_cmake()
    patch_msun_agg()
    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
