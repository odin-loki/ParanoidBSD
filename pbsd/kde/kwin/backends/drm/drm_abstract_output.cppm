export module pbsd.kde.drm_abstract_output;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (drm_abstract_output.cpp).
/// Upstream: kde/kwin/src/backends/drm/drm_abstract_output.cpp
export namespace pbsd::kde::drm_abstract_output {

inline constexpr const char kConnectorType[] = "Unknown";
inline constexpr unsigned kInvalidCrtc = 0;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/backends/drm/drm_abstract_output.cpp";
}

} // namespace pbsd::kde::drm_abstract_output
