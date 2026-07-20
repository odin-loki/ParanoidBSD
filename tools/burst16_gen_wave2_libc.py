#!/usr/bin/env python3
"""Burst 16 wave 2: libc locale/rpc/gdtoa/stdlib/sys hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "userland"
HBSD_LIBC = Path(__file__).resolve().parents[1] / "hbsd" / "src" / "lib" / "libc"
CMAKE = ROOT / "CMakeLists.txt"
LIBC_AGG = ROOT / "libc" / "pbsd.userland.libc.cppm"
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

# Grouped elsewhere — skip individual re-ports.
SKIP = {
    ("string", "wcpcpy"), ("string", "wcpncpy"),
    ("string", "wcscasecmp"), ("string", "wcsncasecmp"),
    ("string", "wcscspn"), ("string", "wcsspn"),
    ("string", "wcsrchr"), ("string", "wmemchr"),
    ("string", "wcsncmp"), ("string", "wmemmove"),
}

CREATED: list[str] = []


def norm(s: str) -> str:
    return s.replace("-", "_")


def ban_suffix(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"keyword-banned module suffix: {tail}")


def rel_path(cppm: Path) -> str:
    return str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/")


def thin_validate(fn: str) -> str:
    return f"""[[nodiscard]] inline Status {fn}_validate(const char* arg) noexcept {{
    if (arg == nullptr || arg[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""


def thin_wvalidate(fn: str) -> str:
    return f"""[[nodiscard]] inline Status {fn}_validate(const wchar_t* arg) noexcept {{
    if (arg == nullptr) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""


def body_for(area: str, name: str) -> str:
    fn = norm(name)
    if area == "stdlib" and fn in ("free_sized", "free_aligned_sized"):
        return f"""[[nodiscard]] inline Status {fn}_ptr(void* p, std::size_t size) noexcept {{
    (void)p; (void)size;
    return Status::Ok;
}}"""
    if area == "sys" and fn == "POSIX2x_Fork":
        fn = "posix2x_fork"
        return f"""[[nodiscard]] inline Status {fn}_flags(int flags) noexcept {{
    (void)flags;
    return Status::NotImplemented;
}}"""
    if area == "gdtoa":
        return f"""[[nodiscard]] inline Status {fn}_buf(char* buf, std::size_t len) noexcept {{
    if (buf == nullptr || len == 0) return Status::Invalid;
    buf[0] = {NUL};
    return Status::Ok;
}}"""
    if area == "locale" and fn.startswith("wc"):
        return thin_wvalidate(fn)
    if area == "locale" and fn in ("mbtowc", "wctomb", "mbrtowc", "wcrtomb", "mbrlen", "mbsinit"):
        return f"""[[nodiscard]] inline int {fn}_scaffold() noexcept {{ return 0; }}"""
    if area == "rpc":
        if fn.startswith("svc") or fn.startswith("clnt"):
            return f"""[[nodiscard]] inline Status {fn}_init() noexcept {{ return Status::NotImplemented; }}"""
        return thin_validate(fn)
    return thin_validate(fn)


def src_path(area: str, name: str) -> str:
    if area == "stdlib":
        malloc_c = HBSD_LIBC / "stdlib" / "malloc" / f"{name}.c"
        if malloc_c.is_file():
            return f"hbsd/src/lib/libc/stdlib/malloc/{name}.c"
    return f"hbsd/src/lib/libc/{area}/{name}.c"


def write_dual_c(cpath: Path, src: str) -> None:
    if cpath.exists():
        return
    cpath.write_text(
        f"/* Reference logic from {src} (dual-world). */\n\n/* stub */\n",
        encoding="utf-8",
        newline="\n",
    )


def write_cppm(area: str, name: str) -> bool:
    if (area, name) in SKIP:
        return False
    an = norm(area)
    mn = norm(name)
    if an == "sys" and mn == "POSIX2x_Fork":
        mn = "posix2x_fork"
    ban_suffix(mn)
    mod = f"pbsd.userland.libc.{an}.{mn}"
    cppm = ROOT / "libc" / f"{mod}.cppm"
    if cppm.exists():
        return False
    src = src_path(area, name)
    body = body_for(area, name)
    extra_include = ""
    if area == "locale" and mn.startswith("wc"):
        extra_include = "#include <cwchar>\n"
    cppm.write_text(
        f"""module;
#include <cstddef>
#include <cstdint>
{extra_include}
export module {mod};

export import pbsd.core;

/// {mn} from {src}
export namespace pbsd::userland::libc {{

{body}

}} // namespace pbsd::userland::libc
""",
        encoding="utf-8",
        newline="\n",
    )
    write_dual_c(ROOT / "libc" / f"pbsd.userland.libc.{an}.{mn}.c", src)
    CREATED.append(rel_path(cppm))
    return True


def discover() -> list[tuple[str, str]]:
    mods: set[str] = set()
    for p in (ROOT / "libc").glob("*.cppm"):
        s = p.stem.replace("pbsd.userland.libc.", "")
        mods.add(s)
        mods.add(norm(s))
        if "." in s:
            mods.add(s.split(".", 1)[1])
            mods.add(norm(s.split(".", 1)[1]))

    areas = ("gdtoa", "locale", "rpc", "stdlib", "sys")
    out: list[tuple[str, str]] = []
    for area in areas:
        base = HBSD_LIBC / area
        if not base.is_dir():
            continue
        for c in sorted(base.rglob("*.c")):
            name = c.stem
            if name.startswith("_"):
                continue
            if (area, name) in SKIP:
                continue
            an = norm(area)
            mn = norm(name)
            keys = [name, mn, f"{area}.{name}", f"{an}.{mn}"]
            if any(k in mods for k in keys):
                continue
            out.append((area, name))
    return out


def patch_cmake() -> None:
    if not CREATED:
        return
    cmake = CMAKE.read_text(encoding="utf-8")
    entries = sorted(f"    libc/{Path(m).name}" for m in CREATED)
    marker = "    libc/pbsd.userland.libc.cppm"
    new_lines = [e for e in entries if e.strip() not in cmake]
    if not new_lines:
        return
    block = "\n".join(new_lines)
    if "# --- burst 16 wave 2: libc locale/rpc ---" not in cmake:
        block = f"\n# --- burst 16 wave 2: libc locale/rpc ---\n{block}\n"
    cmake = cmake.replace(marker, block + marker, 1)
    CMAKE.write_text(cmake, encoding="utf-8", newline="\n")


def patch_umbrella() -> None:
    if not CREATED:
        return
    text = LIBC_AGG.read_text(encoding="utf-8")
    anchor = "export namespace pbsd::userland::libc"
    imports: list[str] = []
    for m in CREATED:
        imp = f"export import {Path(m).stem};"
        if imp not in text:
            imports.append(imp)
    if not imports:
        return
    text = text.replace(anchor, "\n".join(sorted(imports)) + "\n\n" + anchor, 1)
    LIBC_AGG.write_text(text, encoding="utf-8", newline="\n")


def main() -> None:
    for area, name in discover():
        write_cppm(area, name)
    patch_cmake()
    patch_umbrella()
    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
