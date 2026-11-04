export module pbsd.kde.kwin.plugins.screenshotlayer;

import pbsd.core;

/// Wave 3 pass 4 — Screenshot overlay layer id.
/// Upstream: kde/kwin/src/plugins/screenshot/screenshotlayer.cpp
export namespace pbsd::kde::kwin::plugins::screenshotlayer {

    inline constexpr int kScreenshotLayer = 9999;
    inline constexpr const char kEffectId[] = "screenshot";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/screenshot/screenshotlayer.cpp";
}

} // namespace pbsd::kde::kwin::plugins::screenshotlayer
