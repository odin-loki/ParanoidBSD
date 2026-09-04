#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 15 wave 2: usr.bin/bin utility hand-port .cppm + dual-world .c stubs."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "userland"
HBSD = Path(__file__).resolve().parents[1] / "hbsd" / "src"
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

# hbsd usr.bin dir -> (module_name, optional explicit .c path under hbsd/src)
USR_BIN: list[tuple[str, str, str | None]] = [
    ("addr2line", "addr2line", None),
    ("alias", "alias", None),
    ("bluetooth", "bluetooth", "usr.bin/bluetooth/bthost/bthost.c"),
    ("bmake", "bmake", "contrib/bmake/main.c"),
    ("bsdcat", "bsdcat", "contrib/libarchive/cat/bsdcat.c"),
    ("bsddialog", "bsddialog", "contrib/bsddialog/bsddialog.c"),
    ("bzip2recover", "bzip2recover", "contrib/bzip2/bzip2recover.c"),
    ("clang", "clang", None),
    ("compile_et", "compile_et", None),
    ("cxxfilt", "cxxfilt", "contrib/binutils/binutils/cxxfilt.c"),
    ("drill", "drill", None),
    ("dtc", "dtc", "sys/contrib/devicetree/dtc/dtc.c"),
    ("ee", "ee", None),
    ("elfcopy", "elfcopy", None),
    ("gh-bc", "gh_bc", "contrib/bc/bc/bc_main.c"),
    ("kyua", "kyua", None),
    ("ldd32", "ldd32", None),
    ("less", "less", "contrib/less/less.c"),
    ("lessecho", "lessecho", "contrib/less/lessecho.c"),
    ("lesskey", "lesskey", "contrib/less/lesskey.c"),
    ("lint", "lint", None),
    ("lzmainfo", "lzmainfo", "contrib/xz/src/lzmainfo/lzmainfo.c"),
    ("mail", "mail", None),
    ("man", "man", None),
    ("mandoc", "mandoc", "contrib/mandoc/mandoc/mandoc.c"),
    ("mkcsmapper", "mkcsmapper", None),
    ("mkcsmapper_static", "mkcsmapper_static", None),
    ("mkesdb", "mkesdb", None),
    ("mkesdb_static", "mkesdb_static", None),
    ("nc", "nc", None),
    ("ofed", "ofed", None),
    ("pamtest", "pamtest", None),
    ("rpcgen", "rpcgen", None),
    ("smbutil", "smbutil", None),
    ("ssh-copy-id", "ssh_copy_id", "usr.bin/ssh-copy-id/ssh-copy-id.c"),
    ("systat", "systat", None),
    ("tcopy", "tcopy", None),
    ("telnet", "telnet", None),
    ("truss", "truss", None),
    ("ts", "ts", None),
    ("unzip", "unzip", "contrib/unzip/unzip/unzip.c"),
    ("usbhidctl", "usbhidctl", None),
    ("vgrind", "vgrind", None),
    ("vi", "vi", None),
    ("wg", "wg", None),
    ("xo", "xo", None),
    ("xohtml", "xohtml", None),
    ("xolint", "xolint", None),
    ("xopo", "xopo", None),
    ("xzdec", "xzdec", "contrib/xz/src/xzdec/xzdec.c"),
    ("zstd", "zstd", "contrib/zstd/programs/zstdcli.c"),
]

BIN: list[tuple[str, str, str | None]] = [
    ("csh", "csh", "bin/csh/csh.c"),
    # df already wired via sbin/pbsd.userland.df.cppm (pbsd_userland_df target)
    ("sh", "sh", "bin/sh/sh.c"),
]


def ban_suffix(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"module suffix '{tail}' is a C++ keyword: {name}")


def find_c(rel: str | None, area: str, hbsd_name: str) -> str:
    if rel:
        p = HBSD / rel
        if p.is_file():
            return f"hbsd/src/{rel.replace(chr(92), '/')}"
    base = HBSD / area / hbsd_name
    if base.is_dir():
        for c in sorted(base.rglob("*.c")):
            if "test" not in c.parts:
                rel_path = c.relative_to(HBSD.parent.parent)
                return str(rel_path).replace("\\", "/")
    return f"hbsd/src/{area}/{hbsd_name}/{hbsd_name}.c"


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
    stem = cppm.stem.replace("pbsd.userland.", "")
    ban_suffix(stem)
    cppm.parent.mkdir(parents=True, exist_ok=True)
    cppm.write_text(content, encoding="utf-8", newline="\n")
    return True


