export module pbsd.kde.kwin.libinput.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/backends/libinput/libinput_logging.cpp
export namespace pbsd::kde::kwin::libinput::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_LIBINPUT",
    "kwin_libinput",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/backends/libinput/libinput_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::libinput::logging
