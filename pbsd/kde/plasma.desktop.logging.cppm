export module pbsd.kde.plasma.desktop.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — Plasma Desktop KCM logging category (from touchpad/logging.cpp).
/// Upstream: kde/plasma-desktop/kcms/touchpad/logging.cpp
export namespace pbsd::kde::plasma::desktop::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kTouchpadCategory{
    "KCM_TOUCHPAD",
    "kcm_touchpad",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-desktop/kcms/touchpad/logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kTouchpadCategory.name; }

} // namespace pbsd::kde::plasma::desktop::logging
