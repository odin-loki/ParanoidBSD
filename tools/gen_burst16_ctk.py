#!/usr/bin/env python3
"""Burst 16 — kde/theme/compositor/stand/arch/bifrost hand ports + dual-world .c stubs."""
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


def plasma_mod(stem: str, upstream: str, body: str) -> None:
    write(
        f"kde/plasma.{stem}.cppm",
        f"""export module pbsd.kde.plasma.{stem};

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma {stem.replace('.', ' ')} constants.
/// Upstream: {upstream}
export namespace pbsd::kde::plasma::{stem.replace('.', '::')} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace pbsd::kde::plasma::{stem.replace('.', '::')}
""",
        make_c_stub(upstream, stem),
    )


def plasma_glue(
    name: str,
    upstream: str,
    imports: str,
    extra: str,
    ns_path: str,
    *,
    upstream_expr: str | None = None,
) -> None:
    ns = name.replace(".", "::")
    up = upstream_expr if upstream_expr is not None else f"{ns_path}::upstream_path()"
    write(
        f"kde/plasma.{name}.glue.cppm",
        f"""export module pbsd.kde.plasma.{name}.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
{imports}

/// Burst 16 — Plasma {name} ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: {upstream}
export namespace pbsd::kde::plasma::{ns}::glue {{

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

{extra}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return {up};
}}

}} // namespace pbsd::kde::plasma::{ns}::glue
""",
        make_c_stub(upstream, f"{name}_glue"),
    )


def comp_mod(stem: str, upstream: str, ns: str, body: str) -> None:
    mod = f"pbsd.compositor.wayland.{stem}"
    write(
        f"compositor/{mod}.cppm",
        f"""module;

export module {mod};

import pbsd.core;

/// Burst 16 — {upstream} (native compositor).
export namespace pbsd::compositor::wayland::{ns} {{

{body}

}} // namespace pbsd::compositor::wayland::{ns}
""",
        make_c_stub(f"protocols/{upstream}", mod),
    )


def theme_mod(stem: str, body: str, upstream: str) -> None:
    write(
        f"theme/plasma/aero.{stem}.cppm",
        f"""export module pbsd.theme.plasma.aero.{stem};

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero {stem.replace('_', ' ')} visual tokens.
export namespace pbsd::theme::plasma::aero::{stem} {{

{body}

}} // namespace pbsd::theme::plasma::aero::{stem}
""",
        make_c_stub(upstream, f"aero.{stem}"),
    )


