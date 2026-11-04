export module pbsd.kde.slide_config;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (slide_config.cpp).
/// Upstream: kde/kwin/src/plugins/slide/slide_config.cpp
export namespace pbsd::kde::slide_config {

inline constexpr const char kLocationKey[] = "Location";
inline constexpr const char kDefaultLocation[] = "Left";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/slide/slide_config.cpp";
}

} // namespace pbsd::kde::slide_config
