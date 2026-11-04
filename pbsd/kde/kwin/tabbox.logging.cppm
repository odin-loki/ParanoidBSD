export module pbsd.kde.kwin.tabbox.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/tabbox/tabbox_logging.cpp
export namespace pbsd::kde::kwin::tabbox::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_TABBOX",
    "kwin_tabbox",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/tabbox/tabbox_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::tabbox::logging
