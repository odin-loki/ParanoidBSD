#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 16 wave 2 — kde/theme/compositor/stand/arch/bifrost hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"

CREATED: list[str] = []
CMAKE_TOUCHED: list[str] = []

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

# CMake wiring queues
KDE_PLASMA_APPEND: list[str] = []
KDE_FRAMEWORKS_APPEND: list[str] = []
THEME_APPEND: list[str] = []
COMPOSITOR_TARGETS: list[tuple[str, str, str]] = []  # (target, cppm, deps)
STAND_TARGETS: list[tuple[str, str, str]] = []  # (target, cppm, extra deps)
ARCH_TARGETS: list[tuple[str, str, str, str]] = []  # (target, rel, mod, extra deps)
BIFROST_TARGETS: list[tuple[str, str, str]] = []


def ban_suffix(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"module suffix '{tail}' is a C++ keyword")


def write(rel: str, body: str, c_stub: str | None = None) -> bool:
    path = PBSD / rel
    if path.exists():
        return False
    ban_suffix(path.stem.split(".")[-1])
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body.strip() + "\n", encoding="utf-8")
    rel_norm = rel.replace("\\", "/")
    CREATED.append(rel_norm)
    print("wrote", rel_norm)
    if c_stub is not None:
        c_path = path.with_suffix(".c")
        if not c_path.exists():
            c_path.write_text(c_stub, encoding="utf-8")
            CREATED.append(str(c_path.relative_to(PBSD)).replace("\\", "/"))
    return True


def make_c_stub(upstream: str, name: str) -> str:
    return f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for {name} */\n"


def glue(name: str, upstream: str, imports: str, extra: str, ns_path: str) -> str:
    ns = name.replace(".", "::")
    return f"""export module pbsd.kde.plasma.{name}.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
{imports}

/// Burst 16 wave 2 — Plasma {name} ↔ Aero glue.
/// Upstream: {upstream}
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::{ns}::glue {{

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

{extra}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return {ns_path}::upstream_path();
}}

}} // namespace pbsd::kde::plasma::{ns}::glue
"""


def comp_layer_mod(stem: str, upstream: str, body: str) -> None:
    mod = f"pbsd.compositor.wayland.layer.{stem}"
    rel = f"compositor/{mod}.cppm"
    if write(
        rel,
        f"""module;

export module {mod};

import pbsd.core;
import pbsd.compositor.wayland.layer;

/// Burst 16 wave 2 — layer-shell {stem} (native compositor).
/// Upstream: {upstream}
export namespace pbsd::compositor::wayland::layer::{stem} {{

{body}

}} // namespace pbsd::compositor::wayland::layer::{stem}
""",
        make_c_stub(upstream, mod),
    ):
        tgt = f"pbsd_compositor_wayland_layer_{stem.replace('.', '_')}"
        COMPOSITOR_TARGETS.append((tgt, rel, "pbsd_core pbsd_compositor_wayland_layer"))


def comp_mod(stem: str, upstream: str, body: str) -> None:
    mod = f"pbsd.compositor.wayland.{stem}"
    rel = f"compositor/{mod}.cppm"
    if write(
        rel,
        f"""module;

export module {mod};

import pbsd.core;

/// Burst 16 wave 2 — {upstream} (native compositor).
export namespace pbsd::compositor::wayland::{stem} {{

{body}

}} // namespace pbsd::compositor::wayland::{stem}
""",
        make_c_stub(f"protocols/{upstream}", mod),
    ):
        tgt = f"pbsd_compositor_wayland_{stem.replace('.', '_')}"
        COMPOSITOR_TARGETS.append((tgt, rel, "pbsd_core"))


def theme_mod(stem: str, body: str, upstream: str) -> None:
    rel = f"theme/plasma/aero.{stem}.cppm"
    if write(
        rel,
        f"""export module pbsd.theme.plasma.aero.{stem};

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 wave 2 — Aero {stem.replace('_', ' ')} visual tokens.
export namespace pbsd::theme::plasma::aero::{stem} {{

{body}

}} // namespace pbsd::theme::plasma::aero::{stem}
""",
        make_c_stub(upstream, f"aero.{stem}"),
    ):
        THEME_APPEND.append(f"plasma/aero.{stem}.cppm")


