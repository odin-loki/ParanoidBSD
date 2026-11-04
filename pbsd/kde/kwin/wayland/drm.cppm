export module pbsd.kde.kwin.wayland.drm;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — wp_linux_drm_syncobj_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/drm.cpp
export namespace pbsd::kde::kwin::wayland::drm {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_DRM",
    "kwin_drm",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/drm.cpp",
};

inline constexpr const char kSyncobjManagerInterface[] = "wp_linux_drm_syncobj_manager_v1";
inline constexpr const char kSyncobjTimelineInterface[] = "wp_linux_drm_syncobj_timeline_v1";
inline constexpr int kVersion = 1;

enum class TimelineRequest : unsigned char {
    Destroy = 0,
    ExportSyncFile = 1,
    ImportSyncFile = 2,
};

[[nodiscard]] inline bool is_timeline_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(TimelineRequest::ImportSyncFile);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/drm.cpp";
}

} // namespace pbsd::kde::kwin::wayland::drm
