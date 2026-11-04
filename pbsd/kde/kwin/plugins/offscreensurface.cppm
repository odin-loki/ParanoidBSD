export module pbsd.kde.kwin.plugins.offscreensurface;

import pbsd.core;

/// Wave 3 pass 4 — QPA offscreen surface stub.
/// Upstream: kde/kwin/src/plugins/qpa/offscreensurface.cpp
export namespace pbsd::kde::kwin::plugins::offscreensurface {

    inline constexpr unsigned kDefaultWidth = 1;
    inline constexpr unsigned kDefaultHeight = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/qpa/offscreensurface.cpp";
}

} // namespace pbsd::kde::kwin::plugins::offscreensurface
