export module pbsd.kde.kxmlgui.kmainwindow;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kmainwindowtest.cpp).
/// Upstream: kde/frameworks/kxmlgui/tests/kmainwindowtest.cpp
export namespace pbsd::kde::frameworks::kxmlgui::kmainwindow {

inline constexpr const char kMainWindowGroup[] = "MainWindow";
inline constexpr const char kStateKey[] = "State";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kxmlgui/tests/kmainwindowtest.cpp";
}

} // namespace pbsd::kde::frameworks::kxmlgui::kmainwindow
