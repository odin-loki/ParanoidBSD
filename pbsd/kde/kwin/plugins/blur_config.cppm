export module pbsd.kde.kwin.plugins.blur_config;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — Blur effect KCM config keys.
/// Upstream: kde/kwin/src/plugins/blur/blur_config.cpp
export namespace pbsd::kde::kwin::plugins::blur_config {

    inline constexpr const char kStrengthKey[] = "Strength";
    inline constexpr const char kNoiseKey[] = "Noise";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/blur/blur_config.cpp";
}

} // namespace pbsd::kde::kwin::plugins::blur_config
