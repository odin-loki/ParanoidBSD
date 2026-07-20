export module pbsd.kde.presentation;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (presentation.cpp).
/// Upstream: kde/kwin/src/wayland/presentation.cpp
export namespace pbsd::kde::presentation {

inline constexpr const char kPresentation[] = "wp_presentation";
inline constexpr unsigned kRefreshNs = 16666666;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/presentation.cpp";
}

} // namespace pbsd::kde::presentation
