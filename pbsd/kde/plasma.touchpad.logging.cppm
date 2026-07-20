export module pbsd.kde.plasma.touchpad.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — touchpad KCM logging category.
/// Upstream: kde/plasma-desktop/kcms/touchpad/logging.cpp
export namespace pbsd::kde::plasma::touchpad::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KCM_TOUCHPAD",
    "kcm_touchpad",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-desktop/kcms/touchpad/logging.cpp",
};

} // namespace pbsd::kde::plasma::touchpad::logging
