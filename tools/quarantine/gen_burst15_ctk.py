#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 15 — kde/theme/compositor/stand/arch/bifrost hand ports + dual-world .c stubs."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"

CREATED: list[str] = []


def write(rel: str, body: str, c_stub: str | None = None) -> None:
    path = PBSD / rel
    if path.exists():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body.strip() + "\n", encoding="utf-8")
    CREATED.append(rel.replace("\\", "/"))
    print("wrote", rel)
    if c_stub is not None:
        c_path = path.with_suffix(".c")
        if not c_path.exists():
            c_path.write_text(c_stub, encoding="utf-8")
            CREATED.append(str(c_path.relative_to(PBSD)).replace("\\", "/"))


def make_c_stub(upstream: str, name: str) -> str:
    return f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for {name} */\n"


def glue(name: str, upstream: str, imports: str, extra: str, ns_path: str) -> str:
    ns = name.replace(".", "::")
    return f"""export module pbsd.kde.plasma.{name}.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
{imports}

/// Burst 15 — Plasma {name} ↔ Aero glue.
/// Upstream: {upstream}
export namespace pbsd::kde::plasma::{ns}::glue {{

{extra}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return {ns_path}::upstream_path();
}}

}} // namespace pbsd::kde::plasma::{ns}::glue
"""


def comp_mod(stem: str, upstream: str, body: str) -> None:
    mod = f"pbsd.compositor.wayland.{stem}"
    write(
        f"compositor/{mod}.cppm",
        f"""module;

export module {mod};

import pbsd.core;

/// Burst 15 — {upstream} (native compositor).
export namespace pbsd::compositor::wayland::{stem} {{

{body}

}} // namespace pbsd::compositor::wayland::{stem}
""",
        make_c_stub(f"protocols/{upstream}", mod),
    )


def theme_mod(stem: str, body: str, upstream: str) -> None:
    ns = stem
    write(
        f"theme/plasma/aero.{stem}.cppm",
        f"""export module pbsd.theme.plasma.aero.{stem};

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero {stem.replace('_', ' ')} visual tokens.
export namespace pbsd::theme::plasma::aero::{ns} {{

{body}

}} // namespace pbsd::theme::plasma::aero::{ns}
""",
        make_c_stub(upstream, f"aero.{stem}"),
    )


def stand_mod(stem: str, upstream: str, body: str) -> None:
    write(
        f"stand/pbsd.stand.{stem}.cppm",
        f"""module;
#include <cstddef>

export module pbsd.stand.{stem};

import pbsd.core;

/// PROVENANCE: {upstream}
export namespace pbsd::stand::{stem} {{

{body}

}} // namespace pbsd::stand::{stem}
""",
    )


def arch_mod(rel: str, mod: str, ns: str, upstream: str, body: str) -> None:
    write(
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
    )


def bif_mod(stem: str, upstream: str, body: str) -> None:
    write(
        f"bifrost/pbsd.bifrost.{stem}.cppm",
        f"""module;
#include <cstdint>

export module pbsd.bifrost.{stem};

import pbsd.core;

/// PROVENANCE: {upstream}
export namespace pbsd::bifrost::{stem} {{

{body}

}} // namespace pbsd::bifrost::{stem}
""",
    )


