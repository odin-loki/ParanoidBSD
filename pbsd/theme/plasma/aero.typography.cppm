export module pbsd.theme.plasma.aero.typography;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 11 — Aero typography scale (Segoe UI / Noto Sans fallback).
export namespace pbsd::theme::plasma::aero::typography {

struct TypeScale {
    int caption{11};
    int body{13};
    int title{15};
    int heading{18};
    int display{24};
};

inline constexpr TypeScale kDefault{};
inline constexpr const char kUiFont[] = "Segoe UI";
inline constexpr const char kMonospaceFont[] = "Consolas";
inline constexpr const char kFallbackFont[] = "Noto Sans";
inline constexpr float kLineHeight{1.35f};

[[nodiscard]] inline int panel_font_size() noexcept {
    return kDefault.body;
}

[[nodiscard]] inline int start_menu_heading_size() noexcept {
    return kDefault.heading;
}

[[nodiscard]] inline int title_bar_font_size() noexcept {
    return kDefault.title;
}

[[nodiscard]] inline Status validate_size(int px) noexcept {
    if (px < 8 || px > 48) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::typography
