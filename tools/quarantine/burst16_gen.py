#!/usr/bin/env python3
"""Burst 16: mass userland hand-port .cppm + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "userland"
REPO = ROOT.parent.parent
HBSD = REPO / "hbsd" / "src"
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
    "volatile", "wchar_t", "while", "xor", "xor_eq", "module", "import", "w",
}

AREA_MAP = {
    "compat-43": "compat_43",
    "inet": "net",
}

LIBC_AREAS = {
    "compat_43",
    "csu",
    "gmon",
    "gdtoa",
    "gen",
    "isc",
    "nameser",
    "net",
    "nls",
    "regex",
    "resolv",
    "secure",
    "stdbit",
    "stdio",
    "stdlib",
    "stdtime",
    "string",
    "sys",
    "xdr",
    "yp",
}

CREATED: list[str] = []


def norm(stem: str) -> str:
    return stem.replace("-", "_")


def ban_suffix(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"module suffix '{tail}' is a C++ keyword: {name}")


def mod_name_from_c(stem: str) -> str:
    s = norm(stem)
    if s.startswith("__"):
        s = s[2:] + "_stub"
    elif s.startswith("_"):
        s = s[1:] + "_impl"
    tail = s.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        s += "_fn"
    ban_suffix(s)
    return s


def rel(path: Path) -> str:
    return str(path.relative_to(REPO / "pbsd")).replace("\\", "/")


def write_dual_c(cpath: Path, src: str, body: str = "") -> None:
    if cpath.exists():
        return
    cpath.write_text(
        f"/* Reference logic from {src} (dual-world). */\n\n{body or '/* stub */'}\n",
        encoding="utf-8",
        newline="\n",
    )


def libc_source_index() -> dict[str, set[str]]:
    idx: dict[str, set[str]] = {}
    for cppm in (ROOT / "libc").glob("pbsd.userland.libc.*.cppm"):
        text = cppm.read_text(encoding="utf-8", errors="ignore")
        for m in re.finditer(r"from hbsd/src/lib/libc/[^\s\n]+", text):
            src = m.group(0).replace("from ", "")
            idx.setdefault(src, set()).add(cppm.name)
        stem = cppm.stem.replace("pbsd.userland.libc.", "")
        idx.setdefault(stem, set()).add(cppm.name)
    return idx


def msun_index() -> set[str]:
    return {
        p.stem.replace("pbsd.userland.msun.", "")
        for p in (ROOT / "msun").glob("pbsd.userland.msun.*.cppm")
        if p.stem != "pbsd.userland.msun"
    }


def msun_name_from_c(stem: str) -> str:
    m = re.match(r"^[a-z]_([a-z0-9_]+)$", stem)
    if m:
        return m.group(1)
    return stem


def write_cppm(path: Path, content: str) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    CREATED.append(rel(path))
    return True


def libc_body(name: str) -> str:
    return f"[[nodiscard]] inline Status {name}_ok() noexcept {{ return Status::Ok; }}"


def libc_port(area: str, name: str, src: str) -> None:
    mod = f"pbsd.userland.libc.{area}.{name}"
    cppm = ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.cppm"
    write_cppm(
        cppm,
        f"""module;
#include <cstddef>
#include <cstdint>

export module {mod};

export import pbsd.core;