def main() -> None:
    # --- KDE frameworks layershell siblings ---------------------------------
    write(
        "kde/frameworks/layershell.qwaylandlayershellintegrationplugin.cppm",
        """export module pbsd.kde.layershell.qwaylandlayershellintegrationplugin;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Burst 15 — layer-shell-qt Wayland shell integration plugin stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegrationplugin.cpp
export namespace pbsd::kde::layershell::qwaylandlayershellintegrationplugin {

inline constexpr const char kPluginIid[] =
    "org.qt-project.Qt.WaylandClient.QWaylandShellIntegrationFactoryInterface.5.4";
inline constexpr const char kMetadataFile[] = "layer-shell.json";

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* shell_surface() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegrationplugin.cpp";
}

} // namespace pbsd::kde::layershell::qwaylandlayershellintegrationplugin
""",
        make_c_stub(
            "kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegrationplugin.cpp",
            "qwaylandlayershellintegrationplugin",
        ),
    )

    write(
        "kde/frameworks/layershell.interfaces_window.cppm",
        """export module pbsd.kde.layershell.interfaces_window;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Burst 15 — layer-shell-qt window interface stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/interfaces/window.cpp
export namespace pbsd::kde::frameworks::layershell::interfaces_window {

inline constexpr const char kInterfaceId[] = "org.kde.layershell.window";

[[nodiscard]] inline const char* shell_surface() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/window.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_window
""",
        make_c_stub(
            "kde/frameworks/layer-shell-qt/src/interfaces/window.cpp",
            "interfaces_window",
        ),
    )

    # --- KDE plasma glue ----------------------------------------------------
    glue_specs = [
        (
            "tastenbrett",
            "kde/plasma-desktop/kcms/keyboard/tastenbrett/section.cpp",
            "import pbsd.kde.plasma.tastenbrett.section;",
            "::pbsd::kde::plasma::tastenbrett::section",
            """struct KeyboardLayoutStyle {
    const char* alphanumeric{tastenbrett::section::kAlphanumeric};
    const char* modifier{tastenbrett::section::kModifier};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline KeyboardLayoutStyle default_style() noexcept {
    return KeyboardLayoutStyle{};
}""",
        ),
        (
            "touchscreen",
            "kde/plasma-desktop/kcms/touchscreen/touchscreenmoduledata.cpp",
            "import pbsd.kde.plasma.touchscreen;",
            "::pbsd::kde::plasma::touchscreen",
            """struct TouchscreenStyle {
    const char* kcm_id{touchscreen::kKcmId};
    const char* settings_svg{aero::kSettingsSvg};
    float panel_opacity{0.75f};
};

[[nodiscard]] inline TouchscreenStyle default_style() noexcept {
    return TouchscreenStyle{};
}""",
        ),
        (
            "tablet",
            "kde/plasma-desktop/kcms/tablet/tabletmoduledata.cpp",
            "import pbsd.kde.plasma.tablet;",
            "::pbsd::kde::plasma::tablet",
            """struct TabletStyle {
    const char* kcm_id{tablet::kKcmId};
    const char* config_group{tablet::kConfigGroup};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline TabletStyle default_style() noexcept {
    return TabletStyle{};
}""",
        ),
        (
            "taskmanager",
            "kde/plasma-desktop/applets/taskmanager/package/contents/config/main.xml",
            "import pbsd.kde.plasma.taskmanager;",
            "::pbsd::kde::plasma::taskmanager",
            """struct TaskbarStyle {
    int icon_size{taskmanager::kDefaultIconSize};
    int max_tasks{taskmanager::kMaxTasks};
    const char* tasks_svg{taskmanager::kTasksSvg};
    float glass_opacity{0.72f};
};

[[nodiscard]] inline TaskbarStyle default_style() noexcept {
    return TaskbarStyle{};
}

[[nodiscard]] inline Status validate_icon_size(int px) noexcept {
    return px >= 16 && px <= 64 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "wheelinterceptor",
            "kde/plasma-desktop/containments/desktop/plugins/folder/wheelinterceptor.cpp",
            "import pbsd.kde.plasma.wheelinterceptor;",
            "::pbsd::kde::plasma::wheelinterceptor",
            """struct WheelStyle {
    int max_delta{wheelinterceptor::kMaxDelta};
    int min_delta{wheelinterceptor::kMinDelta};
    const char* hover_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline WheelStyle default_style() noexcept {
    return WheelStyle{};
}""",
        ),
        (
            "knetattach",
            "kde/plasma-desktop/knetattach/main.cpp",
            "import pbsd.kde.plasma.knetattach;",
            "::pbsd::kde::plasma::knetattach",
            """struct NetworkAttachStyle {
    const char* app_id{knetattach::kAppId};
    const char* protocol_key{knetattach::kProtocolKey};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline NetworkAttachStyle default_style() noexcept {
    return NetworkAttachStyle{};
}""",
        ),
        (
            "folder.applauncher",
            "kde/plasma-desktop/containments/desktop/plugins/folder/applauncher.cpp",
            "import pbsd.kde.plasma.folder.applauncher;",
            "::pbsd::kde::plasma::folder::applauncher",
            """struct LauncherStyle {
    const char* desktop_suffix{folder::applauncher::kDesktopEntrySuffix};
    unsigned max_apps{folder::applauncher::kMaxApps};
    const char* hover_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline LauncherStyle default_style() noexcept {
    return LauncherStyle{};
}""",
        ),
        (
            "kimpanel.ibus",
            "kde/plasma-desktop/applets/kimpanel/backend/ibus/ibus15/main.cpp",
            "import pbsd.kde.plasma.kimpanel.ibus;",
            "::pbsd::kde::plasma::kimpanel::ibus",
            """struct InputPanelStyle {
    const char* ibus_service{kimpanel::ibus::kIbusService};
    const char* ibus_path{kimpanel::ibus::kIbusPath};
    float popup_opacity{0.82f};
};

[[nodiscard]] inline InputPanelStyle default_style() noexcept {
    return InputPanelStyle{};
}""",
        ),
        (
            "mouse.kapplymousetheme",
            "kde/plasma-desktop/kcms/mouse/kapplymousetheme.cpp",
            "import pbsd.kde.plasma.mouse.kapplymousetheme;",
            "::pbsd::kde::plasma::mouse::kapplymousetheme",
            """struct MouseThemeStyle {
    const char* theme_group{mouse::kapplymousetheme::kCursorThemeGroup};
    const char* theme_key{mouse::kapplymousetheme::kThemeKey};
    const char* settings_svg{aero::kSettingsSvg};
    unsigned cursor_size{24};
};

[[nodiscard]] inline MouseThemeStyle default_style() noexcept {
    return MouseThemeStyle{};
}""",
        ),
    ]

    for name, upstream, imports, ns_path, extra in glue_specs:
        body = glue(name, upstream, imports, extra, ns_path)
        write(f"kde/plasma.{name}.glue.cppm", body, make_c_stub(upstream, f"{name}_glue"))

    # --- Compositor ---------------------------------------------------------
    comp_specs = [
        (
            "tablet_v2",
            "tablet-v2.xml",
            """inline constexpr const char kInterface[] = "wl_tablet";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetSurface = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetSurface);
}""",
        ),
        (
            "linux_dmabuf",
            "linux-dmabuf-v1.xml",
            """inline constexpr const char kInterface[] = "zwp_linux_dmabuf_v1";
inline constexpr int kVersion = 4;

enum class Request : unsigned char {
    Destroy = 0,
    CreateParams = 1,
    GetDefaultFeedback = 2,
    GetSurfaceFeedback = 3,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetSurfaceFeedback);
}""",
        ),
        (
            "shm",
            "wayland.xml",
            """inline constexpr const char kInterface[] = "wl_shm";
inline constexpr int kVersion = 1;

enum class Format : unsigned char {
    Argb8888 = 0,
    Xrgb8888 = 1,
};

[[nodiscard]] inline bool is_format(unsigned fmt) noexcept {
    return fmt <= static_cast<unsigned>(Format::Xrgb8888);
}""",
        ),
        (
            "foreign_toplevel",
            "ext-foreign-toplevel-list-v1.xml",
            """inline constexpr const char kManagerInterface[] = "ext_foreign_toplevel_list_v1";
inline constexpr const char kHandleInterface[] = "ext_foreign_toplevel_handle_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Destroy);
}""",
        ),
        (
            "tearing_control_v1",
            "wp-tearing-control-v1.xml",
            """inline constexpr const char kInterface[] = "wp_tearing_control_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetTearingControl = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetTearingControl);
}""",
        ),
        (
            "xdg_toplevel_icon_v1",
            "xdg-toplevel-icon-v1.xml",
            """inline constexpr const char kManagerInterface[] = "xdg_toplevel_icon_manager_v1";
