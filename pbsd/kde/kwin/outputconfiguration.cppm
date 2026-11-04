export module pbsd.kde.kwin.outputconfiguration;

import pbsd.core;

/// Wave 3 pass 4 — Output configuration apply flags.
/// Upstream: kde/kwin/src/core/outputconfiguration.cpp
export namespace pbsd::kde::kwin::outputconfiguration {

    inline constexpr unsigned kApplyMode = 1;
    inline constexpr unsigned kApplyTransform = 2;
    inline constexpr unsigned kApplyScale = 4;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/outputconfiguration.cpp";
}

} // namespace pbsd::kde::kwin::outputconfiguration
