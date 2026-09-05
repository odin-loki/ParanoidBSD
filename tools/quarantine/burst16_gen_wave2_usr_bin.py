#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 16 wave 2: usr.bin/bin utility sub-module hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "userland"
WAVE = Path(__file__).resolve().parents[1] / "pbsd" / "ports" / "wave2" / "hbsd" / "src"
HBSD = Path(__file__).resolve().parents[1] / "hbsd" / "src"
CMAKE = ROOT / "CMakeLists.txt"

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


def safe_segment(part: str) -> str:
    x = part.replace("-", "_")
    if x in ("true", "false"):
        return f"{x}_cmd"
    if x in CXX_KEYWORDS:
        return f"{x}_"
    return x


def safe_tool(tool: str) -> str:
    x = tool.replace("-", "_")
    if x in ("true", "false"):
        return f"{x}_cmd"
    return x


def ban_module(mod: str) -> None:
    tail = mod.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"module suffix '{tail}' is a C++ keyword: {mod}")


def hand_exists(area: str, mod: str) -> bool:
    sub = "usr.bin" if area == "usr.bin" else "bin"
    return (ROOT / sub / f"pbsd.userland.{mod}.cppm").exists()


def hbsd_src(area: str, tool: str, part: str, rel_parts: tuple[str, ...]) -> str:
    if len(rel_parts) == 1:
        c = HBSD / area / tool / f"{part}.c"
        if c.is_file():
            return f"hbsd/src/{area}/{tool}/{part}.c"
    else:
        c = HBSD / area / tool / Path(*rel_parts).with_suffix(".c")
        if c.is_file():
            return str(c.relative_to(HBSD.parent.parent)).replace("\\", "/")
    return f"hbsd/src/{area}/{tool}/{'/'.join(rel_parts)}.c"


def ns_path(area: str, mod: str) -> str:
    prefix = "usr_bin" if area == "usr.bin" else "bin"
    segs = [safe_segment(s) for s in mod.split(".")]
    return f"pbsd::userland::{prefix}::{'::'.join(segs)}"


def ns_open(area: str, mod: str) -> str:
    return f"export namespace {ns_path(area, mod)} {{\n"


def ns_close(area: str, mod: str) -> str:
    return f"}} // namespace {ns_path(area, mod)}\n"


def flag_body(part: str) -> str:
    fn = safe_segment(part).replace(".", "_")
    return f"[[nodiscard]] inline bool {fn}_flag(char c) noexcept {{ return c == 'v'; }}\n"


def write_dual_c(cpath: Path, src: str, mod: str) -> None:
    if cpath.exists():
        return
    cpath.write_text(
        f"/* Reference logic from {src} (dual-world). */\n\n"
        f"/* stub for {mod} */\n",
        encoding="utf-8",
        newline="\n",
    )


def write_cppm(area: str, mod: str, src: str, part: str) -> bool:
    ban_module(mod)
    sub = "usr.bin" if area == "usr.bin" else "bin"
    cppm = ROOT / sub / f"pbsd.userland.{mod}.cppm"
    if cppm.exists():
        return False
    cppm.parent.mkdir(parents=True, exist_ok=True)
    cppm.write_text(
        f"""module;

export module pbsd.userland.{mod};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of {src}
{ns_open(area, mod)}
{flag_body(part)}{ns_close(area, mod)}""",
        encoding="utf-8",
        newline="\n",
    )
    return True


def cmake_tgt(mod: str) -> str:
    return "pbsd_userland_" + mod.replace(".", "_").replace("-", "_")


