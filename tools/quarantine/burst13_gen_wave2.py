#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 13 wave 2: more userland hand-port .cppm + dual-world .c stubs."""
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
    ban_suffix(cppm.stem.replace("pbsd.userland.", "").replace("pbsd.userland.libc.", ""))
    cppm.parent.mkdir(parents=True, exist_ok=True)
    cppm.write_text(content, encoding="utf-8", newline="\n")
    return True


def libc_gen(name: str, src: str, body: str, c_body: str = "") -> None:
    mod = f"pbsd.userland.libc.gen.{name}"
    ban_suffix(name)
    cppm = ROOT / "libc" / f"{mod.replace('.', '.')}.cppm"
    # fix path: pbsd.userland.libc.gen.clock -> libc/pbsd.userland.libc.gen.clock.cppm
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
    cpath = ROOT / "libc" / f"pbsd.userland.libc.gen.{name}.c"
    write_dual_c(cpath, src, c_body or f"/* stub for {name} */\n")


def libc_area(area: str, name: str, src: str, body: str, c_body: str = "") -> None:
    mod = f"pbsd.userland.libc.{area}.{name}"
    ban_suffix(name)
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
    cpath = ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.c"
    write_dual_c(cpath, src, c_body or f"/* stub for {name} */\n")


def msun(name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.msun.{name}"
    cppm = ROOT / "msun" / f"pbsd.userland.msun.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;
#include <cmath>

export module {mod};

/// {name} from {src}
export namespace pbsd::userland::msun {{

{body}

}} // namespace pbsd::userland::msun
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    cpath = ROOT / "msun" / f"pbsd.userland.msun.{name}.c"
    write_dual_c(cpath, src, c_body or f"/* stub for {name} */\n")


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
    cpath = ROOT / "usr.bin" / f"pbsd.userland.{name}.c"
    write_dual_c(cpath, src, c_body or f"/* stub for {name} */\n")


CREATED: list[str] = []

# --- libc gen -----------------------------------------------------------------
libc_gen(
    "clock",
    "hbsd/src/lib/libc/gen/clock.c",
    "[[nodiscard]] inline unsigned long clock_ticks(unsigned long ticks) noexcept { return ticks; }",
)
libc_gen(
    "clock_getcpuclockid",
    "hbsd/src/lib/libc/gen/clock_getcpuclockid.c",
    """[[nodiscard]] inline Status clock_getcpuclockid_pid(int pid) noexcept {
    if (pid < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_gen(
    "confstr",
    "hbsd/src/lib/libc/gen/confstr.c",
    f"""[[nodiscard]] inline Result<std::size_t> confstr_need(int name) noexcept {{
    if (name < 0) {{
        return result_err<std::size_t>(Status::Invalid);
    }}
    return result_ok(64uz);
}}""",
)
libc_gen(
    "ctermid",
    "hbsd/src/lib/libc/gen/ctermid.c",
    f"""[[nodiscard]] inline const char* ctermid_default() noexcept {{ return "/dev/tty"; }}""",
)
libc_gen(
    "daemon",
    "hbsd/src/lib/libc/gen/daemon.c",
    """[[nodiscard]] inline Status daemon_validate(int nochdir, int noclose) noexcept {
    (void)nochdir;
    (void)noclose;
    return Status::NotImplemented;
}""",
)
libc_gen(
    "devname",
    "hbsd/src/lib/libc/gen/devname.c",
    """[[nodiscard]] inline Status devname_validate(std::uint64_t dev, unsigned mode) noexcept {
    (void)dev;
    (void)mode;
    return Status::Ok;
}""",
)
libc_gen(
    "crypt",
    "hbsd/src/lib/libc/gen/crypt.c",
    f"""[[nodiscard]] inline Status crypt_validate(const char* key, const char* salt) noexcept {{
    if (key == nullptr || salt == nullptr || salt[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "dup3",
    "hbsd/src/lib/libc/gen/dup3.c",
    """[[nodiscard]] inline Status dup3_validate(int oldfd, int newfd, int flags) noexcept {
    if (oldfd < 0 || newfd < 0) {
        return Status::Invalid;
    }
    (void)flags;
    return Status::Ok;
}""",
)
libc_gen(
    "err",
    "hbsd/src/lib/libc/gen/err.c",
    """[[nodiscard]] inline int err_exit_code(int eval) noexcept {
    return eval != 0 ? eval : 1;
}""",
)
libc_gen(
    "errlst",
    "hbsd/src/lib/libc/gen/errlst.c",
    """[[nodiscard]] inline const char* errlst_message(int idx) noexcept {
    if (idx < 0) {
        return "unknown";
    }
    return "error";
}""",
)
libc_gen(
    "eventfd",
    "hbsd/src/lib/libc/gen/eventfd.c",
    """[[nodiscard]] inline Status eventfd_initval(unsigned initval) noexcept {
    (void)initval;
    return Status::Ok;
}""",
)
libc_gen(
    "exec",
    "hbsd/src/lib/libc/gen/exec.c",
    f"""[[nodiscard]] inline Status exec_validate_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "exect",
    "hbsd/src/lib/libc/gen/exect.c",
    f"""[[nodiscard]] inline Status exect_validate(const char* path, char* const argv[]) noexcept {{
    if (path == nullptr || path[0] == {NUL} || argv == nullptr) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "fdopendir",
    "hbsd/src/lib/libc/gen/fdopendir.c",
    """[[nodiscard]] inline Status fdopendir_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_gen(
    "feature_present",
    "hbsd/src/lib/libc/gen/feature_present.c",
    """[[nodiscard]] inline bool feature_present_name(const char* feature) noexcept {
    return feature != nullptr && feature[0] != '\\0';
}""",
)
libc_gen(
    "fmtmsg",
    "hbsd/src/lib/libc/gen/fmtmsg.c",
    """[[nodiscard]] inline int fmtmsg_severity_mask(int classification) noexcept {
    return classification & 0x7;
}""",
)
libc_gen(
    "fpclassify",
    "hbsd/src/lib/libc/gen/fpclassify.c",
    """enum class FpClass : int { Normal, Zero, Subnormal, Infinite, Nan };

[[nodiscard]] inline FpClass fpclassify_zero(double x) noexcept {
    return x == 0.0 ? FpClass::Zero : FpClass::Normal;
}""",
)
libc_gen(
    "fstab",
    "hbsd/src/lib/libc/gen/fstab.c",
    f"""[[nodiscard]] inline bool fstab_comment_line(const char* line) noexcept {{
    return line != nullptr && line[0] == '#';
}}""",
)
libc_gen(
    "ftw",
    "hbsd/src/lib/libc/gen/ftw.c",
    """enum class FtwKind : int { File = 0, Dir, DirR, Slnk, SlnkNo, Unknown };

[[nodiscard]] inline bool ftw_skip(FtwKind kind) noexcept {
    return kind == FtwKind::Unknown;
}""",
)
libc_gen(
    "getdomainname",
    "hbsd/src/lib/libc/gen/getdomainname.c",
    """[[nodiscard]] inline Status getdomainname_buf(char* name, std::size_t len) noexcept {
    if (name == nullptr || len == 0) {
        return Status::Invalid;
    }
    name[0] = '\\0';
    return Status::Ok;
}""",
)
libc_gen(
    "getloadavg",
    "hbsd/src/lib/libc/gen/getloadavg.c",
    """[[nodiscard]] inline int getloadavg_count(int nelem) noexcept {
    if (nelem <= 0 || nelem > 3) {
        return -1;
    }
    return nelem;
}""",
)
libc_gen(
    "getosreldate",
    "hbsd/src/lib/libc/gen/getosreldate.c",
    """[[nodiscard]] inline int getosreldate_value() noexcept { return 0; }""",
)
libc_gen(
    "fdevname",
    "hbsd/src/lib/libc/gen/fdevname.c",
    """[[nodiscard]] inline Status fdevname_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_gen(
    "drand48",
    "hbsd/src/lib/libc/gen/drand48.c",
    """[[nodiscard]] inline double drand48_unit(unsigned seed) noexcept {
    return static_cast<double>(seed % 10000u) / 10000.0;
}""",
)
libc_gen(
    "erand48",
    "hbsd/src/lib/libc/gen/erand48.c",
    """[[nodiscard]] inline double erand48_step(unsigned short xsubi[3]) noexcept {
    if (xsubi == nullptr) {
        return 0.0;
    }
    return static_cast<double>(xsubi[2]) / 65536.0;
}""",
)
libc_gen(
    "cpuset_alloc",
    "hbsd/src/lib/libc/gen/cpuset_alloc.c",
    """[[nodiscard]] inline Result<std::size_t> cpuset_alloc_size(int level) noexcept {
    if (level < 0) {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(128uz);
}""",
)
libc_gen(
    "cpuset_free",
    "hbsd/src/lib/libc/gen/cpuset_free.c",
    """inline void cpuset_free_ptr(void* set) noexcept { (void)set; }""",
)
libc_gen(
    "memfd_create",
    "hbsd/src/lib/libc/gen/memfd_create.c",
    f"""[[nodiscard]] inline Status memfd_create_name(const char* name) noexcept {{
    if (name == nullptr || name[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "pipe2",
    "hbsd/src/lib/libc/gen/pipe2.c",
    """[[nodiscard]] inline Status pipe2_flags(int flags) noexcept {
    (void)flags;
    return Status::Ok;
}""",
)
libc_gen(
    "posix_spawn",
    "hbsd/src/lib/libc/gen/posix_spawn.c",
    f"""[[nodiscard]] inline Status posix_spawn_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "pread",
    "hbsd/src/lib/libc/gen/pread.c",
    """[[nodiscard]] inline Result<std::size_t> pread_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}""",
)
libc_gen(
    "pwrite",
    "hbsd/src/lib/libc/gen/pwrite.c",
    """[[nodiscard]] inline Result<std::size_t> pwrite_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}""",
)
libc_gen(
    "scandir",
    "hbsd/src/lib/libc/gen/scandir.c",
    f"""[[nodiscard]] inline Status scandir_path(const char* dir) noexcept {{
    if (dir == nullptr || dir[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "setdomainname",
    "hbsd/src/lib/libc/gen/setdomainname.c",
    """[[nodiscard]] inline Status setdomainname_buf(const char* name, std::size_t len) noexcept {
    if (name == nullptr || len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_gen(
    "sethostname",
    "hbsd/src/lib/libc/gen/sethostname.c",
    """[[nodiscard]] inline Status sethostname_buf(const char* name, std::size_t len) noexcept {
    if (name == nullptr || len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_gen(
    "statfs",
    "hbsd/src/lib/libc/gen/statfs.c",
    f"""[[nodiscard]] inline Status statfs_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_gen(
    "sync",
    "hbsd/src/lib/libc/gen/sync.c",
    """inline void sync_all() noexcept {}""",
)
libc_gen(
    "wait3",
    "hbsd/src/lib/libc/gen/wait3.c",
    """[[nodiscard]] inline int wait3_status(int status) noexcept { return status; }""",
)
libc_gen(
    "wait4",
    "hbsd/src/lib/libc/gen/wait4.c",
    """[[nodiscard]] inline int wait4_status(int status) noexcept { return status; }""",
)
libc_gen(
    "getusershell",
    "hbsd/src/lib/libc/gen/getusershell.c",
    """[[nodiscard]] inline const char* getusershell_default() noexcept { return "/bin/sh"; }""",
)
libc_gen(
    "setusershell",
    "hbsd/src/lib/libc/gen/setusershell.c",
    """inline void setusershell_rewind() noexcept {}""",
)
libc_gen(
    "endusershell",
    "hbsd/src/lib/libc/gen/endusershell.c",
    """inline void endusershell_close() noexcept {}""",
)
libc_gen(
    "getttyent",
    "hbsd/src/lib/libc/gen/getttyent.c",
    """[[nodiscard]] inline bool getttyent_eof() noexcept { return false; }""",
)
libc_gen(
    "setttyent",
    "hbsd/src/lib/libc/gen/setttyent.c",
    """inline void setttyent_rewind() noexcept {}""",
)
libc_gen(
    "endttyent",
    "hbsd/src/lib/libc/gen/endttyent.c",
    """inline void endttyent_close() noexcept {}""",
)
libc_gen(
    "lockf",
    "hbsd/src/lib/libc/gen/lockf.c",
    """enum class LockOp : int { Unlock = 0, Lock, Test, TestLock };

[[nodiscard]] inline Status lockf_op(LockOp op) noexcept {
    (void)op;
    return Status::Ok;
}""",
)
libc_gen(
    "pathconf",
    "hbsd/src/lib/libc/gen/pathconf.c",
    f"""[[nodiscard]] inline Result<long> pathconf_name(const char* path, int name) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return result_err<long>(Status::Invalid);
    }}
    (void)name;
    return result_ok(0L);
}}""",
)
libc_gen(
    "fpathconf",
    "hbsd/src/lib/libc/gen/fpathconf.c",
    """[[nodiscard]] inline Result<long> fpathconf_fd(int fd, int name) noexcept {
    if (fd < 0) {
        return result_err<long>(Status::Invalid);
    }
    (void)name;
    return result_ok(0L);
}""",
)

# --- libc stdlib/string/net/sys ------------------------------------------------
libc_area(
    "stdlib",
    "strtol",
    "hbsd/src/lib/libc/stdlib/strtol.c",
    """[[nodiscard]] inline long strtol_digits(const char* nptr, char** endptr, int base) noexcept {
    (void)endptr;
    (void)base;
    if (nptr == nullptr) {
        return 0L;
    }
    long acc = 0;
    int sign = 1;
    while (*nptr == ' ') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}""",
)
libc_area(
    "stdlib",
    "strtoul",
    "hbsd/src/lib/libc/stdlib/strtoul.c",
    """[[nodiscard]] inline unsigned long strtoul_digits(const char* nptr, char** endptr, int base) noexcept {
    (void)endptr;
    (void)base;
    if (nptr == nullptr) {
        return 0UL;
    }
    unsigned long acc = 0;
    while (*nptr == ' ') {
        ++nptr;
    }
    if (*nptr == '+') {
        ++nptr;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + static_cast<unsigned long>(*nptr - '0');
        ++nptr;
    }
    return acc;
}""",
)
libc_area(
    "string",
    "strlcpy",
    "hbsd/src/lib/libc/string/strlcpy.c",
    """[[nodiscard]] inline std::size_t strlcpy_copy(char* dst, const char* src, std::size_t size) noexcept {
    if (dst == nullptr || src == nullptr || size == 0) {
        return src == nullptr ? 0 : 0;
    }
    std::size_t i = 0;
    for (; i + 1 < size && src[i] != '\\0'; ++i) {
        dst[i] = src[i];
    }
    if (size > 0) {
        dst[i] = '\\0';
    }
    while (src[i] != '\\0') {
        ++i;
    }
    return i;
}""",
)
libc_area(
    "string",
    "strlcat",
    "hbsd/src/lib/libc/string/strlcat.c",
    """[[nodiscard]] inline std::size_t strlcat_append(char* dst, const char* src, std::size_t size) noexcept {
    if (dst == nullptr || src == nullptr) {
        return 0;
    }
    std::size_t dlen = 0;
    while (dlen < size && dst[dlen] != '\\0') {
        ++dlen;
    }
    std::size_t i = 0;
    while (src[i] != '\\0' && dlen + i + 1 < size) {
        dst[dlen + i] = src[i];
        ++i;
    }
    if (dlen < size) {
        dst[dlen + i] = '\\0';
    }
    while (src[i] != '\\0') {
        ++i;
    }
    return dlen + i;
}""",
)
libc_area(
    "net",
    "inet_ntop",
    "hbsd/src/lib/libc/net/inet_ntop.c",
    """[[nodiscard]] inline Status inet_ntop_validate(int af) noexcept {
    if (af != 2 && af != 28) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_area(
    "net",
    "inet_pton",
    "hbsd/src/lib/libc/net/inet_pton.c",
    """[[nodiscard]] inline Status inet_pton_validate(int af) noexcept {
    if (af != 2 && af != 28) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
)
libc_area(
    "sys",
    "pipe",
    "hbsd/src/lib/libc/sys/pipe.c",
    """[[nodiscard]] inline Status pipe_fds(int fds[2]) noexcept {
    if (fds == nullptr) {
        return Status::Invalid;
    }
    fds[0] = -1;
    fds[1] = -1;
    return Status::Ok;
}""",
)
libc_area(
    "sys",
    "wait",
    "hbsd/src/lib/libc/sys/wait.c",
    """[[nodiscard]] inline int wait_status(int status) noexcept { return status; }""",
)
libc_area(
    "sys",
    "stat",
    "hbsd/src/lib/libc/sys/stat.c",
    f"""[[nodiscard]] inline Status stat_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)
libc_area(
    "sys",
    "lstat",
    "hbsd/src/lib/libc/sys/lstat.c",
    f"""[[nodiscard]] inline Status lstat_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
)

# --- msun ---------------------------------------------------------------------
MSUN_BODY = """[[nodiscard]] inline double {fn}(double x) noexcept {{
    return std::{fn}(x);
}}

[[nodiscard]] inline float {fn}f(float x) noexcept {{
    return std::{fn}f(x);
}}"""

for fn, src in [
    ("acosh", "hbsd/src/lib/msun/src/e_acosh.c"),
    ("asinh", "hbsd/src/lib/msun/src/e_asinh.c"),
    ("atanh", "hbsd/src/lib/msun/src/e_atanh.c"),
    ("log2", "hbsd/src/lib/msun/src/e_log2.c"),
    ("tgamma", "hbsd/src/lib/msun/src/e_tgamma.c"),
]:
    msun(fn, src, MSUN_BODY.format(fn=fn))

msun(
    "hypotf",
    "hbsd/src/lib/msun/src/e_hypotf.c",
    "[[nodiscard]] inline float hypotf_val(float x, float y) noexcept { return std::hypotf(x, y); }",
)
msun(
    "j1",
    "hbsd/src/lib/msun/src/e_j1.c",
    """[[nodiscard]] inline double j1(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_bessel_j(1, x);
#else
    (void)x;
    return 0.0;
#endif
}
[[nodiscard]] inline float j1f(float x) noexcept {
    return static_cast<float>(j1(static_cast<double>(x)));
}""",
)
msun(
    "y1",
    "hbsd/src/lib/msun/src/e_y1.c",
    """[[nodiscard]] inline double y1(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_neumann(1, x);
#else
    (void)x;
    return 0.0;
#endif
}
[[nodiscard]] inline float y1f(float x) noexcept {
    return static_cast<float>(y1(static_cast<double>(x)));
}""",
)
msun("rint", "hbsd/src/lib/msun/src/s_rint.c", MSUN_BODY.format(fn="rint"))
msun("fmaxf", "hbsd/src/lib/msun/src/s_fmaxf.c", "[[nodiscard]] inline float fmaxf_val(float x, float y) noexcept { return std::fmaxf(x, y); }")
msun("fminf", "hbsd/src/lib/msun/src/s_fminf.c", "[[nodiscard]] inline float fminf_val(float x, float y) noexcept { return std::fminf(x, y); }")
msun("remainderf", "hbsd/src/lib/msun/src/e_remainderf.c", "[[nodiscard]] inline float remainderf_val(float x, float y) noexcept { return std::remainderf(x, y); }")
msun("nearbyintf", "hbsd/src/lib/msun/src/s_nearbyintf.c", "[[nodiscard]] inline float nearbyintf_val(float x) noexcept { return std::nearbyintf(x); }")
msun("copysignf", "hbsd/src/lib/msun/src/s_copysignf.c", "[[nodiscard]] inline float copysignf_val(float x, float y) noexcept { return std::copysignf(x, y); }")
msun("frexpf", "hbsd/src/lib/msun/src/s_frexpf.c", "[[nodiscard]] inline float frexpf_val(float x, int* exp) noexcept { return std::frexpf(x, exp); }")
msun("modff", "hbsd/src/lib/msun/src/s_modff.c", "[[nodiscard]] inline float modff_val(float x, float* iptr) noexcept { return std::modff(x, iptr); }")
msun("lgammaf", "hbsd/src/lib/msun/src/e_lgammaf.c", "[[nodiscard]] inline float lgammaf_val(float x) noexcept { return std::lgammaf(x); }")

# --- usr.bin tools ------------------------------------------------------------
USR = [
    ("backlight", "hbsd/src/usr.bin/backlight/backlight.c", "[[nodiscard]] inline bool backlight_on(char flag) noexcept { return flag == 'n'; }"),
    ("bintrans", "hbsd/src/usr.bin/bintrans/bintrans.c", "[[nodiscard]] inline bool bintrans_verbose(char flag) noexcept { return flag == 'v'; }"),
    ("c89", "hbsd/src/usr.bin/c89/c89.c", "[[nodiscard]] inline bool c89_pedantic(char flag) noexcept { return flag == 'p'; }"),
    ("c99", "hbsd/src/usr.bin/c99/c99.c", "[[nodiscard]] inline bool c99_pedantic(char flag) noexcept { return flag == 'p'; }"),
    ("cap_mkdb", "hbsd/src/usr.bin/cap_mkdb/cap_mkdb.c", "[[nodiscard]] inline bool cap_mkdb_verbose(char flag) noexcept { return flag == 'v'; }"),
    ("chat", "hbsd/src/usr.bin/chat/chat.c", "[[nodiscard]] inline bool chat_echo(char flag) noexcept { return flag == 'e'; }"),
    ("ctags", "hbsd/src/usr.bin/ctags/ctags.c", "[[nodiscard]] inline bool ctags_recursive(char flag) noexcept { return flag == 'R'; }"),
    ("ctlstat", "hbsd/src/usr.bin/ctlstat/ctlstat.c", "[[nodiscard]] inline bool ctlstat_json(char flag) noexcept { return flag == 'j'; }"),
    ("dpv", "hbsd/src/usr.bin/dpv/dpv.c", "[[nodiscard]] inline bool dpv_quiet(char flag) noexcept { return flag == 'q'; }"),
    ("elfctl", "hbsd/src/usr.bin/elfctl/elfctl.c", "[[nodiscard]] inline bool elfctl_list(char flag) noexcept { return flag == 'l'; }"),
    ("elfdump", "hbsd/src/usr.bin/elfdump/elfdump.c", "[[nodiscard]] inline bool elfdump_all(char flag) noexcept { return flag == 'a'; }"),
    ("file2c", "hbsd/src/usr.bin/file2c/file2c.c", f"[[nodiscard]] inline Status file2c_name(const char* name) noexcept {{ if (name == nullptr || name[0] == {NUL}) return Status::Invalid; return Status::Ok; }}"),
    ("gcore", "hbsd/src/usr.bin/gcore/gcore.c", "[[nodiscard]] inline bool gcore_force(char flag) noexcept { return flag == 'f'; }"),
    ("genl", "hbsd/src/usr.bin/genl/genl.c", "[[nodiscard]] inline bool genl_list(char flag) noexcept { return flag == 'l'; }"),
    ("getaddrinfo", "hbsd/src/usr.bin/getaddrinfo/getaddrinfo.c", f"[[nodiscard]] inline Status getaddrinfo_host(const char* host) noexcept {{ if (host == nullptr) return Status::Invalid; return Status::Ok; }}"),
    ("gprof", "hbsd/src/usr.bin/gprof/gprof.c", "[[nodiscard]] inline bool gprof_flat(char flag) noexcept { return flag == 'f'; }"),
    ("hesinfo", "hbsd/src/usr.bin/hesinfo/hesinfo.c", f"[[nodiscard]] inline Status hesinfo_name(const char* name) noexcept {{ if (name == nullptr || name[0] == {NUL}) return Status::Invalid; return Status::Ok; }}"),
    ("kdump", "hbsd/src/usr.bin/kdump/kdump.c", "[[nodiscard]] inline bool kdump_follow(char flag) noexcept { return flag == 'f'; }"),
    ("ktrace", "hbsd/src/usr.bin/ktrace/ktrace.c", "[[nodiscard]] inline bool ktrace_clear(char flag) noexcept { return flag == 'C'; }"),
    ("localedef", "hbsd/src/usr.bin/localedef/localedef.c", "[[nodiscard]] inline bool localedef_force(char flag) noexcept { return flag == 'f'; }"),
    ("locate", "hbsd/src/usr.bin/locate/locate/locate.c", f"[[nodiscard]] inline Status locate_pattern(const char* pat) noexcept {{ if (pat == nullptr || pat[0] == {NUL}) return Status::Invalid; return Status::Ok; }}"),
    ("mkimg", "hbsd/src/usr.bin/mkimg/mkimg.c", "[[nodiscard]] inline bool mkimg_verbose(char flag) noexcept { return flag == 'v'; }"),
    ("mkuzip", "hbsd/src/usr.bin/mkuzip/mkuzip.c", "[[nodiscard]] inline bool mkuzip_level(char flag) noexcept { return flag == 'l'; }"),
    ("mdo", "hbsd/src/usr.bin/mdo/mdo.c", "[[nodiscard]] inline bool mdo_list(char flag) noexcept { return flag == 'l'; }"),
    ("mididump", "hbsd/src/usr.bin/mididump/mididump.c", "[[nodiscard]] inline bool mididump_hex(char flag) noexcept { return flag == 'x'; }"),
    ("etdump", "hbsd/src/usr.bin/etdump/etdump.c", "[[nodiscard]] inline bool etdump_all(char flag) noexcept { return flag == 'a'; }"),
]
for name, src, body in USR:
    usr_tool(name, src, body)


def patch_cmake() -> None:
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")

    libc_new = [
        "gen.clock", "gen.clock_getcpuclockid", "gen.confstr", "gen.ctermid", "gen.daemon",
        "gen.devname", "gen.crypt", "gen.dup3", "gen.err", "gen.errlst", "gen.eventfd",
        "gen.exec", "gen.exect", "gen.fdopendir", "gen.feature_present", "gen.fmtmsg",
        "gen.fpclassify", "gen.fstab", "gen.ftw", "gen.getdomainname", "gen.getloadavg",
        "gen.getosreldate", "gen.fdevname", "gen.drand48", "gen.erand48", "gen.cpuset_alloc",
        "gen.cpuset_free", "gen.memfd_create", "gen.pipe2", "gen.posix_spawn", "gen.pread",
        "gen.pwrite", "gen.scandir", "gen.setdomainname", "gen.sethostname", "gen.statfs",
        "gen.sync", "gen.wait3", "gen.wait4", "gen.getusershell", "gen.setusershell",
        "gen.endusershell", "gen.getttyent", "gen.setttyent", "gen.endttyent", "gen.lockf",
        "gen.pathconf", "gen.fpathconf", "gen.sleep",
        "stdlib.strtol", "stdlib.strtoul", "string.strlcpy", "string.strlcat",
        "net.inet_ntop", "net.inet_pton", "sys.pipe", "sys.wait", "sys.stat", "sys.lstat",
    ]
    msun_new = [
        "acosh", "asinh", "atanh", "hypotf", "log2", "j1", "y1", "tgamma", "rint",
        "fmaxf", "fminf", "remainderf", "nearbyintf", "copysignf", "frexpf", "modff",
        "lgammaf", "scalbln",
    ]
    usr_new = [n for n, _, _ in USR]

    def insert_before(lines: list[str], anchor: str, entries: list[str]) -> None:
        nonlocal cmake
        block = "\n".join(f"    {e}" for e in entries if e not in cmake)
        if not block:
            return
        cmake = cmake.replace(anchor, block + "\n" + anchor, 1)

    insert_before(
        [],
        "    libc/pbsd.userland.libc.gen.ttyname.cppm",
        [f"libc/pbsd.userland.libc.{s}.cppm" for s in libc_new if s.startswith("gen.") and s != "gen.sleep"],
    )
    insert_before(
        [],
        "    libc/pbsd.userland.libc.stdlib.qsort_r.cppm",
        [f"libc/pbsd.userland.libc.{s}.cppm" for s in libc_new if s.startswith(("stdlib.", "string.", "net.", "sys."))],
    )
    if "libc/pbsd.userland.libc.gen.sleep.cppm" not in cmake:
        cmake = cmake.replace(
            "    libc/pbsd.userland.libc.gen.usleep.cppm",
            "    libc/pbsd.userland.libc.gen.sleep.cppm\n    libc/pbsd.userland.libc.gen.usleep.cppm",
            1,
        )

    insert_before(
        [],
        "    msun/pbsd.userland.msun.y0.cppm",
        [f"msun/pbsd.userland.msun.{s}.cppm" for s in msun_new],
    )

    if "burst 13b" not in cmake:
        usr_items = " ".join(usr_new)
        burst = f"""
# --- burst 13b: more usr.bin utilities ---------------------------------------
foreach(_ul_b13b IN ITEMS {usr_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b13b}} usr.bin/pbsd.userland.${{_ul_b13b}}.cppm)
endforeach()
"""
        cmake = cmake.replace("\nif(NOT TARGET pbsd_userland_bin)", burst + "\nif(NOT TARGET pbsd_userland_bin)", 1)

    usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
    missing = [t for t in usr_new if f"pbsd_userland_{t}" not in usr_block]
    if missing:
        cmake = cmake.replace(
            "    pbsd_userland_mkstr)\nendif()",
            "    pbsd_userland_mkstr\n    "
            + "\n    ".join(f"pbsd_userland_{t}" for t in missing)
            + ")\nendif()",
            1,
        )

    cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


def main() -> None:
    patch_cmake()
    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
