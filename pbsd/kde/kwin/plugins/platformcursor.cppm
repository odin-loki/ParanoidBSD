export module pbsd.kde.kwin.plugins.platformcursor;

import pbsd.core;

/// Wave 3 pass 4 — Platform cursor shape ids.
/// Upstream: kde/kwin/src/plugins/qpa/platformcursor.cpp
export namespace pbsd::kde::kwin::plugins::platformcursor {

    inline constexpr unsigned kArrowShape = 0;
    inline constexpr unsigned kIbeamShape = 1;
    inline constexpr unsigned kWaitShape = 3;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/qpa/platformcursor.cpp";
}

} // namespace pbsd::kde::kwin::plugins::platformcursor
