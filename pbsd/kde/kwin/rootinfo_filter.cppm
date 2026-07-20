export module pbsd.kde.kwin.rootinfo_filter;

import pbsd.core;

/// Wave 3 pass 4 — X11 root info filter property names.
/// Upstream: kde/kwin/src/rootinfo_filter.cpp
export namespace pbsd::kde::kwin::rootinfo_filter {

    inline constexpr const char kNetActiveWindow[] = "_NET_ACTIVE_WINDOW";
    inline constexpr const char kNetSupported[] = "_NET_SUPPORTED";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/rootinfo_filter.cpp";
}

} // namespace pbsd::kde::kwin::rootinfo_filter