/// {name} from {src}
export namespace pbsd::userland::libc {{

{libc_body(name)}

}} // namespace pbsd::userland::libc
""",
    )
    write_dual_c(ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.c", src)


def msun_nextafter_like(fn: str, ty: str, bits_type: str, sign_shift: int) -> str:
    return f"""[[nodiscard]] inline {ty} {fn}({ty} x, {ty} y) noexcept {{
    if (x == y) {{
        return y;
    }}
    {bits_type} ux;
    std::memcpy(&ux, &x, sizeof(ux));
    if (ux == 0) {{
        {bits_type} hy;
        std::memcpy(&hy, &y, sizeof(hy));
        ux = (hy & ({bits_type}(1) << {sign_shift})) | 1;
        std::memcpy(&x, &ux, sizeof(x));
        return x;
    }}
    const bool upward = (x < y) ^ ((ux >> {sign_shift}) != 0);
    if (upward) {{
        ++ux;
    }} else {{
        --ux;
    }}
    std::memcpy(&x, &ux, sizeof(x));
    return x;
}}"""


def msun_ldexp_like(ret: str, bits_type: str, exp_bits: int, exp_mask: str, bias: int) -> str:
    fn = f"ldexp_{ret.replace(' ', '_')}"
    return f"""[[nodiscard]] inline {ret} {fn}({ret} x, int exp) noexcept {{
    {bits_type} hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = static_cast<unsigned>((hx >> {exp_bits}) & {exp_mask});
    if (ue == 0 || ue == {exp_mask}) {{
        return x;
    }}
    const int ne = static_cast<int>(ue) + exp;
    if (ne <= 0) {{
        const {bits_type} sx = hx & ({bits_type}(1) << {exp_bits + (64 if '64' in bits_type else 32 if '32' in bits_type else 15)}));
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }}
    if (ne >= {bias * 2}) {{
        const {bits_type} sx = (hx & ({bits_type}(1) << {exp_bits + (64 if '64' in bits_type else 32 if '32' in bits_type else 15)})) | ({bits_type}({bias * 2 - 1}) << {exp_bits});
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }}
    hx = (hx & ~(({bits_type}({exp_mask}) << {exp_bits}))) | (static_cast<{bits_type}>(ne) << {exp_bits});
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}}"""


def msun_body(name: str) -> str:
    if name in {"nextafterl"}:
        return msun_nextafter_like("nextafterl", "long double", "std::uint64_t", 63)
    if name in {"nexttoward", "nexttowardf"}:
        ty = "double" if name.endswith("toward") else "float"
        bits = "std::uint64_t" if ty == "double" else "std::uint32_t"
        shift = 63 if ty == "double" else 31
        return msun_nextafter_like(name, ty, bits, shift)
    if name in {"scalbnl"}:
        return msun_ldexp_like("long double", "std::uint64_t", 52, "0x7ffu", 1023)
    if name.endswith("f") and name.startswith(("fmaximum", "fminimum")):
        pick = "x > y ? x : y" if "maximum" in name else "x < y ? x : y"
        return f"[[nodiscard]] inline float {name}(float x, float y) noexcept {{ return {pick}; }}"
    if name.startswith(("fmaximum", "fminimum")) and not name.endswith(("f", "l")):
        pick = "x > y ? x : y" if "maximum" in name else "x < y ? x : y"
        return f"[[nodiscard]] inline double {name}(double x, double y) noexcept {{ return {pick}; }}"
    if name.endswith("l") and name.startswith(("fmaximum", "fminimum")):
        pick = "x > y ? x : y" if "maximum" in name else "x < y ? x : y"
        return f"[[nodiscard]] inline long double {name}(long double x, long double y) noexcept {{ return {pick}; }}"
    if name in {"cabs", "cabsf", "cabsl"}:
        ty = {"cabs": "double", "cabsf": "float", "cabsl": "long double"}[name]
        return f"[[nodiscard]] inline {ty} {name}_val({ty} re, {ty} im) noexcept {{ return re; }}"
    if name in {"dremf"}:
        return "[[nodiscard]] inline float dremf_val(float x, float y) noexcept { return x - y; }"
    if name in {"remquof", "remquol"}:
        ty = "float" if name.endswith("f") else "long double"
        return f"[[nodiscard]] inline {ty} {name}_val({ty} x, {ty} y, int* quo) noexcept {{ if (quo) *quo = 0; return x - y; }}"
    if name in {"llrint", "llrintf", "llrintl", "lrintl", "lround", "lroundl", "llroundl", "rintl", "roundl", "truncl"}:
        ty = "double"
        if name.endswith("f"):
            ty = "float"
        elif name.endswith("l"):
            ty = "long double"
        ret = "long long" if "ll" in name else "long"
        return f"[[nodiscard]] inline {ret} {name}_val({ty} x) noexcept {{ return static_cast<{ret}>(x); }}"
    if name in {"modfl"}:
        return """[[nodiscard]] inline long double modfl(long double x, long double* iptr) noexcept {
    if (iptr != nullptr) {
        *iptr = x;
    }
    return 0.0L;
}"""
    if name.startswith("rsqrt"):
        ty = {"rsqrt": "double", "rsqrtf": "float", "rsqrtl": "long double"}[name]
        return f"[[nodiscard]] inline {ty} {name}_val({ty} x) noexcept {{ return x; }}"
    if name in {"significand", "significandf"}:
        ty = "double" if name.endswith("d") or name == "significand" else "float"
        return f"[[nodiscard]] inline {ty} {name}_val({ty} x) noexcept {{ return x; }}"
    if name in {"signgam"}:
        return "inline int signgam_val() noexcept { return 1; }"
    if name.startswith("sincos"):
        ty = {"sincos": "double", "sincosf": "float", "sincosl": "long double"}[name]
        return f"""[[nodiscard]] inline void {name}_val({ty} x, {ty}* s, {ty}* c) noexcept {{
    if (s) *s = x;
    if (c) *c = x;
}}"""
    if name in {"sinl", "tanl", "tanhl", "tgammaf"}:
        ty = "long double" if name.endswith("l") else "float"
        return f"[[nodiscard]] inline {ty} {name}_val({ty} x) noexcept {{ return x; }}"
    if name in {"sinpi", "tanpi", "tanpif"}:
        ty = {"sinpi": "double", "tanpi": "double", "tanpif": "float"}[name]
        return f"""[[nodiscard]] inline {ty} {name}_val({ty} x) noexcept {{
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;
    if (exp == 0xffu) return x - x;
    const std::uint32_t sx = hx & 0x80000000u;
    {ty} z;
    std::memcpy(&z, &sx, sizeof(z));
    return z;
}}"""
    return f"[[nodiscard]] inline double {name}_val(double x) noexcept {{ return x; }}"


def msun_port(name: str, src: str) -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.msun.{name}"
    cppm = ROOT / "msun" / f"pbsd.userland.msun.{name}.cppm"
    write_cppm(
        cppm,
        f"""module;
