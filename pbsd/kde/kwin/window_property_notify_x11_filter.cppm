export module pbsd.kde.kwin.window_property_notify_x11_filter;

import pbsd.core;

/// Wave 3 pass 4 — X11 property notify filter.
/// Upstream: kde/kwin/src/window_property_notify_x11_filter.cpp
export namespace pbsd::kde::kwin::window_property_notify_x11_filter {

    inline constexpr unsigned kMaxProperties = 32;
    inline constexpr const char kWmState[] = "WM_STATE";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/window_property_notify_x11_filter.cpp";
}

} // namespace pbsd::kde::kwin::window_property_notify_x11_filter