def stand_mod(stem: str, upstream: str, body: str, deps: str = "pbsd_core") -> None:
    rel = f"stand/pbsd.stand.{stem}.cppm"
    if write(
        rel,
        f"""module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.{stem};

import pbsd.core;

/// PROVENANCE: {upstream}
export namespace pbsd::stand::{stem} {{

{body}

}} // namespace pbsd::stand::{stem}
""",
    ):
        STAND_TARGETS.append((f"pbsd_stand_{stem}", rel, deps))


def arch_mod(rel: str, mod: str, ns: str, upstream: str, body: str, deps: str = "pbsd_core") -> None:
    if write(
        rel,
        f"""module;
#include <cstdint>

export module {mod};

import pbsd.core;

/// PROVENANCE: {upstream}
export namespace pbsd::arch::{ns} {{

{body}

}} // namespace pbsd::arch::{ns}
""",
    ):
        tgt = mod.replace(".", "_").replace("pbsd_", "pbsd_")
        ARCH_TARGETS.append((tgt, rel, mod, deps))


def bif_mod(stem: str, upstream: str, body: str, deps: str = "pbsd_core") -> None:
    rel = f"bifrost/pbsd.bifrost.{stem}.cppm"
    if write(
        rel,
        f"""module;
#include <cstdint>

export module pbsd.bifrost.{stem};

import pbsd.core;

/// PROVENANCE: {upstream}
export namespace pbsd::bifrost::{stem} {{

{body}

}} // namespace pbsd::bifrost::{stem}
""",
    ):
        BIFROST_TARGETS.append((f"pbsd_bifrost_{stem}", rel, deps))


def patch_list(cmake: Path, marker: str, entries: list[str]) -> None:
    if not entries:
        return
    text = cmake.read_text(encoding="utf-8")
    for entry in entries:
        if entry in text:
            continue
        text = text.replace(marker, f"    {entry}\n{marker}", 1)
    cmake.write_text(text, encoding="utf-8")
    CMAKE_TOUCHED.append(str(cmake.relative_to(ROOT)).replace("\\", "/"))


def patch_compositor_cmake() -> None:
    cmake = PBSD / "compositor" / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    changed = False
    insert_before = "if(NOT TARGET pbsd_compositor_frame_pacing)"
    blocks: list[str] = []
    link_append: list[str] = []
    for tgt, rel, deps in COMPOSITOR_TARGETS:
        cppm = rel.split("/", 1)[1]
        if f"if(NOT TARGET {tgt})" in text:
            continue
        dep_list = deps.split()
        dep_links = " ".join(dep_list)
        blocks.append(
            f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES {cppm})
target_link_libraries({tgt} PUBLIC {dep_links})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()

"""
        )
        link_append.append(f"    {tgt}")
        changed = True
    if not changed:
        return
    text = text.replace(insert_before, "".join(blocks) + insert_before, 1)
    for tgt, _, _ in COMPOSITOR_TARGETS:
        needle = f"    {tgt}\n"
        if needle not in text:
            text = text.replace(
                "    pbsd_compositor_frame_pacing pbsd_compositor_layer_sync",
                f"    pbsd_compositor_frame_pacing pbsd_compositor_layer_sync\n    {tgt}",
                1,
            )
    cmake.write_text(text, encoding="utf-8")
    CMAKE_TOUCHED.append(str(cmake.relative_to(ROOT)).replace("\\", "/"))


def patch_stand_cmake() -> None:
    cmake = PBSD / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    changed = False
    for tgt, rel, deps in STAND_TARGETS:
        cppm = rel
        if f"if(NOT TARGET {tgt})" in text:
            continue
        dep_links = deps
        block = f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES {cppm})
target_link_libraries({tgt} PUBLIC {dep_links})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()

"""
        anchor = "add_library(pbsd_stand)\n"
        text = text.replace(anchor, block + anchor, 1)
        short = tgt.replace("pbsd_stand_", "pbsd_stand_")
        text = text.replace(
            "    pbsd_stand_lua pbsd_stand_nullfs)",
            f"    pbsd_stand_lua pbsd_stand_nullfs {short})",
            1,
        )
        changed = True
    if changed:
        cmake.write_text(text, encoding="utf-8")
        CMAKE_TOUCHED.append(str(cmake.relative_to(ROOT)).replace("\\", "/"))


