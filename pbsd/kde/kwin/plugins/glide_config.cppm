export module pbsd.kde.kwin.plugins.glide_config;

import pbsd.core;

/// Wave 3 pass 4 — Glide effect KCM keys.
/// Upstream: kde/kwin/src/plugins/glide/glide_config.cpp
export namespace pbsd::kde::kwin::plugins::glide_config {

    inline constexpr const char kDurationKey[] = "Duration";
    inline constexpr const char kCurveKey[] = "Curve";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/glide/glide_config.cpp";
}

} // namespace pbsd::kde::kwin::plugins::glide_config
