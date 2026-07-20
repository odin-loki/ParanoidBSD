export module pbsd.kde.plasma.plasmaquick_utils;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (utils.cpp).
/// Upstream: kde/frameworks/plasma-framework/src/plasmaquick/utils.cpp
export namespace pbsd::kde::frameworks::plasma::plasmaquick_utils {

inline constexpr const char kAppletPrefix[] = "org.kde.plasma";
inline constexpr unsigned kMaxApplets = 64;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasmaquick/utils.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::plasmaquick_utils
