export module pbsd.kde.plasma.workspace.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — Plasma workspace logging category.
/// Upstream: kde/plasma-workspace/shell/shellcorona.cpp
export namespace pbsd::kde::plasma::workspace::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "PLASMA_SHELL",
    "plasma_shell",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-workspace/shell/shellcorona.cpp",
};

} // namespace pbsd::kde::plasma::workspace::logging
