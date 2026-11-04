export module pbsd.kde.plasma.tastenbrett.application;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (application.cpp).
/// Upstream: kde/plasma-desktop/kcms/keyboard/tastenbrett/application.cpp
export namespace pbsd::kde::plasma::tastenbrett::application {

inline constexpr const char kAppGroup[] = "KeyboardLayout";
inline constexpr unsigned kMaxLayouts = 8;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/tastenbrett/application.cpp";
}

} // namespace pbsd::kde::plasma::tastenbrett::application
