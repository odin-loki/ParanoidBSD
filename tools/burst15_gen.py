#!/usr/bin/env python3
"""Burst 15: mass userland hand-port .cppm + dual-world .c stubs."""
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
UPDATED: list[str] = []


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


def overwrite_cppm(cppm: Path, content: str) -> bool:
    stem = cppm.stem.replace("pbsd.userland.", "").replace("libc.", "")
    ban_suffix(stem)
    cppm.parent.mkdir(parents=True, exist_ok=True)
    old = cppm.read_text(encoding="utf-8") if cppm.exists() else ""
    if old == content:
        return False
    cppm.write_text(content, encoding="utf-8", newline="\n")
    rel = str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/")
    UPDATED.append(rel)
    return True


def rel(path: Path) -> str:
    return str(path.relative_to(ROOT.parent.parent)).replace("\\", "/")


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
        CREATED.append(rel(cppm))
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
        CREATED.append(rel(cppm))
    write_dual_c(
        ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.c",
        src,
        c_body or f"/* stub for {name} */\n",
    )


def msun(name: str, src: str, body: str, c_body: str = "", freestanding: bool = True) -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.msun.{name}"
    cppm = ROOT / "msun" / f"pbsd.userland.msun.{name}.cppm"
    headers = "#include <cstdint>\n#include <cstring>" if freestanding else "#include <cmath>\n#include <cstdint>\n#include <cstring>"
    if write_cppm(
        cppm,
        f"""module;
{headers}

export module {mod};

/// {name} from {src}
export namespace pbsd::userland::msun {{

{body}

}} // namespace pbsd::userland::msun
""",
    ):
        CREATED.append(rel(cppm))
    write_dual_c(ROOT / "msun" / f"pbsd.userland.msun.{name}.c", src, c_body or f"/* stub for {name} */\n")


def msun_float_fn(name: str, src: str) -> None:
    msun(
        name,
        src,
        f"[[nodiscard]] inline float {name}_val(float x) noexcept {{ return ::{name}(x); }}",
        freestanding=False,
    )


def msun_double_fn(name: str, src: str) -> None:
    msun(
        name,
        src,
        f"[[nodiscard]] inline double {name}_val(double x) noexcept {{ return ::{name}(x); }}",
        freestanding=False,
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
        CREATED.append(rel(cppm))
    write_dual_c(ROOT / "usr.bin" / f"pbsd.userland.{name}.c", src, c_body or f"/* stub for {name} */\n")


def bin_tool(name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.{name}"
    cppm = ROOT / "bin" / f"pbsd.userland.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;

export module {mod};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of {src}
export namespace pbsd::userland::bin::{name} {{

{body}

}} // namespace pbsd::userland::bin::{name}
""",
    ):
        CREATED.append(rel(cppm))
    write_dual_c(ROOT / "bin" / f"pbsd.userland.{name}.c", src, c_body or f"/* stub for {name} */\n")


def libthr(name: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.libthr.{name}"
    cppm = ROOT / "libthr" / f"pbsd.userland.libthr.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;

export module {mod};

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// {name} from {src}
export namespace pbsd::userland::libthr {{

{body}

}} // namespace pbsd::userland::libthr
""",
    ):
        CREATED.append(rel(cppm))
    write_dual_c(ROOT / "libthr" / f"pbsd.userland.libthr.{name}.c", src, c_body or f"/* stub for {name} */\n")


PATH_OK = f"""[[nodiscard]] inline Status path_ok(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""

SYS_FD = "[[nodiscard]] inline Status sys_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }"

LDEXP_DOUBLE = """[[nodiscard]] inline double ldexp_val(double x, int exp) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = static_cast<unsigned>((hx >> 52) & 0x7ffu);
    if (ue == 0 || ue == 0x7ffu) {
        return x;
    }
    const int ne = static_cast<int>(ue) + exp;
    if (ne <= 0) {
        const std::uint64_t sx = hx & 0x8000000000000000ULL;
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }
    if (ne >= 0x7ff) {
        const std::uint64_t sx = (hx & 0x8000000000000000ULL) | 0x7ff0000000000000ULL;
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }
    hx = (hx & 0x800fffffffffffffULL) | (static_cast<std::uint64_t>(ne) << 52);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}"""

SCALBN_BODY = """[[nodiscard]] inline double scalbn(double x, int n) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = static_cast<unsigned>((hx >> 52) & 0x7ffu);
    if (ue == 0 || ue == 0x7ffu) {
        return x;
    }
    const int ne = static_cast<int>(ue) + n;
    if (ne <= 0) {
        const std::uint64_t sx = hx & 0x8000000000000000ULL;
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }
    if (ne >= 0x7ff) {
        const std::uint64_t sx = (hx & 0x8000000000000000ULL) | 0x7ff0000000000000ULL;
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }
    hx = (hx & 0x800fffffffffffffULL) | (static_cast<std::uint64_t>(ne) << 52);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}