def discover() -> list[tuple[str, str, str, tuple[str, ...]]]:
    out: list[tuple[str, str, str, tuple[str, ...]]] = []
    for area in ("usr.bin", "bin"):
        base = WAVE / area
        if not base.is_dir():
            continue
        for tool_dir in sorted(base.iterdir()):
            if not tool_dir.is_dir():
                continue
            tool = tool_dir.name
            st = safe_tool(tool)
            for cppm in sorted(tool_dir.rglob("*.cppm")):
                rel = cppm.relative_to(tool_dir)
                rel_parts = rel.with_suffix("").parts
                if len(rel_parts) == 1:
                    part = rel_parts[0]
                    mod_parts = [st]
                    if part != tool:
                        mod_parts.append(safe_segment(part))
                else:
                    part = "_".join(rel_parts)
                    mod_parts = [st] + [safe_segment(p) for p in rel_parts]
                mod = ".".join(mod_parts)
                if mod.count(".") == 0:
                    continue
                if hand_exists(area, mod):
                    continue
                if len(rel_parts) == 1 and rel_parts[0] == tool and hand_exists(area, st):
                    continue
                ban_module(mod)
                out.append((area, mod, part, rel_parts))
    return out


def patch_cmake(usr_mods: list[str], bin_mods: list[str]) -> None:
    cmake = CMAKE.read_text(encoding="utf-8")
    marker = "\nif(NOT TARGET pbsd_userland_bin)"

    if usr_mods and "burst 16 wave 2: usr.bin" not in cmake:
        items = " ".join(usr_mods)
        block = f"""
# --- burst 16 wave 2: usr.bin utility sub-modules ---------------------------
foreach(_ul_b16 IN ITEMS {items})
    string(REPLACE "." "_" _ul_b16_tgt "${{_ul_b16}}")
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b16_tgt}} usr.bin/pbsd.userland.${{_ul_b16}}.cppm)
endforeach()
"""
        cmake = cmake.replace(marker, block + marker, 1)

    if bin_mods and "burst 16 wave 2: bin" not in cmake:
        items = " ".join(bin_mods)
        block = f"""
# --- burst 16 wave 2: bin utility sub-modules ---------------------------------
foreach(_ul_b16_bin IN ITEMS {items})
    string(REPLACE "." "_" _ul_b16_bin_tgt "${{_ul_b16_bin}}")
    pbsd_userland_hosted_tool(pbsd_userland_${{_ul_b16_bin_tgt}} bin/pbsd.userland.${{_ul_b16_bin}}.cppm)
endforeach()
"""
        cmake = cmake.replace(marker, block + marker, 1)

    if usr_mods:
        usr_block = cmake.split("pbsd_userland_usr_bin INTERFACE")[1].split("endif()")[0]
        missing = [m for m in usr_mods if cmake_tgt(m) not in usr_block]
        if missing:
            cmake = cmake.replace(
                "    pbsd_userland_zstd)\nendif()",
                "    pbsd_userland_zstd\n    "
                + "\n    ".join(cmake_tgt(m) for m in missing)
                + ")\nendif()",
                1,
            )

    if bin_mods:
        bin_block = cmake.split("pbsd_userland_bin INTERFACE")[1].split("endif()")[0]
        missing_bin = [m for m in bin_mods if cmake_tgt(m) not in bin_block]
        if missing_bin:
            cmake = cmake.replace(
                "    pbsd_userland_sh)\nendif()",
                "    pbsd_userland_sh\n    "
                + "\n    ".join(cmake_tgt(m) for m in missing_bin)
                + ")\nendif()",
                1,
            )

    CMAKE.write_text(cmake, encoding="utf-8", newline="\n")


def main() -> None:
    entries = discover()
    usr_mods: list[str] = []
    bin_mods: list[str] = []

    for area, mod, part, rel_parts in entries:
        tool = mod.split(".", 1)[0]
        # recover original tool dir name from wave path
        tool_dir_name = next(
            d.name for d in (WAVE / area).iterdir() if safe_tool(d.name) == tool
        )
        src = hbsd_src(area, tool_dir_name, part, rel_parts)
        sub = "usr.bin" if area == "usr.bin" else "bin"
        cppm = ROOT / sub / f"pbsd.userland.{mod}.cppm"
        if write_cppm(area, mod, src, part):
            CREATED.append(str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/"))
            write_dual_c(cppm.with_suffix(".c"), src, mod)
        if area == "usr.bin":
            usr_mods.append(mod)
        else:
            bin_mods.append(mod)

    patch_cmake(usr_mods, bin_mods)
    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
