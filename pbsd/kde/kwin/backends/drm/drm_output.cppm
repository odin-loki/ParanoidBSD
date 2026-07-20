export module pbsd.kde.drm_output;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (drm_output.cpp).
/// Upstream: kde/kwin/src/backends/drm/drm_output.cpp
export namespace pbsd::kde::drm_output {

inline constexpr const char kOutputNamePrefix[] = "HDMI-A";
inline constexpr unsigned kMaxModes = 32;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/backends/drm/drm_output.cpp";
}

} // namespace pbsd::kde::drm_output
