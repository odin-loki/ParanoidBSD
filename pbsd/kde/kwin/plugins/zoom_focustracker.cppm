export module pbsd.kde.zoom_focustracker;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (focustracker.cpp).
/// Upstream: kde/kwin/src/plugins/zoom/focustracker.cpp
export namespace pbsd::kde::zoom_focustracker {

inline constexpr float kDefaultZoom = 1.0f;
inline constexpr float kMaxZoom = 4.0f;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/zoom/focustracker.cpp";
}

} // namespace pbsd::kde::zoom_focustracker
