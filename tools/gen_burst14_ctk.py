#!/usr/bin/env python3
"""Burst 14 — kde/theme/compositor/stand/arch/bifrost hand ports + dual-world .c stubs."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"

CREATED: list[str] = []


def write(rel: str, body: str, c_stub: str | None = None) -> None:
    path = PBSD / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body.strip() + "\n", encoding="utf-8")
    CREATED.append(rel.replace("\\", "/"))
    print("wrote", rel)
    if c_stub is not None:
        c_path = path.with_suffix(".c")
        c_path.write_text(c_stub, encoding="utf-8")
        CREATED.append(str(c_path.relative_to(PBSD)).replace("\\", "/"))


def make_c_stub(upstream: str, name: str) -> str:
    return f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for {name} */\n"


def glue(name: str, upstream: str, extra: str) -> str:
    mod = name.replace(".", ".")
    ns = name.replace(".", "::")
    return f"""export module pbsd.kde.plasma.{name}.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.{name};

/// Burst 14 — Plasma {name} ↔ Aero glue.
/// Upstream: {upstream}
export namespace pbsd::kde::plasma::{ns}::glue {{

{extra}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return {name}::upstream_path();
}}

}} // namespace pbsd::kde::plasma::{ns}::glue
"""


def main() -> None:
    # --- KDE frameworks layershell bridge -----------------------------------
    write(
        "kde/frameworks/layershell.bridge.cppm",
        """export module pbsd.kde.layershell.bridge;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;
import pbsd.kde.layershell.qwaylandlayershell;
import pbsd.kde.layershell.qwaylandlayersurface;

/// Burst 14 — layer-shell-qt sibling wiring via fully qualified names.
/// Upstream: kde/frameworks/layer-shell-qt/src/
export namespace pbsd::kde::frameworks::layershell::bridge {

struct IntegrationWire {
    const char* plugin{
        ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin};
    const char* shell_surface{
        ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface};
    const char* layer_shell_iface{
        ::pbsd::kde::layershell::qwaylandlayershell::kInterface};
    const char* layer_surface_iface{
        ::pbsd::kde::layershell::qwaylandlayersurface::kInterface};
    int shell_version{::pbsd::kde::layershell::qwaylandlayershell::kVersion};
    int surface_version{::pbsd::kde::layershell::qwaylandlayersurface::kVersion};
};

[[nodiscard]] inline IntegrationWire default_wire() noexcept {
    return IntegrationWire{};
}

[[nodiscard]] inline Status validate_wire(const IntegrationWire& w) noexcept {
    if (w.plugin == nullptr || w.shell_surface == nullptr) {
        return Status::Invalid;
    }
    if (w.shell_version < 1 || w.surface_version < 1) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::upstream_path();
}

} // namespace pbsd::kde::frameworks::layershell::bridge
""",
    )

    # --- KDE plasma glue modules --------------------------------------------
    glue_specs = [
        (
            "panel",
            "kde/plasma-desktop/containments/panel/Panel.qml",
            """struct PanelStyle {
    int height{panel::kDefaultHeight};
    float opacity{panel::kDefaultOpacity};
    int blur_radius{panel::kDefaultBlurRadius};
    const char* layout_svg{aero::kPanelGlassSvg};
};

[[nodiscard]] inline PanelStyle default_style() noexcept {
    return PanelStyle{};
}

