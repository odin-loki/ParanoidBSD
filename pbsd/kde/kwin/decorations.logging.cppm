export module pbsd.kde.kwin.decorations.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/decorations/decorations_logging.cpp
export namespace pbsd::kde::kwin::decorations::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_DECORATIONS",
    "kwin_decorations",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/decorations/decorations_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::decorations::logging
