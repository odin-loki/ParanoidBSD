export module pbsd.kde.kwin.x11.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/backends/x11/x11_windowed_logging.cpp
export namespace pbsd::kde::kwin::x11::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_X11WINDOWED",
    "kwin_wayland_x11windowed",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/backends/x11/x11_windowed_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::x11::logging
