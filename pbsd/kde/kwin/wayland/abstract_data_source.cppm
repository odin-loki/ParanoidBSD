export module pbsd.kde.kwin.wayland.abstract_data_source;

import pbsd.core;

/// Wave 3 pass 4 — Data source mime type cap.
/// Upstream: kde/kwin/src/wayland/abstract_data_source.cpp
export namespace pbsd::kde::kwin::wayland::abstract_data_source {

    inline constexpr unsigned kMaxMimeTypes = 16;
    inline constexpr unsigned kMaxMimeLen = 128;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/abstract_data_source.cpp";
}

} // namespace pbsd::kde::kwin::wayland::abstract_data_source
