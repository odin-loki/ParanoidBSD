export module pbsd.kde.kwin.virtual_backend.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/backends/virtual/virtual_logging.cpp
export namespace pbsd::kde::kwin::virtual_::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_VIRTUAL",
    "kwin_platform_virtual",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/backends/virtual/virtual_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept {
    return kCategory;
}

} // namespace pbsd::kde::kwin::virtual_::logging