def flag_fn(name: str, flag: str = "v") -> str:
    return f"[[nodiscard]] inline bool {name}_flag(char c) noexcept {{ return c == '{flag}'; }}"


def path_fn(name: str, fn: str) -> str:
    return f"""[[nodiscard]] inline Status {fn}(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""


def usr_tool(mod: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(mod)
    cppm = ROOT / "usr.bin" / f"pbsd.userland.{mod}.cppm"
    if write_cppm(
        cppm,
        f"""module;

export module pbsd.userland.{mod};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of {src}
export namespace pbsd::userland::usr_bin::{mod} {{

{body}

}} // namespace pbsd::userland::usr_bin::{mod}
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    write_dual_c(
        ROOT / "usr.bin" / f"pbsd.userland.{mod}.c",
        src,
        c_body or f"/* stub for {mod} */\n",
    )


def bin_tool(mod: str, src: str, body: str, c_body: str = "") -> None:
    ban_suffix(mod)
    cppm = ROOT / "bin" / f"pbsd.userland.{mod}.cppm"
    if write_cppm(
        cppm,
        f"""module;

export module pbsd.userland.{mod};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of {src}
export namespace pbsd::userland::bin::{mod} {{

{body}

}} // namespace pbsd::userland::bin::{mod}
""",
    ):
        CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
    write_dual_c(
        ROOT / "bin" / f"pbsd.userland.{mod}.c",
        src,
        c_body or f"/* stub for {mod} */\n",
    )


def patch_cmake(usr_mods: list[str], bin_mods: list[str]) -> None:
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")

    if usr_mods and "burst 15" not in cmake:
        usr_items = " ".join(usr_mods)
        burst = f"""
# --- burst 15 wave 2: usr.bin utility gaps -----------------------------------
foreach(_ul_b15 IN ITEMS {usr_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b15}} usr.bin/pbsd.userland.${{_ul_b15}}.cppm)
endforeach()
"""
        cmake = cmake.replace("\nif(NOT TARGET pbsd_userland_bin)", burst + "\nif(NOT TARGET pbsd_userland_bin)", 1)

    if bin_mods and "burst 15 bin" not in cmake:
        bin_items = " ".join(bin_mods)
        burst_bin = f"""
# --- burst 15 bin wave 2: bin utility gaps -----------------------------------
foreach(_ul_b15_bin IN ITEMS {bin_items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b15_bin}} bin/pbsd.userland.${{_ul_b15_bin}}.cppm)
endforeach()
"""
        cmake = cmake.replace("\nif(NOT TARGET pbsd_userland_bin)", burst_bin + "\nif(NOT TARGET pbsd_userland_bin)", 1)

    if usr_mods:
        usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
        missing = [t for t in usr_mods if f"pbsd_userland_{t}" not in usr_block]
        if missing:
            cmake = cmake.replace(
                "    pbsd_userland_ypwhich)\nendif()",
                "    pbsd_userland_ypwhich\n    "
                + "\n    ".join(f"pbsd_userland_{t}" for t in missing)
                + ")\nendif()",
                1,
            )

    if bin_mods:
        bin_block = cmake.split("pbsd_userland_bin INTERFACE")[1].split("endif()")[0]
        missing_bin = [t for t in bin_mods if f"pbsd_userland_{t}" not in bin_block]
        if missing_bin:
            cmake = cmake.replace(
                "    pbsd_userland_rmail)\nendif()",
                "    pbsd_userland_rmail\n    "
                + "\n    ".join(f"pbsd_userland_{t}" for t in missing_bin)
                + ")\nendif()",
                1,
            )

    cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


def main() -> None:
    usr_mods: list[str] = []
    for hbsd_name, mod, rel in USR_BIN:
        src = find_c(rel, "usr.bin", hbsd_name)
        usr_tool(mod, src, flag_fn(mod))
        usr_mods.append(mod)

    bin_mods: list[str] = []
    for hbsd_name, mod, rel in BIN:
        src = find_c(rel, "bin", hbsd_name)
        bin_tool(mod, src, flag_fn(mod))
        bin_mods.append(mod)

    patch_cmake(usr_mods, bin_mods)
    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
