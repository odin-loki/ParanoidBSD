export module pbsd.kde.plasma.cursortheme.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.cursortheme;

/// Burst 14 — Plasma cursortheme ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/kcm_cursortheme/main.cpp
export namespace pbsd::kde::plasma::cursortheme::glue {

struct CursorStyle {
    const char* theme_name{"breeze_cursors"};
    unsigned size_px{24};
};

[[nodiscard]] inline CursorStyle default_style() noexcept {
    return CursorStyle{};
}

[[nodiscard]] inline Status validate_size(unsigned px) noexcept {
    return px >= 16 && px <= 64 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return cursortheme::upstream_path();
}

} // namespace pbsd::kde::plasma::cursortheme::glue
