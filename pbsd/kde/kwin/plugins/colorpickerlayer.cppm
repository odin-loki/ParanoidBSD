export module pbsd.kde.kwin.plugins.colorpickerlayer;

import pbsd.core;

/// Wave 3 pass 4 — Color picker overlay layer.
/// Upstream: kde/kwin/src/plugins/colorpicker/colorpickerlayer.cpp
export namespace pbsd::kde::kwin::plugins::colorpickerlayer {

    inline constexpr int kOverlayLayer = 10000;
    inline constexpr const char kEffectId[] = "colorpicker";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/colorpicker/colorpickerlayer.cpp";
}

} // namespace pbsd::kde::kwin::plugins::colorpickerlayer