[[nodiscard]] inline float scalbnf(float x, int n) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = (hx >> 23) & 0xffu;
    if (ue == 0 || ue == 0xffu) {
        return x;
    }
    const int ne = static_cast<int>(ue) + n;
    if (ne <= 0) {
        const std::uint32_t sx = hx & 0x80000000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    if (ne >= 0xff) {
        const std::uint32_t sx = (hx & 0x80000000u) | 0x7f800000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    hx = (hx & 0x807fffffu) | (static_cast<std::uint32_t>(ne) << 23);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}"""

MODF_BODY = """[[nodiscard]] inline double modf(double x, double* iptr) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = static_cast<unsigned>((hx >> 52) & 0x7ffu);
    if (exp == 0x7ffu) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        return x - x;
    }
    if (exp < 1023u) {
        if (iptr != nullptr) {
            const std::uint64_t sx = hx & 0x8000000000000000ULL;
            std::memcpy(iptr, &sx, sizeof(sx));
        }
        return x;
    }
    if (exp >= 1023u + 52u) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        const std::uint64_t sx = hx & 0x8000000000000000ULL;
        double z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    const unsigned shift = exp - 1023u;
    const std::uint64_t mask = 0x000fffffffffffffULL >> shift;
    const std::uint64_t i_bits = hx & ~mask;
    double ipart;
    std::memcpy(&ipart, &i_bits, sizeof(ipart));
    if (iptr != nullptr) {
        *iptr = ipart;
    }
    return x - ipart;
}

[[nodiscard]] inline float modff(float x, float* iptr) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;
    if (exp == 0xffu) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        return x - x;
    }
    if (exp < 127u) {
        if (iptr != nullptr) {
            const std::uint32_t sx = hx & 0x80000000u;
            std::memcpy(iptr, &sx, sizeof(sx));
        }
        return x;
    }
    if (exp >= 127u + 23u) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        const std::uint32_t sx = hx & 0x80000000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    const unsigned shift = exp - 127u;
    const std::uint32_t mask = 0x007fffffu >> shift;
    const std::uint32_t i_bits = hx & ~mask;
    float ipart;
    std::memcpy(&ipart, &i_bits, sizeof(ipart));
    if (iptr != nullptr) {
        *iptr = ipart;
    }
    return x - ipart;
}"""


def fix_msun_specials() -> None:
    overwrite_cppm(
        ROOT / "msun" / "pbsd.userland.msun.scalbn.cppm",
        f"""module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.scalbn;

/// scalbn/scalbnf from hbsd/src/lib/msun/src/s_scalbn.c
export namespace pbsd::userland::msun {{

{SCALBN_BODY}

}} // namespace pbsd::userland::msun
""",
    )
    overwrite_cppm(
        ROOT / "msun" / "pbsd.userland.msun.modf.cppm",
        f"""module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.modf;

/// modf/modff from hbsd/src/lib/msun/src/s_modf.c
export namespace pbsd::userland::msun {{

{MODF_BODY}

}} // namespace pbsd::userland::msun
""",
    )


def patch_cmake(
    gen_new: list[str],
    area_new: list[tuple[str, str]],
    stdlib_new: list[str],
    msun_new: list[str],
    usr_new: list[str],
    bin_new: list[str],
    libthr_new: list[str],
) -> None:
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")

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
        [f"libc/pbsd.userland.libc.{a}.{n}.cppm" for a, n in area_new],
    )
    insert_before(
        "    libc/pbsd.userland.libc.stdlib.abort.cppm",
        [f"libc/pbsd.userland.libc.stdlib.{s}.cppm" for s in stdlib_new],
    )
    insert_before(
        "    msun/pbsd.userland.msun.y0.cppm",
        [f"msun/pbsd.userland.msun.{s}.cppm" for s in msun_new],
    )
    insert_before(
        "    libthr/pbsd.userland.libthr.cppm",
        [f"libthr/pbsd.userland.libthr.{s}.cppm" for s in libthr_new],
    )

    if "burst 15" not in cmake:
        usr_items = " ".join(usr_new)
        bin_items = " ".join(bin_new)
        burst = f"""
