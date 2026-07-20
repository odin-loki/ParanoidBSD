export module pbsd.kde.kwin.wayland.xdgtopleveltag_v1;

import pbsd.core;

/// Wave 3 pass 4 — xdg toplevel tag protocol.
/// Upstream: kde/kwin/src/wayland/xdgtopleveltag_v1.cpp
export namespace pbsd::kde::kwin::wayland::xdgtopleveltag_v1 {

    inline constexpr const char kProtocolName[] = "xdg_toplevel_tag_v1";
    inline constexpr unsigned kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/xdgtopleveltag_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::xdgtopleveltag_v1