[[nodiscard]] inline Status validate_height(int h) noexcept {
    return h >= 24 && h <= 128 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "colors",
            "pbsd/theme/plasma/color-schemes/PBSDAero.colors",
            """struct ColorStyle {
    colors::Rgb primary{colors::kPrimary};
    const char* scheme_name{colors::kColorSchemeName};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline ColorStyle default_style() noexcept {
    return ColorStyle{};
}

[[nodiscard]] inline colors::Rgb accent(colors::AccentRole role) noexcept {
    return colors::accent(role);
}""",
        ),
        (
            "dialogs",
            "kde/plasma-desktop/containments/panel/dialogs/Dialog.qml",
            """struct DialogStyle {
    float opacity{aero::default_blur().dialog_opacity};
    int blur_radius{aero::default_blur().blur_radius};
    const char* frame_svg{aero::kMessageBoxSvg};
};

[[nodiscard]] inline DialogStyle default_style() noexcept {
    return DialogStyle{};
}""",
        ),
        (
            "widgets",
            "kde/plasma-framework/src/plasmaquick/plasmoid/plasmoid.cpp",
            """struct WidgetStyle {
    const char* containment_svg{aero::kHoverHighlightSvg};
    float panel_opacity{aero::default_blur().panel_opacity};
};

[[nodiscard]] inline WidgetStyle default_style() noexcept {
    return WidgetStyle{};
}""",
        ),
        (
            "kded",
            "kde/plasma-desktop/kcms/kded/kdedconfigdata.cpp",
            """struct KdedStyle {
    const char* config_group{kded::kConfigGroup};
    const char* autoload_key{kded::kAutoloadKey};
};

[[nodiscard]] inline KdedStyle default_style() noexcept {
    return KdedStyle{};
}""",
        ),
        (
            "systemsettings",
            "kde/systemsettings/app/main.cpp",
            """struct SettingsStyle {
    const char* kcm_root{"systemsettings5"};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline SettingsStyle default_style() noexcept {
    return SettingsStyle{};
}""",
        ),
        (
            "globalpaths",
            "kde/plasma-desktop/kcms/desktoppaths/globalpaths.cpp",
            """struct PathsStyle {
    const char* desktop{globalpaths::kDesktopLocation};
    const char* documents{globalpaths::kDocumentsLocation};
    const char* pictures{globalpaths::kPicturesLocation};
};

[[nodiscard]] inline PathsStyle default_style() noexcept {
    return PathsStyle{};
}""",
        ),
        (
            "cursortheme",
            "kde/plasma-desktop/kcms/kcm_cursortheme/main.cpp",
            """struct CursorStyle {
    const char* theme_name{"breeze_cursors"};
    unsigned size_px{24};
};

[[nodiscard]] inline CursorStyle default_style() noexcept {
    return CursorStyle{};
}

[[nodiscard]] inline Status validate_size(unsigned px) noexcept {
    return px >= 16 && px <= 64 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "baloo",
            "kde/plasma-desktop/kcms/kcm_baloo/main.cpp",
            """struct BalooStyle {
    const char* kcm_id{"kcm_baloo"};
    bool indexing_enabled{true};
};

[[nodiscard]] inline BalooStyle default_style() noexcept {
    return BalooStyle{};
}""",
        ),
        (
            "emojier",
            "kde/plasma-desktop/applets/emojier/plugin/emojier.cpp",
            """struct EmojierStyle {
    const char* applet_id{"org.kde.plasma.emojier"};
    unsigned max_recent{32};
};

[[nodiscard]] inline EmojierStyle default_style() noexcept {
    return EmojierStyle{};
}""",
        ),
        (
            "quickaccess",
            "kde/plasma-desktop/containments/panel/quickaccess/QuickAccess.qml",
            """struct QuickAccessStyle {
    const char* icon_svg{aero::kQuickLaunchSvg};
    unsigned max_items{12};
};

[[nodiscard]] inline QuickAccessStyle default_style() noexcept {
    return QuickAccessStyle{};
}""",
        ),
        (
            "menuhelper",
            "kde/plasma-desktop/shell/menuhelper.cpp",
            """struct MenuStyle {
    const char* context_menu_id{"plasma-desktop"};
    unsigned max_depth{8};
};

[[nodiscard]] inline MenuStyle default_style() noexcept {
    return MenuStyle{};
}""",
        ),
        (
            "libinputcommon",
            "kde/plasma-desktop/kcms/kcm_mouse/libinputcommon.cpp",
            """struct InputStyle {
    const char* driver{"libinput"};
    unsigned scroll_factor{120};
};

[[nodiscard]] inline InputStyle default_style() noexcept {
    return InputStyle{};
}""",
        ),
        (
            "qtquicksettings",
            "kde/plasma-framework/src/plasmaquick/plasmoid/plasmoid.cpp",
            """struct QtQuickStyle {
    const char* import_uri{"org.kde.plasma.core"};
    unsigned animation_ms{220};
};

[[nodiscard]] inline QtQuickStyle default_style() noexcept {
    return QtQuickStyle{};
}""",
        ),
        (
            "packagestructure",
            "kde/plasma-framework/packagestructure/packagestructure.cpp",
            """struct PackageStyle {
    const char* structure_id{"Plasma/Applet"};
    const char* fallback_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline PackageStyle default_style() noexcept {
    return PackageStyle{};
}""",
        ),
    ]

    for name, upstream, extra in glue_specs:
        write(f"kde/plasma.{name}.glue.cppm", glue(name, upstream, extra))

    # --- Compositor wayland helpers -----------------------------------------
    comp_specs = [
        (
            "idle_inhibit",
            "idle-inhibit-unstable-v1.xml",
            """inline constexpr const char kInterface[] = "zwp_idle_inhibit_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    CreateInactive = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::CreateInactive);
}""",
        ),
        (
            "data_device",
            "wayland.xml",
            """inline constexpr const char kInterface[] = "wl_data_device";
inline constexpr int kVersion = 3;

enum class Request : unsigned char {
    Destroy = 0,
    StartDrag = 1,
    SetSelection = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetSelection);
}""",
        ),
        (
            "primary_selection",
            "primary-selection-unstable-v1.xml",
            """inline constexpr const char kInterface[] = "zwp_primary_selection_device_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    SetSelection = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetSelection);
}""",
        ),
        (
            "single_pixel_buffer",
            "single-pixel-buffer-v1.xml",
            """inline constexpr const char kInterface[] = "wp_single_pixel_buffer_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    CreateU32RgbaBuffer = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::CreateU32RgbaBuffer);
}""",
        ),
        (
            "xdg_activation",
            "xdg-activation-v1.xml",
            """inline constexpr const char kInterface[] = "xdg_activation_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetActivationToken = 1,
    Activate = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::Activate);
}""",
        ),
    ]

    for stem, upstream, body in comp_specs:
        mod = f"pbsd.compositor.wayland.{stem}"
        write(
            f"compositor/{mod}.cppm",
            f"""module;

