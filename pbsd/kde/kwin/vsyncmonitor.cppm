export module pbsd.kde.kwin.vsyncmonitor;

import pbsd.core;

/// Wave 3 pass 3 — VSync monitor timing constants.
/// Upstream: kde/kwin/src/utils/vsyncmonitor.cpp
export namespace pbsd::kde::kwin::vsyncmonitor {

    inline constexpr unsigned kDefaultRefreshHz{60};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/utils/vsyncmonitor.cpp";
}

} // namespace pbsd::kde::kwin::vsyncmonitor