def patch_arch_cmake() -> None:
    cmake = PBSD / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    changed = False
    for tgt, rel, _mod, deps in ARCH_TARGETS:
        if f"if(NOT TARGET {tgt})" in text:
            continue
        block = f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES
    {rel})
target_link_libraries({tgt} PUBLIC {deps})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()

"""
        if "amd64" in rel:
            anchor = "add_library(pbsd_arch_amd64)\n"
            agg = "    pbsd_arch_amd64_pat)\n"
            insert = f"    {tgt}\n"
            if insert.strip() not in text.split("add_library(pbsd_arch_amd64)")[1].split("target_compile_options(pbsd_arch_amd64")[0]:
                text = text.replace(agg, agg.replace(")", f" {tgt})"), 1)
        else:
            anchor = "add_library(pbsd_arch_arm64)\n"
            agg = "    pbsd_arch_arm64_mmu)\n"
            text = text.replace(agg, agg.replace(")", f" {tgt})"), 1)
        text = text.replace(anchor, block + anchor, 1)
        changed = True
    if changed:
        cmake.write_text(text, encoding="utf-8")
        if str(cmake.relative_to(ROOT)).replace("\\", "/") not in CMAKE_TOUCHED:
            CMAKE_TOUCHED.append(str(cmake.relative_to(ROOT)).replace("\\", "/"))


def patch_bifrost_cmake() -> None:
    cmake = PBSD / "bifrost" / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    changed = False
    insert_before = "add_library(pbsd_bifrost)\n"
    blocks: list[str] = []
    for tgt, rel, deps in BIFROST_TARGETS:
        cppm = rel.split("/", 1)[1]
        if f"if(NOT TARGET {tgt})" in text:
            continue
        blocks.append(
            f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES {cppm})
target_link_libraries({tgt} PUBLIC {deps})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()

"""
        )
        short = tgt
        text = text.replace(
            "    pbsd_bifrost_migration pbsd_bifrost_timer)",
            f"    pbsd_bifrost_migration pbsd_bifrost_timer {short})",
            1,
        )
        changed = True
    if changed:
        text = text.replace(insert_before, "".join(blocks) + insert_before, 1)
        cmake.write_text(text, encoding="utf-8")
        CMAKE_TOUCHED.append(str(cmake.relative_to(ROOT)).replace("\\", "/"))