export module {mod};

import pbsd.core;

/// Burst 14 — {upstream} (native compositor).
export namespace pbsd::compositor::wayland::{stem} {{

{body}

}} // namespace pbsd::compositor::wayland::{stem}
""",
            make_c_stub(f"protocols/{upstream}", mod),
        )

    # Fix namespace for stems with underscores - use the stem as-is for namespace
    # Actually I used stem.replace('.', '_') only for xdg - all are fine

    # --- Theme aero modules -------------------------------------------------
    theme_specs = [
        (
            "clock_widget",
            """struct ClockTokens {
    const char* time_format{"HH:mm"};
    const char* date_format{"ddd, MMM d"};
    unsigned font_px{14};
};

[[nodiscard]] inline ClockTokens default_tokens() noexcept {
    return ClockTokens{};
}""",
        ),
        (
            "media_controls",
            """struct MediaTokens {
    unsigned button_size_px{32};
    float inactive_opacity{0.55f};
    const char* play_svg{"plasma/panel/media-play.svg"};
};

[[nodiscard]] inline MediaTokens default_tokens() noexcept {
    return MediaTokens{};
}""",
        ),
        (
            "task_preview",
            """struct PreviewTokens {
    unsigned thumbnail_width_px{240};
    unsigned animation_ms{180};
    float shadow_opacity{0.35f};
};

[[nodiscard]] inline PreviewTokens default_tokens() noexcept {
    return PreviewTokens{};
}""",
        ),
        (
            "window_snap",
            """struct SnapTokens {
    unsigned snap_margin_px{8};
    unsigned preview_border_px{2};
    float preview_opacity{0.45f};
};

[[nodiscard]] inline SnapTokens default_tokens() noexcept {
    return SnapTokens{};
}""",
        ),
        (
            "start_menu_search",
            """struct SearchTokens {
    const char* placeholder{"Search apps, files, settings"};
    unsigned result_limit{24};
    const char* search_box_svg{"plasma/panel/search-box.svg"};
};

[[nodiscard]] inline SearchTokens default_tokens() noexcept {
    return SearchTokens{};
}""",
        ),
    ]

    for stem, body in theme_specs:
        ns = stem.replace("_", "_")
        write(
            f"theme/plasma/aero.{stem}.cppm",
            f"""export module pbsd.theme.plasma.aero.{stem};

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 14 — Aero {stem.replace('_', ' ')} visual tokens.
export namespace pbsd::theme::plasma::aero::{ns} {{

{body}

}} // namespace pbsd::theme::plasma::aero::{ns}
""",
        )

    # --- Stand ----------------------------------------------------------------
    stand_specs = [
        (
            "pkgfs",
            "hbsd/src/stand/libsa/pkgfs.c",
            """inline constexpr unsigned kMaxPackages = 32;
inline constexpr unsigned kNameLen = 64;

enum class MountState : unsigned char {
    Unmounted = 0,
    Mounted = 1,
};

[[nodiscard]] inline Status validate_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "dosfs",
            "hbsd/src/stand/libsa/dosfs.c",
            """inline constexpr unsigned kSectorSize = 512;
inline constexpr unsigned kClusterMax = 65528;

[[nodiscard]] inline Status validate_cluster(unsigned cluster) noexcept {
    if (cluster < 2 || cluster > kClusterMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "ext2fs",
            "hbsd/src/stand/libsa/ext2fs.c",
            """inline constexpr unsigned kBlockSize = 4096;
