export module pbsd.kde.magiclamp_config;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (magiclamp_config.cpp).
/// Upstream: kde/kwin/src/plugins/magiclamp/magiclamp_config.cpp
export namespace pbsd::kde::magiclamp_config {

inline constexpr const char kDurationKey[] = "Duration";
inline constexpr unsigned kDefaultDurationMs = 250;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/magiclamp/magiclamp_config.cpp";
}

} // namespace pbsd::kde::magiclamp_config
