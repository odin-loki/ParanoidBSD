export module pbsd.kde.kwin.wayland.pointerwarp_v1;

import pbsd.core;

/// Wave 3 pass 4 — Pointer warp protocol stub.
/// Upstream: kde/kwin/src/wayland/pointerwarp_v1.cpp
export namespace pbsd::kde::kwin::wayland::pointerwarp_v1 {

    inline constexpr const char kProtocolName[] = "wp_pointer_warp_v1";
    inline constexpr unsigned kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/pointerwarp_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::pointerwarp_v1