inline constexpr unsigned kInodeSize = 256;

[[nodiscard]] inline Status validate_block_size(unsigned size) noexcept {
    if (size != 1024 && size != 2048 && size != 4096) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "preload",
            "hbsd/src/stand/libsa/preload.c",
            """inline constexpr unsigned kMaxModules = 16;
inline constexpr unsigned kPathLen = 256;

[[nodiscard]] inline Status validate_module_count(unsigned count) noexcept {
    return count <= kMaxModules ? Status::Ok : Status::Invalid;
}""",
        ),
    ]

    for stem, upstream, body in stand_specs:
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

    # --- Arch -----------------------------------------------------------------
    arch_specs = [
        (
            "arch/amd64/pbsd.arch.amd64.rdrand.cppm",
            "pbsd.arch.amd64.rdrand",
            "amd64::rdrand",
            "hbsd/src/sys/x86/x86/rdrand.c",
            """inline constexpr unsigned kEntropyBits = 128;

[[nodiscard]] inline Status validate_entropy_bits(unsigned bits) noexcept {
    return bits == 64 || bits == 128 || bits == 256 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/amd64/pbsd.arch.amd64.mtrr.cppm",
            "pbsd.arch.amd64.mtrr",
            "amd64::mtrr",
            "hbsd/src/sys/x86/x86/mtrr.c",
            """enum class Type : unsigned char {
    Uncacheable = 0,
    WriteCombining = 1,
    WriteThrough = 4,
    WriteProtected = 5,
    WriteBack = 6,
};

inline constexpr unsigned kMaxRegions = 8;

[[nodiscard]] inline Status validate_type(unsigned t) noexcept {
    return t <= static_cast<unsigned>(Type::WriteBack) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.dcz.cppm",
            "pbsd.arch.arm64.dcz",
            "arm64::dcz",
            "hbsd/src/sys/arm64/arm64/dcache.c",
            """inline constexpr unsigned kDczBlockBytes = 64;

[[nodiscard]] inline Status validate_block(unsigned bytes) noexcept {
    return bytes == kDczBlockBytes ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.spinlock.cppm",
            "pbsd.arch.arm64.spinlock",
            "arm64::spinlock",
            "hbsd/src/sys/arm64/arm64/spinlock.c",
            """enum class State : unsigned char {
    Unlocked = 0,
    Locked = 1,
};

[[nodiscard]] inline bool is_locked(State s) noexcept {
    return s == State::Locked;
}""",
        ),
    ]

    for rel, mod, ns, upstream, body in arch_specs:
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

    # --- Bifrost --------------------------------------------------------------
    bif_specs = [
        (
            "pit",
            "hbsd/src/sys/amd64/vmm/io/pit.c",
            """inline constexpr unsigned kFrequencyHz = 1193182;
inline constexpr unsigned kChannel0 = 0;

[[nodiscard]] inline Status validate_divisor(unsigned divisor) noexcept {
    return divisor >= 1 && divisor <= 65535 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "pci_pass",
            "hbsd/src/sys/amd64/vmm/io/vmmio.c",
            """enum class PassMode : unsigned char {
    Disabled = 0,
    Passthrough = 1,
    Emulated = 2,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(PassMode::Emulated) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "vga",
            "hbsd/src/sys/amd64/vmm/io/vga.c",
            """inline constexpr unsigned kFbSize = 0xA0000;
inline constexpr unsigned kMode13Width = 320;
inline constexpr unsigned kMode13Height = 200;

[[nodiscard]] inline Status validate_resolution(unsigned w, unsigned h) noexcept {
    if (w == 0 || h == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
        ),
        (
            "rtc",
            "hbsd/src/sys/amd64/vmm/io/rtc.c",
            """inline constexpr unsigned kPortIndex = 0x70;
inline constexpr unsigned kPortData = 0x71;

[[nodiscard]] inline Status validate_register(unsigned reg) noexcept {
    return reg <= 0x0F ? Status::Ok : Status::Invalid;
}""",
        ),
    ]

    for stem, upstream, body in bif_specs:
        write(
            f"bifrost/pbsd.bifrost.{stem}.cppm",
            f"""module;
#include <cstdint>

export module pbsd.bifrost.{stem};

import pbsd.core;

/// PROVENANCE: {upstream}
export namespace pbsd::bifrost::{stem.replace('_', '_')} {{

{body}

}} // namespace pbsd::bifrost::{stem}
""",
        )

    print(f"\nTotal created: {len(CREATED)}")
    for p in CREATED:
        print(" ", p)


if __name__ == "__main__":
    main()
