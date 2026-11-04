export module pbsd.kde.kwin.core.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — KWin logging category mirror.
/// Upstream: kde/kwin/src/core/renderjournal.cpp
export namespace pbsd::kde::kwin::core::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_CORE",
    "kwin_core",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/core/renderjournal.cpp",
};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/renderjournal.cpp";
}

} // namespace pbsd::kde::kwin::core::logging