def stand_mod(stem: str, upstream: str, body: str) -> None:
    write(
        f"stand/pbsd.stand.{stem}.cppm",
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
    # --- KDE plasma foundation modules --------------------------------------
    plasma_mod(
        "clipboard",
        "kde/plasma-workspace/applets/clipboard/clipboard.cpp",
        """inline constexpr const char kAppletId[] = "org.kde.plasma.clipboard";
inline constexpr unsigned kMaxHistory{20};""",
    )
    plasma_mod(
        "brightness",
        "kde/plasma-desktop/kcms/brightness/brightness.cpp",
        """inline constexpr const char kKcmId[] = "kcm_brightness";
inline constexpr int kMinPercent{1};
inline constexpr int kMaxPercent{100};""",
    )
    plasma_mod(
        "calendar",
        "kde/plasma-desktop/applets/calendar/calendar.cpp",
        """inline constexpr const char kAppletId[] = "org.kde.plasma.calendar";
inline constexpr unsigned kWeekRows{6};""",
    )
    plasma_mod(
        "containmentactions",
        "kde/plasma-framework/containmentactions.cpp",
        """inline constexpr const char kPluginPrefix[] = "org.kde.plasma.containmentactions.";
inline constexpr unsigned kMaxActions{16};""",
    )
    plasma_mod(
        "screenlock",
        "kde/plasma-workspace/screenlocker/main.cpp",
        """inline constexpr const char kGreeterId[] = "org.kde.plasma.screenlocker";
inline constexpr unsigned kGraceMs{5000};""",
    )
    plasma_mod(
        "accessibility",
        "kde/plasma-desktop/kcms/accessibility/accessibility.cpp",
        """inline constexpr const char kKcmId[] = "kcm_accessibility";
inline constexpr float kMinContrast{1.0f};""",
    )

    glue_specs = [
        (
            "clipboard",
            "kde/plasma-workspace/applets/clipboard/clipboard.cpp",
            "import pbsd.kde.plasma.clipboard;",
            "::pbsd::kde::plasma::clipboard",
            """struct ClipboardStyle {
    const char* applet_id{clipboard::kAppletId};
    unsigned max_history{clipboard::kMaxHistory};
    const char* shell_surface{kShellSurface};
    float popup_opacity{0.82f};
};

[[nodiscard]] inline ClipboardStyle default_style() noexcept {
    return ClipboardStyle{};
}""",
        ),
        (
            "brightness",
            "kde/plasma-desktop/kcms/brightness/brightness.cpp",
            "import pbsd.kde.plasma.brightness;",
            "::pbsd::kde::plasma::brightness",
            """struct BrightnessStyle {
    const char* kcm_id{brightness::kKcmId};
    const char* settings_svg{aero::kSettingsSvg};
    int min_percent{brightness::kMinPercent};
    int max_percent{brightness::kMaxPercent};
};

[[nodiscard]] inline BrightnessStyle default_style() noexcept {
    return BrightnessStyle{};
}""",
        ),
        (
            "calendar",
            "kde/plasma-desktop/applets/calendar/calendar.cpp",
            "import pbsd.kde.plasma.calendar;",
            "::pbsd::kde::plasma::calendar",
            """struct CalendarStyle {
    const char* applet_id{calendar::kAppletId};
    unsigned week_rows{calendar::kWeekRows};
    const char* hover_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline CalendarStyle default_style() noexcept {
    return CalendarStyle{};
}""",
        ),
        (
            "containmentactions",
            "kde/plasma-framework/containmentactions.cpp",
            "import pbsd.kde.plasma.containmentactions;",
            "::pbsd::kde::plasma::containmentactions",
            """struct ActionsStyle {
    const char* plugin_prefix{containmentactions::kPluginPrefix};
    unsigned max_actions{containmentactions::kMaxActions};
    const char* layer_shell{kLayerShell};
};

[[nodiscard]] inline ActionsStyle default_style() noexcept {
    return ActionsStyle{};
}""",
        ),
        (
            "screenlock",
            "kde/plasma-workspace/screenlocker/main.cpp",
            "import pbsd.kde.plasma.screenlock;",
            "::pbsd::kde::plasma::screenlock",
            """struct ScreenlockStyle {
    const char* greeter_id{screenlock::kGreeterId};
    unsigned grace_ms{screenlock::kGraceMs};
    const char* lock_svg{aero::kLockSvg};
};

[[nodiscard]] inline ScreenlockStyle default_style() noexcept {
    return ScreenlockStyle{};
}""",
        ),
        (
            "accessibility",
            "kde/plasma-desktop/kcms/accessibility/accessibility.cpp",
            "import pbsd.kde.plasma.accessibility;",
            "::pbsd::kde::plasma::accessibility",
            """struct AccessibilityStyle {
    const char* kcm_id{accessibility::kKcmId};
    float min_contrast{accessibility::kMinContrast};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline AccessibilityStyle default_style() noexcept {
    return AccessibilityStyle{};
}""",
        ),
        (
            "runners.shell",
            "kde/plasma-workspace/runners/shell/shell.cpp",
            "import pbsd.kde.plasma.runners.abstractjob;",
            "::pbsd::kde::plasma::runners::abstractjob",
            """struct ShellRunnerStyle {
    const char* runner_id{"shell"};
    const char* settings_svg{aero::kSettingsSvg};
    const char* shell_surface{kShellSurface};
};

[[nodiscard]] inline ShellRunnerStyle default_style() noexcept {
    return ShellRunnerStyle{};
}""",
        ),
        (
            "kcms.display",
            "kde/plasma-desktop/kcms/display/display.cpp",
            "",
            "aero",
            """struct DisplayKcmStyle {
    const char* kcm_id{"kcm_display"};
    const char* settings_svg{aero::kSettingsSvg};
    const char* layer_shell{kLayerShell};
};

[[nodiscard]] inline DisplayKcmStyle default_style() noexcept {
    return DisplayKcmStyle{};
}""",
            '"kde/plasma-desktop/kcms/display/display.cpp"',
        ),
    ]
    for spec in glue_specs:
        name, upstream, imports, ns_path, extra = spec[:5]
        upstream_expr = spec[5] if len(spec) > 5 else None
        plasma_glue(name, upstream, imports, extra, ns_path, upstream_expr=upstream_expr)

    # --- Compositor ---------------------------------------------------------
    comp_specs = [
        (
            "pointer_constraints",
            "pointer-constraints-unstable-v1.xml",
            "pointer_constraints",
            """inline constexpr const char kInterface[] = "zwp_pointer_constraints_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    LockPointer = 1,
    ConfinePointer = 2,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::ConfinePointer);
}""",
        ),
        (
            "relative_pointer",
            "relative-pointer-unstable-v1.xml",
            "relative_pointer",
            """inline constexpr const char kInterface[] = "zwp_relative_pointer_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::Destroy);
}""",
        ),
        (
            "security_context",
            "security-context-v1.xml",
            "security_context",
            """inline constexpr const char kInterface[] = "wp_security_context_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    CreateSecurityContext = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::CreateSecurityContext);
}""",
        ),
        (
            "color_management",
            "color-management-v1.xml",
            "color_management",
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
            "fifo",
            "fifo-v1.xml",
            "fifo",
            """inline constexpr const char kInterface[] = "wp_fifo_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    SetBarrier = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetBarrier);
}""",
        ),
        (
            "data_control",
            "wlr-data-control-unstable-v1.xml",
            "data_control",
            """inline constexpr const char kManagerInterface[] = "zwlr_data_control_manager_v1";
inline constexpr int kVersion = 2;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetDataDevice = 1,
    GetPrimarySelectionDevice = 2,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetPrimarySelectionDevice);
}""",
        ),
    ]
    for stem, upstream, ns, body in comp_specs:
        comp_mod(stem, upstream, ns, body)

    comp_mod(
        "layer.popup",
        "wlr-layer-shell-unstable-v1.xml",
        "layer::popup",
        """inline constexpr const char kPopupInterface[] = "zwlr_layer_surface_v1";

enum class PopupRequest : unsigned char {
    GetPopup = 5,
    Destroy = 7,
};

[[nodiscard]] inline bool is_popup_request(unsigned opcode) noexcept {
    return opcode == static_cast<unsigned>(PopupRequest::GetPopup)
        || opcode == static_cast<unsigned>(PopupRequest::Destroy);
}""",
    )

    # --- Theme --------------------------------------------------------------
    theme_specs = [
        (
            "brightness_slider",
            """struct BrightnessTokens {
    unsigned slider_width_px{140};
    float dim_opacity{0.40f};
    const char* brightness_svg{"plasma/panel/brightness.svg"};
};

[[nodiscard]] inline BrightnessTokens default_tokens() noexcept {
    return BrightnessTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "calendar_widget",
            """struct CalendarTokens {
    unsigned cell_size_px{32};
    unsigned header_font_px{14};
    float weekend_opacity{0.75f};
};

[[nodiscard]] inline CalendarTokens default_tokens() noexcept {
    return CalendarTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "clipboard_history",
            """struct ClipboardTokens {
    unsigned max_rows{8};
    unsigned row_height_px{28};
    float popup_opacity{0.82f};
};

[[nodiscard]] inline ClipboardTokens default_tokens() noexcept {
    return ClipboardTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "screen_lock",
            """struct ScreenLockTokens {
    unsigned blur_radius_px{32};
    float overlay_opacity{0.55f};
    const char* lock_svg{"plasma/panel/lock.svg"};
};

[[nodiscard]] inline ScreenLockTokens default_tokens() noexcept {
    return ScreenLockTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "privacy_indicator",
            """struct PrivacyTokens {
    unsigned icon_size_px{16};
    float active_opacity{1.0f};
    float idle_opacity{0.45f};
};

[[nodiscard]] inline PrivacyTokens default_tokens() noexcept {
    return PrivacyTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
        (
            "workspace_switcher",
            """struct WorkspaceTokens {
    unsigned thumbnail_width_px{240};
    unsigned animation_ms{180};
    float dim_opacity{0.30f};
};

[[nodiscard]] inline WorkspaceTokens default_tokens() noexcept {
    return WorkspaceTokens{};
}""",
            "pbsd/theme/plasma/panel/layout.json",
        ),
    ]
    for stem, body, upstream in theme_specs:
        theme_mod(stem, body, upstream)

    # --- Stand --------------------------------------------------------------
    stand_specs = [
        (
            "verify",
            "hbsd/src/stand/libsa/verify.c",
            """inline constexpr unsigned kDigestLen = 32;

[[nodiscard]] inline Status validate_digest_len(unsigned len) noexcept {
    return len == kDigestLen ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "hash",
            "hbsd/src/stand/libsa/sha256.c",
            """inline constexpr unsigned kBlockSize = 64;
inline constexpr unsigned kDigestSize = 32;

[[nodiscard]] inline Status validate_block(unsigned block) noexcept {
    return block == kBlockSize ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "uuid",
            "hbsd/src/stand/libsa/uuid.c",
            """inline constexpr unsigned kUuidLen = 36;

[[nodiscard]] inline Status validate_uuid_len(unsigned len) noexcept {
    return len == kUuidLen ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "install",
            "hbsd/src/stand/install/install.c",
            """enum class Mode : unsigned char {
    Copy = 0,
    Move = 1,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(Mode::Move) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "decompress",
            "hbsd/src/stand/libsa/decompress.c",
            """inline constexpr unsigned kWindowBits = 15;

[[nodiscard]] inline Status validate_window_bits(unsigned bits) noexcept {
    return bits >= 8 && bits <= kWindowBits ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for stem, upstream, body in stand_specs:
        stand_mod(stem, upstream, body)

    # --- Arch ---------------------------------------------------------------
    arch_specs = [
        (
            "arch/amd64/pbsd.arch.amd64.nx.cppm",
            "pbsd.arch.amd64.nx",
            "amd64::nx",
            "hbsd/src/sys/x86/x86/nx.c",
            """inline constexpr unsigned kPageSize = 4096;

[[nodiscard]] inline Status validate_page(unsigned page) noexcept {
    return page >= kPageSize ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/amd64/pbsd.arch.amd64.avx.cppm",
            "pbsd.arch.amd64.avx",
            "amd64::avx",
            "hbsd/src/sys/x86/x86/avx.c",
            """enum class Level : unsigned char {
    None = 0,
    Avx = 1,
    Avx2 = 2,
};

[[nodiscard]] inline Status validate_level(unsigned level) noexcept {
    return level <= static_cast<unsigned>(Level::Avx2) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "arch/arm64/pbsd.arch.arm64.pauth.cppm",
            "pbsd.arch.arm64.pauth",
            "arm64::pauth",
            "hbsd/src/sys/arm64/arm64/pauth.c",
            """inline constexpr unsigned kKeySlots = 5;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kKeySlots ? Status::Ok : Status::Invalid;
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

[[nodiscard]] inline Status validate_guard(unsigned guard) noexcept {
    return guard <= static_cast<unsigned>(Guard::Bti) ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for rel, mod, ns, upstream, body in arch_specs:
        arch_mod(rel, mod, ns, upstream, body)

    # --- Bifrost ------------------------------------------------------------
    bif_specs = [
        (
            "gpa_pool",
            "hbsd/src/sys/amd64/vmm/vmm_mem.c",
            """inline constexpr unsigned kMaxPages = 4096;

[[nodiscard]] inline Status validate_pages(unsigned pages) noexcept {
    return pages > 0 && pages <= kMaxPages ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "msr_filter",
            "hbsd/src/sys/amd64/vmm/intel/msr_bitmap.c",
            """inline constexpr unsigned kBitmapBytes = 4096;

[[nodiscard]] inline Status validate_offset(unsigned offset) noexcept {
    return offset < kBitmapBytes ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "vmexit_reason",
            "hbsd/src/sys/amd64/vmm/intel/vmx.c",
            """enum class Reason : unsigned char {
    ExternalInterrupt = 1,
    Hlt = 12,
    IoInstruction = 30,
    MsrRead = 31,
    MsrWrite = 32,
    EptViolation = 48,
};

[[nodiscard]] inline Status validate_reason(unsigned reason) noexcept {
    return reason <= static_cast<unsigned>(Reason::EptViolation) ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "guest_mem",
            "hbsd/src/sys/amd64/vmm/vmm_mem.c",
            """inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned kPageSize = 1u << kPageShift;

[[nodiscard]] inline Status validate_gpa(unsigned long long gpa) noexcept {
    return (gpa & (kPageSize - 1)) == 0 ? Status::Ok : Status::Invalid;
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
