export module pbsd.kde.kwin.scripting.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/scripting/scripting_logging.cpp
export namespace pbsd::kde::kwin::scripting::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_SCRIPTING",
    "kwin_scripting",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/scripting/scripting_logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::scripting::logging