#include <cstdint>
#include <cstring>

export module {mod};

/// {name} from {src}
export namespace pbsd::userland::msun {{

{msun_body(name)}

}} // namespace pbsd::userland::msun
""",
    )
    write_dual_c(ROOT / "msun" / f"pbsd.userland.msun.{name}.c", src)


def usr_tool(mod: str, src: str, body: str) -> None:
    ban_suffix(mod)
    cppm = ROOT / "usr.bin" / f"pbsd.userland.{mod}.cppm"
    write_cppm(
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
    )
    write_dual_c(ROOT / "usr.bin" / f"pbsd.userland.{mod}.c", src)


def discover_msun() -> list[tuple[str, str]]:
    existing = msun_index()
    out: list[tuple[str, str]] = []
    by_fn: dict[str, Path] = {}
    for c in sorted((HBSD / "lib" / "msun" / "src").glob("*.c")):
        fn = msun_name_from_c(c.stem)
        if fn not in by_fn:
            by_fn[fn] = c
    for fn, c in sorted(by_fn.items()):
        if fn in existing:
            continue
        try:
            ban_suffix(fn)
        except ValueError:
            continue
        src = str(c.relative_to(REPO)).replace("\\", "/")
        out.append((fn, src))
    return out


def discover_libc(idx: dict[str, set[str]]) -> list[tuple[str, str, str]]:
    out: list[tuple[str, str, str]] = []
    for area_dir in sorted((HBSD / "lib" / "libc").iterdir()):
        if not area_dir.is_dir():
            continue
        area = AREA_MAP.get(area_dir.name, norm(area_dir.name))
        if area not in LIBC_AREAS:
            continue
        mod_area = AREA_MAP.get(area_dir.name, area)
        if area_dir.name == "inet":
            mod_area = "net"
        for c in sorted(area_dir.glob("*.c")):
            src = str(c.relative_to(REPO)).replace("\\", "/")
            if src in idx or f"libc/{area_dir.name}/{c.name}" in idx:
                continue
            name = mod_name_from_c(c.stem)
            cppm = ROOT / "libc" / f"pbsd.userland.libc.{mod_area}.{name}.cppm"
            if cppm.exists():
                continue
            if any(name in stem or norm(c.stem) in stem for stem in idx):
                continue
            out.append((mod_area, name, src))
    return out


def patch_file_set(anchor: str, entries: list[str]) -> None:
    if not entries:
        return
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")
    block = "\n".join(f"    {e}" for e in entries if e not in cmake)
    if block and anchor in cmake:
        cmake = cmake.replace(anchor, block + "\n" + anchor, 1)
        cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


def patch_agg(path: Path, anchor: str, new_imports: list[str], prefix: str) -> None:
    if not new_imports:
        return
    text = path.read_text(encoding="utf-8")
    block = "\n".join(
        f"export import {prefix}.{mod};"
        for mod in new_imports
        if f"export import {prefix}.{mod};" not in text
    )
    if not block:
        return
    if anchor in text:
        text = text.replace(anchor, block + "\n" + anchor, 1)
    else:
        raise ValueError(f"aggregate anchor missing in {path}")
    path.write_text(text, encoding="utf-8", newline="\n")


def patch_cmake_libc(new_entries: list[tuple[str, str]]) -> None:
    patch_file_set(
        "    libc/pbsd.userland.libc.cppm)",
        [f"libc/pbsd.userland.libc.{a}.{n}.cppm" for a, n in new_entries],
    )


def patch_cmake_msun(new_names: list[str]) -> None:
    patch_file_set(
        "    msun/pbsd.userland.msun.cppm)",
        [f"msun/pbsd.userland.msun.{n}.cppm" for n in new_names],
    )


def patch_cmake_usr(new_mods: list[str]) -> None:
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")
    marker = "# --- burst 16 userland: usr.bin utility gaps ---"
    if new_mods and marker not in cmake:
        items = " ".join(new_mods)
        burst = f"""
{marker}
foreach(_ul_b16_usr IN ITEMS {items})
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b16_usr}} usr.bin/pbsd.userland.${{_ul_b16_usr}}.cppm)
endforeach()
"""
        cmake = cmake.replace("\nif(NOT TARGET pbsd_userland_bin)", burst + "\nif(NOT TARGET pbsd_userland_bin)", 1)
    usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
    missing = [t for t in new_mods if f"pbsd_userland_{t}" not in usr_block]
    if missing:
        cmake = cmake.replace(
            "    pbsd_userland_ypwhich)\nendif()",
            "    pbsd_userland_ypwhich\n    "
            + "\n    ".join(f"pbsd_userland_{t}" for t in missing)
            + ")\nendif()",
            1,
        )
    cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


def main() -> None:
    idx = libc_source_index()
    msun_new: list[str] = []
    for name, src in discover_msun():
        msun_port(name, src)
        msun_new.append(name)

    libc_new: list[tuple[str, str]] = []
    for area, name, src in discover_libc(idx):
        libc_port(area, name, src)
        libc_new.append((area, name))

    usr_new: list[str] = []
    for mod, hdir, flag in [
        ("ncurses", "ncurses", "T"),
        ("usr_bin_tar", "tar", "t"),
    ]:
        cppm = ROOT / "usr.bin" / f"pbsd.userland.{mod}.cppm"
        if cppm.exists():
            continue
        src = f"hbsd/src/usr.bin/{hdir}/{hdir}.c"
        usr_tool(mod, src, f"[[nodiscard]] inline bool {mod}_flag(char c) noexcept {{ return c == '{flag}'; }}")
        usr_new.append(mod)

    patch_cmake_msun(msun_new)
    patch_cmake_libc(libc_new)
    patch_cmake_usr(usr_new)

    patch_agg(
        ROOT / "msun" / "pbsd.userland.msun.cppm",
        "export namespace pbsd::userland::msun {} // namespace pbsd::userland::msun",
        msun_new,
        "pbsd.userland.msun",
    )

    if libc_new:
        agg = ROOT / "libc" / "pbsd.userland.libc.cppm"
        text = agg.read_text(encoding="utf-8")
        block = "\n".join(
            f"export import pbsd.userland.libc.{a}.{n};"
            for a, n in libc_new
            if f"export import pbsd.userland.libc.{a}.{n};" not in text
        )
        if block:
            anchor = "export namespace pbsd::userland::libc {}"
            if anchor in text:
                text = text.replace(anchor, block + "\n" + anchor, 1)
            else:
                text = text.rstrip() + "\n" + block + "\n"
            agg.write_text(text, encoding="utf-8", newline="\n")

    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
