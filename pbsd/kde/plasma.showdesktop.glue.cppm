export module pbsd.kde.plasma.showdesktop.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.showdesktop;

/// Burst 13 — Show-desktop applet ↔ Aero taskbar glue.
export namespace pbsd::kde::plasma::showdesktop::glue {

struct ShowDesktopStyle {
    const char* applet_id{showdesktop::kAppletId};
    const char* peek_key{showdesktop::kPeekKey};
    const char* show_desktop_svg{aero::kShowDesktopSvg};
    float hover_opacity{0.85f};
};

[[nodiscard]] inline ShowDesktopStyle default_style() noexcept {
    return ShowDesktopStyle{};
}

[[nodiscard]] inline Status validate_peek_key(const char* key) noexcept {
    return key != nullptr && key[0] != '\0' ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return showdesktop::upstream_path();
}

} // namespace pbsd::kde::plasma::showdesktop::glue
