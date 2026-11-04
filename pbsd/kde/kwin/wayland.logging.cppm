export module pbsd.kde.kwin.wayland.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/backends/wayland/wayland_logging.cpp
export namespace pbsd::kde::kwin::wayland::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_WAYLAND_BACKEND",
    "kwin_wayland_backend",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/backends/wayland/wayland_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::wayland::logging
