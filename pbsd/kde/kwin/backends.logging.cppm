export module pbsd.kde.kwin.backends.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — KWin logging category mirror.
/// Upstream: kde/kwin/src/backends/drm/drm_blob.cpp
export namespace pbsd::kde::kwin::backends::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_BACKENDS",
    "kwin_backends",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/backends/drm/drm_blob.cpp",
};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/backends/drm/drm_blob.cpp";
}

} // namespace pbsd::kde::kwin::backends::logging