# --- burst 15: libc gen/sys/net/stdlib + msun + usr.bin + bin ---------------
foreach(_ul_b15 IN ITEMS {usr_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b15}} usr.bin/pbsd.userland.${{_ul_b15}}.cppm)
endforeach()
foreach(_ul_b15b IN ITEMS {bin_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b15b}} bin/pbsd.userland.${{_ul_b15b}}.cppm)
endforeach()
"""
        cmake = cmake.replace(
            "\nif(NOT TARGET pbsd_userland_bin)",
            burst + "\nif(NOT TARGET pbsd_userland_bin)",
            1,
        )

    usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
    missing_usr = [t for t in usr_new if f"pbsd_userland_{t}" not in usr_block]
    if missing_usr:
        cmake = cmake.replace(
            "    pbsd_userland_ypwhich)\nendif()",
            "    pbsd_userland_ypwhich\n    "
            + "\n    ".join(f"pbsd_userland_{t}" for t in missing_usr)
            + ")\nendif()",
            1,
        )

    bin_block = cmake.split("pbsd_userland_bin INTERFACE")[1].split("endif()")[0]
    missing_bin = [t for t in bin_new if f"pbsd_userland_{t}" not in bin_block]
    if missing_bin:
        cmake = cmake.replace(
            "    pbsd_userland_rmail)\nendif()",
            "    pbsd_userland_rmail\n    "
            + "\n    ".join(f"pbsd_userland_{t}" for t in missing_bin)
            + ")\nendif()",
            1,
        )

    cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


def patch_msun_agg(new_imports: list[str]) -> None:
    agg = ROOT / "msun" / "pbsd.userland.msun.cppm"
    text = agg.read_text(encoding="utf-8")
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


def patch_libthr_agg(new_imports: list[str]) -> None:
    agg = ROOT / "libthr" / "pbsd.userland.libthr.cppm"
    text = agg.read_text(encoding="utf-8")
    block = "\n".join(
        f"export import pbsd.userland.libthr.{mod};"
        for mod in new_imports
        if f"export import pbsd.userland.libthr.{mod};" not in text
    )
    if block:
        text = text.replace(
            "export import pbsd.userland.libthr.cppm;",
            block + "\nexport import pbsd.userland.libthr.pspinlock;",
            1,
        ) if "export import pbsd.userland.libthr.cppm;" in text else text
        if block not in text:
            text = text.rstrip() + "\n" + block + "\n"
        agg.write_text(text, encoding="utf-8", newline="\n")


GEN_NEW: list[str] = []
AREA_NEW: list[tuple[str, str]] = []
STDLIB_NEW: list[str] = []
MSUN_NEW: list[str] = []
USR_NEW: list[str] = []
BIN_NEW: list[str] = []
LIBTHR_NEW: list[str] = []


def main() -> None:
    # --- libc gen -------------------------------------------------------------
    for name, src, body in [
        ("ulimit", "hbsd/src/lib/libc/gen/ulimit.c", "[[nodiscard]] inline long ulimit_cmd(int cmd, long limit) noexcept { (void)cmd; return limit; }"),
        ("wordexp", "hbsd/src/lib/libc/gen/wordexp.c", PATH_OK.replace("path_ok", "wordexp_words")),
        ("valloc", "hbsd/src/lib/libc/gen/valloc.c", "[[nodiscard]] inline void* valloc_size(std::size_t size) noexcept { (void)size; return nullptr; }"),
        ("memalign", "hbsd/src/lib/libc/gen/memalign.c", "[[nodiscard]] inline void* memalign_boundary(std::size_t align, std::size_t size) noexcept { (void)align; (void)size; return nullptr; }"),
        ("ttyslot", "hbsd/src/lib/libc/gen/ttyslot.c", "[[nodiscard]] inline int ttyslot_fd(int fd) noexcept { return fd >= 0 ? 0 : -1; }"),
        ("dlfcn", "hbsd/src/lib/libc/gen/dlfcn.c", PATH_OK.replace("path_ok", "dlfcn_path")),
        ("scandir_b", "hbsd/src/lib/libc/gen/scandir_b.c", PATH_OK.replace("path_ok", "scandir_b_dir")),
        ("opendir2", "hbsd/src/lib/libc/gen/opendir2.c", PATH_OK.replace("path_ok", "opendir2_path")),
        ("setjmperr", "hbsd/src/lib/libc/gen/setjmperr.c", "[[nodiscard]] inline Status setjmperr_code(int code) noexcept { return code == 0 ? Status::Ok : Status::Invalid; }"),
    ]:
        libc_gen(name, src, body)
        GEN_NEW.append(name)

    # --- libc sys/net ---------------------------------------------------------
    for area, name, src, body in [
        ("sys", "accept4", "hbsd/src/lib/libc/sys/accept4.c", SYS_FD),
        ("sys", "clock_nanosleep", "hbsd/src/lib/libc/sys/clock_nanosleep.c", "[[nodiscard]] inline Status clock_nanosleep_id(int clock_id) noexcept { return clock_id >= 0 ? Status::Ok : Status::Invalid; }"),
        ("sys", "fdatasync", "hbsd/src/lib/libc/sys/fdatasync.c", SYS_FD),
        ("sys", "msync", "hbsd/src/lib/libc/sys/msync.c", "[[nodiscard]] inline Status msync_addr(void* addr, std::size_t len) noexcept { (void)addr; (void)len; return Status::Ok; }"),
        ("sys", "ppoll", "hbsd/src/lib/libc/sys/ppoll.c", "[[nodiscard]] inline Status ppoll_fds(void* fds, unsigned nfds) noexcept { if (!fds && nfds) return Status::Invalid; return Status::Ok; }"),
        ("sys", "lockf", "hbsd/src/lib/libc/sys/lockf.c", "[[nodiscard]] inline Status lockf_fd(int fd, int cmd) noexcept { if (fd < 0) return Status::Invalid; (void)cmd; return Status::Ok; }"),
        ("sys", "creat", "hbsd/src/lib/libc/sys/creat.c", PATH_OK.replace("path_ok", "creat_path")),
        ("sys", "closefrom", "hbsd/src/lib/libc/sys/closefrom.c", "[[nodiscard]] inline Status closefrom_lowfd(int lowfd) noexcept { return lowfd >= 0 ? Status::Ok : Status::Invalid; }"),
        ("sys", "brk", "hbsd/src/lib/libc/sys/brk.c", "[[nodiscard]] inline Status brk_addr(void* addr) noexcept { (void)addr; return Status::Ok; }"),
        ("sys", "getdents", "hbsd/src/lib/libc/sys/getdents.c", SYS_FD),
        ("net", "getifmaddrs", "hbsd/src/lib/libc/net/getifmaddrs.c", "[[nodiscard]] inline Status getifmaddrs_list(void** ifmap) noexcept { if (!ifmap) return Status::Invalid; *ifmap = nullptr; return Status::Ok; }"),
        ("net", "getproto", "hbsd/src/lib/libc/net/getproto.c", PATH_OK.replace("path_ok", "getproto_name")),
        ("net", "getservent", "hbsd/src/lib/libc/net/getservent.c", "[[nodiscard]] inline bool getservent_eof() noexcept { return true; }"),
        ("net", "getprotoent", "hbsd/src/lib/libc/net/getprotoent.c", "[[nodiscard]] inline bool getprotoent_eof() noexcept { return true; }"),
    ]:
        libc_area(area, name, src, body)
        AREA_NEW.append((area, name))

    # --- libc stdlib ----------------------------------------------------------
    for name, src, body in [
        ("getopt_long", "hbsd/src/lib/libc/stdlib/getopt_long.c", "[[nodiscard]] inline int getopt_long_opt(int argc) noexcept { return argc > 0 ? 0 : -1; }"),
        ("getsubopt", "hbsd/src/lib/libc/stdlib/getsubopt.c", "[[nodiscard]] inline Status getsubopt_option(char* const* options) noexcept { return options ? Status::Ok : Status::Invalid; }"),
        ("imaxabs", "hbsd/src/lib/libc/stdlib/imaxabs.c", "[[nodiscard]] inline long long imaxabs_val(long long j) noexcept { return j < 0 ? -j : j; }"),
        ("l64a", "hbsd/src/lib/libc/stdlib/l64a.c", "[[nodiscard]] inline const char* l64a_value(long value) noexcept { (void)value; return \"\"; }"),
        ("bsearch_b", "hbsd/src/lib/libc/stdlib/bsearch_b.c", "[[nodiscard]] inline const void* bsearch_b_key(const void* key) noexcept { return key; }"),
    ]:
        libc_area("stdlib", name, src, body)
        STDLIB_NEW.append(name)

    # --- msun freestanding specials + new funcs --------------------------------
    fix_msun_specials()

    msun("ldexp", "hbsd/src/lib/msun/src/s_ldexp.c", LDEXP_DOUBLE)
    MSUN_NEW.append("ldexp")

    for fn, src in [
        ("acoshf", "hbsd/src/lib/msun/src/e_acoshf.c"),
        ("asinhf", "hbsd/src/lib/msun/src/e_asinhf.c"),
        ("atanhf", "hbsd/src/lib/msun/src/e_atanhf.c"),
        ("log2f", "hbsd/src/lib/msun/src/e_log2f.c"),
        ("exp2f", "hbsd/src/lib/msun/src/e_exp2f.c"),
        ("nextafterf", "hbsd/src/lib/msun/src/e_nextafterf.c"),
        ("cospif", "hbsd/src/lib/msun/src/s_cospif.c"),
        ("sinpif", "hbsd/src/lib/msun/src/s_sinpif.c"),
    ]:
        msun_float_fn(fn, src)
        MSUN_NEW.append(fn)

    for fn, src in [
        ("acosh", "hbsd/src/lib/msun/src/e_acosh.c"),
        ("asinh", "hbsd/src/lib/msun/src/e_asinh.c"),
        ("log2f", "hbsd/src/lib/msun/src/e_log2f.c"),
    ]:
        if fn == "log2f":
            continue
        msun_double_fn(fn, src)
        if fn not in MSUN_NEW:
            MSUN_NEW.append(fn)

    # --- libthr ---------------------------------------------------------------
    libthr(
        "error",
        "hbsd/src/lib/libthr/sys/thr_error.c",
        "[[nodiscard]] inline Status error_code(int err) noexcept { return err == 0 ? Status::Ok : Status::Invalid; }",
    )
    LIBTHR_NEW.append("error")

    # --- usr.bin --------------------------------------------------------------
    USR = [
        ("addr2line", "hbsd/src/usr.bin/addr2line/addr2line.c", "[[nodiscard]] inline bool addr2line_functions(char c) noexcept { return c == 'f'; }"),
        ("alias", "hbsd/src/usr.bin/alias/alias.c", "[[nodiscard]] inline bool alias_list(char c) noexcept { return c == 'l'; }"),
        ("drill", "hbsd/src/usr.bin/drill/drill.c", "[[nodiscard]] inline bool drill_trace(char c) noexcept { return c == 'T'; }"),
        ("less", "hbsd/src/usr.bin/less/less/less.c", "[[nodiscard]] inline bool less_quiet(char c) noexcept { return c == 'q'; }"),
        ("nc", "hbsd/src/usr.bin/nc/nc/nc.c", "[[nodiscard]] inline bool nc_listen(char c) noexcept { return c == 'l'; }"),
        ("systat", "hbsd/src/usr.bin/systat/systat.c", "[[nodiscard]] inline bool systat_batch(char c) noexcept { return c == 'b'; }"),
        ("mail", "hbsd/src/usr.bin/mail/mail/mail.c", "[[nodiscard]] inline bool mail_send(char c) noexcept { return c == 's'; }"),
        ("mandoc", "hbsd/src/usr.bin/mandoc/mandoc/mandoc.c", "[[nodiscard]] inline bool mandoc_whatis(char c) noexcept { return c == 'f'; }"),
        ("rpcgen", "hbsd/src/usr.bin/rpcgen/rpcgen.c", "[[nodiscard]] inline bool rpcgen_cout(char c) noexcept { return c == 'C'; }"),
        ("smbutil", "hbsd/src/usr.bin/smbutil/smbutil.c", "[[nodiscard]] inline bool smbutil_view(char c) noexcept { return c == 'v'; }"),
        ("compile_et", "hbsd/src/usr.bin/compile_et/compile_et.c", "[[nodiscard]] inline bool compile_et_header(char c) noexcept { return c == 'h'; }"),
        ("bsdcat", "hbsd/src/usr.bin/bsdcat/bsdcat.c", "[[nodiscard]] inline bool bsdcat_stdout(char c) noexcept { return c == 'c'; }"),
        ("bmake", "hbsd/src/usr.bin/bmake/bmake/bmake.c", "[[nodiscard]] inline bool bmake_query(char c) noexcept { return c == 'q'; }"),
        ("bsddialog", "hbsd/src/usr.bin/bsddialog/bsddialog.c", "[[nodiscard]] inline bool bsddialog_clear(char c) noexcept { return c == 'c'; }"),
        ("bzip2recover", "hbsd/src/usr.bin/bzip2recover/bzip2recover.c", "[[nodiscard]] inline bool bzip2recover_small(char c) noexcept { return c == 's'; }"),
        ("cxxfilt", "hbsd/src/usr.bin/cxxfilt/cxxfilt.c", "[[nodiscard]] inline bool cxxfilt_types(char c) noexcept { return c == 't'; }"),
        ("dtc", "hbsd/src/usr.bin/dtc/dtc/dtc.c", "[[nodiscard]] inline bool dtc_blob(char c) noexcept { return c == 'b'; }"),
        ("ee", "hbsd/src/usr.bin/ee/ee/ee.c", "[[nodiscard]] inline bool ee_insert(char c) noexcept { return c == 'i'; }"),
        ("elfcopy", "hbsd/src/usr.bin/elfcopy/elfcopy.c", "[[nodiscard]] inline bool elfcopy_strip(char c) noexcept { return c == 'S'; }"),
        ("kyua", "hbsd/src/usr.bin/kyua/kyua/kyua.c", "[[nodiscard]] inline bool kyua_list(char c) noexcept { return c == 'l'; }"),
        ("ldd32", "hbsd/src/usr.bin/ldd32/ldd32.c", "[[nodiscard]] inline bool ldd32_verbose(char c) noexcept { return c == 'v'; }"),
        ("lessecho", "hbsd/src/usr.bin/less/lessecho/lessecho.c", "[[nodiscard]] inline bool lessecho_no_newline(char c) noexcept { return c == 'n'; }"),
        ("lesskey", "hbsd/src/usr.bin/less/lesskey/lesskey.c", "[[nodiscard]] inline bool lesskey_list(char c) noexcept { return c == 'l'; }"),
        ("lint", "hbsd/src/usr.bin/lint/lint/lint.c", "[[nodiscard]] inline bool lint_quiet(char c) noexcept { return c == 'Q'; }"),
        ("lzmainfo", "hbsd/src/usr.bin/lzmainfo/lzmainfo.c", "[[nodiscard]] inline bool lzmainfo_list(char c) noexcept { return c == 'l'; }"),
        ("mkcsmapper", "hbsd/src/usr.bin/mkcsmapper/mkcsmapper.c", "[[nodiscard]] inline bool mkcsmapper_verbose(char c) noexcept { return c == 'v'; }"),
        ("mkesdb", "hbsd/src/usr.bin/mkesdb/mkesdb.c", "[[nodiscard]] inline bool mkesdb_verbose(char c) noexcept { return c == 'v'; }"),
        ("pamtest", "hbsd/src/usr.bin/pamtest/pamtest.c", "[[nodiscard]] inline bool pamtest_verbose(char c) noexcept { return c == 'v'; }"),
        ("tcopy", "hbsd/src/usr.bin/tcopy/tcopy.c", "[[nodiscard]] inline bool tcopy_reverse(char c) noexcept { return c == 'r'; }"),
    ]
    for name, src, body in USR:
        usr_tool(name, src, body)
        USR_NEW.append(name)

    # --- bin ------------------------------------------------------------------
    for name, src, body in [
        ("sh", "hbsd/src/bin/sh/sh/sh.c", "[[nodiscard]] inline bool sh_login(char c) noexcept { return c == 'l'; }"),
        ("csh", "hbsd/src/bin/csh/csh.c", "[[nodiscard]] inline bool csh_login(char c) noexcept { return c == 'l'; }"),
    ]:
        bin_tool(name, src, body)
        BIN_NEW.append(name)

    patch_cmake(GEN_NEW, AREA_NEW, STDLIB_NEW, MSUN_NEW, USR_NEW, BIN_NEW, LIBTHR_NEW)
    patch_msun_agg(MSUN_NEW)
    patch_libthr_agg(LIBTHR_NEW)

    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)
    if UPDATED:
        print(f"Updated {len(UPDATED)} modules")
        for p in UPDATED:
            print(p)


if __name__ == "__main__":
    main()
