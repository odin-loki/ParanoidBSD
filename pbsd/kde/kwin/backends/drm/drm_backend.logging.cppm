export module pbsd.kde.backends.drm.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — logging category mirror.
/// Upstream: kde/kwin/src/backends/drm/drm_logging.cpp
export namespace pbsd::kde::backends::drm::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_DRM",
    "kwin_drm",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/backends/drm/drm_logging.cpp",
};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/backends/drm/drm_logging.cpp";
}

} // namespace pbsd::kde::backends::drm::logging