inline constexpr const char kIconInterface[] = "xdg_toplevel_icon_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetIcon = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetIcon);
}""",
        ),
    ]
    for stem, upstream, body in comp_specs:
        comp_mod(stem, upstream, body)

    # --- Theme --------------------------------------------------------------
    theme_specs = [
        (
            "alt_tab",
            """struct AltTabTokens {
    unsigned thumbnail_width_px{280};
    unsigned animation_ms{200};
    float dim_opacity{0.35f};
    const char* switcher_svg{"plasma/panel/alt-tab.svg"};
};

[[nodiscard]] inline AltTabTokens default_tokens() noexcept {
    return AltTabTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "battery_indicator",
            """struct BatteryTokens {
    unsigned icon_size_px{16};
    float low_threshold{0.15f};
    const char* battery_svg{"plasma/panel/battery.svg"};
};

[[nodiscard]] inline BatteryTokens default_tokens() noexcept {
    return BatteryTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "network_status",
            """struct NetworkTokens {
    unsigned icon_size_px{16};
    float offline_opacity{0.55f};
    const char* network_svg{"plasma/panel/network.svg"};
};

[[nodiscard]] inline NetworkTokens default_tokens() noexcept {
    return NetworkTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "desktop_icons",
            """struct DesktopIconTokens {
    unsigned grid_size_px{96};
    unsigned label_font_px{12};
    float selection_opacity{0.45f};
};

[[nodiscard]] inline DesktopIconTokens default_tokens() noexcept {
    return DesktopIconTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "jump_list",
            """struct JumpListTokens {
    unsigned max_items{10};
    unsigned row_height_px{28};
    float popup_opacity{0.78f};
    const char* jumplist_svg{"plasma/panel/jump-list.svg"};
};

[[nodiscard]] inline JumpListTokens default_tokens() noexcept {
    return JumpListTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "volume_mixer",
            """struct VolumeTokens {
    unsigned slider_width_px{120};
    float mute_opacity{0.50f};
    const char* volume_svg{"plasma/panel/volume.svg"};
};

[[nodiscard]] inline VolumeTokens default_tokens() noexcept {
    return VolumeTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
    ]
    for stem, body, upstream in theme_specs:
        theme_mod(stem, body, upstream)

    # --- Stand --------------------------------------------------------------
    stand_specs = [
        (
            "ffs",
            "hbsd/src/stand/libsa/ffs.c",
            """inline constexpr unsigned kBlockSize = 4096;
inline constexpr unsigned kDirectBlocks = 12;

[[nodiscard]] inline Status validate_block_size(unsigned size) noexcept {
    if (size != 1024 && size != 2048 && size != 4096 && size != 8192) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "zstd",
            "hbsd/src/stand/libsa/zstd.c",
            """inline constexpr unsigned kWindowLogMax = 27;
inline constexpr unsigned kFrameHeaderSize = 4;

[[nodiscard]] inline Status validate_window_log(unsigned log) noexcept {
    return log <= kWindowLogMax ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "smbfs",
            "hbsd/src/stand/libsa/smbfs.c",
            """inline constexpr unsigned kMaxPath = 260;
inline constexpr unsigned kShareLen = 80;

[[nodiscard]] inline Status validate_share_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "lua",
            "hbsd/src/stand/lua/load.lua",
            """inline constexpr unsigned kMaxModules = 8;
inline constexpr unsigned kScriptLen = 4096;

[[nodiscard]] inline Status validate_module_count(unsigned count) noexcept {
    return count <= kMaxModules ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "nullfs",
            "hbsd/src/stand/libsa/nullfs.c",
            """enum class MountMode : unsigned char {
    Passthrough = 0,
    Blackhole = 1,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(MountMode::Blackhole) ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for stem, upstream, body in stand_specs:
        stand_mod(stem, upstream, body)

    # --- Arch ---------------------------------------------------------------
    arch_specs = [
        (
            "arch/amd64/pbsd.arch.amd64.smp.cppm",
            "pbsd.arch.amd64.smp",
            "amd64::smp",
            "hbsd/src/sys/x86/x86/mp_machdep.c",
            """inline constexpr unsigned kMaxCpus = 256;

[[nodiscard]] inline Status validate_apic_id(unsigned id) noexcept {
    return id < kMaxCpus ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/amd64/pbsd.arch.amd64.pat.cppm",
            "pbsd.arch.amd64.pat",
            "amd64::pat",
            "hbsd/src/sys/x86/x86/pat.c",
            """enum class Entry : unsigned char {
    Uncacheable = 0,
    WriteCombining = 1,
    WriteThrough = 4,
    WriteProtected = 5,
    WriteBack = 6,
};

inline constexpr unsigned kEntryCount = 8;

[[nodiscard]] inline Status validate_entry(unsigned e) noexcept {
    return e < kEntryCount ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.smp.cppm",
            "pbsd.arch.arm64.smp",
            "arm64::smp",
            "hbsd/src/sys/arm64/arm64/mp_machdep.c",
            """inline constexpr unsigned kMaxCpus = 256;

[[nodiscard]] inline Status validate_mpidr(unsigned mpidr) noexcept {
    return mpidr != 0 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.mmu.cppm",
            "pbsd.arch.arm64.mmu",
            "arm64::mmu",
            "hbsd/src/sys/arm64/arm64/pmap.c",
            """inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned kTtbrLevels = 4;

[[nodiscard]] inline Status validate_level(unsigned level) noexcept {
    return level < kTtbrLevels ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for rel, mod, ns, upstream, body in arch_specs:
        arch_mod(rel, mod, ns, upstream, body)

    # --- Bifrost ------------------------------------------------------------
    bif_specs = [
        (
            "memslot",
            "hbsd/src/sys/amd64/vmm/intel/vmcs.c",
            """inline constexpr unsigned kMaxSlots = 32;
inline constexpr unsigned kGuestPhysMask = 0x000FFFFFFFFFF000ull;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kMaxSlots ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "irqchip",
            "hbsd/src/sys/amd64/vmm/intel/vlapic.c",
            """enum class DeliveryMode : unsigned char {
    Fixed = 0,
    Lowest = 1,
    Smi = 2,
    Nmi = 4,
    Init = 5,
    ExtInt = 7,
};

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec >= 16 && vec <= 255 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "migration",
            "hbsd/src/sys/amd64/vmm/vmm_snapshot.c",
            """enum class Phase : unsigned char {
    Idle = 0,
    PreCopy = 1,
    StopCopy = 2,
    Resume = 3,
};

[[nodiscard]] inline Status validate_phase(unsigned phase) noexcept {
    return phase <= static_cast<unsigned>(Phase::Resume) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "timer",
            "hbsd/src/sys/amd64/vmm/io/vhpet.c",
            """inline constexpr unsigned kDefaultHz = 100000000;
inline constexpr unsigned kMinPeriodNs = 1000;

[[nodiscard]] inline Status validate_period_ns(unsigned period) noexcept {
    return period >= kMinPeriodNs ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for stem, upstream, body in bif_specs:
        bif_mod(stem, upstream, body)

    print(f"\nTotal created: {len(CREATED)}")
    for p in CREATED:
        print(" ", p)


if __name__ == "__main__":
    main()
