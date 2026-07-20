export module pbsd.kde.plasma.mouse.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — mouse KCM logging stub.
/// Upstream: kde/plasma-desktop/kcms/mouse/inputdevice.cpp
export namespace pbsd::kde::plasma::mouse::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KCM_MOUSE",
    "kcm_mouse",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-desktop/kcms/mouse/inputdevice.cpp",
};

} // namespace pbsd::kde::plasma::mouse::logging