def main() -> None:
    # --- KDE plasma glue (string constants, no layershellintegration) ---------
    glue_specs = [
        (
            "tastenbrett.application",
            "kde/plasma-desktop/kcms/keyboard/tastenbrett/application.cpp",
            "import pbsd.kde.plasma.tastenbrett.application;",
            "::pbsd::kde::plasma::tastenbrett::application",
            """struct LayoutStyle {
    const char* section_key{"application"};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline LayoutStyle default_style() noexcept { return LayoutStyle{}; }""",
        ),
        (
            "tastenbrett.geometry",
            "kde/plasma-desktop/kcms/keyboard/tastenbrett/geometry.cpp",
            "import pbsd.kde.plasma.tastenbrett.geometry;",
            "::pbsd::kde::plasma::tastenbrett::geometry",
            """struct GeometryStyle {
    unsigned row_height_px{32};
    unsigned key_spacing_px{4};
};

[[nodiscard]] inline Status validate_row_height(unsigned px) noexcept {
    return px >= 16 && px <= 64 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "tastenbrett.outline",
            "kde/plasma-desktop/kcms/keyboard/tastenbrett/outline.cpp",
            "import pbsd.kde.plasma.tastenbrett.outline;",
            "::pbsd::kde::plasma::tastenbrett::outline",
            """struct OutlineStyle {
    float stroke_opacity{0.85f};
    unsigned corner_radius_px{3};
};

[[nodiscard]] inline OutlineStyle default_style() noexcept { return OutlineStyle{}; }""",
        ),
        (
            "tastenbrett.row",
            "kde/plasma-desktop/kcms/keyboard/tastenbrett/row.cpp",
            "import pbsd.kde.plasma.tastenbrett.row;",
            "::pbsd::kde::plasma::tastenbrett::row",
            """struct RowStyle {
    unsigned max_keys{20};
    float label_opacity{0.90f};
};

[[nodiscard]] inline Status validate_max_keys(unsigned n) noexcept {
    return n > 0 && n <= 32 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "tastenbrett.shape",
            "kde/plasma-desktop/kcms/keyboard/tastenbrett/shape.cpp",
            "import pbsd.kde.plasma.tastenbrett.shape;",
            "::pbsd::kde::plasma::tastenbrett::shape",
            """struct ShapeStyle {
    unsigned min_width_px{24};
    unsigned min_height_px{24};
};

[[nodiscard]] inline ShapeStyle default_style() noexcept { return ShapeStyle{}; }""",
        ),
        (
            "touchscreen.moduledata",
            "kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp",
            "import pbsd.kde.plasma.touchscreen.moduledata;",
            "::pbsd::kde::plasma::touchscreen::moduledata",
            """struct TouchscreenModuleStyle {
    const char* group{moduledata::kTouchscreenGroup};
    unsigned max_devices{moduledata::kMaxDevices};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline TouchscreenModuleStyle default_style() noexcept {
    return TouchscreenModuleStyle{};
}""",
        ),
        (
            "baloo.baloodata",
            "kde/plasma-desktop/kcms/kcm_baloo/baloodata.cpp",
            "import pbsd.kde.plasma.baloo.baloodata;",
            "::pbsd::kde::plasma::baloo::baloodata",
            """struct BalooDataStyle {
    bool file_indexing{true};
    bool email_indexing{false};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline BalooDataStyle default_style() noexcept { return BalooDataStyle{}; }""",
        ),
        (
            "emojier.category",
            "kde/plasma-desktop/applets/emojier/category.cpp",
            "import pbsd.kde.plasma.emojier.category;",
            "::pbsd::kde::plasma::emojier::category",
            """struct EmojiCategoryStyle {
    unsigned max_recent{32};
    float popup_opacity{0.88f};
};

[[nodiscard]] inline EmojiCategoryStyle default_style() noexcept { return EmojiCategoryStyle{}; }""",
        ),
        (
            "keyboard.layoutnames",
            "kde/plasma-desktop/kcms/keyboard/layoutnames.cpp",
            "import pbsd.kde.plasma.keyboard.layoutnames;",
            "::pbsd::kde::plasma::keyboard::layoutnames",
            """struct LayoutNameStyle {
    const char* settings_svg{aero::kSettingsSvg};
    unsigned max_layouts{8};
};

[[nodiscard]] inline Status validate_layout_count(unsigned n) noexcept {
    return n <= 8 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "runners.abstractjob",
            "kde/plasma-desktop/kcms/runners/plugininstaller/AbstractJob.cpp",
            "import pbsd.kde.plasma.runners.abstractjob;",
            "::pbsd::kde::plasma::runners::abstractjob",
            """struct RunnerJobStyle {
    const char* pending{abstractjob::kJobStatePending};
    const char* finished{abstractjob::kJobStateFinished};
};

[[nodiscard]] inline RunnerJobStyle default_style() noexcept { return RunnerJobStyle{}; }""",
        ),
        (
            "runners.zypperrpmjob",
            "kde/plasma-desktop/kcms/runners/plugininstaller/ZypperRpmJob.cpp",
            "import pbsd.kde.plasma.runners.zypperrpmjob;",
            "::pbsd::kde::plasma::runners::zypperrpmjob",
            """struct ZypperJobStyle {
    const char* package_manager{zypperrpmjob::kPackageManager};
    const char* package_suffix{zypperrpmjob::kRpmExtension};
    unsigned timeout_sec{120};
};

[[nodiscard]] inline Status validate_timeout(unsigned sec) noexcept {
    return sec > 0 && sec <= 600 ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for name, upstream, imports, ns_path, extra in glue_specs:
        rel = f"kde/plasma.{name}.glue.cppm"
        if write(rel, glue(name, upstream, imports, extra, ns_path), make_c_stub(upstream, f"{name}_glue")):
            KDE_PLASMA_APPEND.append(f"plasma.{name}.glue.cppm")

    # --- KDE frameworks layershell siblings (string constants only) -----------
    fw_specs = [
        (
            "layershell.interfaces_region.cppm",
            """export module pbsd.kde.layershell.interfaces_region;

import pbsd.core;

/// Burst 16 wave 2 — layer-shell region interface stub (string constants).
export namespace pbsd::kde::frameworks::layershell::interfaces_region {

inline constexpr const char kInterfaceId[] = "org.kde.layershell.region";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";
inline constexpr const char kIntegrationPlugin[] = "layer-shell-qt";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/region.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_region
""",
            "kde/frameworks/layer-shell-qt/src/interfaces/region.cpp",
        ),
        (
            "layershell.interfaces_output.cppm",
            """export module pbsd.kde.layershell.interfaces_output;

import pbsd.core;

/// Burst 16 wave 2 — layer-shell output interface stub (string constants).
export namespace pbsd::kde::frameworks::layershell::interfaces_output {

inline constexpr const char kInterfaceId[] = "org.kde.layershell.output";
inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/output.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_output
""",
            "kde/frameworks/layer-shell-qt/src/interfaces/output.cpp",
        ),
    ]
    for fname, body, upstream in fw_specs:
        rel = f"kde/frameworks/{fname}"
        if write(rel, body, make_c_stub(upstream, fname)):
            KDE_FRAMEWORKS_APPEND.append(f"frameworks/{fname}")

    # --- Compositor wayland::layer:: submodules -------------------------------
    layer_specs = [
        (
            "configure",
            "wlr-layer-shell-unstable-v1.xml",
            """enum class ConfigureField : unsigned char {
    Width = 0,
    Height = 1,
    Serial = 2,
};

[[nodiscard]] inline Status validate_size(unsigned w, unsigned h) noexcept {
    if (w == 0 || h == 0 || w > 8192 || h > 8192) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "popup",
            "wlr-layer-shell-unstable-v1.xml",
            """enum class PopupRequest : unsigned char {
    GetPopup = 0,
};

[[nodiscard]] inline bool is_popup_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(PopupRequest::GetPopup);
}""",
        ),
        (
            "margin",
            "wlr-layer-shell-unstable-v1.xml",
            """struct Margin {
    int top{};
    int right{};
    int bottom{};
    int left{};
};

[[nodiscard]] inline Status validate_margin(const Margin& m) noexcept {
    if (m.top < -512 || m.bottom < -512 || m.left < -512 || m.right < -512) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "keyboard",
            "wlr-layer-shell-unstable-v1.xml",
            """[[nodiscard]] inline Status validate_interactivity(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(::pbsd::compositor::wayland::layer::KeyboardInteractivity::OnDemand)
        ? Status::Ok : Status::Protocol;
}""",
        ),
    ]
    for stem, upstream, body in layer_specs:
        comp_layer_mod(stem, upstream, body)

    comp_specs = [
        (
            "color_management_v1",
            "color-management-v1.xml",
            """inline constexpr const char kInterface[] = "wp_color_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetOutput = 1,
    GetSurface = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetSurface);
}""",
        ),
        (
            "xdg_dialog_v1",
            "xdg-dialog-v1.xml",
            """inline constexpr const char kManagerInterface[] = "xdg_dialog_manager_v1";
inline constexpr const char kDialogInterface[] = "xdg_dialog_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetDialog = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetDialog);
}""",
        ),
        (
            "wp_commit_timing_v1",
            "wp-commit-timing-v1.xml",
            """inline constexpr const char kInterface[] = "wp_commit_timing_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetTiming = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetTiming);
}""",
        ),
        (
            "ext_workspace_v1",
            "ext-workspace-v1.xml",
            """inline constexpr const char kManagerInterface[] = "ext_workspace_manager_v1";
inline constexpr const char kHandleInterface[] = "ext_workspace_handle_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Commit = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Commit);
}""",
        ),
    ]
    for stem, upstream, body in comp_specs:
        comp_mod(stem, upstream, body)

    # --- Theme ----------------------------------------------------------------
    theme_specs = [
        (
            "action_center",
            """struct ActionCenterTokens {
    unsigned panel_width_px{360};
    float backdrop_opacity{0.55f};
    const char* panel_svg{"plasma/panel/action-center.svg"};
};

[[nodiscard]] inline ActionCenterTokens default_tokens() noexcept {
    return ActionCenterTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "magnifier",
            """struct MagnifierTokens {
    unsigned zoom_percent{200};
    unsigned lens_size_px{128};
    float border_opacity{0.70f};
};

[[nodiscard]] inline MagnifierTokens default_tokens() noexcept {
    return MagnifierTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "screen_reader",
            """struct ScreenReaderTokens {
    unsigned caret_width_px{2};
    float highlight_opacity{0.40f};
    const char* focus_svg{"plasma/widgets/focus-ring.svg"};
};

[[nodiscard]] inline ScreenReaderTokens default_tokens() noexcept {
    return ScreenReaderTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "immersive",
            """struct ImmersiveTokens {
    float chrome_opacity{0.0f};
    unsigned hide_delay_ms{3000};
    bool auto_hide_panel{true};
};

[[nodiscard]] inline ImmersiveTokens default_tokens() noexcept {
    return ImmersiveTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "ribbon",
            """struct RibbonTokens {
    unsigned tab_height_px{28};
    unsigned group_spacing_px{8};
    const char* tab_svg{"plasma/widgets/tab.svg"};
};

[[nodiscard]] inline RibbonTokens default_tokens() noexcept {
    return RibbonTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "task_switcher",
            """struct TaskSwitcherTokens {
    unsigned thumbnail_width_px{240};
    unsigned animation_ms{180};
    float dim_opacity{0.40f};
};

[[nodiscard]] inline TaskSwitcherTokens default_tokens() noexcept {
    return TaskSwitcherTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
    ]
    for stem, body, upstream in theme_specs:
        theme_mod(stem, body, upstream)

    # --- Stand ----------------------------------------------------------------
    stand_specs = [
        ("bzip2", "hbsd/src/stand/libsa/bzipfs.c", """inline constexpr unsigned kBlockSize = 4096;
[[nodiscard]] inline Status validate_block(unsigned size) noexcept {
    return size >= 512 && size <= 65536 ? Status::Ok : Status::Invalid;
}"""),
        ("xz", "hbsd/src/stand/libsa/xz.c", """inline constexpr unsigned kDictMax = 1u << 27;
[[nodiscard]] inline Status validate_dict(unsigned dict) noexcept {
    return dict <= kDictMax ? Status::Ok : Status::Invalid;
}"""),
        ("crc", "hbsd/src/stand/libsa/crc32_libkern.c", """inline constexpr unsigned kPolynomial = 0xEDB88320u;
[[nodiscard]] inline unsigned step(unsigned crc, unsigned byte) noexcept {
    return (crc >> 8) ^ byte;
}"""),
        ("cpio", "hbsd/src/stand/libsa/cpio.c", """inline constexpr unsigned kHeaderSize = 110;
[[nodiscard]] inline Status validate_header_size(unsigned n) noexcept {
    return n >= kHeaderSize ? Status::Ok : Status::Invalid;
}"""),
        ("tar", "hbsd/src/stand/libsa/tar.c", """inline constexpr unsigned kBlockSize = 512;
[[nodiscard]] inline Status validate_block_size(unsigned size) noexcept {
    return size == kBlockSize ? Status::Ok : Status::Invalid;
}"""),
        ("md5", "hbsd/src/stand/libsa/md5.c", """inline constexpr unsigned kDigestBytes = 16;
[[nodiscard]] inline Status validate_digest_len(unsigned len) noexcept {
    return len == kDigestBytes ? Status::Ok : Status::Invalid;
}"""),
        ("sha256", "hbsd/src/stand/libsa/sha256.c", """inline constexpr unsigned kDigestBytes = 32;
[[nodiscard]] inline Status validate_digest_len(unsigned len) noexcept {
    return len == kDigestBytes ? Status::Ok : Status::Invalid;
}"""),
        ("uuid", "hbsd/src/stand/libsa/uuid.c", """inline constexpr unsigned kStringLen = 36;
[[nodiscard]] inline Status validate_string_len(unsigned len) noexcept {
    return len == kStringLen ? Status::Ok : Status::Invalid;
}"""),
        ("random", "hbsd/src/stand/libsa/random.c", """[[nodiscard]] inline unsigned mix(unsigned a, unsigned b) noexcept {
    return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
}"""),
        ("split", "hbsd/src/stand/libsa/split.c", """[[nodiscard]] inline Status validate_part_count(unsigned parts) noexcept {
    return parts > 0 && parts <= 64 ? Status::Ok : Status::Invalid;
}"""),
    ]
    for stem, upstream, body in stand_specs:
        stand_mod(stem, upstream, body)

    # --- Arch -----------------------------------------------------------------
    arch_specs = [
        (
            "arch/amd64/pbsd.arch.amd64.nx.cppm",
            "pbsd.arch.amd64.nx",
            "amd64::nx",
            "hbsd/src/sys/x86/x86/nx.c",
            """inline constexpr unsigned kPageSize = 4096;
[[nodiscard]] inline bool nx_enabled(unsigned efer) noexcept {
    return (efer & (1u << 11)) != 0;
}""",
        ),
        (
            "arch/amd64/pbsd.arch.amd64.tsx.cppm",
            "pbsd.arch.amd64.tsx",
            "amd64::tsx",
            "hbsd/src/sys/x86/x86/tsx.c",
            """enum class Feature : unsigned char {
    Disabled = 0,
    Rtm = 1,
    Hle = 2,
};

[[nodiscard]] inline Status validate_feature(unsigned f) noexcept {
    return f <= static_cast<unsigned>(Feature::Hle) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/amd64/pbsd.arch.amd64.smap.cppm",
            "pbsd.arch.amd64.smap",
            "amd64::smap",
            "hbsd/src/sys/x86/x86/smap.c",
            """[[nodiscard]] inline bool smap_enabled(unsigned cr4) noexcept {
    return (cr4 & (1u << 21)) != 0;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.pac.cppm",
            "pbsd.arch.arm64.pac",
            "arm64::pac",
            "hbsd/src/sys/arm64/arm64/pac.c",
            """inline constexpr unsigned kKeyLen = 16;
[[nodiscard]] inline Status validate_key_len(unsigned len) noexcept {
    return len == kKeyLen ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.bti.cppm",
            "pbsd.arch.arm64.bti",
            "arm64::bti",
            "hbsd/src/sys/arm64/arm64/bti.c",
            """enum class Guard : unsigned char {
    None = 0,
    Bti = 1,
};

[[nodiscard]] inline Status validate_guard(unsigned g) noexcept {
    return g <= static_cast<unsigned>(Guard::Bti) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.pauth.cppm",
            "pbsd.arch.arm64.pauth",
            "arm64::pauth",
            "hbsd/src/sys/arm64/arm64/pauth.c",
            """inline constexpr unsigned kModulus = 16;
[[nodiscard]] inline Status validate_modulus(unsigned m) noexcept {
    return m == kModulus ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for rel, mod, ns, upstream, body in arch_specs:
        arch_mod(rel, mod, ns, upstream, body)

    # --- Bifrost --------------------------------------------------------------
    bif_specs = [
        (
            "msr_filter",
            "hbsd/src/sys/amd64/vmm/intel/msr_bitmap.c",
            """inline constexpr unsigned kBitmapBytes = 4096;
[[nodiscard]] inline Status validate_offset(unsigned off) noexcept {
    return off < kBitmapBytes ? Status::Ok : Status::Invalid;
}""",
            "pbsd_core pbsd_bifrost_hypervisor",
        ),
        (
            "debug",
            "hbsd/src/sys/amd64/vmm/vmm_debug.c",
            """enum class Level : unsigned char {
    Off = 0,
    Exit = 1,
    Verbose = 2,
};

[[nodiscard]] inline Status validate_level(unsigned lvl) noexcept {
    return lvl <= static_cast<unsigned>(Level::Verbose) ? Status::Ok : Status::Invalid;
}""",
            "pbsd_core",
        ),
        (
            "posted_intr",
            "hbsd/src/sys/amd64/vmm/intel/posted_intr.c",
            """inline constexpr unsigned kVectorMax = 255;
[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec <= kVectorMax ? Status::Ok : Status::Invalid;
}""",
            "pbsd_core pbsd_bifrost_hypervisor",
        ),
        (
            "pml",
            "hbsd/src/sys/amd64/vmm/intel/pml.c",
            """inline constexpr unsigned kEntryCount = 512;
[[nodiscard]] inline Status validate_index(unsigned idx) noexcept {
    return idx < kEntryCount ? Status::Ok : Status::Invalid;
}""",
            "pbsd_core pbsd_bifrost_hypervisor",
        ),
        (
            "vmfunc",
            "hbsd/src/sys/amd64/vmm/intel/vmfunc.c",
            """enum class Function : unsigned char {
    EptpSwitching = 0,
};

[[nodiscard]] inline Status validate_function(unsigned fn) noexcept {
    return fn <= static_cast<unsigned>(Function::EptpSwitching) ? Status::Ok : Status::Invalid;
}""",
            "pbsd_core pbsd_bifrost_hypervisor",
        ),
        (
            "unrestricted_guest",
            "hbsd/src/sys/amd64/vmm/intel/unrestricted_guest.c",
            """[[nodiscard]] inline bool allow_real_mode(unsigned ctrl) noexcept {
    return (ctrl & (1u << 16)) != 0;
}""",
            "pbsd_core pbsd_bifrost_hypervisor",
        ),
    ]
    for stem, upstream, body, deps in bif_specs:
        bif_mod(stem, upstream, body, deps)

    # --- CMake wiring ---------------------------------------------------------
    patch_list(
        PBSD / "kde" / "CMakeLists.txt",
        "    plasma.mouse.kapplymousetheme.glue.cppm\n)",
        KDE_PLASMA_APPEND,
    )
    patch_list(
        PBSD / "kde" / "CMakeLists.txt",
        "    frameworks/layershell.qwaylandlayershellintegrationplugin.cppm\n",
        KDE_FRAMEWORKS_APPEND,
    )
    patch_list(
        PBSD / "theme" / "CMakeLists.txt",
        "    plasma/aero.volume_mixer.cppm\n\n)",
        [f"    {e}" for e in THEME_APPEND],
    )
    patch_compositor_cmake()
    patch_stand_cmake()
    patch_arch_cmake()
    patch_bifrost_cmake()

    print(f"\nTotal created: {len(CREATED)}")
    for p in CREATED:
        print(" ", p)
    if CMAKE_TOUCHED:
        print("\nCMake touched:")
        for c in sorted(set(CMAKE_TOUCHED)):
            print(" ", c)


if __name__ == "__main__":
    main()
